// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	auto memory_reserve(u64 size) noexcept -> void *;
	auto memory_commit(void *ptr, u64 size) noexcept -> b8;
	auto memory_decommit(void *ptr, u64 size) noexcept -> b8;
	auto memory_release(void *ptr, u64 size) noexcept -> void;
}
