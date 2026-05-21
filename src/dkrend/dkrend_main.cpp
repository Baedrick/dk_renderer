// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_PLATFORM_GRAPHICAL

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "rhi/rhi.hpp"
#include "thirdparty/imgui/imgui_unity.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "rhi/rhi.cpp"
#include "thirdparty/imgui/imgui_unity.cpp"
#include "dkrend/dkrend.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	dk::dkr_init(cmd_line);
	for (dk::b8 quit = false; !quit; ) {
		quit = dk::dkr_frame();
		FrameMark;
	}
	dk::dkr_shutdown();
	return 0;
}
