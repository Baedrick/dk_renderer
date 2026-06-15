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
	TempArena const scratch = scratch_begin(nullptr, 0);
	b8 success = true;

	// TODO(Dedrick): Name each object using glObjectLabel.

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
	struct { ShaderModule id; GLenum type; String8 name; GLuint out; }
	shader_module_table[] = {
		{ SHADER_MODULE_HELLO_TRIANGLE_VERT, GL_VERTEX_SHADER,   "hello_triangle.vert"_str8, 0 },
		{ SHADER_MODULE_HELLO_TRIANGLE_FRAG, GL_FRAGMENT_SHADER, "hello_triangle.frag"_str8, 0 },
		{ SHADER_MODULE_DUMMY_COMP,          GL_COMPUTE_SHADER,  "dummy.comp"_str8,          0 },
	};
	static_assert(array_count(shader_module_table) == SHADER_MODULE_COUNT, "Mismatch shader modules count");
	for (u64 m = 0; m < array_count(shader_module_table); ++m) {
		DK_ASSERT(shader_module_table[m].id == m);

		//~ Dedrick: Unpack shader module from pak.
		String8 const shader_name = shader_module_table[m].name;
		PAK_Shader const *pak_shader = pak_shader_from_name(pak, shader_name);
		DK_ASSERT(pak_shader != nullptr);
		u8 const *shader_data = gpu_base + pak_shader->gpu_offset;
		u64 const shader_size = pak_shader->gpu_size;

		//~ Dedrick: Compile shader module.
		shader_module_table[m].out = glCreateShader(shader_module_table[m].type);
		glShaderBinary(1, &shader_module_table[m].out, GL_SHADER_BINARY_FORMAT_SPIR_V, shader_data, static_cast<GLsizei>(shader_size));
		glSpecializeShader(shader_module_table[m].out, "main", 0, nullptr, nullptr);

		//~ Dedrick: Query status and logs.
		//
		// NOTE(Dedrick): Do not stop compiling the rest of the shaders if this
		// module fails to compile. Its more helpful to us if we can get as much
		// logs on all of the errors as possible at once to display.
		//
		GLint compile_status = 0;
		GLint info_log_length = 0;
		glGetShaderiv(shader_module_table[m].out, GL_COMPILE_STATUS, &compile_status);
		glGetShaderiv(shader_module_table[m].out, GL_INFO_LOG_LENGTH, &info_log_length);
		if (compile_status != GL_TRUE) {
			success = false;
		}
		if (info_log_length > 0) {
			String8 err = {};
			err.data = arena_push_array<u8>(scratch.arena, info_log_length + 1);
			err.size = info_log_length;
			glGetShaderInfoLog(
				shader_module_table[m].out,
				info_log_length,
				nullptr,
				reinterpret_cast<char *>(const_cast<u8 *>(err.data))
			);
			DK_LOG_ERRORF("[OpenGL] %.*s\n", DK_STR8_VARG(err));
		}
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
	String8 const shader_name_table[] = {
		"hello_triangle"_str8,
		"dummy"_str8
	};
	static_assert(array_count(shader_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader table count");
	static_assert(array_count(shader_name_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader name table count");
	if (success) {
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

			//~ Dedrick: Query status and logs.
			//
			// NOTE(Dedrick): Do not stop linking the rest of the shaders if this
			// shader fails to link. Its more helpful to us if we can get as much
			// logs on all of the errors as possible at once to display.
			//
			GLint link_status = 0;
			GLint info_log_length = 0;
			glGetProgramiv(shader, GL_LINK_STATUS, &link_status);
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
			if (link_status != GL_TRUE) {
				success = false;
			}
			if (info_log_length > 0) {
				String8 err = {};
				err.data = arena_push_array<u8>(scratch.arena, info_log_length + 1);
				err.size = info_log_length;
				glGetProgramInfoLog(
					shader,
					info_log_length,
					nullptr,
					reinterpret_cast<char *>(const_cast<u8 *>(err.data))
				);
				DK_LOG_ERRORF("[OpenGL] %.*s\n", DK_STR8_VARG(err));
			}
			out_assets->shaders[s] = shader;
		}
	}

	//~ Dedrick: Initialize textures.
	struct OGL_TextureFormat { GLenum fmt; GLenum pixel_fmt; GLenum type; u32 bytes_per_pixel; }
	const ogl_fmt_table[] = {
		{ GL_NONE, GL_NONE, GL_NONE, 0 },
		{ GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, 4 }
	};
	static_assert(array_count(ogl_fmt_table) == PAK_TEXTURE_FORMAT_COUNT, "Mismatch texture format table");
	String8 const texture_name_table[] = {
		"tony_mc_mapface.dds"_str8
	};
	for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
		//~ Dedrick: Unpack texture from pak.
		String8 const tex_name = texture_name_table[t];
		PAK_Texture const *pak_tex = pak_texture_from_name(pak, tex_name);
		DK_ASSERT(pak_tex != nullptr);
		u8 const *tex_base = gpu_base + pak_tex->gpu_offset;

		//~ Dedrick: pak texture format -> ogl texture format
		OGL_TextureFormat const *tex_fmt = &ogl_fmt_table[pak_tex->format];

		//~ Dedrick: pak texture kind -> ogl texture kind
		GLenum tex_kind = GL_TEXTURE_2D;
		switch (pak_tex->kind) {
			case PAK_TEXTURE_KIND_2D: { tex_kind = GL_TEXTURE_2D; } break;
			case PAK_TEXTURE_KIND_3D: { tex_kind = GL_TEXTURE_3D; } break;
		}

		GLuint tex = 0;
		glCreateTextures(tex_kind, 1, &tex);
		glObjectLabel(GL_TEXTURE, tex, static_cast<GLsizei>(tex_name.size), reinterpret_cast<char const *>(tex_name.data));

		if (tex_kind == GL_TEXTURE_2D) {
			glTextureStorage2D(tex, pak_tex->mip_count, tex_fmt->fmt, pak_tex->width, pak_tex->height);

			//~ Dedrick: Upload mip maps.
			u64 offset = 0;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage2D(tex, mip, 0, 0, mip_w, mip_h, tex_fmt->pixel_fmt, tex_fmt->type, tex_base + offset);
				offset += static_cast<u64>(mip_w) * mip_h * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
			}
		}
		else if (tex_kind == GL_TEXTURE_3D) {
			glTextureStorage3D(tex, pak_tex->mip_count, tex_fmt->fmt, pak_tex->width, pak_tex->height, pak_tex->depth);

			//~ Dedrick: Upload mip maps.
			u64 offset = 0;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			u32 mip_d = pak_tex->depth;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage3D(tex, mip, 0, 0, 0, mip_w, mip_h, mip_d, tex_fmt->pixel_fmt, tex_fmt->type, tex_base + offset);
				offset += static_cast<u64>(mip_w) * mip_h * mip_d * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
				mip_d = mip_d > 1 ? (mip_d >> 1) : 1;
			}
		}

		out_assets->textures[t] = tex;
	}

	//~ Dedrick: Clean up intermediate objects.
	for (u64 m = 0; m < array_count(shader_module_table); ++m) {
		glDeleteShader(shader_module_table[m].out);
	}

	//~ Dedrick: Clean up on failure.
	if (!success) {
		for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
			glDeleteProgram(out_assets->shaders[s]);
			out_assets->shaders[s] = 0;
		}
		for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
			glDeleteTextures(1, &out_assets->textures[t]);
			out_assets->textures[t] = 0;
		}
	}
	scratch_end(scratch);
	return success;
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

	//~ Dedrick: Set up console.
	{
		DKR_Console *console = &dkr_context->console;
		console->text_buffer_size = mega_bytes(1);
		console->text_buffer = arena_push_array<u8>(dkr_context->arena, console->text_buffer_size);
		console->max_lines = 4096;
		console->lines = arena_push_array<DKR_ConsoleLine>(dkr_context->arena, console->max_lines);
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
	dkr_context->window = plt_window_open("dk_renderer"_str8, 0, 0, 800, 600, RGFW_windowCenter | RGFW_windowScaleToMonitor);
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
					//~ Dedrick: Open file.
					String8 const pak_path = dkr_asset_pak_path(scratch.arena);
					PLT_Handle const file = plt_file_open(pak_path, PLT_ACCESS_FLAG_READ);
					PLT_Handle const file_map = plt_file_map_open(file, PLT_ACCESS_FLAG_READ);
					PLT_FileAttributes const file_attribs = plt_attributes_from_file(file);
					void *const file_base = plt_file_map_view_open(file_map, PLT_ACCESS_FLAG_READ, 0, file_attribs.size);
					if (file_base != nullptr) {
						PAK_Parsed pak = {};
						DKR_RenderAssets assets = {};
						b8 const loaded =
							pak_parse(static_cast<u8 *>(file_base), file_attribs.size, &pak) &&
							dkr_render_assets_load(&pak, &assets);
						if (loaded) {
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
						plt_file_map_view_close(file_map, file_base, 0, file_attribs.size);
					}
					else {
						DK_LOG_ERRORF("ERROR: failed to map pak file to memory. file missing or locked?\n");
					}
					//~ Dedrick: Close file.
					plt_file_map_close(file_map);
					plt_file_close(file);
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

			if (ImGui::Button("[Debug] Emit 10 logs")) {
				static u64 gen = 0;
				String8 const categories[LOG_KIND_COUNT] = { String8{}, "ERROR:"_str8 };
				String8 const words[] = { "Bumfuzzled"_str8, "Cattywampus"_str8, "Snickersnee"_str8, "Abibliophobia"_str8, "Absquatulate"_str8, "Nincompoop"_str8, "Pauciloquent"_str8 };
				for (u64 n = 0; n < 10; ++n) {
					String8 const category = categories[gen % array_count(categories)];
					String8 const word = words[gen % array_count(words)];
					switch (gen % array_count(categories)) {
						case LOG_KIND_INFO: DK_LOG_INFOF(
							"[%05d] %.*s Hello, current time is %.1f, here's a word: '%.*s'\n",
							ImGui::GetFrameCount(), DK_STR8_VARG(category), ImGui::GetTime(), DK_STR8_VARG(word)
						); break;
						case LOG_KIND_ERROR: DK_LOG_ERRORF(
							"[%05d] %.*s Hello, current time is %.1f, here's a word: '%.*s'\n",
							ImGui::GetFrameCount(), DK_STR8_VARG(category), ImGui::GetTime(), DK_STR8_VARG(word)
						); break;
					}
					gen += 1;
				}
			}

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
					struct { LogKind kind; char const *name; } const filters[] = {
						{ LOG_KIND_INFO, "Info" },
						{ LOG_KIND_ERROR, "Error" }
					};
					for (u32 f = 0; f < array_count(filters); ++f) {
						ImGui::SameLine();
						u32 const bit = 1u << filters[f].kind;
						b8 active = (console->hide_mask & bit) == 0;
						if (ImGui::Checkbox(filters[f].name, &active)) {
							if (active) { console->hide_mask &= ~bit; }
							else { console->hide_mask |= bit; }
						}
					}
					ImGui::Separator();

					//~ Dedrick: Text region.
					if (ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
						u64 const max_possible_visible = console->line_write_pos - console->line_read_pos;
						u64 *visible_indices = arena_push_array<u64>(scratch.arena, max_possible_visible);
						u64 visible_count = 0;
						for (u64 i = console->line_read_pos; i < console->line_write_pos; ++i) {
							DKR_ConsoleLine const *line = &console->lines[i % console->max_lines];
							if ((console->hide_mask & (1u << line->kind)) == 0) {
								visible_indices[visible_count] = i;
								visible_count += 1;
							}
						}

						// NOTE(Dedrick): Push text closer to each other.
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

						ImGuiListClipper clipper = {};
						clipper.Begin(static_cast<int>(visible_count));
						while (clipper.Step()) {
							for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
								u64 const line_idx = visible_indices[i];
								DKR_ConsoleLine const *line = &console->lines[line_idx % console->max_lines];
								u64 const text_idx = line->offset % console->text_buffer_size;
								char const *text_start = reinterpret_cast<char const *>(console->text_buffer + text_idx);
								char const *text_end = text_start + line->size;

								ImVec4 color;
								bool has_color = false;
								switch (line->kind) {
									case LOG_KIND_INFO:  { color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); has_color = true; } break;
									case LOG_KIND_ERROR: { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; } break;
									default: break;
								}
								if (has_color) { ImGui::PushStyleColor(ImGuiCol_Text, color); }
								ImGui::TextUnformatted(text_start, text_end);
								if (has_color) { ImGui::PopStyleColor(); }
							}
						}
						clipper.End();
						ImGui::PopStyleVar();

						if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
							ImGui::SetScrollHereY(1.0f);
						}
					}
					ImGui::EndChild();
				}
				ImGui::End();
			}
		}
		ImGui::End();
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
	glBindVertexArray(rhi_ogl_context->all_purpose_vao);
	glUseProgram(dkr_context->render_assets.shaders[DKR_SHADER_KIND_HELLO_TRIANGLE]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//~ Dedrick: Draw UI.
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	rhi_surface_present(dkr_context->window);

	//~ Dedrick: Bump frame counters.
	dkr_context->frame_index += 1;

	//~ Dedrick: Collect logs.
	{
		ZoneScopedN("collect logs");
		LogFrameResult const log = log_frame_end(scratch.arena);
		if (log.count > 0) {
			// TODO(Dedrick): Append to log file.

			//~ Dedrick: Parse to console.
			DKR_Console *const console = &dkr_context->console;
			u64 text_write = console->text_write_pos;
			u64 line_write = console->line_write_pos;
			u64 line_read  = console->line_read_pos;
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
				line_kind = entry->kind;

				//~ Dedrick: Ensure lines occupy contiguous memory.
				u64 write_idx = text_write % console->text_buffer_size;
				if (write_idx + chunk_size > console->text_buffer_size) {
					if (line_size > 0) {
						if (line_write - line_read >= console->max_lines) {
							line_read += 1;
						}
						DKR_ConsoleLine *line = &console->lines[line_write % console->max_lines];
						line->offset = line_start_offset;
						line->size = line_size;
						line->kind = line_kind;
						line_write += 1;
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
						if (line_write - line_read >= console->max_lines) {
							line_read += 1;
						}
						DKR_ConsoleLine *line = &console->lines[line_write % console->max_lines];
						line->offset = line_start_offset;
						line->size = line_size;
						line->kind = line_kind;
						line_write += 1;
						line_start_offset = text_write;
						line_size = 0;
					}
				}

				//~ Dedrick: Advance console line ring buffer.
				for (; line_read < line_write; ++line_read) {
					DKR_ConsoleLine *oldest_line = &console->lines[line_read % console->max_lines];
					bool const stomped = (text_write - oldest_line->offset) > console->text_buffer_size;
					if (!stomped) {
						break;
					}
				}

				//~ Dedrick: Commit last line.
				if (line_size > 0) {
					if (line_write - line_read >= console->max_lines) {
						line_read += 1;
					}
					DKR_ConsoleLine *line = &console->lines[line_write % console->max_lines];
					line->offset = line_start_offset;
					line->size = line_size;
					line->kind = line_kind;
					line_write += 1;
				}

				//~ Dedrick: Commit buffer positions.
				console->text_write_pos = text_write;
				console->line_write_pos = line_write;
				console->line_read_pos = line_read;
			}
		}
	}
	scratch_end(scratch);
	return dkr_context->quit;
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
	rhi_window_unequip(dkr_context->window);
	plt_window_close(dkr_context->window);
}
