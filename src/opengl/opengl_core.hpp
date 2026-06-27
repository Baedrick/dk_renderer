// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/glad/gl.h"

namespace dk {
	struct OGL_Context {
		Arena *arena;
		RGFW_glContext *gl_context;
		GLuint all_purpose_vao;
	};

	extern OGL_Context *ogl_context;

	auto ogl_debug_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void;

	auto ogl_init(CmdLine *cmd_line) noexcept -> void;
	auto ogl_shutdown() noexcept -> void;

	auto ogl_window_equip(RGFW_window *window) noexcept -> void;
	auto ogl_window_unequip(RGFW_window *window) noexcept -> void;
	auto ogl_platform_window_equip(RGFW_window *window, RGFW_glContext *context) noexcept -> void;
}
