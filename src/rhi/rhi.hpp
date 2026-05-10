// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_RHI_INCLUDED
#	define DK_RHI_INCLUDED
#endif

// TODO(Dedrick): Mechanism for choosing backend during compile.
#define DK_RHI_BACKEND_OPENGL

#include "rhi_core.hpp"

#if defined(DK_RHI_BACKEND_OPENGL)
#	include "rhi_opengl.hpp"
#else
#	error "RHI backend not specified."
#endif
