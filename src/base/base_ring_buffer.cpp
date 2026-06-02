// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::ring_write(RingBuffer *ring, void const *src, u64 src_size) noexcept -> void {
	DK_ASSERT(src_size <= ring->size);
	u64 const ring_offset = ring->write_pos % ring->size;
	u64 const size_before_split = ring->size - ring_offset;
	u64 const first_part_size = min(size_before_split, src_size);
	u64 const second_part_size = src_size - first_part_size;
	void const *first_part = src;
	void const *second_part = static_cast<u8 const *>(src) + first_part_size;
	std::memcpy(ring->base + ring_offset, first_part, first_part_size);
	std::memcpy(ring->base, second_part, second_part_size);
	ring->write_pos += src_size;
}

auto dk::ring_read(RingBuffer *ring, void *dst, u64 dst_size) noexcept -> void {
	DK_ASSERT(dst_size <= ring->size);
	u64 const ring_offset = ring->read_pos % ring->size;
	u64 const size_before_split = ring->size - ring_offset;
	u64 const first_part_size = min(size_before_split, dst_size);
	u64 const second_part_size = dst_size - first_part_size;
	std::memcpy(dst, ring->base + ring_offset, first_part_size);
	std::memcpy(static_cast<u8 *>(dst) + first_part_size, ring->base, second_part_size);
	ring->read_pos += dst_size;
}
