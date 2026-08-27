// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::DKR_Context *dk::dkr_context;

auto dk::dkr_frame_arena() noexcept -> Arena * {
	return dkr_context->frame_arenas[dkr_context->frame_index % array_count(dkr_context->frame_arenas)];
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
	dkr_context->vsync_max_error_us = 200;
	for (u32 i = 0; i < array_count(dkr_context->snap_frequencies); ++i) {
		dkr_context->snap_frequencies[i] = dkr_context->target_frame_time_us * (i + 1);
	}
	for (u32 i = 0; i < array_count(dkr_context->time_averager); ++i) {
		dkr_context->time_averager[i] = dkr_context->target_frame_time_us;
	}
	dkr_context->time_averager_residual = 0;
}

auto dk::dkr_pak_path(Arena *arena) noexcept -> String8 {
	return str8f(arena, "%.*s/dkrend.pak", DK_STR8_VARG(get_process_info()->binary_dir));
}

auto dk::dkr_pak_open(String8 path, DKR_PakOpen *out) noexcept -> b8 {
	*out = {};
	out->file = file_open(path, FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_SHARE_READ);
	if (!is_valid(out->file)) {
		return false;
	}
	PAK_Header header_maybe = {};
	file_read(out->file, 0, sizeof(PAK_Header), &header_maybe);
	if (header_maybe.magic != PAK_MAGIC_CONSTANT || header_maybe.version != PAK_VERSION) {
		dkr_pak_close(out);
		return false;
	}
	out->metadata_size = header_maybe.metadata_size;
	u64 const file_size = attributes_from_file(out->file).size;
	if (file_size < out->metadata_size) {
		dkr_pak_close(out);
		return false;
	}
	out->map = file_map_open(out->file, FILE_ACCESS_FLAG_READ);
	out->metadata_view = file_map_view_open(out->map, FILE_ACCESS_FLAG_READ, 0, out->metadata_size);
	Buffer const bytes = buf(out->metadata_view, out->metadata_size);
	if (!pak_parse(bytes, &out->parsed)) {
		dkr_pak_close(out);
		return false;
	}
	return true;
}

auto dk::dkr_pak_close(DKR_PakOpen *open) noexcept -> void {
	if (open->metadata_view != nullptr) {
		file_map_view_close(open->map, open->metadata_view, 0, open->metadata_size);
		open->metadata_view = nullptr;
	}
	if (open->map.v != 0) {
		file_map_close(open->map);
		open->map = {};
	}
	if (is_valid(open->file)) {
		file_close(open->file);
		open->file = {};
	}
}

auto dk::dkr_pak_load_shaders(File file, PAK_Parsed const *pak, DKR_RenderAssets *out_assets) noexcept -> b8 {
	TempArena const scratch = scratch_begin(nullptr, 0);
	b8 success = true;

	u64 constexpr chunk_size = mega_bytes(4);
	u8 *const buffer = arena_push_array<u8>(scratch.arena, chunk_size);

	enum ShaderModule : u32 {
		SHADER_MODULE_HELLO_TRIANGLE_VERT,
		SHADER_MODULE_HELLO_TRIANGLE_FRAG,
		SHADER_MODULE_DUMMY_COMP,
		SHADER_MODULE_COUNT
	};
	struct { GLenum stage; String8 name; } const shader_module_table[] = {
		{ GL_VERTEX_SHADER,   "hello_triangle.vert"_str8 },
		{ GL_FRAGMENT_SHADER, "hello_triangle.frag"_str8 },
		{ GL_COMPUTE_SHADER,  "dummy.comp"_str8 },
	};
	static_assert(array_count(shader_module_table) == SHADER_MODULE_COUNT, "Mismatch shader modules count");
	struct { String8 name; u32 count; ShaderModule modules[2]; } const shader_table[] = {
		{ "hello_triangle"_str8, 2, { SHADER_MODULE_HELLO_TRIANGLE_VERT, SHADER_MODULE_HELLO_TRIANGLE_FRAG } },
		{ "dummy"_str8,          1, { SHADER_MODULE_DUMMY_COMP } },
	};
	static_assert(array_count(shader_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader count");
	u64 const shader_data_offset = pak->sections[PAK_SECTION_KIND_GPU_SHADER_DATA].offset;

	GLuint shader_modules[SHADER_MODULE_COUNT] = {};
	for (u64 m = 0; m < SHADER_MODULE_COUNT; ++m) {
		String8 const name = shader_module_table[m].name;
		PAK_Shader const *pak_shader = pak_shader_from_name(pak, name);
		DK_ASSERT(pak_shader != nullptr);
		DK_ASSERT(pak_shader->size <= chunk_size);
		file_read(
			file,
			shader_data_offset + pak_shader->offset,
			shader_data_offset + pak_shader->offset + pak_shader->size,
			buffer
		);
		Buffer const binary = buf(buffer, pak_shader->size);
		shader_modules[m] = ogl_shader_stage_compile(shader_module_table[m].stage, binary, name);

		// NOTE(Dedrick): Do not stop compiling the rest of the shaders if this
		// module fails to compile. Its more helpful to us if we can get as much
		// logs on all of the errors as possible at once to display.
		if (shader_modules[m] == 0) {
			success = false;
		}
	}

	if (success) {
		for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
			String8 const name = shader_table[s].name;
			u32 const count = shader_table[s].count;
			GLuint modules[array_count(shader_table[0].modules)] = {};
			for (u32 idx = 0; idx < count; ++idx) {
				modules[idx] = shader_modules[shader_table[s].modules[idx]];
			}
			out_assets->shaders[s] = ogl_shader_link(count, modules, name);

			// NOTE(Dedrick): Do not stop linking the rest of the shaders if this
			// shader fails to link. Its more helpful to us if we can get as much
			// logs on all of the errors as possible at once to display.
			if (out_assets->shaders[s] == 0) {
				success = false;
			}
		}
	}

	for (u64 m = 0; m < SHADER_MODULE_COUNT; ++m) {
		glDeleteShader(shader_modules[m]);
	}

	if (!success) {
		for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
			glDeleteProgram(out_assets->shaders[s]);
			out_assets->shaders[s] = 0;
		}
	}

	scratch_end(scratch);
	return success;
}

auto dk::dkr_pak_load_textures(File file, PAK_Parsed const *pak,
							   RingBuffer *ring, GLuint stage_buffer,
							   DKR_RenderAssets *out_assets) noexcept -> b8 {
	struct OGL_TextureFormat { GLenum fmt, pixel_fmt, type; u32 bytes_per_pixel; }
	const ogl_fmt_table[] = {
		{ GL_NONE, GL_NONE, GL_NONE, 0 },
		{ GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, 4 },
	};
	static_assert(array_count(ogl_fmt_table) == PAK_TEXTURE_FORMAT_COUNT);

	String8 const texture_name_table[] = { "tony_mc_mapface.dds"_str8 };
	u64 const texture_data_offset = pak->sections[PAK_SECTION_KIND_GPU_TEXTURE_DATA].offset;
	u64 const texture_data_size   = pak->sections[PAK_SECTION_KIND_GPU_TEXTURE_DATA].size;

	u64 constexpr chunk_size = mega_bytes(4);

	void *stage_base = nullptr;
	if (gpu_ring_try_write(ring, texture_data_size, 16, &stage_base) != GPU_AllocResult::Ok) {
		DK_LOG_ERRORF("ERROR: pak texture blob (%llu bytes) exceeds staging ring capacity\n", texture_data_size);
		return false;
	}

	for (u64 cursor = 0; cursor < texture_data_size; ) {
		u64 const read_size = min(chunk_size, texture_data_size - cursor);
		file_read(file,
				  texture_data_offset + cursor,
				  texture_data_offset + cursor + read_size,
				  static_cast<u8 *>(stage_base) + cursor);
		cursor += read_size;
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, stage_buffer);
	for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
		String8 const tex_name = texture_name_table[t];
		PAK_Texture const *pak_tex = pak_texture_from_name(pak, tex_name);
		DK_ASSERT(pak_tex != nullptr);
		OGL_TextureFormat const *tex_fmt = &ogl_fmt_table[pak_tex->format];
		GLenum tex_kind = GL_TEXTURE_2D;
		switch (pak_tex->kind) {
			case PAK_TEXTURE_KIND_2D: tex_kind = GL_TEXTURE_2D; break;
			case PAK_TEXTURE_KIND_3D: tex_kind = GL_TEXTURE_3D; break;
		}

		GLuint tex = 0;
		glCreateTextures(tex_kind, 1, &tex);
		glObjectLabel(GL_TEXTURE, tex, static_cast<GLsizei>(tex_name.size),
					  reinterpret_cast<char const *>(tex_name.data));

		if (tex_kind == GL_TEXTURE_2D) {
			glTextureStorage2D(tex, pak_tex->mip_count, tex_fmt->fmt, pak_tex->width, pak_tex->height);
			u64 offset = pak_tex->offset;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage2D(tex, mip, 0, 0, mip_w, mip_h,
									tex_fmt->pixel_fmt, tex_fmt->type,
									reinterpret_cast<void const *>(offset));
				offset += static_cast<u64>(mip_w) * mip_h * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
			}
		} else {
			glTextureStorage3D(tex, pak_tex->mip_count, tex_fmt->fmt,
							   pak_tex->width, pak_tex->height, pak_tex->depth);
			u64 offset = pak_tex->offset;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			u32 mip_d = pak_tex->depth;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage3D(tex, mip, 0, 0, 0, mip_w, mip_h, mip_d,
									tex_fmt->pixel_fmt, tex_fmt->type,
									reinterpret_cast<void const *>(offset));
				offset += static_cast<u64>(mip_w) * mip_h * mip_d * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
				mip_d = mip_d > 1 ? (mip_d >> 1) : 1;
			}
		}
		DK_LOG_INFOF("[OpenGL] texture %.*s loaded\n", DK_STR8_VARG(tex_name));
		out_assets->textures[t] = tex;
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	return true;
}

auto dk::dkr_render_assets_load(File file, PAK_Parsed const *pak,
								RingBuffer *ring, GLuint stage_buffer,
								DKR_RenderAssets *out_assets) noexcept -> b8 {
	b8 const shaders_ok = dkr_pak_load_shaders(file, pak, out_assets);
	b8 const textures_ok = dkr_pak_load_textures(file, pak, ring, stage_buffer, out_assets);
	return shaders_ok && textures_ok;
}

auto dk::dkr_render_assets_release(DKR_RenderAssets *assets) noexcept -> void {
	if (assets == nullptr) {
		return;
	}
	for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
		if (assets->shaders[s] != 0) {
			glDeleteProgram(assets->shaders[s]);
			assets->shaders[s] = 0;
		}
	}
	for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
		if (assets->textures[t] != 0) {
			glDeleteTextures(1, &assets->textures[t]);
			assets->textures[t] = 0;
		}
	}
}

auto dk::dkr_pak_reload() noexcept -> void {
	DKR_RenderContext *render = &dkr_context->render;

	//~ Dedrick: Drain any signaled fences and reclaim ring slots.
	while (render->stage_fence_list.first != nullptr) {
		DKR_StageFenceNode *node = render->stage_fence_list.first;
		GPU_FenceStatus status = gpu_fence_status(&node->v.fence);
		if (status == GPU_FenceStatus::Signaled) {
			gpu_ring_reclaim_to(render->stage_ring, node->v.replay_read_pos);
			gpu_fence_release(&node->v.fence);
			forward_list_queue_pop(&render->stage_fence_list.first, &render->stage_fence_list.last);
			render->stage_fence_list.count -= 1;
		} else {
			break;
		}
	}

	//~ Dedrick: If an unsignaled fence remains, wait inline (bounded: at most 1).
	if (render->stage_fence_list.first != nullptr) {
		DKR_StageFenceNode *node = render->stage_fence_list.first;
		gpu_fence_wait(&node->v.fence, U64_MAX);
		gpu_ring_reclaim_to(render->stage_ring, node->v.replay_read_pos);
		gpu_fence_release(&node->v.fence);
		forward_list_queue_pop(&render->stage_fence_list.first, &render->stage_fence_list.last);
		render->stage_fence_list.count -= 1;
	}
	DK_ASSERT(render->stage_fence_list.count == 0);

	//~ Dedrick: Release prior assets.
	dkr_render_assets_release(render->assets);

	//~ Dedrick: Allocate fresh bundle on persistent arena.
	DKR_RenderAssets *new_assets = arena_push<DKR_RenderAssets>(dkr_context->arena);

	TempArena const scratch = scratch_begin(nullptr, 0);
	String8 const pak_path = dkr_pak_path(scratch.arena);
	DKR_PakOpen pak_open = {};
	if (!dkr_pak_open(pak_path, &pak_open)) {
		DK_LOG_ERRORF("ERROR: failed to open pak file for reload\n");
		arena_pop(dkr_context->arena, sizeof(DKR_RenderAssets));
		scratch_end(scratch);
		render->assets = nullptr;
		return;
	}
	b8 const good = dkr_render_assets_load(pak_open.file, &pak_open.parsed,
											render->stage_ring, render->stage_buffer,
											new_assets);
	dkr_pak_close(&pak_open);
	scratch_end(scratch);

	if (!good) {
		DK_LOG_ERRORF("ERROR: failed to load render assets from pak\n");
		dkr_render_assets_release(new_assets);
		arena_pop(dkr_context->arena, sizeof(DKR_RenderAssets));
		render->assets = nullptr;
		return;
	}
	DK_LOG_INFOF("render assets reloaded\n");
	render->assets = new_assets;
}

auto dk::dkr_event_list_push(Arena *arena, DKR_EventList *events, DKR_Event const *event) noexcept -> void {
	DKR_EventNode *node = arena_push<DKR_EventNode>(arena);
	node->v.kind = event->kind;
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
		node = DK_CAST_FROM_MEMBER(DKR_EventNode, v, *event);
		node = node->next;
	}
	*event = nullptr;
	if (node != nullptr) {
		*event = &node->v;
	}
	return *event != nullptr;
}

auto dk::dkr_init(CmdLine *cmd_line) noexcept -> void {
	(void)cmd_line;

	//~ Dedrick: Set up persistent arena and context.
	Arena *arena = arena_alloc();
	dkr_context = arena_push<DKR_Context>(arena);
	dkr_context->arena = arena;
	for (u32 i = 0; i < array_count(dkr_context->frame_arenas); ++i) {
		dkr_context->frame_arenas[i] = arena_alloc();
	}

	//~ Dedrick: Set up log.
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

	//~ Dedrick: Set up staging buffer (persistent mapped PBO).
	{
		GLuint stage_buffer = 0;
		GLbitfield const stage_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		u64 const stage_size = mega_bytes(128);
		glCreateBuffers(1, &stage_buffer);
		glNamedBufferStorage(stage_buffer, static_cast<GLsizeiptr>(stage_size), nullptr, stage_flags);
		void *const mapped_base = glMapNamedBufferRange(stage_buffer, 0, stage_size, stage_flags | GL_MAP_INVALIDATE_BUFFER_BIT);
		dkr_context->render.stage_buffer = stage_buffer;
		dkr_context->render.stage_ring = gpu_ring_make(dkr_context->arena, mapped_base, stage_size);
	}

	//~ Dedrick: Initialize fence list and assets.
	dkr_context->render.stage_fence_list = {};
	dkr_context->render.assets = nullptr;

	//~ Dedrick: Load pak, initialize assets.
	dkr_pak_reload();
	if (dkr_context->render.assets == nullptr) {
		dt_show_dialog(nullptr, "Fatal Error"_str8, "Error loading pak assets"_str8, true);
		abort_self(0);
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
	//~ Dedrick: Force-wait and release any remaining staging fences.
	DKR_RenderContext *render = &dkr_context->render;
	while (render->stage_fence_list.first != nullptr) {
		DKR_StageFenceNode *node = render->stage_fence_list.first;
		gpu_fence_wait(&node->v.fence, U64_MAX);
		gpu_fence_release(&node->v.fence);
		forward_list_queue_pop(&render->stage_fence_list.first, &render->stage_fence_list.last);
		render->stage_fence_list.count -= 1;
	}

	//~ Dedrick: Release render assets.
	dkr_render_assets_release(render->assets);

	//~ Dedrick: Teardown ImGui, log, window.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplRgfw_Shutdown();
	ImGui::DestroyContext();
	log_release(dkr_context->log);
	ogl_window_unequip(dkr_context->window);
	dt_window_close(dkr_context->window);
}

auto dk::dkr_frame() noexcept -> b8 {
	TempArena const scratch = scratch_begin(nullptr, 0);

	//~ Dedrick: Reclaim signaled staging fences from previous frame.
	{
		DKR_RenderContext *render = &dkr_context->render;
		while (render->stage_fence_list.first != nullptr) {
			DKR_StageFenceNode *node = render->stage_fence_list.first;
			GPU_FenceStatus status = gpu_fence_status(&node->v.fence);
			if (status == GPU_FenceStatus::Signaled) {
				gpu_ring_reclaim_to(render->stage_ring, node->v.replay_read_pos);
				gpu_fence_release(&node->v.fence);
				forward_list_queue_pop(&render->stage_fence_list.first, &render->stage_fence_list.last);
				render->stage_fence_list.count -= 1;
			} else {
				break;
			}
		}
	}

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

	//~ Dedrick: Begin log frame scope.
	log_frame_begin();

	//~ Dedrick: Process platform (window) events -> push application events.
	{
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
					dkr_pak_reload();
					break;
				}
				case DKR_EVENT_KIND_OPEN_CONSOLE: {
					dkr_context->console_is_open = true;
					break;
				}
			}
		}
	}

	//~ Dedrick: Begin ImGui frame scope.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplRgfw_NewFrame();
	ImGui::GetIO().DeltaTime = dkr_context->frame_dt;
	ImGui::NewFrame();

	//~ Dedrick: Adjust ImGui for HiDPI screens.
	{
		ImGuiIO &io = ImGui::GetIO();
		f32 const old_scale = io.FontGlobalScale;
		f32 const content_scale = ImGui_ImplRgfw_GetContentScaleForWindow(dkr_context->window);
		if (abs(content_scale - old_scale) > 1e-5f) {
			f32 const scale_factor = content_scale / old_scale;
			io.FontGlobalScale = content_scale;
			ImGui::GetStyle().ScaleAllSizes(scale_factor);
		}
	}

	//~ Dedrick: Build UI.
	if (ImGui::Begin("dkrend - Rendering Engine")) {
		ImGui::Text("Frame Time: %.5f", dkr_context->frame_dt);

		if (ImGui::Button("Console")) {
			if (dkr_context->console_is_open) {
				ImGui::SetWindowFocus("Console");
			}
			else {
				dkr_push_event_kind(DKR_EVENT_KIND_OPEN_CONSOLE);
			}
		}

		if (ImGui::Button("Reload Pak")) {
			dkr_push_event_kind(DKR_EVENT_KIND_RELOAD_PAK);
		}

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

			if (ImGui::BeginChild("TextRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
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

				ImGuiListClipper clipper = {};
				clipper.Begin(static_cast<int>(visible_count));
				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
						u64 const line_idx = visible_indices[i];
						DKR_ConsoleLine const *line = &console->lines[line_idx % console->max_lines];
						u64 const text_idx = line->offset % console->text_buffer_size;
						char const *text_start = reinterpret_cast<char const *>(console->text_buffer + text_idx);
						char const *text_end = text_start + line->size;

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

	//~ Dedrick: Draw Scene.
	{
		s32 width = 0, height = 0;
		RGFW_window_getSizeInPixels(dkr_context->window, &width, &height);
		glViewport(0, 0, width, height);
	}
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(ogl_context->all_purpose_vao);
	glUseProgram(dkr_context->render.assets->shaders[DKR_SHADER_KIND_HELLO_TRIANGLE]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//~ Dedrick: Draw UI.
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	RGFW_window_swapBuffers_OpenGL(dkr_context->window);

	//~ Dedrick: Record staging fence after swap (GPU has consumed the draw).
	//   Fence nodes live on the persistent arena so arena_clear on the
	//   frame arenas cannot reap them mid-flight.
	{
		GPU_Fence fence = {};
		gpu_fence_alloc(&fence);
		DKR_StageFenceNode *node = gpu_fence_list_push(dkr_context->arena, &dkr_context->render.stage_fence_list, fence);
		node->v.replay_read_pos = dkr_context->render.stage_ring->write_pos;
	}

	//~ Dedrick: Hand off events.
	dkr_context->events[1] = dkr_context->events[0];
	dkr_context->events[0] = {};

	//~ Dedrick: Bump frame counters and clear previous frame arena.
	dkr_context->frame_index += 1;
	arena_clear(dkr_frame_arena());

	//~ Dedrick: Collect logs.
	{
		LogFrameResult const log = log_frame_end(scratch.arena);
		if (log.count > 0) {
			append_string_to_file_path(dkr_context->log_path, log.string);

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

				if (line_size > 0 && line_kind != entry->kind) {
					dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
					line_start_offset = text_write;
					line_size = 0;
				}
				line_kind = entry->kind;

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

				DK_ASSERT(chunk_size <= console->text_buffer_size);
				std::memcpy(console->text_buffer + write_idx, chunk, chunk_size);

				for (u32 i = 0; i < chunk_size; ++i) {
					text_write += 1;
					line_size += 1;
					if (chunk[i] == '\n') {
						dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
						line_start_offset = text_write;
						line_size = 0;
					}
				}

				for (; console->line_read_pos < console->line_write_pos; ++console->line_read_pos) {
					DKR_ConsoleLine *line = &console->lines[console->line_read_pos % console->max_lines];
					b8 const stomped = (text_write - line->offset) > console->text_buffer_size;
					if (!stomped) {
						break;
					}
				}

				if (line_size > 0) {
					dkr_console_commit_line(console, line_start_offset, line_size, line_kind);
				}

				console->text_write_pos = text_write;
			}
		}
	}
	scratch_end(scratch);
	return dkr_context->quit;
}
