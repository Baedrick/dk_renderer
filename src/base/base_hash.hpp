// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	auto u64_hash_from_seed_data(u64 seed, void *data, u64 size) noexcept -> u64;
	auto u64_hash_from_data(void *data, u64 size) noexcept -> u64;
}
