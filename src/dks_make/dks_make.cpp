// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	template <typename T>
	static auto dksm_idx_from_indexed_chunk_list_element(T const *ptr) noexcept -> u64 {
		u64 idx = 0;
		if (ptr != nullptr && ptr->chunk != nullptr) {
			idx = ptr->chunk->base_idx + (ptr - ptr->chunk->data) + 1;
		}
		return idx;
	}

	template <typename ChunkNodeType, typename ElementType, typename ChunkListType>
	static auto dksm_indexed_chunk_list_push(Arena *arena, ChunkListType *list, u64 capacity) noexcept -> ElementType * {
		ChunkNodeType *node = list->last;
		if (node == nullptr || node->count >= node->capacity) {
			node = arena_push<ChunkNodeType>(arena);
			node->capacity = capacity;
			node->base_idx = list->total_count;
			node->data = arena_push_array<ElementType>(arena, node->capacity);
			forward_list_queue_push(&list->first, &list->last, node);
			list->chunk_count += 1;
		}
		ElementType *result = &node->data[node->count];
		result->chunk = node;
		node->count += 1;
		list->total_count += 1;
		return result;
	}

	template <typename ChunkNodeType, typename ChunkListType, typename ExtraOp>
	static auto dksm_chunk_list_concat_in_place(ChunkListType *dst, ChunkListType *to_push, ExtraOp extra_op) noexcept -> void {
		for (ChunkNodeType *node = to_push->first; node != nullptr; node = node->next) {
			node->base_idx += dst->total_count;
		}
		if (dst->last != nullptr && to_push->first != nullptr) {
			dst->last->next = to_push->first;
			dst->last = to_push->last;
			dst->chunk_count += to_push->chunk_count;
			dst->total_count += to_push->total_count;
			extra_op(dst, to_push);
		}
		else if (dst->first == nullptr) {
			*dst = *to_push;
		}
		std::memset(to_push, 0, sizeof(ChunkListType));
	}

	template <typename ChunkNodeType, typename ChunkListType>
	static auto dksm_chunk_list_concat_in_place(ChunkListType *dst, ChunkListType *to_push) noexcept -> void {
		dksm_chunk_list_concat_in_place(dst, to_push, [](ChunkListType *, ChunkListType *) { });
	}

	template <typename T>
	static auto dksm_serialized_section_make_unpacked(T *data, u64 count) noexcept -> DKSM_SerializedSection {
		DKSM_SerializedSection s = {};
		s.data = data;
		s.size = count * sizeof(T);
		return s;
	};
}

auto dk::dksm_instance_chunk_list_push(Arena *arena, DKSM_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_Instance * {
	return dksm_indexed_chunk_list_push<DKSM_InstanceChunkNode, DKSM_Instance>(arena, list, capacity);
}

auto dk::dksm_instance_chunk_list_concat_in_place(DKSM_InstanceChunkList *dst, DKSM_InstanceChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_InstanceChunkNode>(dst, to_push);
}

auto dk::dksm_idx_from_instance(DKSM_Instance const *instance) noexcept -> u64 {
	return dksm_idx_from_indexed_chunk_list_element(instance);
}

auto dk::dksm_gpu_instance_chunk_list_push(Arena *arena, DKSM_GPU_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Instance * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_InstanceChunkNode, DKSM_GPU_Instance>(arena, list, capacity);

}

auto dk::dksm_gpu_instance_chunk_list_concat_in_place(DKSM_GPU_InstanceChunkList *dst, DKSM_GPU_InstanceChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_InstanceChunkNode>(dst, to_push);
}

auto dk::dksm_idx_from_gpu_instance(DKSM_GPU_Instance const *gpu_instance) noexcept -> u64 {
	return dksm_idx_from_indexed_chunk_list_element(gpu_instance);
}

auto dk::dksm_gpu_vertex_chunk_list_push(Arena *arena, DKSM_GPU_VertexChunkList *list, u64 capacity) noexcept -> DKS_GPU_Vertex * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_VertexChunkNode, DKS_GPU_Vertex>(arena, list, capacity);
}

auto dk::dksm_gpu_vertex_chunk_list_concat_in_place(DKSM_GPU_VertexChunkList *dst, DKSM_GPU_VertexChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_VertexChunkNode>(dst, to_push);
}

auto dk::dksm_gpu_meshlet_chunk_list_push(Arena *arena, DKSM_GPU_MeshletChunkList *list, u64 capacity) noexcept -> DKS_GPU_Meshlet * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_MeshletChunkNode, DKS_GPU_Meshlet>(arena, list, capacity);
}

auto dk::dksm_gpu_meshlet_chunk_list_concat_in_place(DKSM_GPU_MeshletChunkList *dst, DKSM_GPU_MeshletChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_MeshletChunkNode>(dst, to_push);
}

auto dk::dksm_gpu_meshlet_bounds_chunk_list_push(Arena *arena, DKSM_GPU_MeshletBoundsChunkList *list, u64 capacity) noexcept -> DKS_GPU_MeshletBounds * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_MeshletBoundsChunkNode, DKS_GPU_MeshletBounds>(arena, list, capacity);
}

auto dk::dksm_gpu_meshlet_bounds_chunk_list_concat_in_place(DKSM_GPU_MeshletBoundsChunkList *dst, DKSM_GPU_MeshletBoundsChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_MeshletBoundsChunkNode>(dst, to_push);
}

auto dk::dksm_gpu_meshlet_vertices_chunk_list_push(Arena *arena, DKSM_GPU_MeshletVerticesChunkList *list, u64 capacity) noexcept -> u32 * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_MeshletVerticesChunkNode, u32>(arena, list, capacity);
}

auto dk::dksm_gpu_meshlet_vertices_chunk_list_concat_in_place(DKSM_GPU_MeshletVerticesChunkList *dst, DKSM_GPU_MeshletVerticesChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_MeshletVerticesChunkNode>(dst, to_push);
}

auto dk::dksm_gpu_meshlet_triangles_chunk_list_push(Arena *arena, DKSM_GPU_MeshletTrianglesChunkList *list, u64 capacity) noexcept -> u32 * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_MeshletTrianglesChunkNode, u32>(arena, list, capacity);
}

auto dk::dksm_gpu_meshlet_triangles_chunk_list_concat_in_place(DKSM_GPU_MeshletTrianglesChunkList *dst, DKSM_GPU_MeshletTrianglesChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_MeshletTrianglesChunkNode>(dst, to_push);
}

auto dk::dksm_gpu_meshlet_triangle_from_indices(u32 i0, u32 i1, u32 i2) noexcept -> u32 {
	return return (i0 & 0xFF) | ((i1 & 0xFF) << 8) | ((i2 & 0xFF) << 16);
}

auto dk::dksm_gpu_mesh_chunk_list_push(Arena *arena, DKSM_GPU_MeshChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Mesh * {
	return dksm_indexed_chunk_list_push<DKSM_GPU_MeshChunkNode, DKSM_GPU_Mesh>(arena, list, capacity);
}

auto dk::dksm_gpu_mesh_chunk_list_concat_in_place(DKSM_GPU_MeshChunkList *dst, DKSM_GPU_MeshChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_GPU_MeshChunkNode>(dst, to_push,
		[](DKSM_GPU_MeshChunkList *dst, DKSM_GPU_MeshChunkList *to_push) {
			dst->total_vertex_count += to_push->total_vertex_count;
			dst->total_meshlet_count += to_push->total_meshlet_count;
			dst->total_meshlet_vertex_count += to_push->total_meshlet_vertex_count;
			dst->total_meshlet_triangle_count += to_push->total_meshlet_triangle_count;
		}
	);
}

auto dk::dksm_idx_from_gpu_mesh(DKSM_GPU_Mesh const *gpu_mesh) noexcept -> u64 {
	return dksm_idx_from_indexed_chunk_list_element(gpu_mesh);
}

auto dk::dksm_bake_string_chunk_list_push(Arena *arena, DKSM_BakeStringChunkList *list, u64 capacity) noexcept -> DKSM_BakeString * {
	dksm_indexed_chunk_list_push<DKSM_BakeStringChunkNode, DKSM_BakeString>(arena, list, capacity);
}

auto dk::dksm_bake_string_chunk_list_concat_in_place(DKSM_BakeStringChunkList *dst, DKSM_BakeStringChunkList *to_push) noexcept -> void {
	dksm_chunk_list_concat_in_place<DKSM_BakeStringChunkNode>(dst, to_push);
}

auto dk::dksm_bake_string_chunk_list_sorted_from_unsorted(Arena *arena, DKSM_BakeStringChunkList const *list) noexcept -> DKSM_BakeStringChunkList {
	//~ Dedrick: Produce result list with a single chunk node.
	DKSM_BakeStringChunkList result = {};
	for (DKSM_BakeStringChunkNode const *node = list->first; node != nullptr; node = node->next) {
		for (u64 idx = 0; idx < node->count; idx += 1) {
			DKSM_BakeString *str = dksm_bake_string_chunk_list_push(arena, &result, list->total_count);
			*str = node->data[idx];
		}
	}

	//~ Dedrick: Sort chunk node.
	if (result.first != nullptr) {
		insertion_sort(result.first->data, result.first->count,
			[](DKSM_BakeString const &lhs, DKSM_BakeString const &rhs) {
				return str8_compare(lhs.string, rhs.string, STRING_MATCH_FLAG_NONE) < 0;
			}
		);
	}

	//~ Dedrick: Remove and count duplicates.
	u64 num_duplicates = 0;
	if (result.first != nullptr) {
		u64 last_idx = 0;
		DKSM_BakeString *const arr = result.first->data;
		for (u64 idx = 1; idx < result.first->count; ++idx) {
			if (str8_equals(arr[last_idx].string, arr[idx].string, STRING_MATCH_FLAG_NONE)) {
				std::memset(&arr[idx], 0, sizeof(DKSM_BakeString));
				num_duplicates += 1;
			}
			else {
				last_idx = idx;
			}
		}
	}

	//~ Dedrick: Make non-empty elements contiguous and pop extras.
	if (num_duplicates > 0) {
		u64 last_idx = 0;
		DKSM_BakeString *const arr = result.first->data;
		for (u64 idx = 0; idx < result.first->count; ++idx) {
			if (arr[idx].string.size > 0) {
				arr[last_idx] = arr[idx];
				if (last_idx != idx) {
					std::memset(&arr[idx], 0, sizeof(DKSM_BakeString));
				}
				last_idx += 1;
			}
		}

		//~ Dedrick: Pop extras.
		u64 const arena_pos_pre = arena_pos(arena);
		arena_pop_to(arena, arena_pos_pre - (num_duplicates * sizeof(DKSM_BakeString)));
		result.first->count -= num_duplicates;
		result.first->capacity -= num_duplicates;
		result.total_count -= num_duplicates;
	}

	return result;
}

auto dk::dksm_bake_string_map_loose_make(Arena *arena, DKSM_BakeStringMapTopology *topology) noexcept -> DKSM_BakeStringMapLoose * {
	DKSM_BakeStringMapLoose *map = arena_push<DKSM_BakeStringMapLoose>(arena);
	map->slots = arena_push_array<DKSM_BakeStringChunkList *>(arena, topology->slots_count);
	for (u64 idx = 0; idx < topology->slots_count; ++idx) {
		maps->slots[idx] = arena_push<DKSM_BakeStringChunkList>(arena);
	}
	return map;
}

auto dk::dksm_bake_string_map_loose_insert(Arena *arena, DKSM_BakeStringMapTopology *map_topology, DKSM_BakeStringMapLoose *map, u64 chunk_cap, String8 str) noexcept -> DKSM_BakeString * {
	DKSM_BakeString *result = nullptr;
	if (str.size > 0) {
		u64 const hash = u64_hash_from_str8(str);
		u64 const slot_idx = hash % map_topology->slots_count;
		DKSM_BakeStringChunkList *const slot = &map->slots[slot_idx];
		for (DKSM_BakeStringChunkNode const *node = slot->first; node != nullptr; node = node->next) {
			for (u64 idx = 0; idx < node->count; ++idx) {
				if (node->data[idx].hash == hash && str8_equals(str, node->data[idx].string, STRING_MATCH_FLAG_NONE)) {
					result = &node->data[idx];
					break;
				}
			}
			if (result != nullptr) {
				break;
			}
		}
		if (result == nullptr) {
			result = dksm_bake_string_chunk_list_push(arena, slot, chunk_cap);
			result.string = str;
			result.hash = hash;
		}
	}
	return result;
}

auto dk::dksm_bake_string_map_base_indices_from_map_loose(Arena *arena, DKSM_BakeStringMapTopology const *map_topology, DKSM_BakeStringMapLoose const *map) noexcept -> DKSM_BakeStringMapBaseIndices {
	DKSM_BakeStringMapBaseIndices indices = {};
	indices.slots_base_idxs = arena_push_array<u64>(arena, map_topology->slots_count);
	u64 current_idx = 1;
	for (u64 slot_idx = 0; slot_idx < map_topology->slots_count; ++slot_idx) {
		indices.slots_base_idxs[slot_idx] = current_idx;
		current_idx += map->slots[slot_idx]->total_count;
	}
	return indices;
}

auto dk::dksm_bake_idx_from_string(DKSM_BakeStringMapTight const *map, String8 str) noexcept -> u32 {
	u32 bake_idx = 0;
	if (str.size > 0) {
		u64 const hash = u64_hash_from_str8(str);
		u64 const slot_idx = hash % map->slots_count;
		DKSM_BakeStringChunkList const *const slot = &map->slots[slot_idx];
		for (DKSM_BakeStringChunkNode const *node = slot->first; node != nullptr; node = node->next) {
			for (u64 idx = 0; idx < node->count; ++idx) {
				if (node->data[idx].hash == hash && str8_equals(str, node->data[idx].string, STRING_MATCH_FLAG_NONE)) {
					bake_idx = static_cast<u32>(map->slots_base_idxs[slot_idx] + node->base_idx + idx + 1);
					break;
				}
			}
			if (bake_idx != 0) {
				break;
			}
		}
	}
	return bake_idx;
}

auto dk::dksm_bake(Arena *arena, DKSM_BakeParams const *params) noexcept -> DKSM_BakeResults {

}

auto dk::dksm_serialized_section_bundle_from_bake_results(DKSM_BakeResults const *bake_results) noexcept -> DKSM_SerializedSectionBundle {
	DKSM_SerializedSectionBundle bundle = {};
	bundle.sections[DKS_SECTION_KIND_TOP_LEVEL_INFO] = dksm_serialized_section_make_unpacked(bake_results->top_level_info.top_level_info, 1);
	bundle.sections[DKS_SECTION_KIND_STRING_DATA] = dksm_serialized_section_make_unpacked(bake_results->strings.string_data, bake_results->strings.string_data_size);
	bundle.sections[DKS_SECTION_KIND_STRING_TABLE] = dksm_serialized_section_make_unpacked(bake_results->strings.string_table, bake_results->strings.string_table_count);
	bundle.sections[DKS_SECTION_KIND_INSTANCES] = dksm_serialized_section_make_unpacked(bake_results->instances.instances, bake_results->instances.instances_count);
	bundle.sections[DKS_SECTION_KIND_GPU_INSTANCES] = dksm_serialized_section_make_unpacked(bake_results->gpu_instances.gpu_instances, bake_results->gpu_instances.gpu_instances_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.gpu_meshes, bake_results->gpu_meshes.gpu_meshes_count);
	bundle.sections[DKS_SECTION_KIND_GPU_VERTICES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.vertices, bake_results->gpu_meshes.vertex_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLETS] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlets, bake_results->gpu_meshes.meshlet_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_BOUNDS] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_bounds, bake_results->gpu_meshes.meshlet_bounds_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_VERTICES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_vertices, bake_results->gpu_meshes.meshlet_vertex_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_TRIANGLES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_triangles, bake_results->gpu_meshes.meshlet_triangle_count);
	return bundle;
}

auto dk::dksm_buffer_blobs_from_section_bundle(Arena *arena, DKSM_SerializedSectionBundle const *bundle) noexcept -> BufferList {
	BufferList list = {};

	DKS_Header *const header = arena_push<DKS_Header>(arena);
	DKS_Section *const sections = arena_push_array<DKS_Section>(arena, DKS_SECTION_KIND_COUNT);

	//~ Dedrick: Fill header and data section table.
	{
		buf_list_push(arena, &list, buf(header, sizeof(DKS_Header)));
		buf_list_push_align(arena, &list, 8);
		u32 const section_offset = static_cast<u32>(list.total_size);
		buf_list_push(arena, &list, buf(sections, sizeof(DKS_Section) * DKS_SECTION_KIND_COUNT));

		//~ Dedrick: Fill header.
		header->magic = DKS_MAGIC_CONSTANT;
		header->version = DKS_VERSION;
		header->section_offset = section_offset;
		header->section_count = DKS_SECTION_KIND_COUNT;
	}

	//~ Dedrick: Fill baked data sections.
	for (u32 k = 0; k < DKS_SECTION_KIND_COUNT; ++k) {
		DKS_Section *const dst = sections + k;
		u64 data_section_offset = 0;
		if (bundle->sections[k].size > 0) {
			buf_list_push_align(arena, &list, 8);
			data_section_offset = list.total_size;
			buf_list_push(arena, &list, buf(bundle->sections[k].data, bundle->sections[k].size));
		}
		dst->offset = data_section_offset;
		dst->size = bundle->sections[k].size;
	}

	//~ Dedrick: Fill metadata size.
	{
		header->metadata_size = list.total_size;
		u32 constexpr gpu_section_start = DKS_SECTION_KIND_GPU_INSTANCES;
		for (u32 k = gpu_section_start; k < DKS_SECTION_KIND_COUNT; ++k) {
			if (sections[k].size > 0) {
				header->metadata_size = sections[k].offset;
				break;
			}
		}
	}
	return list;
}
