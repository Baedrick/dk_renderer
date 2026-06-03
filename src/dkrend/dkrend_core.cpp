// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_frame_arena() noexcept -> Arena * {
	return dkr_context->frame_arenas[dkr_context->frame_index % array_count(dkr_context->frame_arenas)];
}

auto dk::dkr_init(CmdLine *cmd_line) noexcept -> void {
	ZoneScoped;
	(void)cmd_line;

	//~ Dedrick: Set up state.
	Arena *arena = arena_alloc();
	dkr_context = arena_push<DKR_Context>(arena);
	dkr_context->arena = arena;
	for (u32 i = 0; i < array_count(dkr_context->frame_arenas); ++i) {
		dkr_context->frame_arenas[i] = arena_alloc();
	}
	dkr_context->log = log_alloc();
	log_select(dkr_context->log);
	{
		TempArena const scratch = scratch_begin(nullptr, 0);
		String8 const user_program_data_dir = plt_get_process_info()->user_program_data_dir;
		String8 const user_data_folder = str8f(scratch.arena, "%.*s/dkrend", DK_STR8_VARG(user_program_data_dir));
		dkr_context->log_path = str8f(dkr_context->arena, "%.*s/dkrend.log", DK_STR8_VARG(user_data_folder));
		plt_make_directory(user_data_folder);
		plt_write_bytes_to_file_path(dkr_context->log_path, Buffer8{});
		scratch_end(scratch);
	}

	//~ Dedrick: Set up main window.
	dkr_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter | RGFW_windowScaleToMonitor);
	rhi_window_equip(dkr_context->window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplRgfw_InitForOpenGL(dkr_context->window, true);
	ImGui_ImplOpenGL3_Init();
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	TempArena const scratch = scratch_begin(nullptr, 0);
	log_frame_begin();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplRgfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();

	//~ Dedrick: Do per-frame resets.
	arena_clear(dkr_frame_arena());

	//~ Dedrick: Begin measuring actual per-frame work.
	u64 const begin_time_us = plt_now_microseconds();

	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event = {}; RGFW_window_checkEvent(dkr_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			dkr_context->quit = true;
		}
	}

	ImGuiIO &io = ImGui::GetIO();
	//~ Dedrick: Adjust ImGui for HiDPI screens.
	{
		float const old_scale = io.FontGlobalScale;
		float const content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(dkr_context->window);
		io.FontGlobalScale = content_scale;
		ImGui::GetStyle().ScaleAllSizes(content_scale / old_scale);
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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	rhi_surface_present(dkr_context->window);

	//~ Dedrick: Determine frame time.
	u64 const end_time_us = plt_now_microseconds();
	u64 const frame_time_us = end_time_us - begin_time_us;
	f32 const frame_dt = min(static_cast<f32>(frame_time_us) / 1000000.0f, 0.1f);
	dkr_context->frame_dt = frame_dt;
	dkr_context->time_in_seconds += frame_dt;

	//~ Dedrick: Bump frame counters.
	dkr_context->frame_index += 1;

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
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplRgfw_Shutdown();
	ImGui::DestroyContext();
	log_release(dkr_context->log);
	rhi_window_unequip(dkr_context->window);
	plt_window_close(dkr_context->window);
}
