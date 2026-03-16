// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	thread_local LogContext *log_context_local = nullptr;
}

auto dk::log_alloc() noexcept -> LogContext * {
	Arena *arena = arena_alloc(&ARENA_DEFAULT_PARAMS);
	LogContext *context = arena_push<LogContext>(arena);
	context->arena = arena;
	return context;
}

auto dk::log_release(LogContext *context) noexcept -> void {
	arena_release(context->arena);
}

auto dk::log_select(LogContext *context) noexcept -> void {
	log_context_local = context;
}

auto dk::log_frame_begin() noexcept -> void {
	if (log_context_local != nullptr) {
		u64 const pos = arena_pos(log_context_local->arena);
		LogFrame *frame = arena_push<LogFrame>(log_context_local->arena);
		frame->pos = pos;
		forward_list_stack_push(&log_context_local->stack, frame);
	}
}

auto dk::log_frame_end(Arena *arena) noexcept -> LogFrameResult {
	LogFrameResult result = {};
	if (log_context_local != nullptr) {
		LogFrame *frame = log_context_local->stack;
		if (frame != nullptr) {
			forward_list_stack_pop(&log_context_local->stack);
			if (arena != nullptr) {
				for (u32 kind = LOG_KIND_INFO; kind < LOG_KIND_COUNT; ++kind) {
					TempArena const scratch = scratch_begin(&arena, 1);
					String8 const unindented_str = str8_list_join(scratch.arena, frame->strings[kind], nullptr);
					result.strings[kind] = str8_indent(arena, unindented_str);
					scratch_end(scratch);
				}
			}
			arena_pop_to(log_context_local->arena, frame->pos);
		}
	}
	return result;
}

auto dk::log_msg(LogKind kind, String8 str) noexcept -> void {
	if (log_context_local != nullptr && log_context_local->stack != nullptr) {
		String8 const copy = str8_copy(log_context_local->arena, str);
		str8_list_push(log_context_local->arena, &log_context_local->stack->strings[kind], copy);
	}
}

auto dk::log_msgf(LogKind kind, char const *fmt, ...) noexcept -> void {
	if (log_context_local != nullptr) {
		TempArena const scratch = scratch_begin(nullptr, 0);
		va_list args;
		va_start(args, fmt);
		log_msg(kind, str8fv(scratch.arena, fmt, args));
		va_end(args);
		scratch_end(scratch);
	}
}
