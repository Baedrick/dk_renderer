// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::Buffer8::operator[](u64 index) noexcept -> u8 & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::Buffer8::operator[](u64 index) const noexcept -> u8 const & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::buf8_list_push_node(Buffer8List *list, Buffer8Node *node) noexcept -> Buffer8Node * {
	forward_list_queue_push(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->buffer.size;
	return node;
}

auto dk::buf8_list_push_node_front(Buffer8List *list, Buffer8Node *node) noexcept -> Buffer8Node * {
	forward_list_queue_push_front(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->buffer.size;
	return node;
}

auto dk::buf8_list_push(Arena *arena, Buffer8List *list, Buffer8 buf) noexcept -> Buffer8Node * {
	Buffer8Node *node = arena_push<Buffer8Node>(arena);
	node->buffer = buf;
	buf8_list_push_node(list, node);
	return node;
}

auto dk::buf8_list_push_front(Arena *arena, Buffer8List *list, Buffer8 buf) noexcept -> Buffer8Node * {
	Buffer8Node *node = arena_push<Buffer8Node>(arena);
	node->buffer = buf;
	buf8_list_push_node(list, node);
	return node;
}

auto dk::buf8_list_push_align(Arena *arena, Buffer8List *list, u64 min, u64 align) noexcept -> Buffer8Node * {
	u8 constexpr zeroes[64] = {};
	DK_ASSERT(is_pow2_or_zero(align));
	u64 const padding = max(min, align_pad_pow2(list->total_size, align));
	if (padding < sizeof(zeroes)) {
		return buf8_list_push(arena, list, Buffer{ zeroes, padding });
	}
	else {
		return buf8_list_push(arena, list, Buffer{ arena_push_array<u8>(arena, padding), padding });
	}
}

auto dk::buf8_list_copy(Arena *arena, Buffer8List const *list) noexcept -> Buffer8List {

}

auto dk::buf8_list_join(Arena *arena, Buffer8List const *list) noexcept -> Buffer8 {
	u8 *arr = arena_push_array<u8>(arena, list->total_size);
	u8 *p = arr;
	for (Buffer8Node *node = list->first; node != nullptr; node = node->next) {
		std::memcpy(p, node->buffer.data, node->buffer.size);
		p += node->buffer.size;
	}
	return { .data = arr, .size = list->total_size };
}
