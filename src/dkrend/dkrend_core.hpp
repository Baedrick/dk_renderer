// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_CmdKind : u32 {
		DKR_CMD_KIND_NULL = 0,
		DKR_CMD_KIND_OPEN_WINDOW,
		DKR_CMD_KIND_COUNT
	};

	enum DKR_ShaderKind : u32 {
		DKR_SHADER_KIND_HELLO_TRIANGLE,
		DKR_SHADER_KIND_DUMMY,
		DKR_SHADER_KIND_COUNT
	};

	struct DKR_RenderContext {
		GLuint shaders[DKR_SHADER_KIND_COUNT];
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

		//~ Dedrick: Rendering
		DKR_RenderContext render;

		//~ Dedrick: Window.
		RGFW_window *window;
	};

	extern DKR_Context *dkr_context;

	auto dkr_frame_arena() noexcept -> Arena *;

	auto dkr_init(CmdLine *cmd_line) noexcept -> void;
	auto dkr_frame() noexcept -> b8;
	auto dkr_shutdown() noexcept -> void;
}
