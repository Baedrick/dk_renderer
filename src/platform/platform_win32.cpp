// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#undef min
#undef max

#pragma comment(lib, "bcrypt")
#pragma comment(lib, "shell32")

namespace dk {
	struct PLT_Win32_Context {
		PLT_SystemInfo system_info;
		PLT_ProcessInfo process_info;
		u64 perf_frequency;
	};
	PLT_Win32_Context plt_win32_context;
}

auto dk::plt_get_system_info() noexcept -> PLT_SystemInfo * {
	return &plt_win32_context.system_info;
}

auto dk::plt_get_process_info() noexcept -> PLT_ProcessInfo * {
	return &plt_win32_context.process_info;
}

auto dk::plt_get_entropy(void *data, u64 size) noexcept -> void {
	DK_ASSERT_ALWAYS(size <= 256); // NOTE(Dedrick): Limit of 256 bytes to follow linux.
	BCryptGenRandom(
		nullptr,
		static_cast<u8 *>(data),
		static_cast<u32>(size),
		BCRYPT_USE_SYSTEM_PREFERRED_RNG
	);
}

auto dk::plt_abort(s32 code) noexcept -> void {
	ExitProcess(static_cast<u32>(code));
}

auto dk::plt_reserve(u64 size) noexcept -> void * {
	return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
}

auto dk::plt_commit(void *ptr, u64 size) noexcept -> b8 {
	return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
}

auto dk::plt_decommit(void *ptr, u64 size) noexcept -> b8 {
	return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

auto dk::plt_release(void *ptr, u64 size) noexcept -> void {
	// NOTE(Dedrick): Size not used, not necessary on Windows.
	(void)size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}

auto dk::plt_file_open(String8 path, PLT_AccessFlags flags) noexcept -> PLT_Handle {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const path16 = str16_from_8(scratch.arena, path);

	DWORD access_flags = 0;
	DWORD creation_disposition = OPEN_EXISTING;
	SECURITY_ATTRIBUTES security_attributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
	if (flags & PLT_ACCESS_FLAG_READ) { access_flags |= GENERIC_READ; }
	if (flags & PLT_ACCESS_FLAG_WRITE) { access_flags |= GENERIC_WRITE; }
	if (flags & PLT_ACCESS_FLAG_WRITE) { creation_disposition = CREATE_ALWAYS; }
	if (flags & PLT_ACCESS_FLAG_APPEND) { creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }

	PLT_Handle result = plt_handle_invalid();
	HANDLE const file = CreateFileW(
		reinterpret_cast<WCHAR const *>(path16.data),
		access_flags,
		0,
		&security_attributes,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (file != INVALID_HANDLE_VALUE) {
		result.v = reinterpret_cast<uintptr_t>(file);
	}
	scratch_end(scratch);
	return result;
}

auto dk::plt_file_close(PLT_Handle file) noexcept -> void {

}

auto dk::plt_file_read(PLT_Handle file, u64 begin, u64 end, void *out_data) noexcept -> u64 {

}

auto dk::plt_file_write(PLT_Handle file, u64 begin, u64 end, void const *data) noexcept -> u64 {

}

extern auto entry_point(int argc, char **argv) noexcept -> int;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd) {
	(void)instance;
	(void)prev_instance;
	(void)lp_cmd_line;
	(void)n_show_cmd;

	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		std::FILE *fp = nullptr;
		(void)freopen_s(&fp, "CONOUT$", "w", stdout);
		(void)freopen_s(&fp, "CONOUT$", "w", stderr);
		(void)freopen_s(&fp, "CONIN$", "r", stdin);
	}

	{
		dk::plt_win32_context.perf_frequency = 1;
		LARGE_INTEGER freq{};
		if (QueryPerformanceFrequency(&freq)) {
			dk::plt_win32_context.perf_frequency = freq.QuadPart;
		}
	}
	{
		SYSTEM_INFO sys_info = {};
		GetSystemInfo(&sys_info);

		dk::PLT_SystemInfo *info = &dk::plt_win32_context.system_info;
		info->logical_processor_count = static_cast<dk::u32>(sys_info.dwNumberOfProcessors);
		info->page_size = sys_info.dwPageSize;
	}
	{
		dk::PLT_ProcessInfo *info = &dk::plt_win32_context.process_info;
		info->pid = GetCurrentProcessId();
	}

	dk::ThreadContext *const thread_context = dk::thread_context_alloc();
	dk::thread_context_select(thread_context);

	int argc = 0;
	LPWSTR *argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argvw == nullptr) {
		return 0;
	}

	// TODO(Dedrick): Clean this up. Use string functions.
	dk::ArenaParams const args_arena_params = {
		.reserve_size = dk::mega_bytes(1),
		.commit_size = dk::kilo_bytes(32)
	};
	dk::Arena *args_arena = dk::arena_alloc(&args_arena_params);

	char **argv = dk::arena_push_array<char *>(args_arena, argc + 1);
	for (int i = 0; i < argc; ++i) {
		dk::String16 const arg16 = dk::str16_cstring(reinterpret_cast<dk::u16 const *>(argvw[i]));
		dk::String8 const arg8 = dk::str8_from_16(args_arena, arg16);
		argv[i] = const_cast<char *>(reinterpret_cast<char const *>(arg8.data));
	}
	argv[argc] = nullptr;
	LocalFree(static_cast<void *>(argvw));

	int const result = entry_point(argc, argv);

	dk::arena_release(args_arena);

	dk::thread_context_select(nullptr);
	dk::thread_context_release(thread_context);

	return result;
}
