// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_EventKind : u32 {
		DKR_EVENT_KIND_NULL = 0,
		DKR_EVENT_KIND_QUIT,
		DKR_EVENT_KIND_OPEN_CONSOLE,
		DKR_EVENT_KIND_RELOAD_PAK,
		DKR_EVENT_KIND_COUNT
	};

	struct DKR_EventReloadPak {
		DKR_EventKind kind;
		String8 file_path;
	};

	union DKR_Event {
		DKR_EventKind kind;
		DKR_EventReloadPak reload_pak;
	};

	struct DKR_EventNode {
		DKR_EventNode *next;
		DKR_EventNode *prev;
		DKR_Event event;
	};

	struct DKR_EventList {
		DKR_EventNode *first;
		DKR_EventNode *last;
		u64 count;
	};

	enum DKR_ShaderKind : u32 {
		DKR_SHADER_KIND_HELLO_TRIANGLE,
		DKR_SHADER_KIND_DUMMY,
		DKR_SHADER_KIND_COUNT
	};

	enum DKR_TextureKind : u32 {
		DKR_TEXTURE_KIND_TONY_MC_MAPFACE,
		DKR_TEXTURE_KIND_COUNT,
	};

	struct DKR_RenderAssets {
		GLuint shaders[DKR_SHADER_KIND_COUNT];
		GLuint textures[DKR_TEXTURE_KIND_COUNT];
	};

	struct DKR_RenderContext {

	};

	struct DKR_ConsoleLine {
		u64 offset;
		u32 size;
		LogKind kind;
	};

	struct DKR_Console {
		u8 *text_buffer;
		u64 text_buffer_size;
		u64 text_write_pos;
		DKR_ConsoleLine *lines;
		u64 max_lines;
		u64 line_write_pos;
		u64 line_read_pos;
		u32 hide_mask;
	};

	struct DKR_Context {
		Arena *arena;
		b8 quit;

		//~ Dedrick: Log.
		LogContext *log;
		String8 log_path;

		//~ Dedrick: Frame history.
		u64 frame_index;
		Arena *frame_arenas[2];
		f64 time_in_seconds;

		//~ Dedrick: Frame timing.
		u64 last_frame_time_us;
		s64 target_frame_time_us;
		s64 vsync_max_error_us;
		s64 snap_frequencies[8];
		s64 time_averager[4];
		s64 time_averager_residual;
		u32 time_averager_head;
		f32 frame_dt;

		//~ Dedrick: Events.
		DKR_EventList events[2];

		//~ Dedrick: Rendering.
		DKR_RenderContext render;
		DKR_RenderAssets render_assets;

		//~ Dedrick: Window.
		RGFW_window *window;

		//~ Dedrick: Console.
		b8 console_is_open;
		DKR_Console console;
	};

	extern DKR_Context *dkr_context;

	auto dkr_frame_arena() noexcept -> Arena *;

	auto dkr_event_list_push(Arena *arena, DKR_EventList *events, DKR_Event const *event) noexcept -> void;
	auto dkr_push_event(DKR_Event const *event) noexcept -> void;
	auto dkr_push_event_kind(DKR_EventKind kind) noexcept -> void;
	auto dkr_next_event(DKR_Event **event) noexcept -> b8;

	auto dkr_asset_pak_path(Arena *arena) noexcept -> String8;
	auto dkr_render_assets_load(PAK_Parsed const *pak, DKR_RenderAssets *out_assets) noexcept -> b8;

	auto dkr_console_commit_line(DKR_Console *console, u64 offset, u32 size, LogKind kind) noexcept -> void;

	auto dkr_init(CmdLine *cmd_line) noexcept -> void;
	auto dkr_frame() noexcept -> b8;
	auto dkr_shutdown() noexcept -> void;
}
