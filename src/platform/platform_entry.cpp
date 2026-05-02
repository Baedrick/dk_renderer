// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::main_thread_entry_point(int argc, char **argv) noexcept -> int {

#ifdef DK_BUILD_PLATFORM_GRAPHICAL
	plt_gfx_init();
#endif

	int const result = entry_point(argc, argv);
	return result;
}

auto dk::thread_entry_point(PLT_ThreadFunction *func, void *func_params) noexcept -> void {
	ThreadContext *thread_context = thread_context_alloc();
	thread_context_select(thread_context);
	func(func_params);
	thread_context_release(thread_context);
}
