// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "gpu_allocator/gpu_allocator.hpp"
#include "thirdparty/glad/gl.h"

auto dk::gpu_ring_make(Arena *arena, void *base, u64 size) noexcept -> RingBuffer * {
	RingBuffer *ring = arena_push<RingBuffer>(arena);
	ring->base = static_cast<u8 *>(base);
	ring->size = size;
	return ring;
}

auto dk::gpu_ring_try_write(RingBuffer *ring, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult {
	GPU_AllocResult result = GPU_AllocResult::Ok;
	u64 alloc_offset = align_pow2(ring->write_pos, align);
	u64 next_write_pos = alloc_offset + size;
	if (size > ring->size) {
		result = GPU_AllocResult::Error_ExceedsBufferSize;
	} else if (ring->write_pos >= ring->read_pos) {
		if (next_write_pos > ring->size) {
			alloc_offset = align_pow2(0, align);
			next_write_pos = alloc_offset + size;
			if (next_write_pos >= ring->read_pos) {
				result = GPU_AllocResult::Error_OutOfMemory;
			}
		}
	} else if (next_write_pos >= ring->read_pos) {
		result = GPU_AllocResult::Error_OutOfMemory;
	}
	if (result == GPU_AllocResult::Ok) {
		*out = ring->base + (alloc_offset % ring->size);
		ring->write_pos = next_write_pos;
	}
	return result;
}

auto dk::gpu_ring_reclaim_to(RingBuffer *ring, u64 read_pos) noexcept -> void {
	ring->read_pos = read_pos;
}

auto dk::gpu_ring_allocated_byte_count(RingBuffer *ring) noexcept -> u64 {
	return ring->write_pos - ring->read_pos;
}

auto dk::gpu_ring_can_write(RingBuffer *ring, u64 size, u64 align) noexcept -> b8 {
	u64 alloc_offset = align_pow2(ring->write_pos, align);
	u64 next_write_pos = alloc_offset + size;
	if (size > ring->size)                                       return false;
	if (ring->write_pos >= ring->read_pos) {
		if (next_write_pos > ring->size) {
			u64 wrap_offset  = align_pow2(0, align);
			u64 wrap_next    = wrap_offset + size;
			if (wrap_next >= ring->read_pos)                    return false;
		}
	} else if (next_write_pos >= ring->read_pos)                return false;
	return true;
}

auto dk::gpu_heap_alloc() noexcept -> GPU_Heap * {
	return nullptr;
}

auto dk::gpu_heap_release(GPU_Heap * /*heap*/) noexcept -> void {
}

auto dk::gpu_fence_alloc(GPU_Fence *out_fence) noexcept -> void {
	out_fence->handle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

auto dk::gpu_fence_release(GPU_Fence *fence) noexcept -> void {
	glDeleteSync(static_cast<GLsync>(fence->handle));
	fence->handle = nullptr;
}

auto dk::gpu_fence_status(GPU_Fence const *fence) noexcept -> GPU_FenceStatus {
	GLint const r = glClientWaitSync(static_cast<GLsync>(fence->handle), 0, 0);
	switch (r) {
		case GL_CONDITION_SATISFIED: case GL_ALREADY_SIGNALED: return GPU_FenceStatus::Signaled;
		case GL_TIMEOUT_EXPIRED:                               return GPU_FenceStatus::Unsignaled;
		case GL_WAIT_FAILED: default:                          return GPU_FenceStatus::WaitInvalid;
	}
}

auto dk::gpu_fence_wait(GPU_Fence *fence, u64 timeout_us) noexcept -> GPU_FenceStatus {
	GLint const r = glClientWaitSync(static_cast<GLsync>(fence->handle), 0, timeout_us);
	switch (r) {
		case GL_CONDITION_SATISFIED: case GL_ALREADY_SIGNALED: return GPU_FenceStatus::Signaled;
		case GL_TIMEOUT_EXPIRED:                               return GPU_FenceStatus::Unsignaled;
		case GL_WAIT_FAILED: default:                          return GPU_FenceStatus::WaitInvalid;
	}
}

auto dk::gpu_fence_list_push(Arena *arena, GPU_FenceList *list, GPU_Fence fence) noexcept -> GPU_FenceNode * {
	GPU_FenceNode *node = arena_push<GPU_FenceNode>(arena);
	node->v = fence;
	forward_list_queue_push(&list->first, &list->last, node);
	list->count += 1;
	return node;
}
