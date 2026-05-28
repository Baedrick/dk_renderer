// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::arena_alloc(ArenaParams const *params) noexcept -> Arena * {
	u64 const page_size = plt_get_system_info()->page_size;
	u64 const reserve_size = align_pow2(params->reserve_size, page_size);
	u64 const commit_size = align_pow2(params->commit_size, page_size);

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
	arena->base_pos = 0;
	arena->pos = ARENA_HEADER_SIZE;
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
	u64 pre_pos = align_pow2(current->pos, align);
	u64 post_pos = pre_pos + size;

	// NOTE(Dedrick): Chain new blocks if needed.
	if (current->reserved < post_pos && (arena->flags & ARENA_FLAG_NO_CHAIN) == 0) {
		u64 reserve_size = current->reserve_size;
		u64 commit_size = current->commit_size;

		u64 const required_size = size + ARENA_HEADER_SIZE + align;
		if (required_size > reserve_size) {
			u64 const page_size = plt_get_system_info()->page_size;
			reserve_size = align_pow2(required_size, page_size);
			commit_size = align_pow2(required_size, page_size);
		}

		ArenaParams const params = {
			.reserve_size = reserve_size,
			.commit_size = commit_size,
			.flags = current->flags
		};
		Arena *new_block = arena_alloc(&params);
		new_block->base_pos = current->base_pos + current->reserved;

		forward_list_stack_push(&arena->current, new_block);

		current = new_block;
		pre_pos = align_pow2(current->pos, align);
		post_pos = pre_pos + size;
	}

	// NOTE(Dedrick): Commit new pages if needed.
	if (current->committed < post_pos) {
		u64 const commit_pos_aligned = align_pow2(post_pos, current->commit_size);
		u64 const commit_pos_clamped = min(commit_pos_aligned, current->reserved);
		u64 const commit_size = commit_pos_clamped - current->committed;
		u8 *commit_ptr = reinterpret_cast<u8 *>(current) + current->committed;

		if (!plt_commit(commit_ptr, commit_size)) {
			plt_abort(1);
		}
		DK_ASAN_POISON_MEMORY_REGION(commit_ptr, commit_size);
		current->committed = commit_pos_clamped;
	}

	// NOTE(Dedrick): Push onto current block.
	void *result = nullptr;
	if (current->committed >= post_pos) {
		result = reinterpret_cast<u8 *>(current) + pre_pos;
		current->pos = post_pos;
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

auto dk::arena_pos(Arena *arena) noexcept -> u64 {
	Arena *current = arena->current;
	return current->base_pos + current->pos;
}

auto dk::arena_pop(Arena *arena, u64 amount) noexcept -> void {
	u64 const curr_pos = arena_pos(arena);
	u64 new_pos = curr_pos;
	if (amount < curr_pos) {
		new_pos = curr_pos - amount;
	}
	arena_pop_to(arena, new_pos);
}

auto dk::arena_pop_to(Arena *arena, u64 pos) noexcept -> void {
	u64 const clamped_pos = max(ARENA_HEADER_SIZE, pos);
	Arena *current = arena->current;
	for (Arena *next = nullptr; current->base_pos >= clamped_pos; current = next) {
		next = current->next;
		plt_release(current, current->reserved);
	}
	arena->current = current;
	u64 const new_pos = clamped_pos - current->base_pos;
	DK_ASSERT_ALWAYS(new_pos <= current->pos);
	DK_ASAN_POISON_MEMORY_REGION(reinterpret_cast<u8 *>(current) + new_pos, (current->pos - new_pos));
	current->pos = new_pos;
}

auto dk::arena_temp_begin(Arena *arena) noexcept -> TempArena {
	return { arena, arena_pos(arena) };
}

auto dk::arena_temp_end(TempArena temp) noexcept -> void {
	arena_pop_to(temp.arena, temp.pos);
}
