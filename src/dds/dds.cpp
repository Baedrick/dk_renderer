// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::dds_check_magic(Buffer bytes) noexcept -> b8 {
	b8 is_dds = false;
	if (bytes.size >= sizeof(DDS_Header)) {
		DDS_Header *const header = reinterpret_cast<DDS_Header *>(bytes.data);
		if (header->magic == DDS_MAGIC && header->size == 124) {
			is_dds = true;
		}
	}
	return is_dds;
}

auto dk::dds_has_dxt10_header(DDS_Header const *header) noexcept -> b8 {
	return (header->pixel_format.flags & DDS_DDPF_FOURCC) != 0 && header->pixel_format.four_cc == DDS_DX10_EXT_MAGIC;
}
