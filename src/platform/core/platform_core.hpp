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

	auto plt_now_time_us() noexcept -> u64;
	auto plt_sleep(u64 milliseconds) noexcept -> void;

	auto plt_process_launch(PLT_ProcessLaunchParams const *params) noexcept -> PLT_Handle;
	auto plt_process_join(PLT_Handle process, u64 end_time_us, u64 *out_exit_code) noexcept -> b8;
	auto plt_process_kill(PLT_Handle process) noexcept -> b8;
}
