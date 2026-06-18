// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct SystemInfo {
		u64 page_size;
		u32 logical_processor_count;
	};

	struct ProcessInfo {
		u32 pid;
		String8 binary_dir;
		String8 user_program_data_dir;
	};

	struct Process {
		u64 v;
	};

	using ProcessLaunchFlags = u32;
	enum : u32 {
		PROCESS_LAUNCH_FLAG_NONE = 0,
		PROCESS_LAUNCH_FLAG_NO_WINDOW = 1u << 0,
	};

	struct ProcessLaunchParams {
		String8List cmd_line;
		String8 working_dir;
		ProcessLaunchFlags flags;
	};

	auto operator==(Process a, Process b) noexcept -> b8;
	auto operator!=(Process a, Process b) noexcept -> b8;
	auto is_valid(Process process) noexcept -> b8;

	auto get_system_info() noexcept -> SystemInfo *;
	auto get_process_info() noexcept -> ProcessInfo *;

	auto get_entropy(void *data, u64 size) noexcept -> void;

	[[noreturn]] auto abort_self(s32 code) noexcept -> void;

	auto now_time_us() noexcept -> u64;
	auto sleep(u64 milliseconds) noexcept -> void;

	auto process_launch(ProcessLaunchParams const *params) noexcept -> Process;
	auto process_join(Process process, u64 end_time_us, u64 *out_exit_code) noexcept -> b8;
	auto process_kill(Process process) noexcept -> b8;
}
