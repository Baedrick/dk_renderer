// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#include <processthreadsapi.h>
#include <bcrypt.h>

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

	enum PLT_W32_EntityKind {
		PLT_W32_ENTITY_NULL = 0,
		PLT_W32_ENTITY_THREAD
	};

	struct PLT_W32_Entity {
		PLT_W32_Entity *next;
		PLT_W32_EntityKind kind;
		union {
			PLT_W32_Thread thread;
		};
	};

	struct PLT_W32_Context {
		PLT_SystemInfo system_info;
		PLT_ProcessInfo process_info;
		u64 perf_frequency;

		CRITICAL_SECTION entity_mutex;
		Arena *entity_arena;
		PLT_W32_Entity *entity_free;
	} plt_w32_context;

	auto plt_w32_file_flags_from_dw_file_attributes(DWORD dw_file_attributes) noexcept -> PLT_FileFlags {
		PLT_FileFlags flags = PLT_FILE_FLAG_NONE;
		if ((dw_file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			flags |= PLT_FILE_FLAG_DIRECTORY;
		}
		return flags;
	}

	auto plt_w32_entity_alloc(PLT_W32_EntityKind kind) noexcept -> PLT_W32_Entity * {
		PLT_W32_Entity *result = nullptr;
		EnterCriticalSection(&plt_w32_context.entity_mutex);
		{
			result = plt_w32_context.entity_free;
			if (result == nullptr) {
				forward_list_stack_pop(&plt_w32_context.entity_free);
			} else {
				result = arena_push<PLT_W32_Entity>(plt_w32_context.entity_arena);
			}
			std::memset(result, 0, sizeof(PLT_W32_Entity));
		}
		LeaveCriticalSection(&plt_w32_context.entity_mutex);
		result->kind = kind;
		return result;
	}

	auto plt_w32_entity_release(PLT_W32_Entity *entity) noexcept -> void {
		entity->kind = PLT_W32_ENTITY_NULL;
		EnterCriticalSection(&plt_w32_context.entity_mutex);
		forward_list_stack_push(&plt_w32_context.entity_free, entity);
		LeaveCriticalSection(&plt_w32_context.entity_mutex);
	}

	auto plt_w32_thread_entry(void *params) noexcept -> DWORD {
		(void)params;
		DK_ASSERT_ALWAYS(false); // function not implemented
		return 0;
	}
}

auto dk::plt_handle_invalid() noexcept -> PLT_Handle {
	return { reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE) };
}

auto dk::plt_get_system_info() noexcept -> PLT_SystemInfo * {
	return &plt_w32_context.system_info;
}

auto dk::plt_get_process_info() noexcept -> PLT_ProcessInfo * {
	return &plt_w32_context.process_info;
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
	PLT_Handle result = plt_handle_invalid();

	DWORD access_flags = 0;
	DWORD creation_disposition = OPEN_EXISTING;
	SECURITY_ATTRIBUTES security_attributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
	if (flags & PLT_ACCESS_FLAG_READ) { access_flags |= GENERIC_READ; }
	if (flags & PLT_ACCESS_FLAG_WRITE) { access_flags |= GENERIC_WRITE; }
	if (flags & PLT_ACCESS_FLAG_WRITE) { creation_disposition = CREATE_ALWAYS; }
	if (flags & PLT_ACCESS_FLAG_APPEND) { creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }

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
	// TODO(Dedrick): Append to errors.
	scratch_end(scratch);
	return result;
}

auto dk::plt_file_close(PLT_Handle file) noexcept -> void {
	if (file == plt_handle_invalid()) {
		return;
	}
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	DK_ASSERT(handle != INVALID_HANDLE_VALUE);
	CloseHandle(handle);
}

auto dk::plt_file_read(PLT_Handle file, u64 begin, u64 end, void *out_data) noexcept -> u64 {
	if (file == plt_handle_invalid()) {
		return 0;
	}
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	u8 *dst = static_cast<u8 *>(out_data);

	// NOTE(Dedrick): Clamp range by file size.
	u64 size = 0;
	GetFileSizeEx(handle, reinterpret_cast<LARGE_INTEGER *>(&size));
	u64 const clamped_begin = min<u64>(begin,size);
	u64 const clamped_end = min<u64>(end, size);
	u64 const to_read = clamped_end > clamped_begin ? clamped_end - clamped_begin : 0;

	u64 total_read_size = 0;
	u64 current_offset = clamped_begin;
	while (total_read_size < to_read) {
		u64 const remaining = to_read - total_read_size;
		DWORD const read_size = static_cast<DWORD>(min<u64>(remaining, 0xFFFFFFFF));
		// NOTE(Dedrick): Use overlapped structure reads to avoid
		// calls to SetFilePointer (syscall) and is multithreading safe.
		DWORD bytes_read = 0;
		OVERLAPPED overlapped = {};
		overlapped.Offset = static_cast<DWORD>(current_offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = static_cast<DWORD>((current_offset >> 32) & 0xFFFFFFFF);
		ReadFile(handle, dst + total_read_size, read_size, &bytes_read, &overlapped);
		current_offset += bytes_read;
		total_read_size += bytes_read;
		if (bytes_read != read_size) {
			break;
		}
	}
	return total_read_size;
}

auto dk::plt_file_write(PLT_Handle file, u64 begin, u64 end, void const *data) noexcept -> u64 {
	if (file == plt_handle_invalid()) {
		return 0;
	}
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	u8 const *src = static_cast<u8 const *>(data);

	u64 const to_write = end > begin ? end - begin : 0;

	u64 total_write_size = 0;
	u64 current_offset = begin;
	while (total_write_size < to_write) {
		u64 const remaining = to_write - total_write_size;
		DWORD const write_size = static_cast<DWORD>(min<u64>(remaining, mega_bytes(1)));
		// NOTE(Dedrick): Use overlapped structure reads to avoid
		// calls to SetFilePointer (syscall) and is multithreading safe.
		DWORD bytes_written = 0;
		OVERLAPPED overlapped = {};
		overlapped.Offset = static_cast<DWORD>(current_offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = static_cast<DWORD>((current_offset >> 32) & 0xFFFFFFFF);
		if (!WriteFile(handle, src + total_write_size, write_size, &bytes_written, &overlapped)) {
			break;
		}
		total_write_size += bytes_written;
		current_offset += bytes_written;
	}
	return total_write_size;
}

auto dk::plt_attributes_from_file(PLT_Handle file) noexcept -> PLT_FileAttributes {
	if (file == plt_handle_invalid()) {
		PLT_FileAttributes result = {};
		return result;
	}
	PLT_FileAttributes attr = {};
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	BY_HANDLE_FILE_INFORMATION info = {};
	BOOL const info_good = GetFileInformationByHandle(handle, &info);
	if (info_good) {
		u64 const size_lo = info.nFileSizeLow;
		u64 const size_hi = info.nFileSizeHigh;
		attr.size = size_hi | (size_lo << 32);
		attr.flags = plt_w32_file_flags_from_dw_file_attributes(info.dwFileAttributes);
	}
	return attr;
}

auto dk::plt_now_microseconds() noexcept -> u64 {
	u64 result = 0;
	LARGE_INTEGER ticks = {};
	if (QueryPerformanceCounter(&ticks)) {
		result = (ticks.QuadPart * 1'000'000) / plt_w32_context.perf_frequency;
	}
	return result;
}

auto dk::plt_sleep(u64 milliseconds) noexcept -> void {
	Sleep(static_cast<DWORD>(milliseconds));
}

auto dk::plt_set_thread_name(String8 name) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HRESULT const hr = SetThreadDescription(
		GetCurrentThread(),
		reinterpret_cast<WCHAR const *>(name.data)
	);
	DK_ASSERT(SUCCEEDED(hr));
	scratch_end(scratch);
}

auto dk::plt_thread_launch(PLT_ThreadFunction *func, void *params) noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	PLT_W32_Entity *const entity = plt_w32_entity_alloc(PLT_W32_ENTITY_THREAD);
	if (entity != nullptr) {
		entity->thread.func = func;
		entity->thread.params = params;
		entity->thread.handle = CreateThread(nullptr, 0, plt_w32_thread_entry, entity, 0, &entity->thread.tid);
		result.v = reinterpret_cast<uintptr_t>(entity);
	}
	return result;
}

auto dk::plt_thread_join(PLT_Handle thread) noexcept -> void {
	if (thread == plt_handle_invalid()) {
		return;
	}
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(thread.v);
	if (entity->thread.handle != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(entity->thread.handle, INFINITE);
		CloseHandle(entity->thread.handle);
	}
	plt_w32_entity_release(entity);
}

auto dk::plt_thread_detach(PLT_Handle thread) noexcept -> void {
	if (thread == plt_handle_invalid()) {
		return;
	}
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(thread.v);
	if (entity->thread.handle != INVALID_HANDLE_VALUE) {
		CloseHandle(entity->thread.handle);
	}
	plt_w32_entity_release(entity);
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
		dk::plt_w32_context.perf_frequency = 1;
		LARGE_INTEGER freq{};
		if (QueryPerformanceFrequency(&freq)) {
			dk::plt_w32_context.perf_frequency = freq.QuadPart;
		}
	}
	{
		SYSTEM_INFO sys_info = {};
		GetSystemInfo(&sys_info);

		dk::PLT_SystemInfo *info = &dk::plt_w32_context.system_info;
		info->logical_processor_count = static_cast<dk::u32>(sys_info.dwNumberOfProcessors);
		info->page_size = sys_info.dwPageSize;
	}
	{
		dk::PLT_ProcessInfo *info = &dk::plt_w32_context.process_info;
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
