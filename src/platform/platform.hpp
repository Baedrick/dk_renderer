// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
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

	struct PLT_Handle {
		u64 v;

		auto operator==(PLT_Handle o) const noexcept -> b8;
		auto operator!=(PLT_Handle o) const noexcept -> b8;
	};

	auto plt_handle_invalid() noexcept -> PLT_Handle;

	auto plt_get_system_info() noexcept -> PLT_SystemInfo *;
	auto plt_get_process_info() noexcept -> PLT_ProcessInfo *;
	auto plt_get_entropy(void *data, u64 size) noexcept -> void;

	[[noreturn]] auto plt_abort(s32 code) noexcept -> void;

	auto plt_reserve(u64 size) noexcept -> void *;
	auto plt_commit(void *ptr, u64 size) noexcept -> b8;
	auto plt_decommit(void *ptr, u64 size) noexcept -> b8;
	auto plt_release(void *ptr, u64 size) noexcept -> void;

	auto plt_file_open(String8 path, PLT_AccessFlags flags) noexcept -> PLT_Handle;
	auto plt_file_close(PLT_Handle file) noexcept -> void;
	auto plt_file_read(PLT_Handle file, u64 begin, u64 end, void *out_data) noexcept -> u64;
	auto plt_file_write(PLT_Handle file, u64 begin, u64 end, void const *data) noexcept -> u64;
	auto plt_attributes_from_file(PLT_Handle file) noexcept -> PLT_FileAttributes;

	auto plt_now_microseconds() noexcept -> u64;
	auto plt_sleep(u64 milliseconds) noexcept -> void;
}
