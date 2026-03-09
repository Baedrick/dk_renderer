// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace {
	thread_local dk::ThreadContext *thread_context_local = nullptr;
}

auto dk::thread_context_alloc() noexcept -> ThreadContext * {
	Arena *arena = arena_alloc(&ARENA_DEFAULT_PARAMS);
	ThreadContext *thread_context = arena_push<ThreadContext>(arena);
	thread_context->scratch_arenas[0] = arena;
	thread_context->scratch_arenas[1] = arena_alloc(&ARENA_DEFAULT_PARAMS);
	return thread_context;
}

auto dk::thread_context_release(ThreadContext *context) noexcept -> void {
	DK_ASSERT(context != nullptr);
	arena_release(context->scratch_arenas[1]);
	arena_release(context->scratch_arenas[0]);
}

auto dk::thread_context_select(ThreadContext *context) noexcept -> void {
	thread_context_local = context;
}

auto dk::thread_context_selected() noexcept -> ThreadContext * {
	return thread_context_local;
}

auto dk::thread_context_get_scratch(Arena **conflicts, u64 count) noexcept -> Arena * {
	ThreadContext *context = thread_context_selected();
	DK_ASSERT(context != nullptr);

	for (Arena *const candidate : context->scratch_arenas) {
		b8 is_conflicting = false;
		for (u32 conflict_idx = 0; conflict_idx < count; ++conflict_idx) {
			if (candidate == conflicts[conflict_idx]) {
				is_conflicting = true;
				break;
			}
		}
		if (!is_conflicting) {
			return candidate;
		}
	}
	return nullptr;
}

auto dk::scratch_begin(Arena **conflicts, u64 count) noexcept -> TempArena {
	return arena_temp_begin(thread_context_get_scratch(conflicts, count));
}

auto dk::scratch_end(TempArena scratch) noexcept -> void {
	arena_temp_end(scratch);
}
