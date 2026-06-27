// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "opengl_core.cpp"

#if defined(DK_PLATFORM_WIN32)
#	include "win32/win32_opengl.cpp"
#else
#	error "OpenGL layer platform backend not implemented."
#endif
