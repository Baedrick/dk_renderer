// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include <wrl/client.h>
#include <shellapi.h>

namespace dk {
	FILEOPENDIALOGOPTIONS constexpr W32_DT_FILE_DIALOG_COMMON_FLAGS =
		FOS_PATHMUSTEXIST
		| FOS_FORCEFILESYSTEM
		| FOS_NOCHANGEDIR;

	struct W32_DT_Context {
		Arena *arena;
		RGFW_window *first_window;
		RGFW_window *last_window;
		RGFW_window *free_window;
	};

	extern W32_DT_Context *w32_dt_context;

	auto w32_dt_window_alloc() noexcept -> RGFW_window *;
	auto w32_dt_window_release(RGFW_window *window) noexcept -> void;

	auto w32_dt_create_filter_specs(Arena *arena, DT_FileDialogFilter const *filters, u64 filter_count, UINT *out_count) -> COMDLG_FILTERSPEC *;
}
