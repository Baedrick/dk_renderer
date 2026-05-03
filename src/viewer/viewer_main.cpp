// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_PLATFORM_GRAPHICAL

#include "base/base.hpp"
#include "platform/platform.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"

auto entry_point(int argc, char **argv) noexcept -> int {
	(void)argc;
	(void)argv;

	using namespace dk;

	RGFW_window *window = plt_window_open(
		str8_literal("Viewer"),
		0, 0,
		800, 600,
		RGFW_windowCenter | RGFW_windowCenterCursor
	);
	while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
		RGFW_pollEvents();
	}
	plt_window_close(window);

	return 0;
}
