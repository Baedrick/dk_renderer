// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::main_thread_entry_point(int argc, char **argv) noexcept -> int {
	plt_set_thread_name("main_thread"_str8);
	TempArena const scratch = scratch_begin(nullptr, 0);

	String8List cmd_line_strings = {};
	for (int arg_index = 0; arg_index < argc; ++arg_index) {
		str8_list_push(
			scratch.arena,
			&cmd_line_strings,
			str8_cstring(reinterpret_cast<u8 *>(argv[arg_index]))
		);
	}

#if defined(DK_PLATFORM_GFX_INCLUDED)
	plt_gfx_init();
#endif
#if defined(DK_RHI_INCLUDED)
	rhi_init(cmd_line_strings);
#endif

	int const result = entry_point(cmd_line_strings);

#if defined(DK_RHI_INCLUDED)
	rhi_shutdown();
#endif
#if defined(DK_PLATFORM_GFX_INCLUDED)
	plt_gfx_shutdown();
#endif

	scratch_end(scratch);
	return result;
}

auto dk::thread_entry_point(void (*func)(void *params), void *func_params) noexcept -> void {
	ThreadContext *thread_context = thread_context_alloc();
	thread_context_select(thread_context);
	func(func_params);
	thread_context_release(thread_context);
}
