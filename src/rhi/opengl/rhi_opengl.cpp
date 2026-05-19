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
	(void)source;
	(void)type;
	(void)id;
	(void)severity;
	(void)user;
	DK_LOG_INFOF("[OpenGL] %.*s\n", static_cast<int>(length), message);
	std::fprintf(stdout, "[OpenGL] %.*s\n", static_cast<int>(length), message);
}

auto dk::rhi_init(String8List args) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	rhi_ogl_context = arena_push<RHI_OGL_Context>(arena);
	rhi_ogl_context->arena = arena;

	b8 debug_mode = false;
	for (String8Node const *node = args.first; node != nullptr; node = node->next) {
		if (str8_equals("--opengl_debug"_str8, node->string, STRING_MATCH_FLAG_NONE)) {
			debug_mode = true;
			break;
		}
	}
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

	// NOTE(Dedrick): Intentiona leak to keep OpenGL context alive.
	// RGFW_window_close(window);
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

auto dk::rhi_surface_current_texture(RGFW_window *window) noexcept -> RHI_Handle {
	RGFW_window_makeCurrentContext_OpenGL(window);
	return rhi_handle_invalid();
}

auto dk::rhi_surface_present(RGFW_window *window) noexcept -> void {
	RGFW_window_swapBuffers_OpenGL(window);
}
