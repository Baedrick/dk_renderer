/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick.
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 */

#pragma once

namespace dk {
	struct PLT_Handle {
		u64 v;

		auto operator==(PLT_Handle o) const noexcept -> b8;
		auto operator!=(PLT_Handle o) const noexcept -> b8;
	};

	struct PLT_SystemInfo {
		u64 page_size;
		u32 logical_processor_count;
	};

	struct PLT_ProcessInfo {
		u32 pid;
	};

	auto plt_handle_invalid() noexcept -> PLT_Handle;

	auto plt_get_system_info() noexcept -> PLT_SystemInfo *;
	auto plt_get_process_info() noexcept -> PLT_ProcessInfo *;
	auto plt_get_entropy(void *data, u64 size) noexcept -> void;

	[[noreturn]] auto plt_abort(s32 code) noexcept -> void;

	auto plt_reserve(u64 size) noexcept -> void *;
	auto plt_commit(void *ptr, u64 size) noexcept -> b8;
	auto plt_decommit(void *ptr, u64 size) noexcept -> void;
	auto plt_release(void *ptr, u64 size) noexcept -> void;
	auto plt_malloc(u64 size) noexcept -> void *;
	auto plt_free(void *ptr) noexcept -> void;
}
