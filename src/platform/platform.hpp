// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/rgfw/RGFW.h"

namespace dk {
	struct PLT_Handle {
		u64 v;
	};

	struct PLT_SystemInfo {
		u64 page_size;
		u32 logical_processor_count;
	};

	struct PLT_ProcessInfo {
		u32 pid;
	};

	using PLT_AccessFlags = u32;
	enum : u32 {
		PLT_ACCESS_FLAG_READ = 1u << 0,
		PLT_ACCESS_FLAG_WRITE = 1u << 1,
		PLT_ACCESS_FLAG_APPEND = 1u << 2
	};

	using PLT_FileFlags = u32;
	enum : u32 {
		PLT_FILE_FLAG_NONE = 0,
		PLT_FILE_FLAG_DIRECTORY = 1u << 0
	};

	struct PLT_FileAttributes {
		u64 size;
		PLT_FileFlags flags;
	};

	using PLT_ThreadFunction = void (void *params);

	struct PLT_FileDialogFilter {
		String8 display_name; ///< e.g. "Text files"
		String8 extensions; ///< e.g. "txt,text"
	};

	auto operator==(PLT_Handle a, PLT_Handle b) noexcept -> b8;
	auto operator!=(PLT_Handle a, PLT_Handle b) noexcept -> b8;

	auto plt_handle_invalid() noexcept -> PLT_Handle;

	auto plt_get_system_info() noexcept -> PLT_SystemInfo *;
	auto plt_get_process_info() noexcept -> PLT_ProcessInfo *;
	auto plt_get_entropy(void *data, u64 size) noexcept -> void;

	[[noreturn]] auto plt_abort(s32 code) noexcept -> void;

	auto plt_reserve(u64 size) noexcept -> void *;
	auto plt_commit(void *ptr, u64 size) noexcept -> b8;
	auto plt_decommit(void *ptr, u64 size) noexcept -> b8;
	auto plt_release(void *ptr, u64 size) noexcept -> void;

	auto plt_shared_memory_create(u64 size, String8 name) noexcept -> PLT_Handle;
	auto plt_shared_memory_open(String8 name) noexcept -> PLT_Handle;
	auto plt_shared_memory_close(PLT_Handle handle) noexcept -> void;
	auto plt_shared_memory_map(PLT_Handle handle, u64 begin, u64 end) noexcept -> void *;
	auto plt_shared_memory_unmap(PLT_Handle handle, void *ptr, u64 begin, u64 end) noexcept -> void;

	auto plt_file_open(String8 path, PLT_AccessFlags flags) noexcept -> PLT_Handle;
	auto plt_file_close(PLT_Handle file) noexcept -> void;
	auto plt_file_read(PLT_Handle file, u64 begin, u64 end, void *out_data) noexcept -> u64;
	auto plt_file_write(PLT_Handle file, u64 begin, u64 end, void const *data) noexcept -> u64;
	auto plt_attributes_from_file(PLT_Handle file) noexcept -> PLT_FileAttributes;

	auto plt_make_directory(String8 path) noexcept -> b8;

	auto plt_now_microseconds() noexcept -> u64;
	auto plt_sleep(u64 milliseconds) noexcept -> void;

	auto plt_set_thread_name(String8 name) noexcept -> void;
	auto plt_thread_launch(PLT_ThreadFunction *func, void *params) noexcept -> PLT_Handle;
	auto plt_thread_join(PLT_Handle thread) noexcept -> void;
	auto plt_thread_detach(PLT_Handle thread) noexcept -> void;

	auto plt_mutex_alloc() noexcept -> PLT_Handle;
	auto plt_mutex_release(PLT_Handle mutex) noexcept -> void;
	auto plt_mutex_scope_enter(PLT_Handle mutex) noexcept -> void;
	auto plt_mutex_scope_leave(PLT_Handle mutex) noexcept -> void;

	auto plt_rw_mutex_alloc() noexcept -> PLT_Handle;
	auto plt_rw_mutex_release(PLT_Handle rw_mutex) noexcept -> void;
	auto plt_rw_mutex_scope_enter_w(PLT_Handle rw_mutex) noexcept -> void;
	auto plt_rw_mutex_scope_leave_w(PLT_Handle rw_mutex) noexcept -> void;
	auto plt_rw_mutex_scope_enter_r(PLT_Handle rw_mutex) noexcept -> void;
	auto plt_rw_mutex_scope_leave_r(PLT_Handle rw_mutex) noexcept -> void;

	auto plt_cond_var_alloc() noexcept -> PLT_Handle;
	auto plt_cond_var_release(PLT_Handle cond_var) noexcept -> void;
	auto plt_cond_var_wait(PLT_Handle cond_var, PLT_Handle mutex, u64 end_time_us) noexcept -> b8;
	auto plt_cond_var_wait_rw_w(PLT_Handle cond_var, PLT_Handle rw_mutex, u64 end_time_us) noexcept -> b8;
	auto plt_cond_var_wait_rw_r(PLT_Handle cond_var, PLT_Handle rw_mutex, u64 end_time_us) noexcept -> b8;
	auto plt_cond_var_signal(PLT_Handle cond_var) noexcept -> void;
	auto plt_cond_var_signal_all(PLT_Handle cond_var) noexcept -> void;

	auto plt_show_dialog(RGFW_window const *parent, String8 title, String8 message, b8 error) noexcept -> void;

	auto plt_show_in_file_browser(String8 path) noexcept -> void;

	auto plt_file_dialog_pick_file(Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count) noexcept -> String8;
	auto plt_file_dialog_pick_multiple_files(Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count) noexcept -> String8List;
	auto plt_file_dialog_save(Arena *arena, RGFW_window const *parent, String8 default_name, PLT_FileDialogFilter const *filters, u64 filter_count, u64 *out_filter_index) noexcept -> String8;
	auto plt_file_dialog_pick_folder(Arena *arena, RGFW_window const *parent) noexcept -> String8;
}

#ifdef DK_PLATFORM_WIN32
#	include "platform_win32.hpp"
#else
#	error "Platform layer not implemented for this platform."
#endif
