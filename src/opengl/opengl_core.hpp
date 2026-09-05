// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/glad/gl.h"

namespace dk {
	enum class OGL_FenceStatus {
		Signaled,
		Timeout,
		Error
	};

	struct OGL_Context {
		Arena *arena;
		RGFW_glContext *gl_context;
		GLuint all_purpose_vao;
	};

	extern OGL_Context *ogl_context;

	auto ogl_init(CmdLine *cmd_line) noexcept -> void;
	auto ogl_shutdown() noexcept -> void;

	auto ogl_window_equip(RGFW_window *window) noexcept -> void;
	auto ogl_window_unequip(RGFW_window *window) noexcept -> void;
	auto ogl_platform_window_equip(RGFW_window *window, RGFW_glContext *context) noexcept -> void;

	auto ogl_fence_alloc() noexcept -> GLsync;
	auto ogl_fence_release(GLsync fence) noexcept -> void;
	auto ogl_fence_wait(GLsync fence, u64 end_time_us) noexcept -> OGL_FenceStatus;

	auto ogl_shader_stage_compile(GLenum stage, Buffer source, String8 name) noexcept -> GLuint;
	auto ogl_shader_link(u64 count, GLuint const *stages, String8 name) noexcept -> GLuint;

	auto ogl__wait_us_from_end_time_us(u64 end_time_us) noexcept -> u64;
	auto ogl__debug_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void;
}
