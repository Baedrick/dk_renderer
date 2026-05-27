// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct Buffer8 {
		u8 *data;
		u64 size;

		auto operator[](u64 index) noexcept -> u8 &;
		auto operator[](u64 index) const noexcept -> u8 const &;
	};

	struct Buffer8Node {
		Buffer8Node *next;
		Buffer8 buffer;
	};

	struct Buffer8List {
		Buffer8Node *first;
		Buffer8Node *last;
		u64 node_count;
		u64 total_size;
	};

	auto buf8_list_push_node(Buffer8List *list, Buffer8Node *node) noexcept -> Buffer8Node *;
	auto buf8_list_push_node_front(Buffer8List *list, Buffer8Node *node) noexcept -> Buffer8Node *;
	auto buf8_list_push(Arena *arena, Buffer8List *list, Buffer8 buf) noexcept -> Buffer8Node *;
	auto buf8_list_push_front(Arena *arena, Buffer8List *list, Buffer8 buf) noexcept -> Buffer8Node *;
	auto buf8_list_push_align(Arena *arena, Buffer8List *list, u64 min, u64 align) noexcept -> Buffer8Node *;
	auto buf8_list_copy(Arena *arena, Buffer8List const *list) noexcept -> Buffer8List;

	auto buf8_list_join(Arena *arena, Buffer8List const *list) noexcept -> Buffer8;
}
