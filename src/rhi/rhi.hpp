// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_RHI_INCLUDED
#	define DK_RHI_INCLUDED
#endif

// TODO(Dedrick): Move OpenGL implementation to its own
// backend file. This is a render hardware interface.
#include "thirdparty/glad/gl.h"

namespace dk {
	auto rhi_init() noexcept -> void;
	auto rhi_shutdown() noexcept -> void;
}

