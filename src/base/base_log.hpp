// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum LogKind {
		LOG_KIND_INFO = 0,
		LOG_KIND_USER_ERROR,
		LOG_KIND_COUNT
	};

	struct LogFrame {
		LogFrame *next;
		u64 pos;
		String8List strings[LOG_KIND_COUNT];
	};

	struct LogFrameResult {
		String8 strings[LOG_KIND_COUNT];
	};

	struct LogContext {
		Arena *arena;
		LogFrame *stack;
	};

	struct LogScope {
		LogScope(String8 str) noexcept;
		~LogScope();
		LogScope(LogScope const &) = delete;
		auto operator=(LogScope const &) -> LogScope & = delete;
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

#define DK_LOG_INFO(s)          dk::log_msg(dk::LOG_KIND_INFO, (s))
#define DK_LOG_INFOF(...)       dk::log_msgf(dk::LOG_KIND_INFO, __VA_ARGS__)
#define DK_LOG_USER_ERROR(s)    dk::log_msg(dk::LOG_KIND_USER_ERROR, (s))
#define DK_LOG_USER_ERRORF(...) dk::dk_log_msgf(dk::LOG_KIND_USER_ERROR, __VA_ARGS__)

#define DK_LOG_INFO_SCOPE(s)    dk::LogScope DK_GLUE(_log_scope_, __LINE__)(s)
