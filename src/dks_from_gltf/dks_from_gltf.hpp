// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct G2D_ConvertParams {
		String8 file_name;
		Buffer file_data;
		GLTF_BufferArray buffers;
		b8 is_glb;
	};

	auto g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams;
}
