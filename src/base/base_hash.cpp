// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define XXH_IMPLEMENTATION
#define XXH_INLINE_ALL
#define XXH_STATIC_LINKING_ONLY
#include "thirdparty/xxHash/xxhash.h"
#undef XXH_IMPLEMENTATION

auto dk::u64_hash_from_seed_data(u64 seed, void const *data, u64 size) noexcept -> u64 {
	u64 const result = XXH3_64bits_withSeed(data, size, seed);
	return result;
}

auto dk::u64_hash_from_data(void const *data, u64 size) noexcept -> u64 {
	u64 const result = u64_hash_from_seed_data(5381, data, size);
	return result;
}
