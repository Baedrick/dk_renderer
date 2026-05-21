// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_init(CmdLine *cmd_line) noexcept -> void {
	ZoneScoped;
	(void)cmd_line;

	Arena *arena = arena_alloc();
	dkr_context = arena_push<DKR_Context>(arena);
	dkr_context->arena = arena;
	dkr_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter);
	rhi_window_equip(dkr_context->window);

	dkr_context->log = log_alloc();
	log_select(dkr_context->log);
	{
		// TODO(Dedrick): Obtain user data folder path and log there.
		String8 const user_data_folder = "log"_str8;
		dkr_context->log_path = str8f(dkr_context->arena, "%.*s/dkrend.log", DK_STR8_VARG(user_data_folder));
		plt_make_directory(dkr_context->log_path);
	}
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	TempArena const scratch = scratch_begin(nullptr, 0);
	log_frame_begin();

	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event; RGFW_window_checkEvent(dkr_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			dkr_context->quit = true;
		}
	}
	{
		s32 width = 0, height = 0;
		RGFW_window_getSizeInPixels(dkr_context->window, &width, &height);
		glViewport(0, 0, width, height);
	}
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(rhi_ogl_context->all_purpose_vao);
	glUseProgram(rhi_ogl_context->shader);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	rhi_surface_present(dkr_context->window);

	{
		ZoneScopedN("collect logs");
		LogFrameResult const log = log_frame_end(scratch.arena);
		// TODO(Dedrick): Log to console buffer.
		(void)log;
	}
	scratch_end(scratch);
	return dkr_context->quit;
}

auto dk::dkr_shutdown() noexcept -> void {
	ZoneScoped;
	log_release(dkr_context->log);
	rhi_window_unequip(dkr_context->window);
	plt_window_close(dkr_context->window);
}
