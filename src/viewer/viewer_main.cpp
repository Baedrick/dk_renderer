// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_PLATFORM_GRAPHICAL

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "viewer/viewer.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "viewer/viewer.cpp"

auto entry_point(int argc, char **argv) noexcept -> int {
	dk::vw_init(argc, argv);
	for (dk::b8 quit = false; !quit; ) {
		quit = dk::vw_frame();
		FrameMark;
	}
	dk::vw_shutdown();
	return 0;
}
