// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum class GPU_AllocResult : u8 {
		Ok,
		Error_OutOfMemory,
		Error_ExceedsBufferSize,
		Error_Unknown,
	};

	struct GPU_Heap {
		// TODO(Dedrick): Persistent GPU buffer allocator backing the global
		// asset_server buffer for transforms / instances / materials.
		// Distinct from the ringbuffer's transient duplicate-data role;
		// lifecycle is open/close at shutdown, not per-frame.
	};

	struct GPU_Fence {
		void *handle;
	};

	struct GPU_FenceNode {
		GPU_FenceNode *next;
		GPU_Fence v;
	};

	struct GPU_FenceList {
		GPU_FenceNode *first;
		GPU_FenceNode *last;
		u64 count;
	};

	enum class GPU_FenceStatus : u8 {
		Signaled,
		Unsignaled,
		ErrorConditionLost,
		WaitInvalid,
	};

	auto gpu_ring_make(Arena *arena, void *base, u64 size) noexcept -> RingBuffer *;
	auto gpu_ring_try_write(RingBuffer *ring, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult;
	auto gpu_ring_reclaim_to(RingBuffer *ring, u64 read_pos) noexcept -> void;
	auto gpu_ring_allocated_byte_count(RingBuffer *ring) noexcept -> u64;
	auto gpu_ring_can_write(RingBuffer *ring, u64 size, u64 align) noexcept -> b8;

	auto gpu_heap_alloc() noexcept -> GPU_Heap *;
	auto gpu_heap_release(GPU_Heap *heap) noexcept -> void;

	auto gpu_fence_alloc(GPU_Fence *out_fence) noexcept -> void;
	auto gpu_fence_release(GPU_Fence *fence) noexcept -> void;
	auto gpu_fence_status(GPU_Fence const *fence) noexcept -> GPU_FenceStatus;
	auto gpu_fence_wait(GPU_Fence *fence, u64 timeout_us) noexcept -> GPU_FenceStatus;

	auto gpu_fence_list_push(Arena *arena, GPU_FenceList *list, GPU_Fence fence) noexcept -> GPU_FenceNode *;
}
