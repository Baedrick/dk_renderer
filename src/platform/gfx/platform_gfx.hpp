// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_PLATFORM_GFX_INCLUDED
#	define DK_PLATFORM_GFX_INCLUDED
#endif

#define RGFW_OPENGL
#include "thirdparty/rgfw/RGFW.h"

namespace dk {
	auto plt_gfx_init() noexcept -> void;

	auto plt_show_dialog(RGFW_window const *parent, String8 title, String8 message, b8 error) noexcept -> void;

	auto plt_show_in_file_browser(String8 path) noexcept -> void;

	auto plt_file_dialog_pick_file(Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count) noexcept -> String8;
	auto plt_file_dialog_pick_multiple_files(Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count) noexcept -> String8List;
	auto plt_file_dialog_save(Arena *arena, RGFW_window const *parent, String8 default_name, PLT_FileDialogFilter const *filters, u64 filter_count, u64 *out_filter_index) noexcept -> String8;
	auto plt_file_dialog_pick_folder(Arena *arena, RGFW_window const *parent) noexcept -> String8;
}
