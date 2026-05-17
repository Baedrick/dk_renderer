// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_RHI_INCLUDED
#	define DK_RHI_INCLUDED
#endif

#define DK_RHI_BACKEND_OPENGL 0

#if !defined(DK_RHI_BACKEND) && defined(DK_PLATFORM_WIN32)
#	define DK_RHI_BACKEND DK_RHI_BACKEND_OPENGL
#endif

#include "rhi_core.hpp"

#if DK_RHI_BACKEND == DK_RHI_BACKEND_OPENGL
#	include "opengl/rhi_opengl.hpp"
#else
#	error "RHI backend not specified."
#endif
