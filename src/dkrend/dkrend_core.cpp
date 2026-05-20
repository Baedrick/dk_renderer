// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_init(CmdLine *cmd_line) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	dkr_context = arena_push<DKR_Context>(arena);
	dkr_context->arena = arena;
	dkr_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter);
	rhi_window_equip(dkr_context->window);
	(void)cmd_line;
	#if 0
	if (!cmd_line_has_flag(cmd_line, "child"_str8)) {
		TempArena const scratch = scratch_begin(nullptr, 0);
		PLT_ProcessLaunchParams params = {};
		params.working_dir = plt_get_process_info()->binary_dir;
		str8_list_push(scratch.arena, &params.cmd_line, "dkrend"_str8);
		str8_list_push(scratch.arena, &params.cmd_line, "--child"_str8);
		plt_process_launch(&params);
		scratch_end(scratch);
	}
	#endif
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event; RGFW_window_checkEvent(dkr_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			dkr_context->quit = true;
		}
	}
	rhi_surface_current_texture(dkr_context->window);
	glViewport(0, 0, 800, 600);
	glClearColor(0.3f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(rhi_ogl_context->all_purpose_vao);
	glUseProgram(rhi_ogl_context->shader);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	rhi_surface_present(dkr_context->window);
	return dkr_context->quit;
}

auto dk::dkr_shutdown() noexcept -> void {
	ZoneScoped;
	rhi_window_unequip(dkr_context->window);
	plt_window_close(dkr_context->window);
}
