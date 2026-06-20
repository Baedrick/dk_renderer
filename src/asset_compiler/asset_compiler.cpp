// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::asc_entry_point(CmdLine *cmd_line) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	u64 const threads_count = get_system_info()->logical_processor_count;
	Thread *threads = arena_push_array<Thread>(scratch.arena, threads_count);
	ASC_ThreadParams *threads_params = arena_push_array<ASC_ThreadParams>(scratch.arena, threads_count);
	Barrier const barrier = barrier_alloc(threads_count);
	u64 broadcast_value = 0;
	for (u64 idx = 0; idx < threads_count; ++idx) {
		threads_params[idx].cmd_line = cmd_line;
		threads_params[idx].lane_context.lane_idx = idx;
		threads_params[idx].lane_context.lane_count = threads_count;
		threads_params[idx].lane_context.barrier = barrier;
		threads_params[idx].lane_context.broadcast_memory = &broadcast_value;
		threads[idx] = thread_launch(asc_thread_entry_point, &threads_params[idx]);
	}
	for (u64 i = 0; i < threads_count; ++i) {
		thread_join(threads[i]);
	}
	scratch_end(scratch);
}

auto dk::asc_thread_entry_point(void *p) noexcept -> void {
	//~ Dedrick: Set up thread state.
	ASC_ThreadParams *params = static_cast<ASC_ThreadParams *>(p);
	CmdLine *cmd_line = params->cmd_line;
	(void)cmd_line;
	LaneContext lane_context = params->lane_context;
	set_thread_namef("compiler_thread_%llu", lane_context.lane_idx);
	lane_context_select(lane_context);
	Arena *arena = arena_alloc();
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	// TODO(Dedrick): Start parsing, figure out file format and write binary

	//~ Dedrick: Collect logs
	LogFrameResult const log_frame = log_frame_end(arena);
	if (lane_idx() == 0) {
		std::fwrite(log_frame.string.data, log_frame.string.size, 1, stdout);
	}
	lane_sync();
}
