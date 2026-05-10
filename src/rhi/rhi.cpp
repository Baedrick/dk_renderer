// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "rhi_core.cpp"

#if defined(DK_RHI_BACKEND_OPENGL)
#	include "rhi_opengl.cpp"
#else
#	error "RHI backend not specified."
#endif
