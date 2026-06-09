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

auto dk::dkr_asset_pak_path(Arena *arena) noexcept -> String8 {
	return str8f(arena, "%.*s/dkrend.pak", DK_STR8_VARG(plt_get_process_info()->binary_dir));
}

auto dk::dkr_render_assets_load(PAK_Parsed const *pak, DKR_RenderAssets *out_assets) noexcept -> b8 {
	ZoneScoped;

	//~ Dedrick: Obtain start of gpu binary data of pak.
	PAK_GpuHeader const *gpu_header = pak_element_from_kind_idx<PAK_SECTION_KIND_GPU_HEADER>(pak, 0);
	u8 const *const gpu_base = pak->raw_data + gpu_header->gpu_offset;

	//~ Dedrick: Unpack shader modules and compile them.
	enum ShaderModule : u32 {
		SHADER_MODULE_HELLO_TRIANGLE_VERT,
		SHADER_MODULE_HELLO_TRIANGLE_FRAG,
		SHADER_MODULE_DUMMY_COMP,
		SHADER_MODULE_COUNT
	};
	struct { ShaderModule id; GLenum type; String8 name; GLuint out; String8 errors; }
	shader_module_table[] = {
		{ SHADER_MODULE_HELLO_TRIANGLE_VERT, GL_VERTEX_SHADER,   "hello_triangle.vert"_str8, 0 },
		{ SHADER_MODULE_HELLO_TRIANGLE_FRAG, GL_FRAGMENT_SHADER, "hello_triangle.frag"_str8, 0 },
		{ SHADER_MODULE_DUMMY_COMP,          GL_COMPUTE_SHADER,  "dummy.comp"_str8, 0 },
	};
	static_assert(array_count(shader_module_table) == SHADER_MODULE_COUNT, "Mismatch shader modules count");
	for (u64 m = 0; m < array_count(shader_module_table); ++m) {
		DK_ASSERT(shader_module_table[m].id == m);

		//~ Dedrick: Unpack shader module from pak.
		PAK_Shader const *pak_shader = pak_shader_from_name(pak, shader_module_table[m].name);
		u8 const *shader_data = gpu_base + pak_shader->gpu_offset;
		u64 const shader_size = pak_shader->gpu_size;

		//~ Dedrick: Compile shader module.
		shader_module_table[m].out = glCreateShader(shader_module_table[m].type);
		glShaderBinary(1, &shader_module_table[m].out, GL_SHADER_BINARY_FORMAT_SPIR_V, shader_data, static_cast<GLsizei>(shader_size));
		glSpecializeShader(shader_module_table[m].out, "main", 0, nullptr, nullptr);

		// TODO(Dedrick): Log errors.
		GLint compile_status = 0;
		glGetShaderiv(shader_module_table[m].out, GL_COMPILE_STATUS, &compile_status);
	}

	//~ Dedrick: Link shader programs.
	enum ShaderKind { SHADER_KIND_SURFACE, SHADER_KIND_COMPUTE };
	struct ShaderDesc {
		ShaderKind kind;
		union {
			struct { u32 vs; u32 fs; } surface;
			struct { u32 cs; } compute;
		};
	} const shader_table[] = {
		{ SHADER_KIND_SURFACE, { .surface = { SHADER_MODULE_HELLO_TRIANGLE_VERT, SHADER_MODULE_HELLO_TRIANGLE_FRAG } } },
		{ SHADER_KIND_COMPUTE, { .compute = { SHADER_MODULE_DUMMY_COMP } } }
	};
	static_assert(array_count(shader_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader table count");
	for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
		GLuint const shader = glCreateProgram();
		ShaderDesc const *desc = &shader_table[s];
		switch (desc->kind) {
			case SHADER_KIND_SURFACE: {
				glAttachShader(shader, shader_module_table[desc->surface.vs].out);
				glAttachShader(shader, shader_module_table[desc->surface.fs].out);
				break;
			}
			case SHADER_KIND_COMPUTE: {
				glAttachShader(shader, shader_module_table[desc->compute.cs].out);
				break;
			}
		}
		glLinkProgram(shader);

		// TODO(Dedrick): Log errors.
		GLint link_status = 0;
		glGetProgramiv(shader, GL_LINK_STATUS, &link_status);

		out_assets->shaders[s] = shader;
	}

#if 0
	//~ Dedrick: Initialize textures.
	String8 const texture_name_table[] = {
		"tony_mc_mapface.dds"_str8
	};
	for (u64 kind = 0; kind < DKR_TEXTURE_KIND_COUNT; ++kind) {
		(void)texture_name_table[kind];
	}
#endif

	//~ Dedrick: Clean up intermediate objects.
	for (u64 m = 0; m < array_count(shader_module_table); ++m) {
		glDeleteShader(shader_module_table[m].out);
	}
	return true;
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

	//~ Dedrick: Load pak.
	{
		//~ Dedrick: Open file.
		TempArena const scratch = scratch_begin(nullptr, 0);
		String8 const pak_path = dkr_asset_pak_path(scratch.arena);
		PLT_Handle const file = plt_file_open(pak_path, PLT_ACCESS_FLAG_READ);
		PLT_Handle const file_map = plt_file_map_open(file, PLT_ACCESS_FLAG_READ);
		PLT_FileAttributes const file_attribs = plt_attributes_from_file(file);
		void *const file_base = plt_file_map_view_open(file_map, PLT_ACCESS_FLAG_READ, 0, file_attribs.size);

		//~ Dedrick: Parse pak.
		PAK_Parsed pak = {};
		b8 good = pak_parse(static_cast<u8 *>(file_base), file_attribs.size, &pak);
		if (!good) {
			plt_show_dialog(nullptr, "Fatal Error"_str8, "Invalid pak file; rebuild with `build assets`."_str8, true);
			plt_abort(0);
		}
		good = dkr_render_assets_load(&pak, &dkr_context->render_assets);
		if (!good) {
			plt_show_dialog(nullptr, "Fatal Error"_str8, "Error loading pak assets"_str8, true);
			plt_abort(0);
		}

		//~ Dedrick: Close file.
		plt_file_map_view_close(file_map, file_base, 0, file_attribs.size);
		plt_file_map_close(file_map);
		plt_file_close(file);
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
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui_ImplRgfw_InitForOpenGL(dkr_context->window, true);
	ImGui_ImplOpenGL3_Init();

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
	dkr_context->events[1] = dkr_context->events[0];
	dkr_context->events[0] = {};

	//~ Dedrick: Begin frame scope.
	log_frame_begin();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplRgfw_NewFrame();
	ImGui::NewFrame();

	// TODO(Dedrick): Process asset unload events, process asset load events.
	// TODO(Dedrick): Wait for gpu fences

	//~ Dedrick: Process platform (window) events -> push application events.
	{
		ZoneScopedN("process platform events");
		for (RGFW_event event = {}; RGFW_window_checkEvent(dkr_context->window, &event); ) {
			switch (event.type) {
				case RGFW_windowClose: {
					dkr_push_event_kind(DKR_EVENT_KIND_QUIT);
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
				case DKR_EVENT_KIND_RELOAD_PAK: {
					// TODO(Dedrick): Needs error checking; do NOT crash here!
					// This case needs to be structured better.

					//~ Dedrick: Open file.
					String8 const pak_path = dkr_asset_pak_path(scratch.arena);
					PLT_Handle const file = plt_file_open(pak_path, PLT_ACCESS_FLAG_READ);
					PLT_Handle const file_map = plt_file_map_open(file, PLT_ACCESS_FLAG_READ);
					PLT_FileAttributes const file_attribs = plt_attributes_from_file(file);
					void *const file_base = plt_file_map_view_open(file_map, PLT_ACCESS_FLAG_READ, 0, file_attribs.size);

					//~ Dedrick: Parse pak.
					PAK_Parsed pak = {};
					b8 good = pak_parse(static_cast<u8 *>(file_base), file_attribs.size, &pak);
					if (good) {
						DKR_RenderAssets assets = {};
						good = dkr_render_assets_load(&pak, &assets);
						if (good) {
							DKR_RenderAssets *stale_assets = &dkr_context->render_assets;
							for (u64 k = 0; k < DKR_SHADER_KIND_COUNT; ++k) {
								glDeleteProgram(stale_assets->shaders[k]);
							}
							dkr_context->render_assets = assets;
							DK_LOG_INFOF("pak reloaded");
						}
					}
					if (!good) {
						DK_LOG_ERRORF("ERROR: loading render assets, check logs for reason");
					}

					//~ Dedrick: Close file.
					plt_file_map_view_close(file_map, file_base, 0, file_attribs.size);
					plt_file_map_close(file_map);
					plt_file_close(file);
					break;
				}
			}
		}
	}

	// TODO(Dedrick): Update scene stuff (camera, lights, etc.)

	//~ Dedrick: Build UI.
	{
		ZoneScopedN("build ui");

		//~ Dedrick: Adjust ImGui for HiDPI screens.
		// TODO(Dedrick): Only adjust when moving window because it can be moved to another monitor.
		ImGuiIO &io = ImGui::GetIO();
		float const old_scale = io.FontGlobalScale;
		float const content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(dkr_context->window);
		io.FontGlobalScale = content_scale;
		ImGui::GetStyle().ScaleAllSizes(content_scale / old_scale);

		ImGui::Text("Frame Time: %.5f", dkr_context->frame_dt);

		if (ImGui::Button("Reload Pak")) {
			dkr_push_event_kind(DKR_EVENT_KIND_RELOAD_PAK);
		}

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
	glUseProgram(dkr_context->render_assets.shaders[DKR_SHADER_KIND_HELLO_TRIANGLE]);
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
