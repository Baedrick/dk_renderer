// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::main_thread_entry_point(int argc, char **argv) noexcept -> int {
	plt_set_thread_name("main_thread"_str8);

#if defined(DK_PLATFORM_GFX_INCLUDED)
	plt_gfx_init();
#endif
#if defined(DK_RHI_INCLUDED)
	rhi_init();
#endif

	int const result = entry_point(argc, argv);

#if defined(DK_RHI_INCLUDED)
	rhi_shutdown();
#endif
#if defined(DK_PLATFORM_GFX_INCLUDED)
	plt_gfx_shutdown();
#endif

	return result;
}

auto dk::thread_entry_point(void (*func)(void *params), void *func_params) noexcept -> void {
	ThreadContext *thread_context = thread_context_alloc();
	thread_context_select(thread_context);
	func(func_params);
	thread_context_release(thread_context);
}
