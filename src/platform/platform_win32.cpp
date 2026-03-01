/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.
 */

#define NOMINMAX
#include <Windows.h>
#undef min
#undef max

#pragma comment(lib, "bcrypt")

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
	DK_ASSERT_ALWAYS(size <= 256); // NOTE(Dedrick): Limit of 256 to follow linux.
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

auto dk::plt_malloc(u64 size) noexcept -> void * {
	return HeapAlloc(GetProcessHeap(), 0, size);
}

auto dk::plt_free(void *ptr) noexcept -> void {
	HeapFree(GetProcessHeap(), 0, ptr);
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

	// TODO(Dedrick): Parse CommandLineW, pass it to entry point.
	int const result = entry_point(0, nullptr);
	return result;
}
