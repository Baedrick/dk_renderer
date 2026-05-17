// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::VW_Context *dk::vw_context;

auto dk::vw_init(String8List args) noexcept -> void {
	ZoneScoped;

	Arena *arena = arena_alloc();
	vw_context = arena_push<VW_Context>(arena);
	vw_context->arena = arena;
	vw_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter);
	RGFW_window_setContext_OpenGL(vw_context->window, rhi_ogl_rgfw_context);
	RGFW_window_makeCurrentContext_OpenGL(vw_context->window);

	b8 is_parent = true;
	for (String8Node const *node = args.first; node != nullptr; node = node->next) {
		if (str8_equals("--child"_str8, node->string, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
			is_parent = false;
		}
	}
	if (is_parent) {
		TempArena const scratch = scratch_begin(nullptr, 0);
		PLT_ProcessLaunchParams params = {};
		params.working_dir = plt_get_process_info()->binary_dir;
		str8_list_push(scratch.arena, &params.cmd_line, "viewer"_str8);
		str8_list_push(scratch.arena, &params.cmd_line, "--child"_str8);
		plt_process_launch(&params);
		scratch_end(scratch);
	}
}

auto dk::vw_frame() noexcept -> b8 {
	ZoneScoped;
	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event; RGFW_window_checkEvent(vw_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			vw_context->quit = true;
		}
	}
	glClearColor(0.3f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	RGFW_window_swapBuffers_OpenGL(vw_context->window);
	return vw_context->quit;
}

auto dk::vw_shutdown() noexcept -> void {
	ZoneScoped;
	plt_window_close(vw_context->window);
}
