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

	template <typename ChunkNodeType, typename ChunkListType>
	static auto dksm_chunk_list_concat_in_place(ChunkListType *dst, ChunkListType *to_push) noexcept -> void {
		for (ChunkNodeType *node = to_push->first; node != nullptr; node = node->next) {
			node->base_idx += dst->total_count;
		}
		if (dst->last != nullptr && to_push->first != nullptr) {
			dst->last->next = to_push->first;
			dst->last = to_push->last;
			dst->chunk_count += to_push->chunk_count;
			dst->total_count += to_push->total_count;
		}
		else if (dst->first == nullptr) {
			*dst = *to_push;
		}
		std::memset(to_push, 0, sizeof(ChunkListType));
	}
}

auto dk::dksm_instance_chunk_list_push(Arena *arena, DKSM_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_Instance * {

}

auto dk::dksm_instance_chunk_list_concat_in_place(DKSM_InstanceChunkList *dst, DKSM_InstanceChunkList *to_push) noexcept -> void {

}

auto dk::dksm_idx_from_instance(DKSM_Instance const *instance) noexcept -> u64 {

}

auto dk::dksm_gpu_instance_chunk_list_push(Arena *arena, DKSM_GPU_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Instance * {

}

auto dk::dksm_gpu_instance_chunk_list_concat_in_place(DKSM_GPU_InstanceChunkList *dst, DKSM_GPU_InstanceChunkList *to_push) noexcept -> void {

}

auto dk::dksm_idx_from_gpu_instance(DKSM_GPU_Instance const *gpu_instance) noexcept -> u64 {

}

auto dk::dksm_gpu_vertex_chunk_list_push(Arena *arena, DKSM_GPU_VertexChunkList *list, u64 capacity) noexcept -> DKS_GPU_Vertex * {

}

auto dk::dksm_gpu_vertex_chunk_list_concat_in_place(DKSM_GPU_VertexChunkList *dst, DKSM_GPU_VertexChunkList *to_push) noexcept -> void {

}

auto dk::dksm_gpu_meshlet_chunk_list_push(Arena *arena, DKSM_GPU_MeshletChunkList *list, u64 capacity) noexcept -> DKS_GPU_Meshlet * {

}

auto dk::dksm_gpu_meshlet_chunk_list_concat_in_place(DKSM_GPU_MeshletChunkList *dst, DKSM_GPU_MeshletChunkList *to_push) noexcept -> void {

}

auto dk::dksm_gpu_meshlet_bounds_chunk_list_push(Arena *arena, DKSM_GPU_MeshletBoundsChunkList *list, u64 capacity) noexcept -> DKS_GPU_MeshletBounds * {

}

auto dk::dksm_gpu_meshlet_bounds_chunk_list_concat_in_place(DKSM_GPU_MeshletBoundsChunkList *dst, DKSM_GPU_MeshletBoundsChunkList *to_push) noexcept -> void {

}

auto dk::dksm_gpu_meshlet_vertices_chunk_list_push(Arena *arena, DKSM_GPU_MeshletVerticesChunkList *list, u64 capacity) noexcept -> u32 * {

}

auto dk::dksm_gpu_meshlet_vertices_chunk_list_concat_in_place(DKSM_GPU_MeshletVerticesChunkList *dst, DKSM_GPU_MeshletVerticesChunkList *to_push) noexcept -> void {

}

auto dk::dksm_gpu_meshlet_triangles_chunk_list_push(Arena *arena, DKSM_GPU_MeshletTrianglesChunkList *list, u64 capacity) noexcept -> u32 * {

}

auto dk::dksm_gpu_meshlet_triangles_chunk_list_concat_in_place(DKSM_GPU_MeshletTrianglesChunkList *dst, DKSM_GPU_MeshletTrianglesChunkList *to_push) noexcept -> void {

}

auto dk::dksm_gpu_mesh_chunk_list_push(Arena *arena, DKSM_GPU_MeshChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Mesh * {

}

auto dk::dksm_gpu_mesh_chunk_list_concat_in_place(DKSM_GPU_MeshChunkList *dst, DKSM_GPU_MeshChunkList *to_push) noexcept -> void {

}

auto dk::dksm_idx_from_gpu_mesh(DKSM_GPU_Mesh const *gpu_mesh) noexcept -> u64 {

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

}

auto dk::dksm_buffer_blobs_from_section_bundle(Arena *arena, DKSM_SerializedSectionBundle const *bundle) noexcept -> BufferList {

}
