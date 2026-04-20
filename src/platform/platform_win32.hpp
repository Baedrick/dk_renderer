// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <shellapi.h>
#include <ShlObj_core.h>
#include <processthreadsapi.h>
#include <bcrypt.h>

#pragma comment(lib, "ole32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "bcrypt")

namespace dk {
	struct PLT_W32_Thread {
		PLT_ThreadFunction *func;
		void *params;
		HANDLE handle;
		DWORD tid;
	};

	struct PLT_W32_Mutex {
		CRITICAL_SECTION handle;
	};

	struct PLT_W32_RWMutex {
		SRWLOCK handle;
	};

	struct PLT_W32_ConditionalVariable {
		CONDITION_VARIABLE handle;
	};

	enum PLT_W32_EntityKind {
		PLT_W32_ENTITY_NULL = 0,
		PLT_W32_ENTITY_THREAD,
		PLT_W32_ENTITY_MUTEX,
		PLT_W32_ENTITY_RW_MUTEX,
		PLT_W32_ENTITY_CONDITIONAL_VARIABLE
	};

	struct PLT_W32_Entity {
		PLT_W32_Entity *next;
		PLT_W32_EntityKind kind;
		union {
			PLT_W32_Thread thread;
			PLT_W32_Mutex mutex;
			PLT_W32_RWMutex rw_mutex;
			PLT_W32_ConditionalVariable cond_var;
		};
	};

	FILEOPENDIALOGOPTIONS constexpr PLT_W32_FILE_DIALOG_COMMON_FLAGS =
		FOS_PATHMUSTEXIST
		| FOS_FORCEFILESYSTEM
		| FOS_NOCHANGEDIR;

	struct PLT_W32_Context {
		PLT_SystemInfo system_info;
		PLT_ProcessInfo process_info;
		u64 perf_frequency;

		CRITICAL_SECTION entity_mutex;
		Arena *entity_arena;
		PLT_W32_Entity *entity_free;
	};

	extern PLT_W32_Context plt_w32_context;

	auto plt_w32_file_flags_from_dw_file_attributes(DWORD dw_file_attributes) noexcept -> PLT_FileFlags;

	auto plt_w32_entity_alloc(PLT_W32_EntityKind kind) noexcept -> PLT_W32_Entity *;
	auto plt_w32_entity_release(PLT_W32_Entity *entity) noexcept -> void;

	auto plt_w32_create_filter_specs(Arena *arena, PLT_FileDialogFilter const *filters, u64 filter_count, UINT *out_count) -> COMDLG_FILTERSPEC *;

	auto plt_w32_main_thread_entry(int argc, WCHAR **wargv) noexcept -> int;
	auto plt_w32_thread_entry(void *params) noexcept -> DWORD;
}
