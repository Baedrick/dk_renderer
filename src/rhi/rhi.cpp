// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "rhi_core.cpp"

#if DK_RHI_BACKEND == DK_RHI_BACKEND_OPENGL
#	include "opengl/rhi_opengl.cpp"
#else
#	error "RHI backend not specified."
#endif
