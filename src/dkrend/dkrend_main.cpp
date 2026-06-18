// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_GRAPHICAL

#include "base/base.hpp"
#include "desktop/desktop.hpp"
#include "rhi/rhi.hpp"
#include "thirdparty/imgui/imgui_unity.hpp"
#include "pak/pak.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "desktop/desktop.cpp"
#include "rhi/rhi.cpp"
#include "thirdparty/imgui/imgui_unity.cpp"
#include "pak/pak.cpp"
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
