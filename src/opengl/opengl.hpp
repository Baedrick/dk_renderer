// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_OGL_INCLUDED
#	define DK_OGL_INCLUDED
#endif

#include "opengl_core.hpp"

#if defined(DK_PLATFORM_WIN32)
#	include "win32/win32_opengl.hpp"
#else
#	error "OpenGL layer platform backend not implemented."
#endif
