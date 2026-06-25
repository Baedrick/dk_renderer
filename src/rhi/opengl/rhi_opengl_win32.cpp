// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#ifdef _WIN32
extern "C" {
	// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

auto dk::rhi_ogl_plt_window_equip(RGFW_window *window, RGFW_glContext *context) noexcept -> void {
	window->src.ctx.native = context;
	window->src.gfxType = RGFW_gfxNativeOpenGL;

	RGFW_glHints *const hints = RGFW_getGlobalHints_OpenGL();
	s32 pixel_format_attribs[50];
	RGFW_attribStack stack = {};
	RGFW_attribStack_init(&stack, pixel_format_attribs, array_count(pixel_format_attribs));
	RGFW_attribStack_pushAttribs(&stack, WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB);
	RGFW_attribStack_pushAttribs(&stack, WGL_DRAW_TO_WINDOW_ARB, 1);
	RGFW_attribStack_pushAttribs(&stack, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
	RGFW_attribStack_pushAttribs(&stack, WGL_SUPPORT_OPENGL_ARB, 1);
	RGFW_attribStack_pushAttribs(&stack, WGL_COLOR_BITS_ARB, 32);
	RGFW_attribStack_pushAttribs(&stack, WGL_DOUBLE_BUFFER_ARB, 1);
	RGFW_attribStack_pushAttribs(&stack, WGL_ALPHA_BITS_ARB, hints->alpha);
	RGFW_attribStack_pushAttribs(&stack, WGL_DEPTH_BITS_ARB, hints->depth);
	RGFW_attribStack_pushAttribs(&stack, WGL_STENCIL_BITS_ARB, hints->stencil);
	RGFW_attribStack_pushAttribs(&stack, WGL_STEREO_ARB, hints->stereo);
	RGFW_attribStack_pushAttribs(&stack, WGL_AUX_BUFFERS_ARB, hints->auxBuffers);
	RGFW_attribStack_pushAttribs(&stack, WGL_RED_BITS_ARB, hints->red);
	RGFW_attribStack_pushAttribs(&stack, WGL_GREEN_BITS_ARB, hints->blue);
	RGFW_attribStack_pushAttribs(&stack, WGL_BLUE_BITS_ARB, hints->green);
	RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_RED_BITS_ARB, hints->accumRed);
	RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_GREEN_BITS_ARB, hints->accumGreen);
	RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_BLUE_BITS_ARB, hints->accumBlue);
	RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_ALPHA_BITS_ARB, hints->accumAlpha);
	if(hints->sRGB) {
		if (hints->profile != RGFW_glES) {
			RGFW_attribStack_pushAttribs(&stack, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, 1);
		}
		else {
			RGFW_attribStack_pushAttribs(&stack, WGL_COLORSPACE_SRGB_EXT, hints->sRGB);
		}
	}
	RGFW_attribStack_pushAttribs(&stack, WGL_COVERAGE_SAMPLES_NV, hints->samples);
	RGFW_attribStack_pushAttribs(&stack, 0, 0);

	int pixel_format = 0;
	UINT num_formats = 0;
	wglChoosePixelFormatARB(window->src.hdc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);

	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	DescribePixelFormat(window->src.hdc, pixel_format, sizeof(pfd), &pfd);
	SetPixelFormat(window->src.hdc, pixel_format, &pfd);
}
