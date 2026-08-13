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

auto dk::g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams {
	ZoneScoped;
	TempArena const scratch = scratch_begin(&arena, 1);

	cgltf_data *gltf = nullptr;
	{
		ZoneScopedN("load and parse gltf file(s)");
		if (lane_idx() == 0) {
			//~ Dedrick: Parse glTF file.
			cgltf_options options = {};
			options.file.read = g2d_cgltf_file_read;
			options.file.release = g2d_cgltf_file_release;
			options.file.user_data = arena;
			cgltf_result const parse_status = cgltf_parse(&options, params->file_data.data, static_cast<cgltf_size>(params->file_data.size), &gltf);
			if (parse_status != cgltf_result_success) {
				DK_LOG_ERRORF("[cgltf]: ERROR: failed to parse %.*s.\n", DK_STR8_VARG(params->file_path));
				gltf = nullptr;
			}

			//~ Dedrick: Load glTF data buffers.
			if (gltf != nullptr) {
				cgltf_result const load_status = cgltf_load_buffers(&options, gltf, reinterpret_cast<char const *>(params->file_path.data));
				if (load_status != cgltf_result_success) {
					DK_LOG_ERRORF("[cgltf]: ERROR: failed to load mesh data.\n");
					cgltf_free(gltf);
					gltf = nullptr;
				}
			}
		}
		lane_sync_broadcast(&gltf, 0);

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

	//~ Dedrick: @g2d_stage Build primitive map.
	G2D_PrimitiveMap *primitive_map = nullptr;
	if (gltf != nullptr) {
		ZoneScopedN("build primitive map");
		if (lane_idx() == 0) {
			primitive_map = arena_push<G2D_PrimitiveMap>(scratch.arena);
			primitive_map->mesh_base_idxs = arena_push_array<>(scratch.arena, gltf->meshes_count);
			u64 total_primitive_count = 0;
			for (cgltf_size idx = 0; idx < gltf->meshes_count; ++idx) {
				prim_map->mesh_base_idxs[i] = total_prims;
				total_primitive_count += gltf->meshes[i].primitives_count;
			}
			primitive_map->total_primitive_count = total_primitive_count;
			primitive_map->primitives = arena_push_array<DKSM_GPU_Mesh *>(scratch.arena, primitive_map->total_primitive_count);
		}
	}

	//~ Dedrick: @g2d_stage Covert primitives.
	DKSM_GPU_MeshChunkList *lane_gpu_meshes = nullptr;
	{
		ZoneScopedN("convert primitives");
		u64 *mesh_take_counter = nullptr;
		if (lane_idx() == 0) {
			lane_gpu_meshes = arena_push_array<DKSM_GPU_MeshChunkList>(scratch.arena, lane_count());
			mesh_take_counter = arena_push<u64>(scratch.arena);
		}
		lane_sync_broadcast(&lane_gpu_meshes, 0);
		lane_sync_broadcast(&mesh_take_counter, 0);

		//~ Dedrick: Wide fill.
		{
			ZoneScopedN("wide fill");
			while (true) {
				//~ Dedrick: Take next glTF mesh.
				u64 const mesh_idx = atomic_u64_inc_fetch(mesh_take_counter) - 1;
				if (mesh_idx >= gltf->meshes_count) {
					break;
				}

				//~ Dedrick:
				TempArena const scratch2 = scratch_begin(&scratch.arena, 1);
				u64 const primitive_base_idx = primitive_map->mesh_base_idxs[mesh_idx];
				cgltf_mesh const *const src_mesh = &gltf->meshes[mesh_idx];
				for (cgltf_size p_idx = 0; p_idx < src_mesh->primitives_count; ++p_idx) {
					cgltf_primitive const *src_prim = &src_mesh->primitives[p_idx];
					if (src_mesh->primitives[p_idx].type != cgltf_primitive_type_triangles) {
						continue;
					}

					//~ Dedrick:
					for (cgltf_size a_idx = 0; a_idx < src_prim->attributes_count; ++a_idx) {
						if (src_prim->attributes[attr_idx].type == cgltf_attribute_type_position) {
							cgltf_accessor const *attribute = src_prim->attributes[a_idx].data;

						}
						else if (src_prim->attributes[attr_idx].type == cgltf_attribute_type_normal) {
							// TODO(Dedrick)
						}
						else if (src_prim->attributes[attr_idx].type == cgltf_attribute_type_tangent) {
							// TODO(Dedrick)
						}
						else if (src_prim->attributes[attr_idx].type == cgltf_attribute_type_texcoord) {
							// TODO(Dedrick)
						}
					}
				}
			}
			lane_sync();
		}
	}


	//~ Dedrick: @g2d_stage Parse and build instances.


	//~ Dedrick: @g2d_stage Link instance hierarchy.


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
		if (lane_idx() == lane_from_task_idx(0)) {
			ZoneScopedN("join gpu instances");
			for (u64 l = 0; l < lane_count(); ++l) {
				dksm_gpu_instance_chunk_list_concat_in_place(all_gpu_instances_ptr, &lane_gpu_instances[l]);
			}
		}
		if (lane_idx() == lane_from_task_idx(0)) {
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
