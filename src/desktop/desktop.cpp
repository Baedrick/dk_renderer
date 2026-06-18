// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "desktop_core.cpp"

#if defined(DK_PLATFORM_WIN32)
#	include "platform/win32/win32_desktop.cpp"
#else
#	error "Desktop layer platform backend not implemented."
#endif
