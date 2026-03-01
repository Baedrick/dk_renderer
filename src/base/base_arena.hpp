/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.
 */

#pragma once

namespace dk {
	constexpr u64 ARENA_HEADER_SIZE = 128;

	using ArenaFlags = u32;
	enum : u32 {
		ARENA_FLAG_NONE = 0,
		ARENA_FLAG_NO_CHAIN = 1u << 0
	};

	struct ArenaParams {
		u64 reserve_size;
		u64 commit_size;
		ArenaFlags flags;
	};

	struct Arena {
		Arena *current; ///< Previous arena in chain.
		Arena *next; ///< Next arena in chain.
		u64 commit_size;
		u64 reserve_size;
		u64 base_offset;
		u64 offset;
		u64 committed;
		u64 reserved;
		ArenaFlags flags;
	};
	static_assert(sizeof(Arena) <= ARENA_HEADER_SIZE);

	struct TempArena {
		Arena *arena;
		u64 position;
	};

	constexpr u64 ARENA_DEFAULT_RESERVE_SIZE = mega_bytes(64);
	constexpr u64 ARENA_DEFAULT_COMMIT_SIZE = kilo_bytes(64);
	constexpr u64 ARENA_DEFAULT_FLAGS = ARENA_FLAG_NONE;

	auto arena_alloc(ArenaParams const *params) noexcept -> Arena *;
	auto arena_release(Arena *arena) noexcept -> void;

	auto arena_push_bytes(Arena *arena, u64 size, u64 align) noexcept -> void *;
	auto arena_push_bytes_no_zero(Arena *arena, u64 size, u64 align) noexcept -> void *;

	auto arena_clear(Arena *arena) noexcept -> void;
	auto arena_offset(Arena *arena) noexcept -> u64;
	auto arena_pop(Arena *arena, u64 amount) noexcept -> void;
	auto arena_pop_to(Arena *arena, u64 offset) noexcept -> void;

	auto arena_temp_begin(Arena *arena) noexcept -> TempArena;
	auto arena_temp_end(TempArena temp) noexcept -> void;

	template <typename T>
	auto arena_push(Arena *arena) noexcept -> T * {
		return static_cast<T *>(arena_push_bytes(arena, sizeof(T), alignof(T)));
	}

	template <typename T>
	auto arena_push_array(Arena *arena, u64 count) noexcept -> T * {
		return static_cast<T *>(arena_push_bytes(arena, sizeof(T) * count, alignof(T)));
	}
}
