// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_PLATFORM_GRAPHICAL

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "rhi/rhi.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "rhi/rhi.cpp"
#include "dkrend/dkrend.cpp"

auto entry_point(dk::String8List args) noexcept -> int {
	dk::dkr_init(args);
	for (dk::b8 quit = false; !quit; ) {
		quit = dk::dkr_frame();
		FrameMark;
	}
	dk::dkr_shutdown();
	return 0;
}
