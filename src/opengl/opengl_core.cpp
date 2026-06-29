// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define GLAD_GL_IMPLEMENTATION
#include "thirdparty/glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

dk::OGL_Context *dk::ogl_context;

auto dk::ogl_debug_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void {
	(void)user;

	// NOTE(Dedrick): Ignore insignificant error/warning codes.
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) { return; }

	String8 source_str = "Unknown"_str8;
	String8 type_str = "Unknown"_str8;
	String8 severity_str = "Unknown"_str8;
	switch (source) {
		case GL_DEBUG_SOURCE_API: source_str = "API"_str8; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: source_str = "Window System"_str8; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "Shader Compiler"_str8; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: source_str = "Third Party"_str8; break;
		case GL_DEBUG_SOURCE_APPLICATION: source_str = "Application"_str8; break;
		case GL_DEBUG_SOURCE_OTHER: source_str = "Other"_str8; break;
	}
	switch (type) {
		case GL_DEBUG_TYPE_ERROR: type_str = "Error"_str8; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "Deprecated Behavior"_str8; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_str = "Undefined Behavior"_str8; break;
		case GL_DEBUG_TYPE_PORTABILITY: type_str = "Portability"_str8; break;
		case GL_DEBUG_TYPE_PERFORMANCE: type_str = "Performance"_str8; break;
		case GL_DEBUG_TYPE_MARKER: type_str = "Marker"_str8; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: type_str = "Push Group"_str8; break;
		case GL_DEBUG_TYPE_POP_GROUP: type_str = "Pop Group"_str8; break;
		case GL_DEBUG_TYPE_OTHER: type_str = "Other"_str8; break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH: severity_str = "High"_str8; break;
		case GL_DEBUG_SEVERITY_MEDIUM: severity_str = "Medium"_str8; break;
		case GL_DEBUG_SEVERITY_LOW: severity_str = "Low"_str8; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: severity_str = "Notification"_str8; break;
	}
	TempArena const scratch = scratch_begin(nullptr, 0);
	String8 const log_msg = str8f(
		scratch.arena,
		"[OpenGL] %.*s (%d) of %.*s severity from %.*s: %.*s\n",
		DK_STR8_VARG(type_str),
		id,
		DK_STR8_VARG(severity_str),
		DK_STR8_VARG(source_str),
		static_cast<int>(length), message
	);
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		DK_LOG_ERROR(log_msg);
		std::fwrite(log_msg.data, 1, log_msg.size, stderr);
	} else {
		DK_LOG_INFO(log_msg);
	}
	scratch_end(scratch);
}

auto dk::ogl_init(CmdLine *cmd_line) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	ogl_context = arena_push<OGL_Context>(arena);
	ogl_context->arena = arena;

	b8 debug_mode = cmd_line_has_flag(cmd_line, "opengl_debug"_str8);
#if !defined(NDEBUG)
	debug_mode = true;
#endif

	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
	if (debug_mode) {
		hints->debug = RGFW_TRUE;
	}
	RGFW_setGlobalHints_OpenGL(hints);

	RGFW_window *window = RGFW_createWindow("ogl_bootstrap_window", 0, 0, 1, 1, RGFW_windowHide);
	ogl_context->gl_context = arena_push<RGFW_glContext>(arena);
	RGFW_window_createContextPtr_OpenGL(window, ogl_context->gl_context, hints);
	gladLoadGL(reinterpret_cast<GLADloadfunc>(RGFW_getProcAddress_OpenGL));
	RGFW_window_swapInterval_OpenGL(window, 1);

	if (debug_mode) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(ogl_debug_msg_callback, nullptr);
	}

	// NOTE(Dedrick): Intentional leak to keep OpenGL context alive.
	// RGFW_window_close(window);

	{
		ZoneScopedN("create vertex array");
		glCreateVertexArrays(1, &ogl_context->all_purpose_vao);
	}
}

auto dk::ogl_shutdown() noexcept -> void {

}

auto dk::ogl_window_equip(RGFW_window *window) noexcept -> void {
	ZoneScoped;
	ogl_plt_window_equip(window, ogl_context->gl_context);
	RGFW_window_makeCurrentContext_OpenGL(window);
}

auto dk::ogl_window_unequip(RGFW_window *window) noexcept -> void {
	(void)window;
}

auto dk::ogl_shader_stage_compile(GLenum stage, Buffer source, String8 name) noexcept -> GLuint {
	TempArena const scratch = scratch_begin(nullptr, 0);

	//~ Dedrick: Compile shader stage.
	GLuint shader = glCreateShader(type);
	glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source.data, static_cast<GLsizei>(source.size));
	glSpecializeShader(shader, "main", 0, nullptr, nullptr);
	if (name.size > 0) {
		glObjectLabel(GL_SHADER, shader, static_cast<GLsizei>(name.size), reinterpret_cast<char const *>(name.data));
	}

	//~ Dedrick: Query status and logs.
	GLint status = 0;
	GLint info_log_length = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		String8 log = {};
		log.data = arena_push_array<u8>(scratch.arena, info_log_length + 1);
		log.size = info_log_length;
		glGetShaderInfoLog(
			shader,
			static_cast<GLsizei>(log.size),
			nullptr,
			reinterpret_cast<char *>(const_cast<u8 *>(log.data))
		);
		DK_LOG_ERRORF("[OpenGL] %.*s\n", DK_STR8_VARG(log));
	}
	if (status != GL_TRUE) {
		glDeleteShader(shader);
		shader = 0;
	}

	scratch_end(scratch);
	return shader;
}

auto dk::ogl_shader_link(u64 count, GLuint const *stages, String8 name) noexcept -> GLuint {
	TempArena const scratch = scratch_begin(nullptr, 0);

	//~ Dedrick: Link program.
	GLuint program = glCreateProgram();
	for (u64 idx = 0; idx < count; ++idx) {
		glAttachShader(program, stages[idx]);
	}
	glLinkProgram(program);
	if (name.size > 0) {
		glObjectLabel(GL_PROGRAM, shader, static_cast<GLsizei>(name.size), reinterpret_cast<char const *>(name.data));
	}

	//~ Dedrick: Query status and logs.
	GLint status = 0;
	GLint info_log_length = 0;
	glGetProgramiv(shader, GL_LINK_STATUS, &status);
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		String8 log = {};
		log.data = arena_push_array<u8>(scratch.arena, info_log_length + 1);
		log.size = info_log_length;
		glGetProgramInfoLog(
			program,
			static_cast<GLsizei>(log.size),
			nullptr,
			reinterpret_cast<char *>(const_cast<u8 *>(log.data))
		);
		DK_LOG_ERRORF("[OpenGL] %.*s\n", DK_STR8_VARG(log));
	}
	if (status != GL_TRUE) {
		glDeleteProgram(program);
		program = 0;
	}

	scratch_end(scratch);
	return program;
}
