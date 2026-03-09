// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::arena_alloc(ArenaParams const *params) noexcept -> Arena * {
	u64 const page_size = plt_get_system_info()->page_size;
	u64 const reserve_size = align_forward_pow2(params->reserve_size, page_size);
	u64 const commit_size = align_forward_pow2(params->commit_size, page_size);

	void *memory = plt_reserve(reserve_size);
	if (!plt_commit(memory, commit_size)) {
		memory = nullptr;
	}

	if (memory == nullptr) [[unlikely]] {
		// TODO(Dedrick): Fatal error graphical message.
		plt_abort(1);
	}

	Arena *arena = static_cast<Arena *>(memory);
	arena->current = arena;
	arena->next = nullptr;
	arena->commit_size = params->commit_size;
	arena->reserve_size = params->reserve_size;
	arena->base_offset = 0;
	arena->offset = ARENA_HEADER_SIZE;
	arena->committed = commit_size;
	arena->reserved = reserve_size;
	arena->flags = params->flags;
	DK_ASAN_POISON_MEMORY_REGION(memory, commit_size);
	DK_ASAN_UNPOISON_MEMORY_REGION(memory, ARENA_HEADER_SIZE);
	return arena;
}

auto dk::arena_release(Arena *arena) noexcept -> void {
	for (Arena *curr = arena->current, *prev = nullptr; curr != nullptr; curr = prev) {
		prev = curr->next;
		plt_release(curr, curr->reserved);
	}
}

auto dk::arena_push_bytes(Arena *arena, u64 size, u64 align) noexcept -> void * {
	void *ptr = arena_push_bytes_no_zero(arena, size, align);
	DK_ASSERT(ptr != nullptr);
	std::memset(ptr, 0, size);
	return ptr;
}

auto dk::arena_push_bytes_no_zero(Arena *arena, u64 size, u64 align) noexcept -> void * {
	Arena *current = arena->current;
	u64 pre_offset = align_forward_pow2(current->offset, align);
	u64 post_offset = pre_offset + size;

	// NOTE(Dedrick): Chain new blocks if needed.
	if (current->reserved < post_offset && (arena->flags & ARENA_FLAG_NO_CHAIN) == 0) {
		u64 reserve_size = current->reserve_size;
		u64 commit_size = current->commit_size;

		u64 const required_size = size + ARENA_HEADER_SIZE + align;
		if (required_size > reserve_size) {
			u64 const page_size = plt_get_system_info()->page_size;
			reserve_size = align_forward_pow2(required_size, page_size);
			commit_size = align_forward_pow2(required_size, page_size);
		}

		ArenaParams const params = {
			.reserve_size = reserve_size,
			.commit_size = commit_size,
			.flags = current->flags
		};
		Arena *new_block = arena_alloc(&params);
		new_block->base_offset = current->base_offset + current->reserved;

		forward_list_stack_push(&arena->current, new_block);

		current = new_block;
		pre_offset = align_forward_pow2(current->offset, align);
		post_offset = pre_offset + size;
	}

	// NOTE(Dedrick): Commit new pages if needed.
	if (current->committed < post_offset) {
		u64 const commit_offset_aligned = align_forward_pow2(post_offset, current->commit_size);
		u64 const commit_offset_clamped = min(commit_offset_aligned, current->reserved);
		u64 const commit_size = commit_offset_clamped - current->committed;
		u8 *commit_ptr = reinterpret_cast<u8 *>(current) + current->committed;

		if (!plt_commit(commit_ptr, commit_size)) {
			plt_abort(1);
		}
		DK_ASAN_POISON_MEMORY_REGION(commit_ptr, commit_size);
		current->committed = commit_offset_clamped;
	}

	// NOTE(Dedrick): Push onto current block.
	void *result = nullptr;
	if (current->committed >= post_offset) {
		result = reinterpret_cast<u8 *>(current) + pre_offset;
		current->offset = post_offset;
		DK_ASAN_UNPOISON_MEMORY_REGION(result, size);
	}

	if (result == nullptr) [[unlikely]] {
		// TODO(Dedrick): Fatal error graphical message.
		plt_abort(1);
	}

	return result;
}

auto dk::arena_clear(Arena *arena) noexcept -> void {
	arena_pop_to(arena, 0);
}

auto dk::arena_offset(Arena *arena) noexcept -> u64 {
	Arena *current = arena->current;
	return current->base_offset + current->offset;
}

auto dk::arena_pop(Arena *arena, u64 amount) noexcept -> void {
	u64 const curr_offset = arena_offset(arena);
	u64 new_offset = curr_offset;
	if (amount < curr_offset) {
		new_offset = curr_offset - amount;
	}
	arena_pop_to(arena, new_offset);
}

auto dk::arena_pop_to(Arena *arena, u64 offset) noexcept -> void {
	u64 const clamped_offset = max(ARENA_HEADER_SIZE, offset);
	Arena *current = arena->current;
	for (Arena *next = nullptr; current->base_offset >= clamped_offset; current = next) {
		next = current->next;
		plt_release(current, current->reserved);
	}
	arena->current = current;
	u64 const new_offset = clamped_offset - current->base_offset;
	DK_ASSERT_ALWAYS(new_offset <= current->offset);
	DK_ASAN_POISON_MEMORY_REGION(reinterpret_cast<u8 *>(current) + new_offset, (current->offset - new_offset));
	current->offset = new_offset;
}

auto dk::arena_temp_begin(Arena *arena) noexcept -> TempArena {
	return { arena, arena_offset(arena) };
}

auto dk::arena_temp_end(TempArena temp) noexcept -> void {
	arena_pop_to(temp.arena, temp.position);
}
