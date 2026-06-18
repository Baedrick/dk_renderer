// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef NOMINMAX
#	define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <processthreadsapi.h>
#include <bcrypt.h>
#include <ShlObj_core.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "bcrypt")

namespace dk {
	struct W32_Thread {
		ThreadFunction *func;
		void *params;
		HANDLE handle;
		DWORD tid;
	};

	struct W32_Mutex {
		CRITICAL_SECTION handle;
	};

	struct W32_RWMutex {
		SRWLOCK handle;
	};

	struct W32_ConditionalVariable {
		CONDITION_VARIABLE handle;
	};

	struct W32_DirIter {
		HANDLE handle;
		WIN32_FIND_DATAW find_data;
		DirIterFlags flags;
	};

	enum W32_EntityKind {
		W32_ENTITY_NULL = 0,
		W32_ENTITY_THREAD,
		W32_ENTITY_MUTEX,
		W32_ENTITY_RW_MUTEX,
		W32_ENTITY_CONDITIONAL_VARIABLE,
		W32_ENTITY_DIR_ITER
	};

	struct W32_Entity {
		W32_Entity *next;
		W32_EntityKind kind;
		union {
			W32_Thread thread;
			W32_Mutex mutex;
			W32_RWMutex rw_mutex;
			W32_ConditionalVariable cond_var;
			W32_DirIter dir_iter;
		};
	};

	struct W32_Context {
		Arena *arena;

		SystemInfo system_info;
		ProcessInfo process_info;
		u64 perf_frequency;

		CRITICAL_SECTION entity_mutex;
		Arena *entity_arena;
		W32_Entity *entity_free;
	};

	extern W32_Context w32_context;

	auto w32_file_flags_from_dw_file_attributes(DWORD dw_file_attributes) noexcept -> FileFlags;

	auto w32_sleep_ms_from_end_time_us(u64 end_time_us) noexcept -> DWORD;

	auto w32_entity_alloc(W32_EntityKind kind) noexcept -> W32_Entity *;
	auto w32_entity_release(W32_Entity *entity) noexcept -> void;

	auto w32_main_thread_entry_caller(int argc, WCHAR **wargv) noexcept -> int;
	auto w32_thread_entry_caller(void *params) noexcept -> DWORD;
}
