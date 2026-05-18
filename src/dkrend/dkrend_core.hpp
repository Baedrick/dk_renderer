// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct DKR_Context {
		Arena *arena;
		b8 quit;

		RGFW_window *window;
	};

	extern DKR_Context *dkr_context;

	auto dkr_init(String8List args) noexcept -> void;
	auto dkr_frame() noexcept -> b8;
	auto dkr_shutdown() noexcept -> void;
}
