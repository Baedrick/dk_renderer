// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct LaneRange {
		u64 begin;
		u64 end;
	};

	struct LaneContext {
		u64 lane_idx;
		u64 lane_count;
		Barrier barrier;
		u64 *broadcast_memory;
	};

	struct ThreadContext {
		Arena *scratch_arenas[2];
		LaneContext lane_context;
	};

	auto thread_context_alloc() noexcept -> ThreadContext *;
	auto thread_context_release(ThreadContext *context) noexcept -> void;
	auto thread_context_select(ThreadContext *context) noexcept -> void;
	auto thread_context_selected() noexcept -> ThreadContext *;

	auto thread_context_get_scratch(Arena **conflicts, u64 count) noexcept -> Arena *;
	auto scratch_begin(Arena **conflicts, u64 count) noexcept -> TempArena;
	auto scratch_end(TempArena scratch) noexcept -> void;

	auto thread_context_lane_context_select(LaneContext context) noexcept -> LaneContext;
	auto thread_context_lane_barrier_wait(void *ptr, u64 size, u64 src_lane_idx) noexcept -> void;
	auto lane_range_from_task_count(u64 lane_idx, u64 lane_count, u64 task_count) noexcept -> LaneRange;

	inline auto lane_idx() noexcept -> u64 { return thread_context_selected()->lane_context.lane_idx; }
	inline auto lane_count() noexcept -> u64 { return thread_context_selected()->lane_context.lane_count; }
	inline auto lane_from_task_idx(u64 idx) noexcept -> u64 { return idx % lane_count(); }
	inline auto lane_context_select(LaneContext context) noexcept -> LaneContext { return thread_context_lane_context_select(context); }
	inline auto lane_sync() noexcept -> void { thread_context_lane_barrier_wait(nullptr, 0, 0); }
	template <typename T>
	auto lane_sync_broadcast(T *ptr, u64 src_lane_idx) noexcept -> void { thread_context_lane_barrier_wait(ptr, sizeof(T), src_lane_idx); }
	inline auto lane_range(u64 count) noexcept -> LaneRange { return lane_range_from_task_count(lane_idx(), lane_count(), count); }
}
