// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_CmdKind : u32 {
		DKR_CMD_KIND_NULL = 0,
		DKR_CMD_KIND_OPEN_WINDOW,
		DKR_CMD_KIND_COUNT
	};

	struct DKR_Context {
		Arena *arena;
		b8 quit;

		LogContext *log;
		String8 log_path;

		u64 frame_index;
		Arena *frame_arenas[2];
		f64 time_in_seconds;
		f32 frame_dt;

		RGFW_window *window;
	};

	extern DKR_Context *dkr_context;

	auto dkr_frame_arena() noexcept -> Arena *;

	auto dkr_init(CmdLine *cmd_line) noexcept -> void;
	auto dkr_frame() noexcept -> b8;
	auto dkr_shutdown() noexcept -> void;
}
