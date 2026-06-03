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

	//~ Dedrick: Pick target frame time.
	// TODO(Dedrick): Query vsync when window is moved.
	{
		f32 target_refresh_rate = 60.0f;
		RGFW_monitor const *monitor = RGFW_window_getMonitor(dkr_context->window);
		if (monitor->mode.refreshRate > 0.0f) {
			target_refresh_rate = monitor->mode.refreshRate;
		}
		dkr_context->target_frame_time_us = static_cast<s64>(1e+6f / target_refresh_rate);
		dkr_context->vsync_max_error_us = 200; // 0.2ms
		for (u32 i = 0; i < array_count(dkr_context->snap_frequencies); ++i) {
			dkr_context->snap_frequencies[i] = dkr_context->target_frame_time_us * (i + 1);
		}
		for (u32 i = 0; i < array_count(dkr_context->time_averager); ++i) {
			dkr_context->time_averager[i] = dkr_context->target_frame_time_us;
		}
		dkr_context->time_averager_residual = 0;
	}

	//~ Dedrick: Initialize ImGui.
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplRgfw_InitForOpenGL(dkr_context->window, true);
		ImGui_ImplOpenGL3_Init();
	}

	//~ Dedrick: Begin measuring actual per-frame work.
	dkr_context->last_frame_time_us = plt_now_microseconds();
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	TempArena const scratch = scratch_begin(nullptr, 0);

	//~ Dedrick: Determine frame time.
	// https://medium.com/@tglaiel/how-to-make-your-game-run-at-60fps-24c61210fe75
	{
		u64 const current_time_us = plt_now_microseconds();
		s64 delta_time = static_cast<s64>(current_time_us - dkr_context->last_frame_time_us);
		dkr_context->last_frame_time_us = current_time_us;
		if (delta_time > dkr_context->target_frame_time_us * 8) {
			delta_time = dkr_context->target_frame_time_us;
		}
		if (delta_time < 0) {
			delta_time = 0;
		}
		for (u64 i = 0; i < array_count(dkr_context->snap_frequencies); ++i) {
			s64 const snap = dkr_context->snap_frequencies[i];
			if (abs(delta_time - snap) < dkr_context->vsync_max_error_us) {
				delta_time = snap;
				break;
			}
		}
		u64 const history_count = array_count(dkr_context->time_averager);
		dkr_context->time_averager[dkr_context->time_averager_head] = delta_time;
		dkr_context->time_averager_head = (dkr_context->time_averager_head + 1) % history_count;

		u64 averager_sum = 0;
		for (u32 i = 0; i < array_count(dkr_context->time_averager); ++i) {
			averager_sum += dkr_context->time_averager[i];
		}

		delta_time = averager_sum / history_count;
		dkr_context->time_averager_residual += averager_sum % history_count;
		delta_time += dkr_context->time_averager_residual / history_count;
		dkr_context->time_averager_residual %= history_count;

		f32 const frame_dt = static_cast<f32>(delta_time) / 1000000.0f;
		dkr_context->frame_dt = frame_dt;
		dkr_context->time_in_seconds += frame_dt;
	}

	//~ Dedrick: Do per-frame resets.
	arena_clear(dkr_frame_arena());

	//~ Dedrick: Begin frame scope.
	log_frame_begin();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplRgfw_NewFrame();
	ImGui::NewFrame();

	// TODO(Dedrick): Process platform (window) events -> push application events.
	for (RGFW_event event = {}; RGFW_window_checkEvent(dkr_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			dkr_context->quit = true;
		}
	}

	// TODO(Dedrick): Process application events -> push asset system events.

	// TODO(Dedrick): Process asset unload events, process asset load events.

	// TODO(Dedrick): Update scene stuff (camera, lights, etc.)
	DK_LOG_INFOF("frame_dt: %.4f\n", dkr_context->frame_dt);

	//~ Dedrick: Build UI.
	{
		ZoneScopedN("build ui");
		ImGui::ShowDemoWindow();

		//~ Dedrick: Adjust ImGui for HiDPI screens.
		// TODO(Dedrick): Only adjust when moving window.
		ImGuiIO &io = ImGui::GetIO();
		float const old_scale = io.FontGlobalScale;
		float const content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(dkr_context->window);
		io.FontGlobalScale = content_scale;
		ImGui::GetStyle().ScaleAllSizes(content_scale / old_scale);

		ImGui::Render();
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
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	rhi_surface_present(dkr_context->window);

	//~ Dedrick: Bump frame counters.
	dkr_context->frame_index += 1;

	//~ Dedrick: Collect logs.
	{
		ZoneScopedN("collect logs");
		LogFrameResult const log = log_frame_end(scratch.arena);
		// TODO(Dedrick): Log to console buffer.
		(void)log;
		if (log.list.count > 0) {
			for (LogEntry *node = log.list.first; node != nullptr; node = node->next) {
				std::fprintf(stdout, "%.*s", DK_STR8_VARG(node->string));
			}
		}
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
