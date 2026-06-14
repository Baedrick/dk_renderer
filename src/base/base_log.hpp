// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum LogKind {
		LOG_KIND_INFO = 0,
		LOG_KIND_ERROR,
		LOG_KIND_COUNT
	};

	struct LogNode {
		LogNode *next;
		String8 string;
		LogKind kind;
	};

	struct LogFrame {
		LogFrame *next;
		u64 arena_start_pos;
		LogNode *first;
		LogNode *last;
		u64 node_count;
		u64 total_string_size;
		u64 kind_counts[LOG_KIND_COUNT];
	};

	struct LogEntry {
		u64 offset;
		u32 size;
		LogKind kind;
	};

	struct LogFrameResult {
		String8 string;
		LogEntry *entries;
		u64 count;
		u64 *kind_indices[LOG_KIND_COUNT];
		u64 kind_counts[LOG_KIND_COUNT];
	};

	struct LogContext {
		Arena *arena;
		LogFrame *stack;
	};

	struct LogScope {
		LogScope(LogKind kind, String8 str) noexcept;
		~LogScope();
		LogScope(LogScope const &) = delete;
		auto operator=(LogScope const &) -> LogScope & = delete;
		LogKind kind_;
	};

	auto log_alloc() noexcept -> LogContext *;
	auto log_release(LogContext *context) noexcept -> void;
	auto log_select(LogContext *context) noexcept -> void;

	auto log_frame_begin() noexcept -> void;
	auto log_frame_end(Arena *arena) noexcept -> LogFrameResult;

	auto log_msg(LogKind kind, String8 str) noexcept -> void;
	auto log_msgfv(LogKind kind, char const *fmt, va_list args) noexcept -> void;
	auto log_msgf(LogKind kind, char const *fmt, ...) noexcept -> void;
}

#define DK_LOG_INFO(s)       dk::log_msg(dk::LOG_KIND_INFO, (s))
#define DK_LOG_INFOF(...)    dk::log_msgf(dk::LOG_KIND_INFO, __VA_ARGS__)
#define DK_LOG_ERROR(s)      dk::log_msg(dk::LOG_KIND_ERROR, (s))
#define DK_LOG_ERRORF(...)   dk::log_msgf(dk::LOG_KIND_ERROR, __VA_ARGS__)

#define DK_LOG_INFO_SCOPE(s) dk::LogScope DK_GLUE(_log_scope_, __LINE__)(dk::LOG_KIND_INFO, (s))
