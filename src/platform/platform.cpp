// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "core/platform_core.cpp"
#ifdef DK_BUILD_PLATFORM_GRAPHICAL
#	include "gfx/platform_gfx.cpp"
#endif

#ifdef DK_PLATFORM_WIN32
#	include "core/platform_core_win32.cpp"
#else
#	error "Platform core layer not implemented for this platform."
#endif

#ifdef DK_BUILD_PLATFORM_GRAPHICAL
#	ifdef DK_PLATFORM_WIN32
#		include "gfx/platform_gfx_win32.cpp"
#	else
#		error "Platform graphical layer not implemented for this platform."
#	endif
#endif
