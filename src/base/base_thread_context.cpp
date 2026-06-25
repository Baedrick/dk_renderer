// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	thread_local ThreadContext *local_thread_context = nullptr;
}

auto dk::thread_context_alloc() noexcept -> ThreadContext * {
	Arena *arena = arena_alloc();
	ThreadContext *thread_context = arena_push<ThreadContext>(arena);
	thread_context->scratch_arenas[0] = arena;
	thread_context->scratch_arenas[1] = arena_alloc();
	return thread_context;
}

auto dk::thread_context_release(ThreadContext *context) noexcept -> void {
	DK_ASSERT(context != nullptr);
	arena_release(context->scratch_arenas[1]);
	arena_release(context->scratch_arenas[0]);
}

auto dk::thread_context_select(ThreadContext *context) noexcept -> void {
	local_thread_context = context;
}

auto dk::thread_context_selected() noexcept -> ThreadContext * {
	return local_thread_context;
}

auto dk::thread_context_get_scratch(Arena **conflicts, u64 count) noexcept -> Arena * {
	ThreadContext *const context = thread_context_selected();
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

auto dk::thread_context_lane_context_select(LaneContext context) noexcept -> LaneContext {
	ThreadContext *const thread_context = thread_context_selected();
	LaneContext const prev = thread_context->lane_context;
	thread_context->lane_context = context;
	return prev;
}

// https://github.com/EpicGames/raddebugger/blob/66bca76eddcd40e3ebf9377c9d65f92ec689d620/src/base/base_thread_context.c#L97
auto dk::thread_context_lane_barrier_wait(void *ptr, u64 size, u64 src_lane_idx) noexcept -> void {
	ZoneScoped;
	ThreadContext *const thread_context = thread_context_selected();

	//~ Dedrick: Want broadcast: Copy to broadcast memory on source lane.
	u64 const broadcast_size_clamped = min(size, sizeof(*thread_context->lane_context.broadcast_memory));
	if (ptr != nullptr && lane_idx() == src_lane_idx) {
		std::memcpy(thread_context->lane_context.broadcast_memory, ptr, broadcast_size_clamped);
	}

	//~ Dedrick: Barrier.
	barrier_wait(thread_context->lane_context.barrier);

	//~ Dedrick: Want broadcast: Copy from broadcast memory on destination lanes.
	if (ptr != nullptr && lane_idx() != src_lane_idx) {
		std::memcpy(ptr, thread_context->lane_context.broadcast_memory, broadcast_size_clamped);
	}

	//~ Dedrick: Want broadcast: Barrier on all lanes.
	if (ptr != nullptr) {
		barrier_wait(thread_context->lane_context.barrier);
	}
}

// https://github.com/EpicGames/raddebugger/blob/66bca76eddcd40e3ebf9377c9d65f92ec689d620/src/base/base_math.c#L838
auto dk::lane_range_from_task_count(u64 lane_idx, u64 lane_count, u64 task_count) noexcept -> LaneRange {
	u64 const tasks_per_lane = task_count / lane_count;
	u64 const leftover_tasks_count = task_count - tasks_per_lane * lane_count;
	u64 const leftover_tasks_before_this_lane_count = min(lane_idx, leftover_tasks_count);
	u64 const lane_base_idx = lane_idx * tasks_per_lane + leftover_tasks_before_this_lane_count;
	u64 const lane_base_idx_clamped = min(lane_base_idx, task_count);
	u64 const lane_opl_idx = lane_base_idx_clamped + tasks_per_lane + (lane_idx < leftover_tasks_count ? 1 : 0);
	u64 const lane_opl_idx_clamped = min(lane_opl_idx, task_count);
	LaneRange const result = { lane_base_idx_clamped, lane_opl_idx_clamped };
	return result;
}
