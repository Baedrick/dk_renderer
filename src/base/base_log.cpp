// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	thread_local LogContext *local_log_context = nullptr;
}

dk::LogScope::LogScope(LogKind kind, String8 str) noexcept
	: kind_(kind) {
	log_msgf(kind_, "%.*s: {\n", DK_STR8_VARG(str));
}

dk::LogScope::~LogScope() {
	log_msgf(kind_, "}\n");
}

auto dk::log_alloc() noexcept -> LogContext * {
	ArenaParams constexpr params = {
		.reserve_size = mega_bytes(1),
		.commit_size = kilo_bytes(16),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *arena = arena_alloc(&params);
	LogContext *context = arena_push<LogContext>(arena);
	context->arena = arena;
	return context;
}

auto dk::log_release(LogContext *context) noexcept -> void {
	arena_release(context->arena);
}

auto dk::log_select(LogContext *context) noexcept -> void {
	local_log_context = context;
}

auto dk::log_frame_begin() noexcept -> void {
	if (local_log_context != nullptr) {
		u64 const pos = arena_pos(local_log_context->arena);
		LogFrame *frame = arena_push<LogFrame>(local_log_context->arena);
		frame->pos = pos;
		forward_list_stack_push(&local_log_context->stack, frame);
	}
}

auto dk::log_frame_end(Arena *arena) noexcept -> LogFrameResult {
	LogFrameResult result = {};
	if (local_log_context != nullptr) {
		LogFrame *frame = local_log_context->stack;
		if (frame != nullptr) {
			forward_list_stack_pop(&local_log_context->stack);
			if (arena != nullptr) {
				for (u32 kind = LOG_KIND_INFO; kind < LOG_KIND_COUNT; ++kind) {
					TempArena const scratch = scratch_begin(&arena, 1);
					String8 const unindented_str = str8_list_join(scratch.arena, frame->strings[kind], nullptr);
					result.strings[kind] = str8_indent(arena, unindented_str);
					scratch_end(scratch);
				}
			}
			arena_pop_to(local_log_context->arena, frame->pos);
		}
	}
	return result;
}

auto dk::log_msg(LogKind kind, String8 str) noexcept -> void {
	if (local_log_context != nullptr && local_log_context->stack != nullptr) {
		String8 const copy = str8_copy(local_log_context->arena, str);
		str8_list_push(local_log_context->arena, &local_log_context->stack->strings[kind], copy);
	}
}

auto dk::log_msgfv(LogKind kind, char const *fmt, va_list args) noexcept -> void {
	if (local_log_context != nullptr) {
		TempArena const scratch = scratch_begin(nullptr, 0);
		log_msg(kind, str8fv(scratch.arena, fmt, args));
		scratch_end(scratch);
	}
}

auto dk::log_msgf(LogKind kind, char const *fmt, ...) noexcept -> void {
	if (local_log_context != nullptr) {
		va_list args;
		va_start(args, fmt);
		log_msgfv(kind, fmt, args);
		va_end(args);
	}
}
