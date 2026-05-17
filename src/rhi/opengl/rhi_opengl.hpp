// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "thirdparty/glad/gl.h"

namespace dk {
	extern RGFW_glContext *rhi_ogl_rgfw_context;
	
	auto rhi_ogl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void;
}
