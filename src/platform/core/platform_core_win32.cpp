// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::PLT_W32_Context dk::plt_w32_context;

auto dk::plt_w32_file_flags_from_dw_file_attributes(DWORD dw_file_attributes) noexcept -> PLT_FileFlags {
	PLT_FileFlags flags = PLT_FILE_FLAG_NONE;
	if ((dw_file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		flags |= PLT_FILE_FLAG_DIRECTORY;
	}
	return flags;
}

auto dk::plt_w32_sleep_ms_from_end_time_us(u64 end_time_us) noexcept -> DWORD {
	if (end_time_us == U64_MAX) {
		return INFINITE;
	}
	DWORD sleep_ms = 0;
	u64 const begin_time_us = plt_now_microseconds();
	if (begin_time_us < end_time_us) {
		u64 const sleep_us = end_time_us - begin_time_us;
		sleep_ms = static_cast<DWORD>((sleep_us + 999) / 1000);
	}
	return sleep_ms;
}

auto dk::plt_w32_entity_alloc(PLT_W32_EntityKind kind) noexcept -> PLT_W32_Entity * {
	PLT_W32_Entity *result = nullptr;
	EnterCriticalSection(&plt_w32_context.entity_mutex);
	{
		result = plt_w32_context.entity_free;
		if (result != nullptr) {
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

auto dk::plt_w32_entity_release(PLT_W32_Entity *entity) noexcept -> void {
	entity->kind = PLT_W32_ENTITY_NULL;
	EnterCriticalSection(&plt_w32_context.entity_mutex);
	forward_list_stack_push(&plt_w32_context.entity_free, entity);
	LeaveCriticalSection(&plt_w32_context.entity_mutex);
}

auto dk::plt_w32_thread_entry_caller(void *params) noexcept -> DWORD {
	PLT_W32_Entity *const entity = static_cast<PLT_W32_Entity *>(params);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_THREAD);
	PLT_ThreadFunction *const func = entity->thread.func;
	void *const func_params = entity->thread.params;
	thread_entry_point(func, func_params);
	return 0;
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

auto dk::plt_shared_memory_create(u64 size, String8 name) noexcept -> PLT_Handle {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const file = CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		0,
		PAGE_READWRITE,
		static_cast<u32>((size & 0xFFFFFFFF00000000) >> 32),
		static_cast<u32>(size & 0x00000000FFFFFFFF),
		reinterpret_cast<WCHAR const *>(name16.data)
	);
	PLT_Handle result = { reinterpret_cast<uintptr_t>(file) };
	scratch_end(scratch);
	return result;
}

auto dk::plt_shared_memory_open(String8 name) noexcept -> PLT_Handle {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const file = OpenFileMappingW(
		FILE_MAP_ALL_ACCESS,
		0,
		reinterpret_cast<WCHAR const *>(name16.data)
	);
	PLT_Handle result = { reinterpret_cast<uintptr_t>(file) };
	scratch_end(scratch);
	return result;
}

auto dk::plt_shared_memory_close(PLT_Handle handle) noexcept -> void {
	HANDLE const file = reinterpret_cast<HANDLE>(handle.v);
	CloseHandle(file);
}

auto dk::plt_shared_memory_map(PLT_Handle handle, u64 begin, u64 end) noexcept -> void * {
	HANDLE const file = reinterpret_cast<HANDLE>(handle.v);
	u64 const offset = begin;
	u64 const size = end - begin;
	void *ptr = MapViewOfFile(
		file,
		FILE_MAP_ALL_ACCESS,
		static_cast<u32>((offset & 0xFFFFFFFF00000000) >> 32),
		static_cast<u32>(offset & 0x00000000FFFFFFFF),
		size
	);
	return ptr;
}

auto dk::plt_shared_memory_unmap(PLT_Handle handle, void *ptr, u64 begin, u64 end) noexcept -> void {
	// NOTE(Dedrick): Range not used, not necessary on Windows.
	(void)begin;
	(void)end;
	if (handle != plt_handle_invalid()) {
		UnmapViewOfFile(ptr);
	}
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
		attr.size = size_lo | (size_hi << 32);
		attr.flags = plt_w32_file_flags_from_dw_file_attributes(info.dwFileAttributes);
	}
	return attr;
}

auto dk::plt_make_directory(String8 path) noexcept -> b8 {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const path16 = str16_from_8(scratch.arena, path);
	b8 result = false;
	WIN32_FILE_ATTRIBUTE_DATA attributes = {};
	GetFileAttributesExW(
		reinterpret_cast<WCHAR const *>(path16.data),
		GetFileExInfoStandard,
		&attributes
	);
	if (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		result = true;
	} else if (CreateDirectoryW(reinterpret_cast<WCHAR const *>(path16.data), 0)) {
		result = true;
	}
	scratch_end(scratch);
	return result;
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

auto dk::plt_process_launch(PLT_ProcessLaunchParams const *params) noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	TempArena const scratch = scratch_begin(nullptr, 0);

	String8 cmd = {};
	{
		String8JoinParams join_params = {};
		join_params.prefix = "\""_str8;
		join_params.postfix = "\""_str8;
		join_params.separator = "\" \""_str8;
		cmd = str8_list_join(scratch.arena, params->cmd_line, &join_params);
	}

	String16 const cmd16 = str16_from_8(scratch.arena, cmd);
	String16 const dir16 = str16_from_8(scratch.arena, params->working_dir);

	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	if ((params->flags & PLT_PROCESS_LAUNCH_FLAG_NO_WINDOW) != 0) {
		creation_flags |= CREATE_NO_WINDOW;
	}

	STARTUPINFOW startup_info = {};
	startup_info.cb = sizeof(STARTUPINFOW);
	PROCESS_INFORMATION process_info = {};
	if (CreateProcessW(
			nullptr,
			reinterpret_cast<WCHAR *>(const_cast<u16 *>(cmd16.data)),
			nullptr,
			nullptr,
			FALSE,
			creation_flags,
			nullptr,
			reinterpret_cast<WCHAR *>(const_cast<u16 *>(dir16.data)),
			&startup_info,
			&process_info
		)
	) {
		result.v = reinterpret_cast<uintptr_t>(process_info.hProcess);
		CloseHandle(process_info.hThread);
	}

	scratch_end(scratch);
	return result;
}

auto dk::plt_process_join(PLT_Handle process, u64 end_time_us, u64 *out_exit_code) noexcept -> b8 {
	HANDLE const handle = reinterpret_cast<HANDLE>(process.v);
	DWORD const sleep_ms = plt_w32_sleep_ms_from_end_time_us(end_time_us);
	DWORD const result = WaitForSingleObject(handle, sleep_ms);
	b8 const process_joined = result == WAIT_OBJECT_0;
	if (process_joined) {
		if (out_exit_code != nullptr) {
			DWORD exit_code = 0;
			if (GetExitCodeProcess(handle, &exit_code)) {
				*out_exit_code = exit_code;
			}
		}
		CloseHandle(handle);
	}
	return process_joined;
}

auto dk::plt_process_kill(PLT_Handle process) noexcept -> b8 {
	HANDLE const handle = reinterpret_cast<HANDLE>(process.v);
	b8 const terminated = TerminateProcess(handle, 999);
	return terminated;
}

auto dk::plt_set_thread_name(String8 name) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HRESULT const hr = SetThreadDescription(
		GetCurrentThread(),
		reinterpret_cast<WCHAR const *>(name16.data)
	);
	DK_ASSERT(SUCCEEDED(hr));
	scratch_end(scratch);
}

auto dk::plt_thread_launch(PLT_ThreadFunction *func, void *params) noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	PLT_W32_Entity *const entity = plt_w32_entity_alloc(PLT_W32_ENTITY_THREAD);
	if (entity != nullptr) {
		// TODO(Dedrick): Error handling.
		entity->thread.func = func;
		entity->thread.params = params;
		entity->thread.handle = CreateThread(nullptr, 0, plt_w32_thread_entry_caller, entity, 0, &entity->thread.tid);
		result.v = reinterpret_cast<uintptr_t>(entity);
	}
	return result;
}

auto dk::plt_thread_join(PLT_Handle thread) noexcept -> void {
	if (thread == plt_handle_invalid()) {
		return;
	}
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(thread.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_THREAD);
	if (entity->thread.handle != nullptr) {
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
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_THREAD);
	if (entity->thread.handle != nullptr) {
		CloseHandle(entity->thread.handle);
	}
	plt_w32_entity_release(entity);
}

auto dk::plt_mutex_alloc() noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	PLT_W32_Entity *const entity = plt_w32_entity_alloc(PLT_W32_ENTITY_MUTEX);
	if (entity != nullptr) {
		InitializeCriticalSection(&entity->mutex.handle);
		result.v = reinterpret_cast<uintptr_t>(entity);
	}
	return result;
}

auto dk::plt_mutex_release(PLT_Handle mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_MUTEX);
	DeleteCriticalSection(&entity->mutex.handle);
	plt_w32_entity_release(entity);
}

auto dk::plt_mutex_scope_enter(PLT_Handle mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_MUTEX);
	EnterCriticalSection(&entity->mutex.handle);
}

auto dk::plt_mutex_scope_leave(PLT_Handle mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_MUTEX);
	LeaveCriticalSection(&entity->mutex.handle);
}

auto dk::plt_rw_mutex_alloc() noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	PLT_W32_Entity *const entity = plt_w32_entity_alloc(PLT_W32_ENTITY_RW_MUTEX);
	if (entity != nullptr) {
		InitializeSRWLock(&entity->rw_mutex.handle);
		result.v = reinterpret_cast<uintptr_t>(entity);
	}
	return result;
}

auto dk::plt_rw_mutex_release(PLT_Handle rw_mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_RW_MUTEX);
	plt_w32_entity_release(entity);
}

auto dk::plt_rw_mutex_scope_enter_w(PLT_Handle rw_mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_RW_MUTEX);
	AcquireSRWLockExclusive(&entity->rw_mutex.handle);
}

auto dk::plt_rw_mutex_scope_leave_w(PLT_Handle rw_mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_RW_MUTEX);
	ReleaseSRWLockExclusive(&entity->rw_mutex.handle);
}

auto dk::plt_rw_mutex_scope_enter_r(PLT_Handle rw_mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_RW_MUTEX);
	AcquireSRWLockShared(&entity->rw_mutex.handle);
}

auto dk::plt_rw_mutex_scope_leave_r(PLT_Handle rw_mutex) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_RW_MUTEX);
	ReleaseSRWLockShared(&entity->rw_mutex.handle);
}

auto dk::plt_cond_var_alloc() noexcept -> PLT_Handle {
	PLT_Handle result = plt_handle_invalid();
	PLT_W32_Entity *const entity = plt_w32_entity_alloc(PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
	if (entity != nullptr) {
		InitializeConditionVariable(&entity->cond_var.handle);
		result.v = reinterpret_cast<uintptr_t>(entity);
	}
	return result;
}

auto dk::plt_cond_var_release(PLT_Handle cond_var) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
	plt_w32_entity_release(entity);
}

auto dk::plt_cond_var_wait(PLT_Handle cond_var, PLT_Handle mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = plt_w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		PLT_W32_Entity *const entity_cv = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
		PLT_W32_Entity *const entity_mutex = reinterpret_cast<PLT_W32_Entity *>(mutex.v);
		DK_ASSERT(entity_mutex->kind == PLT_W32_ENTITY_MUTEX);
		result = SleepConditionVariableCS(
			&entity_cv->cond_var.handle,
			&entity_mutex->mutex.handle,
			sleep_ms
		) != FALSE;
	}
	return result;
}

auto dk::plt_cond_var_wait_rw_w(PLT_Handle cond_var, PLT_Handle rw_mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = plt_w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		PLT_W32_Entity *const entity_cv = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
		PLT_W32_Entity *const entity_rw_mutex = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
		DK_ASSERT(entity_rw_mutex->kind == PLT_W32_ENTITY_RW_MUTEX);
		result = SleepConditionVariableSRW(
			&entity_cv->cond_var.handle,
			&entity_rw_mutex->rw_mutex.handle,
			sleep_ms,
			0
		) != FALSE;
	}
	return result;
}

auto dk::plt_cond_var_wait_rw_r(PLT_Handle cond_var, PLT_Handle rw_mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = plt_w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		PLT_W32_Entity *const entity_cv = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
		PLT_W32_Entity *const entity_rw_mutex = reinterpret_cast<PLT_W32_Entity *>(rw_mutex.v);
		DK_ASSERT(entity_rw_mutex->kind == PLT_W32_ENTITY_RW_MUTEX);
		result = SleepConditionVariableSRW(
			&entity_cv->cond_var.handle,
			&entity_rw_mutex->rw_mutex.handle,
			sleep_ms,
			CONDITION_VARIABLE_LOCKMODE_SHARED
		) != FALSE;
	}
	return result;
}

auto dk::plt_cond_var_signal(PLT_Handle cond_var) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
	WakeConditionVariable(&entity->cond_var.handle);
}

auto dk::plt_cond_var_signal_all(PLT_Handle cond_var) noexcept -> void {
	PLT_W32_Entity *const entity = reinterpret_cast<PLT_W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == PLT_W32_ENTITY_CONDITIONAL_VARIABLE);
	WakeAllConditionVariable(&entity->cond_var.handle);
}

auto dk::plt_semaphore_alloc(u32 initial_count, u32 max_count, String8 name) noexcept -> PLT_Handle {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const handle = CreateSemaphoreW(nullptr, initial_count, max_count, reinterpret_cast<WCHAR const *>(name16.data));
	PLT_Handle result = { reinterpret_cast<uintptr_t>(handle) };
	scratch_end(scratch);
	return result;
}

auto dk::plt_semaphore_release(PLT_Handle semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	CloseHandle(handle);
}

auto dk::plt_semaphore_open(String8 name) noexcept -> PLT_Handle {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const handle = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, reinterpret_cast<WCHAR const *>(name16.data));
	PLT_Handle result = { reinterpret_cast<uintptr_t>(handle) };
	scratch_end(scratch);
	return result;
}

auto dk::plt_semaphore_close(PLT_Handle semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	CloseHandle(handle);
}

auto dk::plt_semaphore_wait(PLT_Handle semaphore, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = plt_w32_sleep_ms_from_end_time_us(end_time_us);
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	DWORD const wait_result = WaitForSingleObject(handle, sleep_ms);
	return wait_result == WAIT_OBJECT_0;
}

auto dk::plt_semaphore_signal(PLT_Handle semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	ReleaseSemaphore(handle, 1, nullptr);
}


extern auto entry_point(int argc, char **argv) noexcept -> int;

auto dk::plt_w32_main_thread_entry_caller(int argc, WCHAR **wargv) noexcept -> int {
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		std::FILE *fp = nullptr;
		(void)freopen_s(&fp, "CONOUT$", "w", stdout);
		(void)freopen_s(&fp, "CONOUT$", "w", stderr);
		(void)freopen_s(&fp, "CONIN$", "r", stdin);
	}

	{
		plt_w32_context.perf_frequency = 1;
		LARGE_INTEGER freq{};
		if (QueryPerformanceFrequency(&freq)) {
			plt_w32_context.perf_frequency = freq.QuadPart;
		}
	}
	{
		SYSTEM_INFO sys_info = {};
		GetSystemInfo(&sys_info);

		PLT_SystemInfo *info = &plt_w32_context.system_info;
		info->logical_processor_count = static_cast<u32>(sys_info.dwNumberOfProcessors);
		info->page_size = sys_info.dwPageSize;
	}
	{
		PLT_ProcessInfo *info = &plt_w32_context.process_info;
		info->pid = GetCurrentProcessId();
	}

	ArenaParams constexpr args_arena_params = {
		.reserve_size = mega_bytes(1),
		.commit_size = kilo_bytes(32)
	};
	Arena *const args_arena = arena_alloc(&args_arena_params);
	char **const argv = arena_push_array<char *>(args_arena, argc + 1);
	for (int i = 0; i < argc; ++i) {
		String16 const arg16 = str16_cstring(reinterpret_cast<u16 const *>(wargv[i]));
		String8 const arg8 = str8_from_16(args_arena, arg16);
		argv[i] = const_cast<char *>(reinterpret_cast<char const *>(arg8.data));
	}
	argv[argc] = nullptr;

	ThreadContext *const thread_context = thread_context_alloc();
	thread_context_select(thread_context);

	Arena *const arena = arena_alloc();
	plt_w32_context.arena = arena;
	{
		PLT_ProcessInfo *info = &plt_w32_context.process_info;
		{
			TempArena const scratch = scratch_begin(nullptr, 0);
			DWORD constexpr size = static_cast<DWORD>(kilo_bytes(32));
			u16 *const buffer = arena_push_array<u16>(scratch.arena, size);
			DWORD const length = GetModuleFileNameW(nullptr, reinterpret_cast<WCHAR *>(buffer), size);
			String8 const path8 = str8_from_16(scratch.arena, str16(buffer, length));
			info->binary_dir = str8_copy(arena, path_chop_last_slash(path8));
			scratch_end(scratch);
		}
		{
			TempArena const scratch = scratch_begin(nullptr, 0);
			DWORD constexpr size = static_cast<DWORD>(kilo_bytes(32));
			u16 *const buffer = arena_push_array<u16>(scratch.arena, size);
			if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, reinterpret_cast<WCHAR *>(buffer)))) {
				info->user_program_data_path = str8_from_16(arena, str16_cstring(buffer));
			}
			scratch_end(scratch);
		}
	}

	InitializeCriticalSection(&plt_w32_context.entity_mutex);
	plt_w32_context.entity_arena = arena_alloc();

	int const result = main_thread_entry_point(argc, argv);

	arena_release(plt_w32_context.entity_arena);
	DeleteCriticalSection(&plt_w32_context.entity_mutex);

	arena_release(args_arena);

	thread_context_select(nullptr);
	thread_context_release(thread_context);

	return result;
}

#ifdef DK_BUILD_PLATFORM_GRAPHICAL
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR lp_cmd_line, int n_show_cmd) {
	(void)instance;
	(void)prev_instance;
	(void)lp_cmd_line;
	(void)n_show_cmd;
	CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	int const result = dk::plt_w32_main_thread_entry_caller(__argc, __wargv);
	CoUninitialize();
	return result;
}
#else
int wmain(int argc, WCHAR **argv) {
	int const result = dk::plt_w32_main_thread_entry_caller(argc, argv);
	return result;
}
#endif
