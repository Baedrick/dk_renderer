// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct G2D_ConvertParams {

	};

	auto g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams;
}
