// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define GLAD_GL_IMPLEMENTATION
#include "thirdparty/glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

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

	for (String8Node const *node = args.first; node != nullptr; node = node->next) {
		if (str8_equals("--opengl_debug"_str8, node->string, STRING_MATCH_FLAG_NONE)) {
			rhi_ogl_context->debug_context = true;
			break;
		}
	}
#if !defined(NDEBUG)
	rhi_ogl_context->debug_context = true;
#endif

	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
	if (rhi_ogl_context->debug_context) {
		hints->debug = RGFW_TRUE;
	}
	RGFW_setGlobalHints_OpenGL(hints);

	RGFW_window* window = RGFW_createWindow("ogl_dummy_window", 0, 0, 1, 1, RGFW_windowHide);
	RGFW_window_createContext_OpenGL(window, hints);
	gladLoadGL(reinterpret_cast<GLADloadfunc>(RGFW_getProcAddress_OpenGL));
	RGFW_window_close(window);
}

auto dk::rhi_shutdown() noexcept -> void {

}

auto dk::rhi_window_hook(RGFW_window *window) noexcept -> void {
	ZoneScoped;
	RGFW_window_createContext_OpenGL(window, RGFW_getGlobalHints_OpenGL());
	if (rhi_ogl_context->debug_context) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(rhi_ogl_debug_message_callback, nullptr);
	}
}

auto dk::rhi_window_unhook(RGFW_window *window) noexcept -> void {
	(void)window;
}
