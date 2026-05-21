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
		frame->arena_start_pos = pos;
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
				for (LogEntry const *node = frame->list.first; node != nullptr; node = node->next) {
					LogEntry *const entry = arena_push<LogEntry>(arena);
					entry->string = str8_copy(arena, node->string);
					entry->kind = node->kind;
					forward_list_queue_push(&result.list.first, &result.list.last, entry);
					result.list.count += 1;
					LogEntryList *const kind_list = &result.kind_lists[entry->kind];
					forward_list_queue_push<LogEntry, &LogEntry::kind_next>(&kind_list->first, &kind_list->last, entry);
					kind_list->count += 1;
				}
			}
			arena_pop_to(local_log_context->arena, frame->arena_start_pos);
		}
	}
	return result;
}

auto dk::log_msg(LogKind kind, String8 str) noexcept -> void {
	if (local_log_context != nullptr && local_log_context->stack != nullptr) {
		LogFrame *const frame = local_log_context->stack;
		LogEntry *const entry = arena_push<LogEntry>(local_log_context->arena);
		entry->string = str8_copy(local_log_context->arena, str);
		entry->kind = kind;

		forward_list_queue_push(&frame->list.first, &frame->list.last, entry);
		frame->list.count += 1;

		LogEntryList *const kind_list = &frame->kind_lists[kind];
		forward_list_queue_push<LogEntry, &LogEntry::kind_next>(&kind_list->first, &kind_list->last, entry);
		kind_list->count += 1;
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
