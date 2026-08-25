// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::gpu_ring_make(Arena *arena, void *base, u64 size) noexcept -> GPU_RingBuffer * {
	GPU_RingBuffer *ring = arena_push<GPU_RingBuffer>(arena);
	ring->base = base;
	ring->size = size;
	return ring;
}

auto dk::gpu_ring_try_write(GPU_RingBuffer *ring, u64 size, u64 align, void **out) noexcept -> GPU_AllocResult {
	GPU_AllocResult result = GPU_AllocResult::Ok;
	u64 alloc_offset = align_pow2(ring->write_pos, align);
	u64 next_write_pos = alloc_offset + size;
	if (size > ring->size) {
		result = GPU_AllocResult::Error_ExceedsBufferSize;
	}
	else if (ring->write_pos >= ring->read_pos) {
		if (next_write_pos > ring->size) {
			alloc_offset = align_pow2(0, align);
			next_write_pos = alloc_offset + size;
			if (next_write_pos >= ring->read_pos) {
				result = GPU_AllocResult::Error_OutOfMemory;
			}
		}
	}
	else if (next_write_pos >= ring->read_pos) {
		result = GPU_AllocResult::Error_OutOfMemory;
	}
	if (result == GPU_AllocResult::Ok) {
		*out = ring_base + pre_pos;
		ring->write_pos = post_pos;
	}
	return result;
}

auto dk::gpu_ring_reclaim_to(GPU_RingBuffer *ring, u64 read_pos) noexcept -> void {
	ring->read_pos = read_pos;
}

auto dk::gpu_heap_alloc() noexcept -> GPU_Heap * {
	return nullptr;
}

auto dk::gpu_heap_release() noexcept -> void {

}
