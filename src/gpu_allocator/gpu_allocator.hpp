// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum class GPU_AllocResult : u8 {
		OK,
		ERROR_OUT_OF_MEMORY,
		ERROR_EXCEEDS_SIZE,
		ERROR_UNKNOWN
	};

	struct GPU_Arena {
		u8 *base;
		u64 size;
		u64 pos;
	};

	auto gpu_make_arena(Arena *arena, void *base, u64 size) noexcept -> GPU_Arena *;
	auto gpu_arena_try_push(GPU_Arena *arena, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult;
	auto gpu_arena_clear(GPU_Arena *arena) noexcept -> void;
	auto gpu_arena_pos(GPU_Arena *arena) noexcept -> u64;
	auto gpu_arena_pop(GPU_Arena *arena, u64 amount) noexcept -> void;
	auto gpu_arena_pop_to(GPU_Arena *arena, u64 pos) noexcept -> void;

	// TODO(Dedrick): General purpose gpu buffer allocator.
	struct GPU_Heap {

	};

	auto gpu_heap_alloc() noexcept -> GPU_Heap *;
	auto gpu_heap_release() noexcept -> void;
}
