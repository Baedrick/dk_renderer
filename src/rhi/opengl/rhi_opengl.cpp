// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define GLAD_GL_IMPLEMENTATION
#include "thirdparty/glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

RGFW_glContext *dk::rhi_ogl_rgfw_context;

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
	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
#if !defined(NDEBUG)
	hints->debug = RGFW_TRUE;
#endif
	RGFW_setGlobalHints_OpenGL(hints);

	RGFW_window* window = RGFW_createWindow("ogl_dummy_window", 0, 0, 1, 1, RGFW_windowHide);
	RGFW_glContext *context = RGFW_window_createContext_OpenGL(window, hints);
	rhi_ogl_rgfw_context = context;

	int const version = gladLoadGL(reinterpret_cast<GLADloadfunc>(RGFW_getProcAddress_OpenGL));
	std::printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	b8 enable_debug = false;
	for (String8Node const *node = args.first; node != nullptr; node = node->next) {
		if (str8_equals("--opengl_debug"_str8, node->string, STRING_MATCH_FLAG_NONE)) {
			enable_debug = true;
		}
	}
	if (enable_debug) {
#if defined(NDEBUG)
		glEnable(GL_DEBUG_OUTPUT);
#endif
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(rhi_ogl_debug_message_callback, nullptr);
	}

	// NOTE(Dedrick): Intentional leak of dummy window to keep context
	// alive for OpenGL function loading and application usage.
	// RGFW_windowClose(window);
}

auto dk::rhi_shutdown() noexcept -> void {

}
