// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	template <typename T>
	static auto dksm_idx_from_indexed_chunk_list_element(T const *ptr) noexcept -> u64 {
		u64 idx = 0;
		if (ptr != nullptr && ptr->chunk != nullptr) {
			idx = ptr->chunk->base_idx + (ptr - ptr->chunk->v) + 1;
		}
		return idx;
	}

	template <typename ChunkListType, typename ChunkNodeType, typename ElementType>
	static auto dksm_indexed_chunk_list_push(Arena *arena, ChunkListType *list, u64 capacity) noexcept -> ElementType * {
		ChunkNodeType *node = list->last;
		if (node == nullptr || node->count >= node->cap) {
			node = arena_push<ChunkNodeType>(arena);
			node->cap = capacity;
			node->base_idx = list->total_count;
			node->v = arena_push_array<ElementType>(arena, node->cap);
			forward_list_queue_push(&list->first, &list->last, node);
			list->chunk_count += 1;
		}
		ElementType *result = &node->v[node->count];
		node->count += 1;
		list->total_count += 1;
		return result;
	}

	template <typename ChunkListType, typename ChunkNodeType>
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
