// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "thirdparty/meshoptimizer/meshoptimizer_unity.cpp"

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

auto dk::dksm_quantize_vertex_position(f32 const position[3], f32 const dequant_summand[3], f32 const dequant_factor[3]) noexcept -> u64 {
	f32 const quant_bits = static_cast<f32>((1 << 21) - 1);
	u64 quantized[3] = {};
	for (u64 axis = 0; axis < 3; ++axis) {
		f32 const normalized = (position[axis] - dequant_summand[axis]) / dequant_factor[axis];
		u64 const q_val = static_cast<u64>(normalized + 0.5f);
		quantized[axis] = (q_val < static_cast<u64>(quant_bits)) ? q_val : static_cast<u64>(quant_bits);
	}
	return (quantized[0] << 42) | (quantized[1] << 21) | quantized[2];
}

auto dk::dksm_gpu_meshlet_triangle_from_indices(u32 i0, u32 i1, u32 i2) noexcept -> u32 {
	return (i0 & 0xFF) | ((i1 & 0xFF) << 8) | ((i2 & 0xFF) << 16);
}

auto dk::dksm_bake_string_chunk_list_push(Arena *arena, DKSM_BakeStringChunkList *list, u64 capacity) noexcept -> DKSM_BakeString * {
	return dksm_indexed_chunk_list_push<DKSM_BakeStringChunkNode, DKSM_BakeString>(arena, list, capacity);
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
	ZoneScoped;

	DKSM_BakeStringMapLoose *map = arena_push<DKSM_BakeStringMapLoose>(arena);
	map->slots = arena_push_array<DKSM_BakeStringChunkList *>(arena, topology->slots_count);
	for (u64 idx = 0; idx < topology->slots_count; ++idx) {
		map->slots[idx] = arena_push<DKSM_BakeStringChunkList>(arena);
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
	indices.slots_base_idxs = arena_push_array<u64>(arena, map_topology->slots_count + 1);
	u64 total_count = 0;
	for (u64 slot_idx = 0; slot_idx < map_topology->slots_count; ++slot_idx) {
		indices.slots_base_idxs[slot_idx] = total_count;
		total_count += map->slots[slot_idx]->total_count;
	}
	indices.slots_base_idxs[map_topology->slots_count] = total_count;
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
	TempArena const scratch = scratch_begin(&arena, 1);

	//~ Dedrick: @dksm_bake_stage Build string map.
	DKSM_BakeStringMapTight *bake_strings = nullptr;
	{
		ZoneScopedN("build string map");
		TempArena const scratch2 = scratch_begin(&scratch.arena, 1);

		//~ Dedrick: Set up per-line outputs.
		DKSM_BakeStringMapTopology *topology = nullptr;
		DKSM_BakeStringMapLoose **lane_maps_loose = nullptr;
		DKSM_BakeStringMapLoose *map_loose = nullptr;
		if (lane_idx() == 0) {
			ZoneScopedN("set up per line outputs");
			bake_strings = arena_push<DKSM_BakeStringMapTight>(scratch.arena);
			topology = arena_push<DKSM_BakeStringMapTopology>(scratch2.arena);
			topology->slots_count = 64 + params->instances.total_count;
			lane_maps_loose = arena_push_array<DKSM_BakeStringMapLoose *>(scratch2.arena, lane_count());
			map_loose = dksm_bake_string_map_loose_make(scratch2.arena, topology);
		}
		lane_sync_broadcast(&bake_strings, 0);
		lane_sync_broadcast(&topology, 0);
		lane_sync_broadcast(&lane_maps_loose, 0);
		lane_sync_broadcast(&map_loose, 0);

		//~ Dedrick: Set up this lane's map.
		lane_maps_loose[lane_idx()] = dksm_bake_string_map_loose_make(scratch2.arena, topology);
		DKSM_BakeStringMapLoose *const lane_map = lane_maps_loose[lane_idx()];

		//~ Dedrick: Push all strings into this lane's map.
		{
			ZoneScopedN("push all strings into this lane's map");

			//~ Dedrick: Push small top-level strings.
			if (lane_idx() == 0) {
				ZoneScopedN("push small top-level strings");
				dksm_bake_string_map_loose_insert(scratch.arena, topology, lane_map, 1, params->top_level_info.model_name);
			}

			//~ Dedrick: Push strings from instances.
			{
				ZoneScopedN("instances");
				for (DKSM_InstanceChunkNode const *node = params->instances.first; node != nullptr; node = node->next) {
					LaneRange const range = lane_range(node->count);
					for (u64 idx = range.begin; idx < range.end; ++idx) {
						dksm_bake_string_map_loose_insert(scratch2.arena, topology, lane_map, 4, node->data[idx].name);
					}
				}
			}

			lane_sync();
		}

		//~ Dedrick: Join.
		{
			ZoneScopedN("join");
			LaneRange const slot_range = lane_range(topology->slots_count);
			for (u64 slot_idx = slot_range.begin; slot_idx < slot_range.end; ++slot_idx) {
				for (u64 src_lane_idx = 0; src_lane_idx < lane_count(); ++src_lane_idx) {
					DKSM_BakeStringMapLoose *src_map = lane_maps_loose[src_lane_idx];
					DKSM_BakeStringMapLoose *dst_map = map_loose;
					dksm_bake_string_chunk_list_concat_in_place(dst_map->slots[slot_idx], src_map->slots[slot_idx]);
				}
			}
			lane_sync();
		}

		//~ Dedrick: Sort.
		{
			ZoneScopedN("sort");
			DKSM_BakeStringMapLoose *const map = map_loose;
			LaneRange const slot_range = lane_range(topology->slots_count);
			for (u64 slot_idx = slot_range.begin; slot_idx < slot_range.end; ++slot_idx) {
				*map->slots[slot_idx] = dksm_bake_string_chunk_list_sorted_from_unsorted(scratch2.arena, map_loose->slots[slot_idx]);
			}
			lane_sync();
		}

		//~ Dedrick: Tighten string table.
		{
			ZoneScopedN("tighten string table");
			DKSM_BakeStringMapLoose *const map = map_loose;
			if (lane_idx() == 0) {
				ZoneScopedN("calculate base indices; set up tight map");
				DKSM_BakeStringMapBaseIndices const bake_string_map_base_indices = dksm_bake_string_map_base_indices_from_map_loose(scratch.arena, topology, map);
				bake_strings->slots_count = topology->slots_count;
				bake_strings->slots = arena_push_array<DKSM_BakeStringChunkList>(scratch.arena, bake_strings->slots_count);
				bake_strings->slots_base_idxs = bake_string_map_base_indices.slots_base_idxs;
				bake_strings->total_count = bake_strings->slots_base_idxs[bake_strings->slots_count];
			}
			lane_sync();

			{
				ZoneScopedN("fill tight map");
				LaneRange const slot_range = lane_range(bake_strings->slots_count);
				for (u64 slot_idx = slot_range.begin; slot_idx < slot_range.end; ++slot_idx) {
					bake_strings->slots[slot_idx] = *map->slots[slot_idx];
				}
				lane_sync();
			}
		}

		scratch_end(scratch2);
	}

	//~ Dedrick: @dksm_bake_stage Bake strings.
	DKSM_StringBakeResult *baked_strings = nullptr;
	{
		ZoneScopedN("bake strings");
		if (lane_idx() == 0) {
			ZoneScopedN("set up");
			baked_strings = arena_push<DKSM_StringBakeResult>(scratch.arena);
			baked_strings->strings_table_count = bake_strings->total_count + 1;
			baked_strings->strings_table = arena_push_array<DKS_StringTable>(arena, baked_strings->strings_table_count);
			u64 offset_cursor = 0;
			for (u64 slot_idx = 0; slot_idx < bake_strings->slots_count; ++slot_idx) {
				DKSM_BakeStringChunkList const *const slot = &bake_strings->slots[slot_idx];
				for (DKSM_BakeStringChunkNode const *node = slot->first; node != nullptr; node = node->next) {
					for (u64 idx = 0; idx < node->count; ++idx) {
						DKSM_BakeString const *src = &node->data[idx];
						u64 const dst_idx = bake_strings->slots_base_idxs[slot_idx] + node->base_idx + idx + 1;
						baked_strings->strings_table[dst_idx].offset = offset_cursor;
						baked_strings->strings_table[dst_idx].size = src->string.size;
						offset_cursor += src->string.size;
					}
				}
			}
			baked_strings->string_data_size = offset_cursor;
			baked_strings->string_data = arena_push_array<u8>(arena, baked_strings->string_data_size);
		}
		lane_sync_broadcast(&baked_strings, 0);

		{
			ZoneScopedN("wide fill");
			LaneRange const slot_range = lane_range(bake_strings->slots_count);
			for (u64 slot_idx = slot_range.begin; slot_idx < slot_range.end; ++slot_idx) {
				DKSM_BakeStringChunkNode const *const slot = &bake_strings->slots[slot_idx];
				for (DKSM_BakeStringChunkNode const *node = slot->first; node != nullptr; node = node->next) {
					for (u64 idx = 0; idx < node->count; ++idx) {
						DKSM_BakeString const *src = &node->data[idx];
						u64 const dst_idx = bake_strings->slots_base_idxs[slot_idx] + node->base_idx + idx + 1;
						u64 const dst_offset = baked_strings->strings_table[dst_idx].offset;
						std::memcpy(baked_strings->string_data + dst_offset, src->string.data, src->string.size);
					}
				}
			}
			lane_sync();
		}
	}

	//~ Dedrick: @dksm_bake_stage Compute gpu instances layout.
	struct InstanceLayout {
		u64 *lane_chunk_gpu_inst_counts;
		u64 *lane_chunk_gpu_inst_offsets;
		u64 total_gpu_inst_count;
	};
	InstanceLayout *inst_layout = nullptr;
	{
		ZoneScopedN("compute gpu instances layout");
		if (lane_idx() == 0) {
			inst_layout = arena_push<InstanceLayout>(scratch.arena);
			u64 const slots_count = lane_count() * params->instances.chunk_count;
			inst_layout->lane_chunk_gpu_inst_counts = arena_push_array<u64>(scratch.arena, slots_count);
			inst_layout->lane_chunk_gpu_inst_offsets = arena_push_array<u64>(scratch.arena, slots_count);
		}
		lane_sync_broadcast(&inst_layout, 0);

		{
			ZoneScopedN("wide count");
			u64 chunk_idx = 0;
			for (DKSM_InstanceChunkNode const *node = params->instances.first; node != nullptr; node = node->next) {
				LaneRange const range = lane_range(node->count);
				u64 const slot_idx = lane_idx() * params->instances.chunk_count + chunk_idx;
				for (u64 idx = range.begin; idx < range.end; ++idx) {
					inst_layout->lane_chunk_gpu_inst_counts[slot_idx] += node->data[idx].gpu_instance_count;
				}
				chunk_idx += 1;
			}
			lane_sync();
		}

		if (lane_idx() == 0) {
			u64 chunk_idx = 0;
			u64 gpu_inst_layout_offset = 1;
			for (DKSM_InstanceChunkNode const *node = params->instances.first; node != nullptr; node = node->next) {
				for (u64 lane = 0; lane < lane_count(); ++lane) {
					u64 const slot_idx = lane * params->instances.chunk_count + chunk_idx;
					inst_layout->lane_chunk_gpu_inst_offsets[slot_idx] = gpu_inst_layout_offset;
					gpu_inst_layout_offset += inst_layout->lane_chunk_gpu_inst_counts[slot_idx];
				}
				chunk_idx += 1;
			}
			inst_layout->total_gpu_inst_count = gpu_inst_layout_offset;
		}
		lane_sync();
	}

	//~ Dedrick: @dksm_bake_stage Bake instances and gpu instances.
	DKSM_InstanceBakeResult *baked_instances = nullptr;
	DKSM_GPU_InstanceBakeResult *baked_gpu_instances = nullptr;
	{
		ZoneScopedN("bake instances and gpu instances");
		if (lane_idx() == 0) {
			ZoneScopedN("set up");
			baked_instances = arena_push<DKSM_InstanceBakeResult>(scratch.arena);
			baked_gpu_instances = arena_push<DKSM_GPU_InstanceBakeResult>(scratch.arena);
		}
		lane_sync_broadcast(&baked_instances, 0);
		lane_sync_broadcast(&baked_gpu_instances, 0);
		if (lane_idx() == lane_from_task_idx(0)) {
			baked_instances->instances_count = params->instances.total_count + 1;
			baked_instances->instances = arena_push_array<DKS_Instance>(arena, baked_instances->instances_count);
		}
		if (lane_idx() == lane_from_task_idx(1)) {
			baked_gpu_instances->gpu_instances_count = inst_layout->total_gpu_inst_count;
			baked_gpu_instances->gpu_instances = arena_push_array<DKS_GPU_Instance>(arena, baked_gpu_instances->gpu_instances_count);
		}
		lane_sync();

		{
			ZoneScopedN("wide fill");
			u64 chunk_idx = 0;
			for (DKSM_InstanceChunkNode const *node = params->instances.first; node != nullptr; node = node->next) {
				LaneRange const range = lane_range(node->count);
				u64 const slot_idx = lane_idx() * params->instances.chunk_count + chunk_idx;
				u64 dst_gpu_inst_offset = inst_layout->lane_chunk_gpu_inst_offsets[slot_idx];
				for (u64 idx = range.begin; idx < range.end; ++idx) {
					u64 const dst_idx = node->base_idx + idx + 1;
					DKSM_Instance const *src = &node->data[idx];
					DKS_Instance *dst = &baked_instances->instances[dst_idx];

					//~ Dedrick: Fill instance data.
					dst->name_string_idx = dksm_bake_idx_from_string(bake_strings, src->name);
					dst->parent       = static_cast<u32>(dksm_idx_from_instance(src->parent));
					dst->first_child  = static_cast<u32>(dksm_idx_from_instance(src->first_child));
					dst->prev_sibling = static_cast<u32>(dksm_idx_from_instance(src->prev_sibling));
					dst->next_sibling = static_cast<u32>(dksm_idx_from_instance(src->next_sibling));
					dst->gpu_instance_offset = static_cast<u32>(dst_gpu_inst_offset);
					dst->gpu_instance_count  = static_cast<u32>(src->gpu_instance_count);

					//~ Dedrick: Fill gpu instance data.
					for (DKSM_GPU_Instance const *src_gpu_inst = src->first_gpu_instance; src_gpu_inst != nullptr; src_gpu_inst = src_gpu_inst->next_gpu_instance) {
						DKS_GPU_Instance *dst_gpu_inst = &baked_gpu_instances->gpu_instances[dst_gpu_inst_offset];
						std::memcpy(dst_gpu_inst->world_from_object, src_gpu_inst->world_from_object, sizeof(dst_gpu_inst->world_from_object));
						dst_gpu_inst->mesh_idx = static_cast<u32>(dksm_idx_from_gpu_mesh(src_gpu_inst->mesh));
						dst_gpu_inst_offset += 1;
					}
				}
				chunk_idx += 1;
			}
			lane_sync();
		}
	}

	//~ Dedrick: @dksm_bake_stage Compute gpu mesh layout.
	struct MeshLayout {
		u64 *lane_chunk_v_counts; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mo_counts; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mv_counts; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mt_counts; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_v_offsets; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mo_offsets; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mv_offsets; // [lane_count * gpu_mesh_chunk_count]
		u64 *lane_chunk_mt_offsets; // [lane_count * gpu_mesh_chunk_count]
		u64 total_v_count;
		u64 total_mo_count;
		u64 total_mv_count;
		u64 total_mt_count;
	};
	MeshLayout *mesh_layout = nullptr;
	{
		ZoneScopedN("compute gpu mesh layout");
		if (lane_idx() == 0) {
			mesh_layout = arena_push<MeshLayout>(scratch.arena);
			u64 const slots_count = lane_count() * params->gpu_meshes.chunk_count;
			mesh_layout->lane_chunk_v_counts = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mo_counts = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mv_counts = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mt_counts = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_v_offsets = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mo_offsets = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mv_offsets = arena_push_array<u64>(scratch.arena, slots_count);
			mesh_layout->lane_chunk_mt_offsets = arena_push_array<u64>(scratch.arena, slots_count);
		}
		lane_sync_broadcast(&mesh_layout, 0);

		{
			ZoneScopedN("wide count");
			u64 chunk_idx = 0;
			for (DKSM_GPU_MeshChunkNode const *node = params->gpu_meshes.first; node != nullptr; node = node->next) {
				LaneRange const range = lane_range(node->count);
				u64 const slot_idx = lane_idx() * params->gpu_meshes.chunk_count + chunk_idx;
				for (u64 idx = range.begin; idx < range.end; ++idx) {
					DKSM_GPU_Mesh const *src = &node->data[idx];
					mesh_layout->lane_chunk_v_counts[slot_idx] += src->vertex_count;
					mesh_layout->lane_chunk_mo_counts[slot_idx] += src->meshlet_count;
					mesh_layout->lane_chunk_mv_counts[slot_idx] += src->meshlet_vertex_count;
					mesh_layout->lane_chunk_mt_counts[slot_idx] += src->meshlet_triangle_count;
				}
				chunk_idx += 1;
			}
			lane_sync();
		}

		if (lane_idx() == 0) {
			u64 chunk_idx = 0;
			u64 v_layout_offset  = 1;
			u64 mo_layout_offset = 1;
			u64 mv_layout_offset = 1;
			u64 mt_layout_offset = 1;
			for (DKSM_GPU_MeshChunkNode const *node = params->gpu_meshes.first; node != nullptr; node = node->next) {
				for (u64 lane = 0; lane < lane_count(); ++lane) {
					u64 const slot_idx = lane * params->gpu_meshes.chunk_count + chunk_idx;
					mesh_layout->lane_chunk_v_offsets[slot_idx]  = v_layout_offset;
					mesh_layout->lane_chunk_mo_offsets[slot_idx] = mo_layout_offset;
					mesh_layout->lane_chunk_mv_offsets[slot_idx] = mv_layout_offset;
					mesh_layout->lane_chunk_mt_offsets[slot_idx] = mt_layout_offset;
					v_layout_offset  += mesh_layout->lane_chunk_v_counts[slot_idx];
					mo_layout_offset += mesh_layout->lane_chunk_mo_counts[slot_idx];
					mv_layout_offset += mesh_layout->lane_chunk_mv_counts[slot_idx];
					mt_layout_offset += mesh_layout->lane_chunk_mt_counts[slot_idx];
				}
				chunk_idx += 1;
			}
			mesh_layout->total_v_count  = v_layout_offset;
			mesh_layout->total_mo_count = mo_layout_offset;
			mesh_layout->total_mv_count = mv_layout_offset;
			mesh_layout->total_mt_count = mt_layout_offset;
		}
		lane_sync();
	}

	//~ Dedrick: @dksm_bake_stage Bake gpu meshes and data.
	DKSM_GPU_MeshBakeResult *baked_gpu_meshes = nullptr;
	{
		ZoneScopedN("bake gpu meshes and data");

		//~ Dedrick: Set up outputs.
		if (lane_idx() == 0) {
			baked_gpu_meshes = arena_push<DKSM_GPU_MeshBakeResult>(scratch.arena);
		}
		lane_sync_broadcast(&baked_gpu_meshes, 0);
		if (lane_idx() == lane_from_task_idx(0)) {
			baked_gpu_meshes->gpu_meshes_count = params->gpu_meshes.total_count + 1;
			baked_gpu_meshes->gpu_meshes = arena_push_array<DKS_GPU_Mesh>(arena, baked_gpu_meshes->gpu_meshes_count);
		}
		if (lane_idx() == lane_from_task_idx(1)) {
			baked_gpu_meshes->vertices_count = mesh_layout->total_v_count;
			baked_gpu_meshes->vertices = arena_push_array<DKS_GPU_Vertex>(arena, baked_gpu_meshes->vertices_count);
		}
		if (lane_idx() == lane_from_task_idx(2)) {
			baked_gpu_meshes->meshlets_count = mesh_layout->total_mo_count;
			baked_gpu_meshes->meshlets = arena_push_array<DKS_GPU_Meshlet>(arena, baked_gpu_meshes->meshlets_count);
		}
		if (lane_idx() == lane_from_task_idx(3)) {
			baked_gpu_meshes->meshlet_bounds_count = mesh_layout->total_mo_count;
			baked_gpu_meshes->meshlet_bounds = arena_push_array<DKS_GPU_MeshletBounds>(arena, baked_gpu_meshes->meshlet_bounds_count);
		}
		if (lane_idx() == lane_from_task_idx(4)) {
			baked_gpu_meshes->meshlet_vertices_count = mesh_layout->total_mv_count;
			baked_gpu_meshes->meshlet_vertices = arena_push_array<u32>(arena, baked_gpu_meshes->meshlet_vertices_count);
		}
		if (lane_idx() == lane_from_task_idx(5)) {
			baked_gpu_meshes->meshlet_triangles_count = mesh_layout->total_mt_count;
			baked_gpu_meshes->meshlet_triangles = arena_push_array<u32>(arena, baked_gpu_meshes->meshlet_triangles_count);
		}
		lane_sync();

		//~ Dedrick: Wide fill.
		u64 chunk_idx = 0;
		for (DKSM_GPU_MeshChunkNode const *node = params->gpu_meshes.first; node != nullptr; node = node->next) {
			LaneRange const range = lane_range(node->count);
			u64 const slot_idx = lane_idx() * params->gpu_meshes.chunk_count + chunk_idx;
			u64 dst_v_offset = mesh_layout->lane_chunk_v_offsets[slot_idx];
			u64 dst_mo_offset = mesh_layout->lane_chunk_mo_offsets[slot_idx];
			u64 dst_mv_offset = mesh_layout->lane_chunk_mv_offsets[slot_idx];
			u64 dst_mt_offset = mesh_layout->lane_chunk_mt_offsets[slot_idx];
			for (u64 idx = range.begin; idx < range.end; ++idx) {
				DKSM_GPU_Mesh const *src = &node->data[idx];
				DKS_GPU_Mesh *dst = &baked_gpu_meshes->gpu_meshes[node->base_idx + idx + 1];

				//~ Dedrick: Fill mesh info.
				std::memcpy(dst->sphere_center, src->sphere_center, sizeof(dst->sphere_center));
				dst->sphere_radius = src->sphere_radius;
				std::memcpy(dst->dequantization_factor, src->dequantization_factor, sizeof(dst->dequantization_factor));
				std::memcpy(dst->dequantization_summand, src->dequantization_summand, sizeof(dst->dequantization_summand));
				dst->meshlet_offset = static_cast<u32>(dst_mo_offset);
				dst->meshlet_count = src->meshlet_count;

				//~ Dedrick: Fill mesh vertex data.
				std::memcpy(&baked_gpu_meshes->vertices[dst_v_offset], src->vertices, src->vertex_count * sizeof(DKS_GPU_Vertex));

				//~ Dedrick: Fill mesh meshlet data.
				std::memcpy(&baked_gpu_meshes->meshlet_bounds[dst_mo_offset], src->meshlet_bounds, src->meshlet_count * sizeof(DKS_GPU_MeshletBounds));
				std::memcpy(&baked_gpu_meshes->meshlet_triangles[dst_mt_offset], src->meshlet_triangles, src->meshlet_triangle_count * sizeof(u32));
				for (u32 ml_idx = 0; ml_idx < src->meshlet_count; ++ml_idx) {
					DKS_GPU_Meshlet meshlet = src->meshlets[ml_idx];
					meshlet.vertex_offset += static_cast<u32>(dst_mv_offset);
					meshlet.triangle_offset += static_cast<u32>(dst_mt_offset);
					baked_gpu_meshes->meshlets[dst_mo_offset + ml_idx] = meshlet;
				}

				//~ Dedrick: Fixup meshlet vertices from local to global.
				for (u32 v_idx = 0; v_idx < src->meshlet_vertex_count; ++v_idx) {
					baked_gpu_meshes->meshlet_vertices[dst_mv_offset + v_idx] = src->meshlet_vertices[v_idx] + static_cast<u32>(dst_v_offset);
				}

				dst_v_offset += src->vertex_count;
				dst_mo_offset += src->meshlet_count;
				dst_mv_offset += src->meshlet_vertex_count;
				dst_mt_offset += src->meshlet_triangle_count;
			}
			chunk_idx += 1;
		}
		lane_sync();
	}

	//~ Dedrick: @dksm_bake_stage Small final baking tasks.
	DKSM_TopLevelInfoBakeResult *baked_top_level_info = nullptr;
	{
		ZoneScopedN("small final baking tasks");
		if (lane_idx() == lane_from_task_idx(0)) {
			ZoneScopedN("bake top level info");
			baked_top_level_info = arena_push<DKSM_TopLevelInfoBakeResult>(scratch.arena);
			baked_top_level_info->top_level_info = arena_push<DKSM_TopLevelInfo>(arena);
			baked_top_level_info->top_level_info->model_name_string_idx = dksm_bake_idx_from_string(bake_strings, params->top_level_info.model_name);
		}
		lane_sync_broadcast(&baked_top_level_info, lane_from_task_idx(0));
	}

	//~ Dedrick: @dksm_bake_stage Package results.
	DKSM_BakeResults result = {};
	{
		result.top_level_info = *baked_top_level_info;
		result.strings = *baked_strings;
		result.instances = *baked_instances;
		result.gpu_instances = *baked_gpu_instances;
		result.gpu_meshes = *baked_gpu_meshes;
	}
	lane_sync();

	scratch_end(scratch);
	return result;
}

auto dk::dksm_serialized_section_bundle_from_bake_results(DKSM_BakeResults const *bake_results) noexcept -> DKSM_SerializedSectionBundle {
	DKSM_SerializedSectionBundle bundle = {};
	bundle.sections[DKS_SECTION_KIND_TOP_LEVEL_INFO] = dksm_serialized_section_make_unpacked(bake_results->top_level_info.top_level_info, 1);
	bundle.sections[DKS_SECTION_KIND_STRING_DATA] = dksm_serialized_section_make_unpacked(bake_results->strings.string_data, bake_results->strings.string_data_size);
	bundle.sections[DKS_SECTION_KIND_STRING_TABLE] = dksm_serialized_section_make_unpacked(bake_results->strings.strings_table, bake_results->strings.strings_table_count);
	bundle.sections[DKS_SECTION_KIND_INSTANCES] = dksm_serialized_section_make_unpacked(bake_results->instances.instances, bake_results->instances.instances_count);
	bundle.sections[DKS_SECTION_KIND_GPU_INSTANCES] = dksm_serialized_section_make_unpacked(bake_results->gpu_instances.gpu_instances, bake_results->gpu_instances.gpu_instances_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.gpu_meshes, bake_results->gpu_meshes.gpu_meshes_count);
	bundle.sections[DKS_SECTION_KIND_GPU_VERTICES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.vertices, bake_results->gpu_meshes.vertices_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLETS] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlets, bake_results->gpu_meshes.meshlets_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_BOUNDS] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_bounds, bake_results->gpu_meshes.meshlet_bounds_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_VERTICES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_vertices, bake_results->gpu_meshes.meshlet_vertices_count);
	bundle.sections[DKS_SECTION_KIND_GPU_MESHLET_TRIANGLES] = dksm_serialized_section_make_unpacked(bake_results->gpu_meshes.meshlet_triangles, bake_results->gpu_meshes.meshlet_triangles_count);
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
