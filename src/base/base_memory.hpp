// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct SharedMemory {
		u64 v;
	};

	auto memory_reserve(u64 size) noexcept -> void *;
	auto memory_commit(void *ptr, u64 size) noexcept -> b8;
	auto memory_decommit(void *ptr, u64 size) noexcept -> b8;
	auto memory_release(void *ptr, u64 size) noexcept -> void;

	auto shared_memory_create(u64 size, String8 name) noexcept -> SharedMemory;
	auto shared_memory_open(String8 name) noexcept -> SharedMemory;
	auto shared_memory_close(SharedMemory handle) noexcept -> void;
	auto shared_memory_map(SharedMemory handle, u64 begin, u64 end) noexcept -> void *;
	auto shared_memory_unmap(SharedMemory handle, void *ptr, u64 begin, u64 end) noexcept -> void;
}
