// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_init(String8List args) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	dkr_context = arena_push<DKR_Context>(arena);
	dkr_context->arena = arena;
	dkr_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter);
	rhi_window_hook(dkr_context->window);
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
		str8_list_push(scratch.arena, &params.cmd_line, "dkrend"_str8);
		str8_list_push(scratch.arena, &params.cmd_line, "--child"_str8);
		plt_process_launch(&params);
		scratch_end(scratch);
	}
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event; RGFW_window_checkEvent(dkr_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			dkr_context->quit = true;
		}
	}
	glClearColor(0.3f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	RGFW_window_swapBuffers_OpenGL(dkr_context->window);
	return dkr_context->quit;
}

auto dk::dkr_shutdown() noexcept -> void {
	ZoneScoped;
	plt_window_close(dkr_context->window);
}
