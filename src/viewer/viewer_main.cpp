// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_PLATFORM_GRAPHICAL

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "rhi/rhi.hpp"
#include "viewer/viewer.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "rhi/rhi.cpp"
#include "viewer/viewer.cpp"

auto entry_point(dk::String8List args) noexcept -> int {
	dk::vw_init(args);
	for (dk::b8 quit = false; !quit; ) {
		quit = dk::vw_frame();
		FrameMark;
	}
	dk::vw_shutdown();
	return 0;
}
