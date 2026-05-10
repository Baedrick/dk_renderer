// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define GLAD_GL_IMPLEMENTATION
#include "thirdparty/glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

auto dk::rhi_init() noexcept -> void {
	gladLoaderLoadGL();
}

auto dk::rhi_shutdown() noexcept -> void {

}
