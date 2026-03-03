/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.
 */

namespace dk {
	struct ThreadContext {
		Arena *scratch_arenas[2];
	};

	auto thread_context_alloc() noexcept -> ThreadContext *;
	auto thread_context_release(ThreadContext *context) noexcept -> void;
	auto thread_context_select(ThreadContext *context) noexcept -> void;
	auto thread_context_selected() noexcept -> ThreadContext *;

	auto thread_context_get_scratch(Arena **conflicts, u64 count) noexcept -> Arena *;
	auto scratch_begin(Arena **conflicts, u64 count) noexcept -> TempArena;
	auto scratch_end(TempArena scratch) noexcept -> void;
}
