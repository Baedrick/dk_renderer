// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define GLAD_GL_IMPLEMENTATION
#include "thirdparty/glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

#if defined(DK_PLATFORM_WIN32)
#	include "rhi_opengl_win32.cpp"
#else
#	error "Platform portion of OpenGL rendering backend not defined."
#endif

dk::RHI_OGL_Context *dk::rhi_ogl_context;

auto dk::rhi_ogl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user) noexcept -> void {
	(void)user;
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
	DK_LOG_INFO(log_msg);
	std::fwrite(log_msg.data, 1, log_msg.size, stdout);
	scratch_end(scratch);
}

auto dk::rhi_init(CmdLine *cmd_line) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	rhi_ogl_context = arena_push<RHI_OGL_Context>(arena);
	rhi_ogl_context->arena = arena;

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
	rhi_ogl_context->gl_context = arena_push<RGFW_glContext>(arena);
	RGFW_window_createContextPtr_OpenGL(window, rhi_ogl_context->gl_context, hints);
	gladLoadGL(reinterpret_cast<GLADloadfunc>(RGFW_getProcAddress_OpenGL));
	RGFW_window_swapInterval_OpenGL(window, 1);

	if (debug_mode) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(rhi_ogl_debug_message_callback, nullptr);
	}

	// NOTE(Dedrick): Intentional leak to keep OpenGL context alive.
	// RGFW_window_close(window);

	{
		ZoneScopedN("create vertex array");
		glCreateVertexArrays(1, &rhi_ogl_context->all_purpose_vao);
	}
	{
		ZoneScopedN("create program");
		TempArena const scratch = scratch_begin(nullptr, 0);
		struct { GLenum type; String8 src; GLuint out; String8 errors; } stages[] = {
			{ GL_VERTEX_SHADER, "hello_triangle.vert.spv"_str8, 0 },
			{ GL_FRAGMENT_SHADER, "hello_triangle.frag.spv"_str8, 0 },
		};
		for (u32 stage_idx = 0; stage_idx < 2; ++stage_idx) {
			PLT_Handle const file = plt_file_open(stages[stage_idx].src, PLT_ACCESS_FLAG_READ);
			PLT_FileAttributes const attrib = plt_attributes_from_file(file);
			u8 *const bytes = static_cast<u8 *>(arena_push_bytes(scratch.arena, attrib.size, 8));
			plt_file_read(file, 0, attrib.size, bytes);
			stages[stage_idx].out = glCreateShader(stages[stage_idx].type);
			glShaderBinary(1, &stages[stage_idx].out, GL_SHADER_BINARY_FORMAT_SPIR_V, bytes, static_cast<GLsizei>(attrib.size));
			glSpecializeShader(stages[stage_idx].out, "main", 0, nullptr, nullptr);
			plt_file_close(file);
		}
		GLuint program = glCreateProgram();
		for (u32 stage_idx = 0; stage_idx < 2; ++stage_idx) {
			glAttachShader(program, stages[stage_idx].out);
		}
		glLinkProgram(program);
		for (u32 stage_idx = 0; stage_idx < 2; ++stage_idx) {
			glDeleteShader(stages[stage_idx].out);
		}
		rhi_ogl_context->shader = program;
		scratch_end(scratch);
	}
}

auto dk::rhi_shutdown() noexcept -> void {

}

auto dk::rhi_window_equip(RGFW_window *window) noexcept -> void {
	ZoneScoped;
	rhi_ogl_plt_window_equip(window, rhi_ogl_context->gl_context);
	RGFW_window_makeCurrentContext_OpenGL(window);
}

auto dk::rhi_window_unequip(RGFW_window *window) noexcept -> void {
	(void)window;
}

auto dk::rhi_surface_present(RGFW_window *window) noexcept -> void {
	RGFW_window_swapBuffers_OpenGL(window);
}
