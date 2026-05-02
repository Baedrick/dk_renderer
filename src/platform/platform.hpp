// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "core/platform_core.hpp"
#ifdef DK_BUILD_PLATFORM_GRAPHICAL
#	include "gfx/platform_gfx.hpp"
#endif
#include "platform_entry.hpp"

#ifdef DK_PLATFORM_WIN32
#	include "core/platform_core_win32.hpp"
#else
#	error "Platform core layer not implemented for this platform."
#endif

#ifdef DK_BUILD_PLATFORM_GRAPHICAL
#	ifdef DK_PLATFORM_WIN32
#		include "gfx/platform_gfx_win32.hpp"
#	else
#		error "Platform graphical layer not implemented for this platform."
#	endif
#endif
