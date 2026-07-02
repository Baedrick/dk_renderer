// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_frame_arena() noexcept -> Arena * {
	return dkr_context->frame_arenas[dkr_context->frame_index % array_count(dkr_context->frame_arenas)];
}

auto dk::dkr_event_list_push(Arena *arena, DKR_EventList *events, DKR_Event const *event) noexcept -> void {
	DKR_EventNode *node = arena_push<DKR_EventNode>(arena);
	node->event.kind = event->kind;
	// NOTE(Dedrick): Attach payload, if required.
	switch (event->kind) {
		case DKR_EVENT_KIND_RELOAD_PAK : {
			node->event.reload_pak.file_path = str8_copy(arena, event->reload_pak.file_path);
			break;
		}
	}
	list_push_back(&events->first, &events->last, node);
	events->count += 1;
}

auto dk::dkr_push_event(DKR_Event const *event) noexcept -> void {
	dkr_event_list_push(dkr_frame_arena(), &dkr_context->events[0], event);
}

auto dk::dkr_push_event_kind(DKR_EventKind kind) noexcept -> void {
	DKR_Event const event = { kind };
	dkr_event_list_push(dkr_frame_arena(), &dkr_context->events[0], &event);
}

auto dk::dkr_next_event(DKR_Event **event) noexcept -> b8 {
	DKR_EventNode *node = dkr_context->events[1].first;
	if (*event != nullptr) {
		node = DK_CAST_FROM_MEMBER(DKR_EventNode, event, *event);
		node = node->next;
	}
	*event = nullptr;
	if (node != nullptr) {
		*event = &node->event;
	}
	return *event != nullptr;
}

auto dk::dkr_console_commit_line(DKR_Console *console, u64 offset, u32 size, LogKind kind) noexcept -> void {
	if (console->line_write_pos - console->line_read_pos >= console->max_lines) {
		console->line_read_pos += 1;
	}
	DKR_ConsoleLine *line = &console->lines[console->line_write_pos % console->max_lines];
	line->offset = offset;
	line->size = size;
	line->kind = kind;
	console->line_write_pos += 1;
}

auto dk::dkr_target_frame_time_update(RGFW_monitor const *monitor) noexcept -> void {
	f32 target_refresh_rate = 60.0f;
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
		String8 const user_program_config_dir = get_process_info()->user_program_config_dir;
		String8 const user_config_folder = str8f(scratch.arena, "%.*s/dkrend", DK_STR8_VARG(user_program_config_dir));
		dkr_context->log_path = str8f(dkr_context->arena, "%.*s/dkrend.log", DK_STR8_VARG(user_config_folder));
		make_directory(user_config_folder);
		write_bytes_to_file_path(dkr_context->log_path, Buffer{});
		scratch_end(scratch);
	}

	//~ Dedrick: Set up console.
	{
		DKR_Console *console = &dkr_context->console;
		console->text_buffer_size = mega_bytes(1);
		console->text_buffer = arena_push_array<u8>(dkr_context->arena, console->text_buffer_size);
		console->max_lines = 4096;
		console->lines = arena_push_array<DKR_ConsoleLine>(dkr_context->arena, console->max_lines);
	}

	//~ Dedrick: Set up staging buffer.
	{
		GLuint stage_buffer = 0;
		GLbitfield const stage_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		u64 const stage_size = mega_bytes(128);
		glCreateBuffers(1, &stage_buffer);
		glNamedBufferStorage(stage_buffer, static_cast<GLsizeiptr>(stage_size), nullptr, stage_flags);
		void *const base = glMapNamedBufferRange(stage_buffer, 0, stage_size, stage_flags | GL_MAP_INVALIDATE_BUFFER_BIT);
		dkr_context->render.stage_buffer = stage_buffer;
		dkr_context->render.stage_arena = gpu_make_arena(dkr_context->arena, base, stage_size);
	}

	//~ Dedrick: Load pak, initialize assets.
	{
		TempArena const scratch = scratch_begin(nullptr, 0);
		String8 const pak_path = dkr_pak_path(scratch.arena);
		File const file = file_open(pak_path, FILE_ACCESS_FLAG_READ);

		PAK_Parsed pak = {};
		b8 good = dkr_pak_read_metadata(scratch.arena, file, &pak);
		if (!good) {
			dt_show_dialog(nullptr, "Fatal Error"_str8, "Invalid pak file; rebuild with `build assets`."_str8, true);
			abort_self(0);
		}
		good = dkr_render_assets_load(file, &pak, &dkr_context->render_assets);
		if (!good) {
			dt_show_dialog(nullptr, "Fatal Error"_str8, "Error loading pak assets"_str8, true);
			abort_self(0);
		}

		file_close(file);
		scratch_end(scratch);
	}

	//~ Dedrick: Set up main window.
	dkr_context->window = dt_window_open("dk_renderer"_str8, 0, 0, 800, 600, RGFW_windowCenter | RGFW_windowScaleToMonitor);
	dkr_context->monitor = RGFW_window_getMonitor(dkr_context->window);
	dkr_target_frame_time_update(dkr_context->monitor);
	ogl_window_equip(dkr_context->window);

	//~ Dedrick: Initialize ImGui.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui_ImplRgfw_InitForOpenGL(dkr_context->window, true);
	ImGui_ImplOpenGL3_Init();

	//~ Dedrick: Begin measuring actual per-frame work.
	dkr_context->last_frame_time_us = now_time_us();
}

auto dk::dkr_shutdown() noexcept -> void {
	ZoneScoped;
	// NOTE(Dedrick): We intentionally skip freeing memory arenas and internal
	// resources for a faster shutdown. The OS will bulk-reclaim the process
	// memory. We only clean up resources that require graceful termination.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplRgfw_Shutdown();
	ImGui::DestroyContext();
	log_release(dkr_context->log);
	ogl_window_unequip(dkr_context->window);
	dt_window_close(dkr_context->window);
}

auto dk::dkr_frame() noexcept -> b8 {
	ZoneScoped;
	TempArena const scratch = scratch_begin(nullptr, 0);

	//~ Dedrick: Determine frame time.
	// https://medium.com/@tglaiel/how-to-make-your-game-run-at-60fps-24c61210fe75
	{
		u64 const current_time_us = now_time_us();
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
	dkr_context->events[1] = dkr_context->events[0];
	dkr_context->events[0] = {};

	//~ Dedrick: Begin log frame scope.
	log_frame_begin();

	// TODO(Dedrick): Wait for gpu fences
	// TODO(Dedrick): Process asset unload events (defragment here?)
	// TODO(Dedrick): Process asset load events

	//~ Dedrick: Process platform (window) events -> push application events.
	{
		ZoneScopedN("process platform events");
		for (RGFW_event event = {}; RGFW_window_checkEvent(dkr_context->window, &event); ) {
			switch (event.type) {
				case RGFW_windowClose: {
					dkr_push_event_kind(DKR_EVENT_KIND_QUIT);
					break;
				}
				case RGFW_windowMoved: {
					RGFW_monitor const *monitor = RGFW_window_getMonitor(dkr_context->window);
					if (monitor != dkr_context->monitor) {
						dkr_context->monitor = monitor;
						dkr_push_event_kind(DKR_EVENT_KIND_UPDATE_TARGET_FRAME_RATE);
					}
					break;
				}
			}
		}
	}

	//~ Dedrick: Process application events.
	{
		ZoneScopedN("process application events");
		for (DKR_Event *event = nullptr; dkr_next_event(&event); ) {
			switch (event->kind) {
				case DKR_EVENT_KIND_QUIT: {
					dkr_context->quit = true;
					break;
				}
				case DKR_EVENT_KIND_UPDATE_TARGET_FRAME_RATE: {
					dkr_target_frame_time_update(dkr_context->monitor);
					break;
				}
				case DKR_EVENT_KIND_RELOAD_PAK: {
					String8 const pak_path = dkr_pak_path(scratch.arena);
					File const file = file_open(pak_path, FILE_ACCESS_FLAG_READ);
					if (is_valid(file)) {
						PAK_Parsed pak = {};
						DKR_RenderAssets assets = {};
						b8 const good =
							dkr_pak_read_metadata(scratch.arena, file, &pak) &&
							dkr_render_assets_load(file, &pak, &assets);
						if (good) {
							//~ Dedrick: Clean up existing render assets.
							DKR_RenderAssets *const stale_assets = &dkr_context->render_assets;
							for (u64 k = 0; k < DKR_SHADER_KIND_COUNT; ++k) {
								glDeleteProgram(stale_assets->shaders[k]);
							}
							for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
								glDeleteTextures(1, &stale_assets->textures[t]);
							}

							//~ Dedrick: Commit new render assets.
							dkr_context->render_assets = assets;
							DK_LOG_INFOF("render assets reloaded\n");
						}
						else {
							DK_LOG_ERRORF("ERROR: failed to parse or load render assets from pak\n");
						}
						file_close(file);
					}
					else {
						DK_LOG_ERRORF("ERROR: failed to map pak file to memory. file missing or locked?\n");
					}
					break;
				}
				case DKR_EVENT_KIND_OPEN_CONSOLE : {
					dkr_context->console_is_open = true;
					break;
				}
			}
		}
	}

	// TODO(Dedrick): Update scene stuff (camera, lights, etc.)

	//~ Dedrick: Build UI.
	{
		ZoneScopedN("build ui");

		//~ Dedrick: Begin ImGui frame scope.
		//
		// NOTE(Dedrick): Override the backend's raw delta time with our snapped
		// engine time. This prevents micro-stutter in UI animations and keeps ImGui
		// timers in sync.
		//
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplRgfw_NewFrame();
		ImGui::GetIO().DeltaTime = dkr_context->frame_dt;
		ImGui::NewFrame();

		ImGuiIO &io = ImGui::GetIO();

		//~ Dedrick: Adjust ImGui for HiDPI screens.
		// https://github.com/ocornut/imgui/discussions/3925
		//
		// NOTE(Dedrick): RGFW uses a small 32-slot event buffer. Moving a window
		// across monitor boundaries floods the queue and overflows this buffer.
		// Instead of increasing the buffer size to 256 for a single edge case,
		// we poll the window's content scale every frame.
		//
		f32 const old_scale = io.FontGlobalScale;
		f32 const content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(dkr_context->window);
		if (abs(content_scale - old_scale) > 1e-5f) {
			f32 const scale_factor = content_scale / old_scale;
			io.FontGlobalScale = content_scale;
			ImGui::GetStyle().ScaleAllSizes(scale_factor);
		}

		if (ImGui::Begin("dkrend - Rendering Engine")) {
			ImGui::Text("Frame Time: %.5f", dkr_context->frame_dt);

			// TODO(Dedrick): Buttons to open "editors".
			// Scene editor
			// Post process
			// Global illumination
			// Camera
			// Developer options

			if (ImGui::Button("Console")) {
				dkr_push_event_kind(DKR_EVENT_KIND_OPEN_CONSOLE);
			}

			if (ImGui::Button("Reload Pak")) {
				dkr_push_event_kind(DKR_EVENT_KIND_RELOAD_PAK);
			}

			// TODO(Dedrick): Remove
			if (ImGui::Button("[Debug] Emit 10 logs")) {
				static u64 gen = 0;
				String8 const categories[LOG_KIND_COUNT] = { String8{}, "ERROR: "_str8 };
				String8 const words[] = { "Bumfuzzled"_str8, "Cattywampus"_str8, "Snickersnee"_str8, "Abibliophobia"_str8, "Absquatulate"_str8, "Nincompoop"_str8, "Pauciloquent"_str8 };
				for (u64 n = 0; n < 10; ++n) {
					String8 const category = categories[gen % array_count(categories)];
					String8 const word = words[gen % array_count(words)];
					switch (gen % array_count(categories)) {
						case LOG_KIND_INFO: DK_LOG_INFOF(
							"[%05d] %.*sHello, current time is %.1f, here's a word: '%.*s'\n",
							ImGui::GetFrameCount(), DK_STR8_VARG(category), ImGui::GetTime(), DK_STR8_VARG(word)
						); break;
						case LOG_KIND_ERROR: DK_LOG_ERRORF(
							"[%05d] %.*sHello, current time is %.1f, here's a word: '%.*s'\n",
							ImGui::GetFrameCount(), DK_STR8_VARG(category), ImGui::GetTime(), DK_STR8_VARG(word)
						); break;
					}
					gen += 1;
				}
			}
		}
		ImGui::End();

		//~ Dedrick: @ui_console Console Widget.
		if (dkr_context->console_is_open) {
			ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Console", &dkr_context->console_is_open)) {
				DKR_Console *const console = &dkr_context->console;

				//~ Dedrick: Toolbar.
				if (ImGui::Button("Clear")) {
					console->line_read_pos = console->line_write_pos;
				}
				ImGui::SameLine();
				ImGui::TextUnformatted("Filters:");
				struct { LogKind kind; char const *name; }
				const filters[] = {
					{ LOG_KIND_INFO, "Info" },
					{ LOG_KIND_ERROR, "Error" }
				};
				for (u32 f = 0; f < array_count(filters); ++f) {
					ImGui::SameLine();
					u32 const filter_bit = 1u << filters[f].kind;
					b8 filter_active = (console->hide_mask & filter_bit) == 0;
					if (ImGui::Checkbox(filters[f].name, &filter_active)) {
						if (filter_active) {
							console->hide_mask &= ~filter_bit;
						}
						else {
							console->hide_mask |= filter_bit;
						}
					}
				}
				ImGui::Separator();

				//~ Dedrick: Text region.
				if (ImGui::BeginChild("TextRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
					//~ Dedrick: Filter lines to show.
					u64 const max_possible_visible = console->line_write_pos - console->line_read_pos;
					u64 *visible_indices = arena_push_array<u64>(scratch.arena, max_possible_visible);
					u64 visible_count = 0;
					for (u64 visible_idx = console->line_read_pos; visible_idx < console->line_write_pos; ++visible_idx) {
						DKR_ConsoleLine const *line = &console->lines[visible_idx % console->max_lines];
						if ((console->hide_mask & (1u << line->kind)) == 0) {
							visible_indices[visible_count] = visible_idx;
							visible_count += 1;
						}
					}

					//~ Dedrick: Clip and draw lines.
					ImGuiListClipper clipper = {};
					clipper.Begin(static_cast<int>(visible_count));
					while (clipper.Step()) {
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
							//~ Dedrick: Clip line.
							u64 const line_idx = visible_indices[i];
							DKR_ConsoleLine const *line = &console->lines[line_idx % console->max_lines];
							u64 const text_idx = line->offset % console->text_buffer_size;
							char const *text_start = reinterpret_cast<char const *>(console->text_buffer + text_idx);
							char const *text_end = text_start + line->size;

							//~ Dedrick: Draw line.
							ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Text];
							switch (line->kind) {
								case LOG_KIND_ERROR: { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); } break;
							}
							ImGui::PushStyleColor(ImGuiCol_Text, color);
							ImGui::TextUnformatted(text_start, text_end);
							ImGui::PopStyleColor();
						}
					}
					clipper.End();

					//~ Dedrick: Autoscroll at bottom.
					if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
						ImGui::SetScrollHereY(1.0f);
					}
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}

		//~ Dedrick: Build UI draw list.
		ImGui::Render();
	}

	//~ Dedrick: Draw Scene.
	{
		s32 width = 0, height = 0;
		RGFW_window_getSizeInPixels(dkr_context->window, &width, &height);
		glViewport(0, 0, width, height);
	}
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(ogl_context->all_purpose_vao);
	glUseProgram(dkr_context->render_assets.shaders[DKR_SHADER_KIND_HELLO_TRIANGLE]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//~ Dedrick: Draw UI.
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	RGFW_window_swapBuffers_OpenGL(dkr_context->window);

	//~ Dedrick: Bump frame counters.
	dkr_context->frame_index += 1;

	//~ Dedrick: Collect logs.
	{
		ZoneScopedN("collect logs");
		LogFrameResult const log = log_frame_end(scratch.arena);
		if (log.count > 0) {
			append_string_to_file_path(dkr_context->log_path, log.string);

			//~ Dedrick: Parse to console.
			DKR_Console *const console = &dkr_context->console;
			u64 text_write = console->text_write_pos;
			u64 line_start_offset = console->text_write_pos;
			u32 line_size = 0;
			LogKind line_kind = log.entries[0].kind;
			for (u64 log_idx = 0; log_idx < log.count; ++log_idx) {
				LogEntry const *entry = &log.entries[log_idx];
				u8 const *chunk = log.string.data + entry->offset;
				u32 const chunk_size = entry->size;
				if (chunk_size == 0) {
					continue;
				}

				//~ Dedrick: Flush pending line when log kind changes.
				if (line_size > 0 && line_kind != entry->kind) {
					dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
					line_start_offset = text_write;
					line_size = 0;
				}
				line_kind = entry->kind;

				//~ Dedrick: Ensure lines occupy contiguous memory.
				u64 write_idx = text_write % console->text_buffer_size;
				if (write_idx + chunk_size > console->text_buffer_size) {
					if (line_size > 0) {
						dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
						line_size = 0;
					}
					u64 const skip_amount = console->text_buffer_size - write_idx;
					text_write += skip_amount;
					line_start_offset = text_write;
					write_idx = 0;
				}

				//~ Dedrick: Copy string chunk into console ring buffer.
				DK_ASSERT(chunk_size <= console->text_buffer_size);
				std::memcpy(console->text_buffer + write_idx, chunk, chunk_size);

				//~ Dedrick: Split new lines and push to console line buffer.
				for (u32 i = 0; i < chunk_size; ++i) {
					text_write += 1;
					line_size += 1;
					if (chunk[i] == '\n') {
						dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
						line_start_offset = text_write;
						line_size = 0;
					}
				}

				//~ Dedrick: Advance console line ring buffer.
				for (; console->line_read_pos < console->line_write_pos; ++console->line_read_pos) {
					DKR_ConsoleLine *line = &console->lines[console->line_read_pos % console->max_lines];
					b8 const stomped = (text_write - line->offset) > console->text_buffer_size;
					if (!stomped) {
						break;
					}
				}

				//~ Dedrick: Commit last line.
				if (line_size > 0) {
					dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
				}

				//~ Dedrick: Commit buffer positions.
				console->text_write_pos = text_write;
			}
		}
	}
	scratch_end(scratch);
	return dkr_context->quit;
}
