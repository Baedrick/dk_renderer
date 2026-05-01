// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include <wrl/client.h>
#include <shellapi.h>
#include <ShlObj_core.h>

namespace dk {
	FILEOPENDIALOGOPTIONS constexpr PLT_W32_FILE_DIALOG_COMMON_FLAGS =
		FOS_PATHMUSTEXIST
		| FOS_FORCEFILESYSTEM
		| FOS_NOCHANGEDIR;

	auto plt_w32_create_filter_specs(Arena *arena, PLT_FileDialogFilter const *filters, u64 filter_count, UINT *out_count) -> COMDLG_FILTERSPEC *;
}
