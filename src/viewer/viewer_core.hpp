// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct VW_Context {
		Arena *arena;
		b8 quit;

		RGFW_window *window;
	};

	extern VW_Context *vw_context;

	auto vw_init(int argc, char **argv) noexcept -> void;
	auto vw_frame() noexcept -> b8;
	auto vw_shutdown() noexcept -> void;
}
