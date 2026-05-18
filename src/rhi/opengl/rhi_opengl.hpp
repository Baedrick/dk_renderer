// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "thirdparty/glad/gl.h"

namespace dk {
	struct RHI_OGL_Context {
		Arena *arena;
		b8 debug_context;
	};

	extern RHI_OGL_Context *rhi_ogl_context;

	auto rhi_ogl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void;
}
