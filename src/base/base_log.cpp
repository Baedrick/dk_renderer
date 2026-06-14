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
			if (arena != nullptr && frame->node_count > 0) {
				result.count = frame->node_count;
				result.entries = arena_push_array<LogEntry>(arena, result.count);
				result.string.size = frame->total_string_size;
				u8 *str = arena_push_array<u8>(arena, result.string.size);
				result.string.data = str;
				for (u32 k = 0; k < LOG_KIND_COUNT; ++k) {
					result.kind_counts[k] = frame->kind_counts[k];
					result.kind_indices[k] = arena_push_array<u64>(arena, result.kind_counts[k]);
				}
				u64 write_offset = 0;
				u64 write_idx = 0;
				u64 kind_write_idx[LOG_KIND_COUNT] = {};
				for (LogNode const *node = frame->first; node != nullptr; node = node->next) {
					std::memcpy(str + write_offset, node->string.data, node->string.size);
					LogEntry *entry = &result.entries[write_idx];
					entry->offset = write_offset;
					entry->size = static_cast<u32>(node->string.size);
					entry->kind = node->kind;
					result.kind_indices[node->kind][kind_write_idx[node->kind]++] = write_idx;
					write_offset += node->string.size;
					write_idx += 1;
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
		LogNode *const node = arena_push<LogNode>(local_log_context->arena);
		node->string = str8_copy(local_log_context->arena, str);
		node->kind = kind;
		forward_list_queue_push(&frame->first, &frame->last, node);
		frame->node_count += 1;
		frame->total_string_size += str.size;
		frame->kind_counts[kind] += 1;
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
