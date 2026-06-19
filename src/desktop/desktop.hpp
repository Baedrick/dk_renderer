// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_DESKTOP_INCLUDED
#	define DK_DESKTOP_INCLUDED
#endif

#include "desktop_core.hpp"

#if defined(DK_PLATFORM_WIN32)
#	include "win32/win32_desktop.hpp"
#else
#	error "Desktop layer platform backend not implemented."
#endif
