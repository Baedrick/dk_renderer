// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

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
		String8 binary_dir;
		String8 user_program_data_dir;
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

	using PLT_DirIterFlags = u32;
	enum : u32 {
		PLT_DIR_ITER_FLAG_NONE = 0,
		PLT_DIR_ITER_FLAG_SKIP_FOLDERS = 1u << 0,
		PLT_DIR_ITER_FLAG_SKIP_FILES = 1u << 1,
		PLT_DIR_ITER_FLAG_SKIP_HIDDEN_FILES = 1u << 2,
		PLT_DIR_ITER_FLAG_DONE = 1u << 31,
	};

	struct PLT_DirIterResult {
		String8 name;
		PLT_FileAttributes attributes;
	};

	using PLT_ProcessLaunchFlags = u32;
	enum : u32 {
		PLT_PROCESS_LAUNCH_FLAG_NONE = 0,
		PLT_PROCESS_LAUNCH_FLAG_NO_WINDOW = 1u << 0,
	};

	struct PLT_ProcessLaunchParams {
		String8List cmd_line;
		String8 working_dir;
		PLT_ProcessLaunchFlags flags;
	};

	using PLT_ThreadFunction = void (void *params);

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

	auto plt_read_bytes_from_file_path(Arena *arena, String8 path) noexcept -> Buffer8;
	auto plt_write_bytes_to_file_path(String8 path, Buffer8 bytes) noexcept -> b8;
	auto plt_append_bytes_to_file_path(String8 path, Buffer8 bytes) noexcept -> b8;

	auto plt_dir_iter_begin(String8 dir, PLT_DirIterFlags flags) noexcept -> PLT_Handle;
	auto plt_dir_iter_next(Arena *arena, PLT_Handle dir_iter, PLT_DirIterResult *out_result) noexcept -> b8;
	auto plt_dir_iter_end(PLT_Handle dir_iter) noexcept -> void;

	auto plt_make_directory(String8 path) noexcept -> b8;

	auto plt_now_microseconds() noexcept -> u64;
	auto plt_sleep(u64 milliseconds) noexcept -> void;

	auto plt_process_launch(PLT_ProcessLaunchParams const *params) noexcept -> PLT_Handle;
	auto plt_process_join(PLT_Handle process, u64 end_time_us, u64 *out_exit_code) noexcept -> b8;
	auto plt_process_kill(PLT_Handle process) noexcept -> b8;

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

	auto plt_semaphore_alloc(u32 initial_count, u32 max_count, String8 name) noexcept -> PLT_Handle;
	auto plt_semaphore_release(PLT_Handle semaphore) noexcept -> void;
	auto plt_semaphore_open(String8 name) noexcept -> PLT_Handle;
	auto plt_semaphore_close(PLT_Handle semaphore) noexcept -> void;
	auto plt_semaphore_wait(PLT_Handle semaphore, u64 end_time_us) noexcept -> b8;
	auto plt_semaphore_signal(PLT_Handle semaphore) noexcept -> void;
}
