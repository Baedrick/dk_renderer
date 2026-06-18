// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct Buffer {
		u8 *data;
		u64 size;

		auto operator[](u64 index) noexcept -> u8 &;
		auto operator[](u64 index) const noexcept -> u8 const &;
	};

	struct BufferNode {
		BufferNode *next;
		Buffer buffer;
	};

	struct BufferList {
		BufferNode *first;
		BufferNode *last;
		u64 node_count;
		u64 total_size;
	};

	struct BufferArray {
		Buffer *data;
		u64 count;

		auto operator[](u64 index) noexcept -> Buffer &;
		auto operator[](u64 index) const noexcept -> Buffer const &;
	};

	auto buf(void *data, u64 size) noexcept -> Buffer;

	auto buf_compare(Buffer b1, Buffer b2) noexcept -> s32;
	auto buf_copy(Arena *arena, Buffer buf) noexcept -> Buffer;

	auto buf_list_push_node(BufferList *list, BufferNode *node) noexcept -> BufferNode *;
	auto buf_list_push_node_front(BufferList *list, BufferNode *node) noexcept -> BufferNode *;
	auto buf_list_push(Arena *arena, BufferList *list, Buffer buf) noexcept -> BufferNode *;
	auto buf_list_push_front(Arena *arena, BufferList *list, Buffer buf) noexcept -> BufferNode *;
	auto buf_list_push_align(Arena *arena, BufferList *list, u64 align) noexcept -> void;
	auto buf_list_copy(Arena *arena, BufferList const *list) noexcept -> BufferList;

	auto buf_list_join(Arena *arena, BufferList const *list) noexcept -> Buffer;

	auto buf_array_from_list(Arena *arena, BufferList const *list) noexcept -> BufferArray;
}
