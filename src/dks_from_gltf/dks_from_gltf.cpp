// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::g2d_cgltf_file_read(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, char const *path, cgltf_size *out_size, void **out_data) noexcept -> cgltf_result {
	(void)mem_opts;

	cgltf_result result = cgltf_result_file_not_found;
	Arena *arena = static_cast<Arena *>(file_opts->user_data);
	String8 const file_path = str8_cstring(reinterpret_cast<u8 const *>(path));
	File const file = file_open(file_path, FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_SHARE_READ);
	if (is_valid(file)) {
		FileAttributes const attr = attributes_from_file(file);
		u8 *const data = arena_push_array<u8>(arena, attr.size);
		u64 read_size = file_read(file, 0, attr.size, data);
		if (read_size < attr.size) {
			arena_pop(arena, attr.size - read_size);
		}
		*out_data = reinterpret_cast<void *>(data);
		*out_size = static_cast<cgltf_size>(read_size);
		result = cgltf_result_success;
		file_close(file);
	}
	return result;
}

auto dk::g2d_cgltf_file_release(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, void *data, cgltf_size size) noexcept -> void {
	(void)mem_opts;
	(void)file_opts;
	(void)data;
	(void)size;
}

namespace dk {
	template <typename SrcType, typename DstType = SrcType>
	static auto g2d_load_attribute(cgltf_accessor const *accessor, u32 num_components, DstType *dst) noexcept -> void {
		u8 const *const base = static_cast<u8 const *>(accessor->buffer_view->buffer->data) + accessor->buffer_view->offset + accessor->offset;
		u64 const stride = accessor->stride;
		for (u64 a_idx = 0; a_idx < accessor->count; ++a_idx) {
			SrcType const *const src_component = reinterpret_cast<SrcType const *>(base + a_idx * stride);
			for (u32 c_idx = 0; c_idx < num_components; ++c_idx) {
				dst[a_idx * num_components + c_idx] = static_cast<DstType>(src_component);
			}
		}
	}
}

// https://github.com/zeux/meshoptimizer/blob/97bbdce4716f6257c9527b051515136882f33e79/gltf/node.cpp#L161
auto dk::g2d_decompose_transform(f32 const *transform, f32 out_translation[3], f32 out_rotation[4], f32 out_scale[3]) noexcept -> void {
	f32 m[4][4] = {};
	std::memcpy(m, transform, 16 * sizeof(f32));

	// extract translation from last row
	out_translation[0] = m[3][0];
	out_translation[1] = m[3][1];
	out_translation[2] = m[3][2];

	// compute determinant to determine handedness
	f32 const det =
		m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	f32 const sign = det < 0.0f ? -1.0f : 1.0f;

	// recover scale from axis lengths
	out_scale[0] = sqrt(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]) * sign;
	out_scale[1] = sqrt(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]) * sign;
	out_scale[2] = sqrt(m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]) * sign;

	// normalize axes to get a pure rotation matrix
	f32 const rsx = out_scale[0] == 0.0f ? 0.0f : 1.0f / out_scale[0];
	f32 const rsy = out_scale[1] == 0.0f ? 0.0f : 1.0f / out_scale[1];
	f32 const rsz = out_scale[2] == 0.0f ? 0.0f : 1.0f / out_scale[2];

	f32 const r00 = m[0][0] * rsx;
	f32 const r10 = m[1][0] * rsy;
	f32 const r20 = m[2][0] * rsz;
	f32 const r01 = m[0][1] * rsx;
	f32 const r11 = m[1][1] * rsy;
	f32 const r21 = m[2][1] * rsz;
	f32 const r02 = m[0][2] * rsx;
	f32 const r12 = m[1][2] * rsy;
	f32 const r22 = m[2][2] * rsz;

	// "branchless" version of Mike Day's matrix to quaternion conversion
	s32 const qc = r22 < 0 ? (r00 > r11 ? 0 : 1) : (r00 < -r11 ? 2 : 3);
	f32 const qs1 = qc & 2 ? -1.0f : 1.0f;
	f32 const qs2 = qc & 1 ? -1.0f : 1.0f;
	f32 const qs3 = (qc - 1) & 2 ? -1.0f : 1.0f;
	f32 const qt = 1.0f - qs3 * r00 - qs2 * r11 - qs1 * r22;
	f32 const qs = 0.5f / sqrt(qt);

	out_rotation[qc ^ 0] = qs * qt;
	out_rotation[qc ^ 1] = qs * (r01 + qs1 * r10);
	out_rotation[qc ^ 2] = qs * (r20 + qs2 * r02);
	out_rotation[qc ^ 3] = qs * (r12 + qs3 * r21);
}

auto dk::g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams {
	ZoneScoped;
	TempArena const scratch = scratch_begin(&arena, 1);
	dk_defer(scratch_end(scratch));

	//~ Dedrick: @g2d_stage Parse glTF; load buffers; scan for draco.
	cgltf_data *gltf = nullptr;
	{
		ZoneScopedN("parse glTF; load buffers; scan for draco");
		if (lane_idx() == 0) {
			cgltf_options options = {};
			options.file.read = g2d_cgltf_file_read;
			options.file.release = g2d_cgltf_file_release;
			options.file.user_data = arena;
			cgltf_result const parse_status = cgltf_parse(&options, params->file_data.data, static_cast<cgltf_size>(params->file_data.size), &gltf);
			if (parse_status != cgltf_result_success) {
				DK_LOG_ERRORF("[cgltf]: ERROR: failed to parse %.*s.\n", DK_STR8_VARG(params->file_path));
				gltf = nullptr;
			}

			//~ Dedrick: Load buffers.
			if (gltf != nullptr) {
				cgltf_result const load_status = cgltf_load_buffers(&options, gltf, reinterpret_cast<char const *>(params->file_path.data));
				if (load_status != cgltf_result_success) {
					DK_LOG_ERRORF("[cgltf]: ERROR: failed to load mesh data.\n");
					cgltf_free(gltf);
					gltf = nullptr;
				}
			}

			//~ Dedrick: Scan for draco compression.
			if (gltf != nullptr) {
				b8 draco_compression = false;
				for (cgltf_size idx = 0; idx < gltf->meshes_count; ++idx) {
					cgltf_mesh const *mesh = &gltf->meshes[idx];
					for (cgltf_size p_idx = 0; p_idx < mesh->primitives_count; ++p_idx) {
						if (mesh->primitives[p_idx].has_draco_mesh_compression) {
							draco_compression = true;
							break;
						}
					}
					if (draco_compression) {
						break;
					}
				}
				if (draco_compression) {
					DK_LOG_ERRORF("[dks_from_gltf]: draco compression not supported.\n");
					cgltf_free(gltf);
					gltf = nullptr;
				}
			}

			//~ Dedrick: Print basic information.
			if (gltf != nullptr) {
				if (gltf->file_type == cgltf_file_type_glb) {
					DK_LOG_INFOF("[dks_from_gltf]: %.*s model (glb) loaded.\n", DK_STR8_VARG(params->file_path));
				}
				else if (gltf->file_type == cgltf_file_type_gltf) {
					DK_LOG_INFOF("[dks_from_gltf]: %.*s model (glTF) loaded.\n", DK_STR8_VARG(params->file_path));
				}
				else {
					DK_LOG_INFOF("[dks_from_gltf]: %.*s model format not recognized.\n", DK_STR8_VARG(params->file_path));
				}
				DK_LOG_INFOF("  > meshes count: %i\n", gltf->meshes_count);
				DK_LOG_INFOF("  > materials count: %i\n", gltf->materials_count);
				DK_LOG_INFOF("  > buffers count: %i\n", gltf->buffers_count);
				DK_LOG_INFOF("  > images count: %i\n", gltf->images_count);
				DK_LOG_INFOF("  > textures count: %i\n", gltf->textures_count);
			}
		}
		lane_sync_broadcast(&gltf, 0);
	}

	//~ Dedrick: @g2d_stage Compute node primitive layout.
	struct PrimitiveLayout {
		u64 *node_primitive_counts;
		u64 *node_primitive_offsets;
		u64 total_primitive_count;
	};
	PrimitiveLayout *primitive_layout = nullptr;
	if (gltf != nullptr) {
		ZoneScopedN("compute node primitive layout");
		if (lane_idx() == 0) {
			primitive_layout = arena_push<PrimitiveLayout>(scratch.arena);
			primitive_layout->node_primitive_counts = arena_push_array<u64>(scratch.arena, gltf->nodes_count);
			primitive_layout->node_primitive_offsets = arena_push_array<u64>(scratch.arena, gltf->nodes_count + 1);
		}
		lane_sync_broadcast(&primitive_layout, 0);

		{
			ZoneScopedN("wide count");
			LaneRange const range = lane_range(gltf->nodes_count);
			for (u64 n_idx = range.begin; n_idx < range.end; ++n_idx) {
				u64 primitives_count = 0;
				cgltf_node const *node = &gltf->nodes[n_idx];
				cgltf_mesh const *mesh = node->mesh;
				if (mesh != nullptr) {
					for (cgltf_size p_idx = 0; p_idx < mesh->primitives_count; ++p_idx) {
						if (mesh->primitives[p_idx].type == cgltf_primitive_type_triangles) {
							primitives_count += 1;
						}
					}
				}
				primitive_layout->node_primitive_counts[n_idx] = primitives_count;
			}
			lane_sync();
		}

		if (lane_idx() == 0) {
			u64 layout_offset = 0;
			for (u64 idx = 0; idx < gltf->nodes_count; ++idx) {
				primitive_layout->node_primitive_offsets[idx] = layout_offset;
				layout_offset += primitive_layout->node_primitive_counts[idx];
			}
			primitive_layout->node_primitive_offsets[gltf->nodes_count] = layout_offset;
			primitive_layout->total_primitive_count = layout_offset;
		}
	}

	// TODO(Dedrick): \/\/\/
	//
	//~ Dedrick: @g2d_stage glTF primitive -> dksm mesh.
	DKSM_GPU_Mesh **mesh_from_primitive_table = nullptr;
	DKSM_GPU_MeshChunkList *lane_gpu_meshes = nullptr;
	if (gltf != nullptr) {
		ZoneScopedN("glTF primitive -> dksm mesh");
		if (lane_idx() == 0) {
			mesh_from_primitive_table = arena_push_array<DKSM_GPU_Mesh *>(scratch.arena, primitive_layout->total_primitive_count);
			lane_gpu_meshes = arena_push_array<DKSM_GPU_MeshChunkList>(scratch.arena, lane_count());
		}
		lane_sync_broadcast(&mesh_from_primitive_table, 0);
		lane_sync_broadcast(&lane_gpu_meshes, 0);

		//~ Dedrick: Wide convert.
		LaneRange const range = lane_range(gltf->nodes_count);
		for (u64 node_idx = range.begin; node_idx < range.end; ++node_idx) {
			cgltf_node const *node = &gltf->nodes[node_idx];
			cgltf_mesh const *mesh = node->mesh;
			if (mesh != nullptr) {
				u64 primitive_idx = primitive_layout->node_primitive_offsets[node_idx];
				for (u64 p_idx = 0; p_idx < mesh->primitives_count; ++p_idx) {
					cgltf_primitive const *primitive = &mesh->primitives[p_idx];
					if (primitive->type != cgltf_primitive_type_triangles) {
						continue;
					}

					//~ Dedrick:
					DKSM_GPU_Mesh *dst = dksm_gpu_mesh_chunk_list_push(arena, &lane_gpu_meshes[lane_idx()], 64);
					// FIXME(Dedrick): Our attributes are interleaved, we can't guarantee the order of attributes
					// so how do we allocate enough space for all (possible) attributes without doing too much extra work?

					//~ Dedrick:
					for (u64 a_idx = 0; a_idx < primitive->attributes_count; ++a_idx) {
						if (primitive->attributes[a_idx].type == cgltf_attribute_type_position) {
							cgltf_accessor const *attribute = primitive->attributes[a_idx].data;
						}
						else if (primitive->attributes[attr_idx].type == cgltf_attribute_type_normal) {
							// TODO(Dedrick)
							// cgltf_accessor const *attribute = primitive->attributes[a_idx].data;
						}
						else if (primitive->attributes[attr_idx].type == cgltf_attribute_type_tangent) {
							// TODO(Dedrick)
							// cgltf_accessor const *attribute = primitive->attributes[a_idx].data;
						}
						else if (primitive->attributes[attr_idx].type == cgltf_attribute_type_texcoord) {
							// TODO(Dedrick)
							// cgltf_accessor const *attribute = primitive->attributes[a_idx].data;
						}
					}

					//~ Dedrick:
					if (primitive->indices != nullptr && primitive->indices->buffer_view != nullptr) {
						cgltf_accessor const *attribute = primitive->indices;
						if (attribute->component_type == cgltf_component_type_r_32u) {
							mesh->indices = arena_push_array<u32>(arena, attribute->count);
							g2d_load_attribute<u32>(accessor, 1, mesh->indices);
						}
						else if (attribute->component_type == cgltf_component_type_r_16u) {
							mesh->indices = arena_push_array<u32>(arena, attribute->count);
							g2d_load_attribute<u16, u32>(accessor, 1, mesh->indices);
						}
						else if (attribute->component_type == cgltf_component_type_r_8u) {
							mesh->indices = arena_push_array<u32>(arena, attribute->count);
							g2d_load_attribute<u16, u32>(accessor, 1, mesh->indices);
						}
						else {
							// TODO(Dedrick): Write with the same format as other logs.
							DK_LOG_INFOF("indices data format not supported, use u32");
						}
					}

					//~ Dedrick: Save to table; bump primitive index.
					mesh_from_primitive_table[primitive_idx] = mesh;
					primitive_idx += 1;
				}
			}
		}
	}

	//~ Dedrick: @g2d_stage Build nodes.
	DKSM_Node **node_from_node_idx_table = nullptr;
	DKSM_NodeChunkList *lane_nodes = nullptr;
	if (gltf != nullptr) {
		ZoneScopedN("build nodes");
		if (lane_idx() == 0) {
			node_from_node_idx_table = arena_push_array<DKSM_Node *>(scratch.arena, gltf->nodes_count);
			lane_count = arena_push_array<DKSM_NodeChunkList>(scratch.arena, lane_count());
		}
		lane_sync_broadcast(&node_from_node_idx_table, 0);
		lane_sync_broadcast(&lane_nodes, 0);

		//~ Dedrick: Wide fill name and transform.
		LaneRange const range = lane_range(gltf->nodes_count);
		for (u64 n_idx = range.begin; n_idx < range.end; ++n_idx) {
			cgltf_node const *src = &gltf->nodes[n_idx];
			DKSM_Node *dst = dksm_node_chunk_list_push(arena, &lane_nodes[lane_idx()], 256);
			dst->name = str8_cstring(reinterpret_cast<u8 const *>(src->name));
			dst->local_translation = { 0.0f, 0.0f, 0.0f };
			dst->local_rotation = quat_identity();
			dst->local_scale = { 1.0f, 1.0f, 1.0f };
			node_from_node_idx_table[n_idx] = dst;

			//~ Dedrick:
			if (src->has_matrix) {
				vec3 translation; quat rotation; vec3 scale;
				g2d_decompose_transform(src->matrix, &translation[0], &rotation[0], &scale[0]);
				dst->local_translation = translation;
				dst->local_rotation = rotation;
				dst->local_scale = scale;
			}
			else {
				if (src->has_translation) { std::memcpy(&dst->local_translation[0], src->translation, sizeof(src->translation)); }
				if (src->has_rotation) { std::memcpy(&dst->local_rotation[0], src->rotation, sizeof(src->rotation)); }
				if (src->has_scale) { std::memcpy(&dst->local_scale[0], src->scale, sizeof(src->scale)); }
			}
		}
	}

	//~ Dedrick: @g2d_stage Link hierarchy.
	if (gltf != nullptr) {
		ZoneScopedN("link hierarchy");
		LaneRange const range = lane_range(gltf->nodes_count);
		for (u64 n_idx = range.begin; n_idx < range.end; ++n_idx) {
			cgltf_node const *src = &gltf->nodes[n_idx];
			DKSM_Node *dst = node_from_node_idx_table[n_idx];
			if (src->parent != nullptr) {
				dst->parent = node_from_node_idx_table[src->parent - gltf->nodes];
			}
			if (src->children_count > 0) {
				dst->first_child = node_from_node_idx_table[src->children[0] - gltf->nodes];
				for (cgltf_size c_idx = 0; c_idx < src->children_count; ++c_idx) {
					DKSM_Node *dst_child = node_from_node_idx_table[src->children[c_idx] - gltf->nodes];
					if (c_idx > 0) {
						dst_child->next_sibling = node_from_node_idx_table[src->children[c_idx - 1] - gltf->nodes];
					}
					if (c_idx + 1 < src->children_count) {
						dst_child->next_sibling = node_from_node_idx_table[src->children[c_idx + 1] - gltf->nodes];
					}
				}
			}
		}
		lane_sync();
	}

	//~ Dedrick: @g2d_stage Build gpu instances.
	DKSM_GPU_InstanceChunkList *lane_gpu_instances = nullptr;
	if (gltf != nullptr) {
		ZoneScopedN("build gpu instances");
		if (lane_idx() == 0) {
			lane_gpu_instances = arena_push_array<DKSM_GPU_InstanceChunkList>(scratch.arena, lane_count());
		}
		lane_sync_broadcast(&lane_gpu_instances, 0);

		//~ Dedrick: Wide fill.
		LaneRange const range = lane_range(gltf->nodes_count);
		for (u64 n_idx = range.begin; n_idx < range.end; ++n_idx) {
			DKSM_Node *dst = node_from_node_idx_table[n_idx];
			u64 const p_begin = primitive_layout->node_primitive_offsets[n_idx];
			u64 const p_end = primitive_layout->node_primitive_offsets[n_idx + 1];
			for (u64 p_idx = p_begin; p_idx < p_end; ++p_idx) {
				DKSM_GPU_Instance *gpu_instance = dksm_gpu_instance_chunk_list_push(arena, &lane_gpu_instances[lane_idx()], 256);
				gpu_instance->mesh = mesh_from_primitive_table[p_idx];
				forward_list_queue_push(&dst->first_gpu_instance, &dst->last_gpu_instance, gpu_instance);
				dst->gpu_instance_count += 1;
			}
		}
	}

	//~ Dedrick: @g2d_stage Join all lane blocks.
	DKSM_InstanceChunkList all_instances = {};
	DKSM_GPU_InstanceChunkList all_gpu_instances = {};
	DKSM_GPU_MeshChunkList all_gpu_meshes = {};
	{
		DKSM_InstanceChunkList *all_instances_ptr = nullptr;
		DKSM_GPU_InstanceChunkList *all_gpu_instances_ptr = nullptr;
		DKSM_GPU_MeshChunkList *all_gpu_meshes_ptr = nullptr;
		if (lane_idx() == 0) {
			all_instances_ptr = arena_push<DKSM_InstanceChunkList>(scratch.arena);
			all_gpu_instances_ptr = arena_push<DKSM_GPU_InstanceChunkList>(scratch.arena);
			all_gpu_meshes_ptr = arena_push<DKSM_GPU_MeshChunkList>(scratch.arena);
		}
		lane_sync_broadcast(&all_instances_ptr, 0);
		lane_sync_broadcast(&all_gpu_instances_ptr, 0);
		lane_sync_broadcast(&all_gpu_meshes_ptr, 0);
		if (lane_idx() == lane_from_task_idx(0)) {
			ZoneScopedN("join instances");
			for (u64 l = 0; l < lane_count(); ++l) {
				dksm_instance_chunk_list_concat_in_place(all_instances_ptr, &lane_instances[l]);
			}
		}
		if (lane_idx() == lane_from_task_idx(1)) {
			ZoneScopedN("join gpu instances");
			for (u64 l = 0; l < lane_count(); ++l) {
				dksm_gpu_instance_chunk_list_concat_in_place(all_gpu_instances_ptr, &lane_gpu_instances[l]);
			}
		}
		if (lane_idx() == lane_from_task_idx(2)) {
			ZoneScopedN("join gpu meshes");
			for (u64 l = 0; l < lane_count(); ++l) {
				dksm_gpu_mesh_chunk_list_concat_in_place(all_gpu_meshes_ptr, &lane_gpu_meshes[l]);
			}
		}
		lane_sync();
		all_instances = *all_instances_ptr;
		all_gpu_instances = *all_gpu_instances_ptr;
		all_gpu_meshes = *all_gpu_meshes_ptr;
		lane_sync();
	}

	//~ Dedrick: @g2d_stage Bundle all outputs.
	DKSM_BakeParams result = {};
	{
		//~ Dedrick: Produce top level info.
		DKSM_TopLeveInfo top_level_info = {};
		{
			top_level_info.model_name = path_skip_last_slash(params->file_path);
		}

		//~ Dedrick: Fill.
		result.top_level_info = top_level_info;
		result.instances      = all_instances;
		result.gpu_instances  = all_gpu_instances;
		result.gpu_meshes     = all_gpu_meshes;
	}

	if (lane_idx() == 0) {
		cgltf_free(gltf);
	}

	scratch_end(scratch);
	return result;
}
