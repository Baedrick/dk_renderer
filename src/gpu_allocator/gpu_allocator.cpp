// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::gpu_make_arena(Arena *arena, void *base, u64 size) noexcept -> GPU_Arena * {
	GPU_Arena *gpu_arena = arena_push<GPU_Arena>(arena);
	gpu_arena->base = static_cast<u8 *>(base);
	gpu_arena->size = size;
	return gpu_arena;
}

auto dk::gpu_arena_try_push(GPU_Arena *arena, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult {
	GPU_AllocResult result = GPU_AllocResult::OK;
	u64 const pre_pos = align_pow2(arena->pos, align);
	u64 const post_pos = pre_pos + size;
	if (size > arena->size) {
		result = GPU_AllocResult::ERROR_EXCEEDS_SIZE;
	}
	else if (post_pos > arena->size) {
		result = GPU_AllocResult::ERROR_OUT_OF_MEMORY;
	}
	if (result == GPU_AllocResult::OK) {
		*out = arena->base + pre_pos;
		arena->pos = post_pos;
	}
	return result;
}

auto dk::gpu_arena_clear(GPU_Arena *arena) noexcept -> void {
	gpu_arena_pop_to(arena, 0);
}

auto dk::gpu_arena_pos(GPU_Arena *arena) noexcept -> u64 {
	return arena->pos;
}

auto dk::gpu_arena_pop(GPU_Arena *arena, u64 amount) noexcept -> void {
	u64 const pos = gpu_arena_pos(arena);
	u64 new_pos = pos;
	if (amount < pos) {
		new_pos = pos - amount;
	}
	gpu_arena_pop_to(arena, new_pos);
}

auto dk::gpu_arena_pop_to(GPU_Arena *arena, u64 pos) noexcept -> void {
	arena->pos = min(arena->size, pos);
}
