// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	u32 constexpr DDS_MAGIC = 0x20534444; // "DDS "
	u32 constexpr DDS_DX10_EXT_MAGIC = 0x30315844; // "DX10"
	u32 constexpr DDS_DDPF_FOURCC = 0x4;

	struct DDS_PixelFormat {
		u32 size;
		u32 flags;
		u32 four_cc;
		u32 rgb_bit_count;
		u32 r_bit_mask;
		u32 g_bit_mask;
		u32 b_bit_mask;
		u32 a_bit_mask;
	};

	struct DDS_Header {
		u32 magic;
		u32 size;
		u32 flags;
		u32 height;
		u32 width;
		u32 linear_size;
		u32 depth;
		u32 mip_map_count;
		u32 reserved1[11];
		DDS_PixelFormat pixel_format;
		u32 caps;
		u32 caps2;
		u32 caps3;
		u32 caps4;
		u32 reserved2;
	};

	// NOTE(Dedrick): Support small subset of formats actually needed.
	// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
	enum DDS_DXGIFormat : u32 {
		DDS_DXGI_FORMAT_NULL = 0,
		DDS_DXGI_FORMAT_R9G9B9E5 = 67,
	};

	struct DDS_DXT10Header {
		DDS_DXGIFormat dxgi_format;
		u32 resource_dimension;
		u32 misc_flag;
		u32 array_size;
		u32 misc_flags2;
	};

	auto dds_check_magic(Buffer bytes) noexcept -> b8;
	auto dds_has_dxt10_header(DDS_Header const *header) noexcept -> b8;
}
