// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::Buffer::operator[](u64 index) noexcept -> u8 & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::Buffer::operator[](u64 index) const noexcept -> u8 const & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::BufferArray::operator[](u64 index) noexcept -> Buffer & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::BufferArray::operator[](u64 index) const noexcept -> Buffer const & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::buf(void *data, u64 size) noexcept -> Buffer {
	return { .data = static_cast<u8 *>(data), .size = size };
}

auto dk::buf_compare(Buffer b1, Buffer b2) noexcept -> s32 {
	return std::memcmp(b1.data, b2.data, min(b1.size, b2.size));
}

auto dk::buf_copy(Arena *arena, Buffer buf) noexcept -> Buffer {
	u8 *arr = arena_push_array<u8>(arena, buf.size);
	std::memcpy(arr, buf.data, buf.size);
	return { .data = arr, .size = buf.size };
}

auto dk::buf_list_push_node(BufferList *list, BufferNode *node) noexcept -> BufferNode * {
	forward_list_queue_push(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->buffer.size;
	return node;
}

auto dk::buf_list_push_node_front(BufferList *list, BufferNode *node) noexcept -> BufferNode * {
	forward_list_queue_push_front(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->buffer.size;
	return node;
}

auto dk::buf_list_push(Arena *arena, BufferList *list, Buffer buf) noexcept -> BufferNode * {
	BufferNode *node = arena_push<BufferNode>(arena);
	node->buffer = buf;
	buf_list_push_node(list, node);
	return node;
}

auto dk::buf_list_push_front(Arena *arena, BufferList *list, Buffer buf) noexcept -> BufferNode * {
	BufferNode *node = arena_push<BufferNode>(arena);
	node->buffer = buf;
	buf_list_push_node_front(list, node);
	return node;
}

auto dk::buf_list_push_align(Arena *arena, BufferList *list, u64 align) noexcept -> void {
	DK_ASSERT(is_pow2_or_zero(align));
	u64 const padding = align_pad_pow2(list->total_size, align);
	if (padding > 0) {
		Buffer const buf = { arena_push_array<u8>(arena, padding), padding };
		buf_list_push(arena, list, buf);
	}
}

auto dk::buf_list_copy(Arena *arena, BufferList const *list) noexcept -> BufferList {
	BufferList result = {};
	for (BufferNode const *node = list->first; node != nullptr; node = node->next) {
		buf_list_push(arena, &result, buf_copy(arena, node->buffer));
	}
	return result;
}

auto dk::buf_list_join(Arena *arena, BufferList const *list) noexcept -> Buffer {
	u8 *arr = arena_push_array<u8>(arena, list->total_size);
	u8 *p = arr;
	for (BufferNode const *node = list->first; node != nullptr; node = node->next) {
		std::memcpy(p, node->buffer.data, node->buffer.size);
		p += node->buffer.size;
	}
	return { .data = arr, .size = list->total_size };
}

auto dk::buf_array_from_list(Arena *arena, BufferList const *list) noexcept -> BufferArray {
	BufferArray array = {};
	array.count = list->node_count;
	array.data = arena_push_array<Buffer>(arena, array.count);
	u64 idx = 0;
	for (BufferNode const *node = list->first; node != nullptr; node = node->next) {
		array[idx++] = node->buffer;
	}
	return array;
}
