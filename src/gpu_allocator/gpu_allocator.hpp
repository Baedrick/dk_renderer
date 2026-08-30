// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum class GPU_AllocResult : u8 {
		Ok,
		Error_OutOfMemory,
		Error_ExceedsBufferSize,
		Error_Unknown
	};

	struct GPU_RingFence {
		GLsync fence;
		u64 read_pos;
	};

	struct GPU_RingFenceNode {
		GPU_RingFenceNode *next;
		GPU_RingFence fence;
	};

	struct GPU_RingFenceList {
		GPU_RingFenceNode *first;
		GPU_RingFenceNode *last;
		u64 count;
	};

	struct GPU_RingBuffer {
		u8 *base;
		u64 size;
		u64 read_pos;
		u64 write_pos;
	};

	auto gpu_ring_make(Arena *arena, void *base, u64 size) noexcept -> GPU_RingBuffer *;
	auto gpu_ring_try_write(GPU_RingBuffer *ring, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult;
	auto gpu_ring_reclaim_to(GPU_RingBuffer *ring, u64 read_pos) noexcept -> void;

	// TODO(Dedrick): General purpose gpu buffer allocator.
	struct GPU_Heap {

	};

	auto gpu_heap_alloc() noexcept -> GPU_Heap *;
	auto gpu_heap_release() noexcept -> void;
}
