// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "thirdparty/glad/gl.h"

#if defined(DK_PLATFORM_WIN32)
#	include "rhi_opengl_win32.hpp"
#else
#	error "Platform portion of OpenGL rendering backend not defined."
#endif

namespace dk {
	struct RHI_OGL_Context {
		Arena *arena;
		RGFW_glContext *gl_context;
	};

	extern RHI_OGL_Context *rhi_ogl_context;

	auto rhi_ogl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void;
}
