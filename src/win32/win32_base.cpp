// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::W32_Context dk::w32_context;


//~ Dedrick: Win32 helpers.

auto dk::w32_file_flags_from_dw_file_attributes(DWORD dw_file_attributes) noexcept -> FileFlags {
	FileFlags flags = FILE_FLAG_NONE;
	if ((dw_file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		flags |= FILE_FLAG_DIRECTORY;
	}
	return flags;
}

auto dk::w32_sleep_ms_from_end_time_us(u64 end_time_us) noexcept -> DWORD {
	if (end_time_us == U64_MAX) {
		return INFINITE;
	}
	DWORD sleep_ms = 0;
	u64 const begin_time_us = now_time_us();
	if (begin_time_us < end_time_us) {
		u64 const sleep_us = end_time_us - begin_time_us;
		sleep_ms = static_cast<DWORD>((sleep_us + 999) / 1000);
	}
	return sleep_ms;
}


//~ Dedrick: @entity

auto dk::w32_entity_alloc(W32_EntityKind kind) noexcept -> W32_Entity * {
	W32_Entity *result = nullptr;
	EnterCriticalSection(&w32_context.entity_mutex);
	{
		result = w32_context.entity_free;
		if (result != nullptr) {
			forward_list_stack_pop(&w32_context.entity_free);
		} else {
			result = arena_push<W32_Entity>(w32_context.entity_arena);
		}
		std::memset(result, 0, sizeof(W32_Entity));
	}
	LeaveCriticalSection(&w32_context.entity_mutex);
	result->kind = kind;
	return result;
}

auto dk::w32_entity_release(W32_Entity *entity) noexcept -> void {
	entity->kind = W32_ENTITY_NULL;
	EnterCriticalSection(&w32_context.entity_mutex);
	forward_list_stack_push(&w32_context.entity_free, entity);
	LeaveCriticalSection(&w32_context.entity_mutex);
}


//~ Dedrick: @base_memory

auto dk::memory_reserve(u64 size) noexcept -> void * {
	return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
}

auto dk::memory_commit(void *ptr, u64 size) noexcept -> b8 {
	return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
}

auto dk::memory_decommit(void *ptr, u64 size) noexcept -> b8 {
	return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

auto dk::memory_release(void *ptr, u64 size) noexcept -> void {
	(void)size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}


//~ Dedrick: @base_shared_memory

auto dk::shared_memory_create(u64 size, String8 name) noexcept -> SharedMemory {
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
	SharedMemory const result = { reinterpret_cast<uintptr_t>(file) };
	scratch_end(scratch);
	return result;
}

auto dk::shared_memory_open(String8 name) noexcept -> SharedMemory {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const file = OpenFileMappingW(
		FILE_MAP_ALL_ACCESS,
		0,
		reinterpret_cast<WCHAR const *>(name16.data)
	);
	SharedMemory const result = { reinterpret_cast<uintptr_t>(file) };
	scratch_end(scratch);
	return result;
}

auto dk::shared_memory_close(SharedMemory handle) noexcept -> void {
	HANDLE const file = reinterpret_cast<HANDLE>(handle.v);
	CloseHandle(file);
}

auto dk::shared_memory_map(SharedMemory handle, u64 begin, u64 end) noexcept -> void * {
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

auto dk::shared_memory_unmap(SharedMemory handle, void *ptr, u64 begin, u64 end) noexcept -> void {
	(void)handle;
	(void)begin;
	(void)end;
	UnmapViewOfFile(ptr);
}


//~ Dedrick: @base_file

auto dk::file_open(String8 path, FileAccessFlags flags) noexcept -> File {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const path16 = str16_from_8(scratch.arena, path);
	File result = {};

	DWORD access_flags = 0;
	DWORD share_mode = 0;
	DWORD creation_disposition = OPEN_EXISTING;
	SECURITY_ATTRIBUTES security_attributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
	if (flags & FILE_ACCESS_FLAG_READ) { access_flags |= GENERIC_READ; }
	if (flags & FILE_ACCESS_FLAG_WRITE) { access_flags |= GENERIC_WRITE; }
	if (flags & FILE_ACCESS_FLAG_SHARE_READ) { share_mode |= FILE_SHARE_READ; }
	if (flags & FILE_ACCESS_FLAG_SHARE_WRITE) { share_mode |= FILE_SHARE_WRITE | FILE_SHARE_DELETE; }
	if (flags & FILE_ACCESS_FLAG_WRITE) { creation_disposition = CREATE_ALWAYS; }
	if (flags & FILE_ACCESS_FLAG_APPEND) { creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }

	HANDLE const file = CreateFileW(
		reinterpret_cast<WCHAR const *>(path16.data),
		access_flags,
		share_mode,
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

auto dk::file_close(File file) noexcept -> void {
	if (!is_valid(file)) {
		return;
	}
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	CloseHandle(handle);
}

auto dk::file_read(File file, u64 begin, u64 end, void *out_data) noexcept -> u64 {
	if (!is_valid(file)) {
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

auto dk::file_write(File file, u64 begin, u64 end, void const *data) noexcept -> u64 {
	if (!is_valid(file)) {
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

auto dk::full_path_from_path(Arena *arena, String8 path) noexcept -> String8 {
	TempArena const scratch = scratch_begin(&arena, 1);
	DWORD buffer_size = static_cast<DWORD>(max<u64>(MAX_PATH, path.size * 2) + 1);
	WCHAR *buffer = arena_push_array<WCHAR>(scratch.arena, buffer_size);
	String16 const path16 = str16_from_8(scratch.arena, path);
	DWORD path16_size = GetFullPathNameW(reinterpret_cast<WCHAR const *>(path16.data), buffer_size, buffer, nullptr);
	if (path16_size > buffer_size) {
		arena_pop(scratch.arena, buffer_size);
		buffer_size = path16_size + 1;
		buffer = arena_push_array<WCHAR>(scratch.arena, buffer_size);
		path16_size = GetFullPathNameW(reinterpret_cast<WCHAR const *>(path16.data), buffer_size, buffer, nullptr);
	}
	String8 const full_path = str8_from_16(arena, str16(reinterpret_cast<u16 *>(buffer), path16_size));
	scratch_end(scratch);
	return full_path;
}

auto dk::file_path_exists(String8 path) noexcept -> b8 {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const path16 = str16_from_8(scratch.arena, path);
	DWORD const attributes = GetFileAttributesW(reinterpret_cast<WCHAR const *>(path16.data));
	b8 const exists = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	scratch_end(scratch);
	return exists;
}

auto dk::folder_path_exists(String8 path) noexcept -> b8 {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const path16 = str16_from_8(scratch.arena, path);
	DWORD const attributes = GetFileAttributesW(reinterpret_cast<WCHAR const *>(path16.data));
	b8 const exists = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	scratch_end(scratch);
	return exists;
}

auto dk::attributes_from_file(File file) noexcept -> FileAttributes {
	if (!is_valid(file)) {
		FileAttributes result = {};
		return result;
	}
	FileAttributes attr = {};
	HANDLE const handle = reinterpret_cast<HANDLE>(file.v);
	BY_HANDLE_FILE_INFORMATION info = {};
	BOOL const info_good = GetFileInformationByHandle(handle, &info);
	if (info_good) {
		u64 const size_lo = info.nFileSizeLow;
		u64 const size_hi = info.nFileSizeHigh;
		attr.size = size_lo | (size_hi << 32);
		attr.flags = w32_file_flags_from_dw_file_attributes(info.dwFileAttributes);
	}
	return attr;
}

auto dk::file_map_open(File file, FileAccessFlags flags) noexcept -> FileMap {
	HANDLE const file_handle = reinterpret_cast<HANDLE>(file.v);
	DWORD protect_flags = 0;
	switch (flags) {
		case FILE_ACCESS_FLAG_READ: {
			protect_flags |= PAGE_READONLY;
			break;
		}
		case FILE_ACCESS_FLAG_WRITE:
		case FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_WRITE: {
			protect_flags |= PAGE_READWRITE;
			break;
		}
		default: break;
	}
	HANDLE const map_handle = CreateFileMappingW(file_handle, nullptr, protect_flags, 0, 0, nullptr);
	FileMap const result = { reinterpret_cast<uintptr_t>(map_handle) };
	return result;
}

auto dk::file_map_close(FileMap map) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(map.v);
	CloseHandle(handle);
}

auto dk::file_map_view_open(FileMap map, FileAccessFlags flags, u64 begin, u64 end) noexcept -> void * {
	HANDLE const handle = reinterpret_cast<HANDLE>(map.v);
	u32 const off_lo = static_cast<u32>(begin & 0x00000000FFFFFFFF);
	u32 const off_hi = static_cast<u32>((begin & 0xFFFFFFFF00000000) >> 32);
	u64 const size = end - begin;
	DWORD access_flags = 0;
	switch (flags) {
		case FILE_ACCESS_FLAG_READ: {
			access_flags = FILE_MAP_READ;
			break;
		}
		case FILE_ACCESS_FLAG_WRITE: {
			access_flags = FILE_MAP_WRITE;
			break;
		}
		case FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_WRITE: {
			access_flags = FILE_MAP_ALL_ACCESS;
			break;
		}
		default: break;
	}
	void *const result = MapViewOfFile(handle, access_flags, off_hi, off_lo, size);
	return result;
}

auto dk::file_map_view_close(FileMap map, void *ptr, u64 begin, u64 end) noexcept -> void {
	(void)map;
	(void)begin;
	(void)end;
	UnmapViewOfFile(ptr);
}

auto dk::dir_iter_begin(String8 dir, DirIterFlags flags) noexcept -> DirIter {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String8 const dir_with_wildcard = str8_cat(scratch.arena, dir, "\\*"_str8);
	String16 const dir16 = str16_from_8(scratch.arena, dir_with_wildcard);
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_DIR_ITER);
	entity->dir_iter.flags = flags;
	entity->dir_iter.handle = FindFirstFileExW(
		reinterpret_cast<WCHAR const *>(dir16.data),
		FindExInfoBasic,
		&entity->dir_iter.find_data,
		FindExSearchNameMatch,
		nullptr,
		FIND_FIRST_EX_LARGE_FETCH
	);
	DirIter const result = { reinterpret_cast<uintptr_t>(entity) };
	scratch_end(scratch);
	return result;
}

auto dk::dir_iter_next(Arena *arena, DirIter dir_iter, DirIterResult *out_result) noexcept -> b8 {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(dir_iter.v);
	DK_ASSERT(entity->kind == W32_ENTITY_DIR_ITER);
	W32_DirIter *const w32_iter = &entity->dir_iter;
	DirIterFlags const flags = w32_iter->flags;
	if ((flags & DIR_ITER_FLAG_DONE) != 0 || w32_iter->handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	b8 found = false;
	do {
		b8 usable = true;
		String16 const name16 = str16_cstring(reinterpret_cast<u16 const *>(w32_iter->find_data.cFileName));
		DWORD const attributes = w32_iter->find_data.dwFileAttributes;
		if (str16_equals(name16, u"."_str16, STRING_MATCH_FLAG_NONE) ||
			str16_equals(name16, u".."_str16, STRING_MATCH_FLAG_NONE)) {
			usable = false;
		}
		if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
			usable = usable && (flags & DIR_ITER_FLAG_SKIP_FOLDERS) == 0;
		}
		else {
			usable = usable && (flags & DIR_ITER_FLAG_SKIP_FILES) == 0;
		}
		if (usable) {
			out_result->name = str8_from_16(arena, name16);
			u64 const size_lo = w32_iter->find_data.nFileSizeLow;
			u64 const size_hi = w32_iter->find_data.nFileSizeHigh;
			out_result->attributes.size = size_lo | (size_hi << 32);
			out_result->attributes.flags = w32_file_flags_from_dw_file_attributes(attributes);
			found = true;
			if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)) {
				w32_iter->flags |= DIR_ITER_FLAG_DONE;
			}
			break;
		}
	} while (FindNextFileW(w32_iter->handle, &w32_iter->find_data));
	if (!found) {
		w32_iter->flags |= DIR_ITER_FLAG_DONE;
	}
	return found;
}

auto dk::dir_iter_end(DirIter dir_iter) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(dir_iter.v);
	DK_ASSERT(entity->kind == W32_ENTITY_DIR_ITER);
	FindClose(entity->dir_iter.handle);
	w32_entity_release(entity);
}

auto dk::make_directory(String8 path) noexcept -> b8 {
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


//~ Dedrick: @base_thread

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

auto dk::thread_launch(ThreadFunction *func, void *params) noexcept -> Thread {
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_THREAD);
	entity->thread.func = func;
	entity->thread.params = params;
	entity->thread.handle = CreateThread(nullptr, 0, w32_thread_entry_caller, entity, 0, &entity->thread.tid);
	Thread const result = { reinterpret_cast<uintptr_t>(entity) };
	return result;
}

auto dk::thread_join(Thread thread) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(thread.v);
	if (entity != nullptr) {
		DK_ASSERT(entity->kind == W32_ENTITY_THREAD);
		WaitForSingleObject(entity->thread.handle, INFINITE);
		CloseHandle(entity->thread.handle);
		w32_entity_release(entity);
	}
}

auto dk::thread_detach(Thread thread) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(thread.v);
	if (entity != nullptr) {
		DK_ASSERT(entity->kind == W32_ENTITY_THREAD);
		CloseHandle(entity->thread.handle);
		w32_entity_release(entity);
	}
}

auto dk::mutex_alloc() noexcept -> Mutex {
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_MUTEX);
	InitializeCriticalSection(&entity->mutex.handle);
	Mutex const result = { reinterpret_cast<uintptr_t>(entity) };
	return result;
}

auto dk::mutex_release(Mutex mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_MUTEX);
	DeleteCriticalSection(&entity->mutex.handle);
	w32_entity_release(entity);
}

auto dk::mutex_scope_enter(Mutex mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_MUTEX);
	EnterCriticalSection(&entity->mutex.handle);
}

auto dk::mutex_scope_leave(Mutex mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_MUTEX);
	LeaveCriticalSection(&entity->mutex.handle);
}

auto dk::rw_mutex_alloc() noexcept -> RWMutex {
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_RW_MUTEX);
	InitializeSRWLock(&entity->rw_mutex.handle);
	RWMutex const result = { reinterpret_cast<uintptr_t>(entity) };
	return result;
}

auto dk::rw_mutex_release(RWMutex rw_mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_RW_MUTEX);
	w32_entity_release(entity);
}

auto dk::rw_mutex_scope_enter_w(RWMutex rw_mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_RW_MUTEX);
	AcquireSRWLockExclusive(&entity->rw_mutex.handle);
}

auto dk::rw_mutex_scope_leave_w(RWMutex rw_mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_RW_MUTEX);
	ReleaseSRWLockExclusive(&entity->rw_mutex.handle);
}

auto dk::rw_mutex_scope_enter_r(RWMutex rw_mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_RW_MUTEX);
	AcquireSRWLockShared(&entity->rw_mutex.handle);
}

auto dk::rw_mutex_scope_leave_r(RWMutex rw_mutex) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(rw_mutex.v);
	DK_ASSERT(entity->kind == W32_ENTITY_RW_MUTEX);
	ReleaseSRWLockShared(&entity->rw_mutex.handle);
}

auto dk::cond_var_alloc() noexcept -> CondVar {
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_CONDITIONAL_VARIABLE);
	InitializeConditionVariable(&entity->cond_var.handle);
	CondVar const result = { reinterpret_cast<uintptr_t>(entity) };
	return result;
}

auto dk::cond_var_release(CondVar cond_var) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
	w32_entity_release(entity);
}

auto dk::cond_var_wait(CondVar cond_var, Mutex mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		W32_Entity *const entity_cv = reinterpret_cast<W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
		W32_Entity *const entity_mutex = reinterpret_cast<W32_Entity *>(mutex.v);
		DK_ASSERT(entity_mutex->kind == W32_ENTITY_MUTEX);
		result = SleepConditionVariableCS(
			&entity_cv->cond_var.handle,
			&entity_mutex->mutex.handle,
			sleep_ms
		) != FALSE;
	}
	return result;
}

auto dk::cond_var_wait_rw_w(CondVar cond_var, RWMutex rw_mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		W32_Entity *const entity_cv = reinterpret_cast<W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
		W32_Entity *const entity_rw_mutex = reinterpret_cast<W32_Entity *>(rw_mutex.v);
		DK_ASSERT(entity_rw_mutex->kind == W32_ENTITY_RW_MUTEX);
		result = SleepConditionVariableSRW(
			&entity_cv->cond_var.handle,
			&entity_rw_mutex->rw_mutex.handle,
			sleep_ms,
			0
		) != FALSE;
	}
	return result;
}

auto dk::cond_var_wait_rw_r(CondVar cond_var, RWMutex rw_mutex, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = w32_sleep_ms_from_end_time_us(end_time_us);
	b8 result = false;
	if (sleep_ms > 0) {
		W32_Entity *const entity_cv = reinterpret_cast<W32_Entity *>(cond_var.v);
		DK_ASSERT(entity_cv->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
		W32_Entity *const entity_rw_mutex = reinterpret_cast<W32_Entity *>(rw_mutex.v);
		DK_ASSERT(entity_rw_mutex->kind == W32_ENTITY_RW_MUTEX);
		result = SleepConditionVariableSRW(
			&entity_cv->cond_var.handle,
			&entity_rw_mutex->rw_mutex.handle,
			sleep_ms,
			CONDITION_VARIABLE_LOCKMODE_SHARED
		) != FALSE;
	}
	return result;
}

auto dk::cond_var_signal(CondVar cond_var) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
	WakeConditionVariable(&entity->cond_var.handle);
}

auto dk::cond_var_signal_all(CondVar cond_var) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(cond_var.v);
	DK_ASSERT(entity->kind == W32_ENTITY_CONDITIONAL_VARIABLE);
	WakeAllConditionVariable(&entity->cond_var.handle);
}

auto dk::semaphore_alloc(u32 initial_count, u32 max_count, String8 name) noexcept -> Semaphore {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const handle = CreateSemaphoreW(nullptr, initial_count, max_count, reinterpret_cast<WCHAR const *>(name16.data));
	Semaphore const result = { reinterpret_cast<uintptr_t>(handle) };
	scratch_end(scratch);
	return result;
}

auto dk::semaphore_release(Semaphore semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	CloseHandle(handle);
}

auto dk::semaphore_open(String8 name) noexcept -> Semaphore {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const name16 = str16_from_8(scratch.arena, name);
	HANDLE const handle = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, reinterpret_cast<WCHAR const *>(name16.data));
	Semaphore const result = { reinterpret_cast<uintptr_t>(handle) };
	scratch_end(scratch);
	return result;
}

auto dk::semaphore_close(Semaphore semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	CloseHandle(handle);
}

auto dk::semaphore_wait(Semaphore semaphore, u64 end_time_us) noexcept -> b8 {
	DWORD const sleep_ms = w32_sleep_ms_from_end_time_us(end_time_us);
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	DWORD const wait_result = WaitForSingleObject(handle, sleep_ms);
	return wait_result == WAIT_OBJECT_0;
}

auto dk::semaphore_signal(Semaphore semaphore) noexcept -> void {
	HANDLE const handle = reinterpret_cast<HANDLE>(semaphore.v);
	ReleaseSemaphore(handle, 1, nullptr);
}

auto dk::barrier_alloc(u64 count) noexcept -> Barrier {
	W32_Entity *const entity = w32_entity_alloc(W32_ENTITY_BARRIER);
	InitializeSynchronizationBarrier(&entity->barrier.handle, static_cast<LONG>(count), -1);
	Barrier const result = { reinterpret_cast<uintptr_t>(entity) };
	return result;
}

auto dk::barrier_release(Barrier barrier) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(barrier.v);
	if (entity != nullptr) {
		DK_ASSERT(entity->kind == W32_ENTITY_BARRIER);
		DeleteSynchronizationBarrier(&entity->barrier.handle);
		w32_entity_release(entity);
	}
}

auto dk::barrier_wait(Barrier barrier) noexcept -> void {
	W32_Entity *const entity = reinterpret_cast<W32_Entity *>(barrier.v);
	if (entity != nullptr) {
		DK_ASSERT(entity->kind == W32_ENTITY_BARRIER);
		EnterSynchronizationBarrier(&entity->barrier.handle, 0);
	}
}


//~ Dedrick: @base_system

auto dk::get_system_info() noexcept -> SystemInfo * {
	return &w32_context.system_info;
}

auto dk::get_process_info() noexcept -> ProcessInfo * {
	return &w32_context.process_info;
}

auto dk::get_entropy(void *data, u64 size) noexcept -> void {
	DK_ASSERT_ALWAYS(size <= 256); // NOTE(Dedrick): Limit of 256 bytes to follow linux.
	BCryptGenRandom(
		nullptr,
		static_cast<u8 *>(data),
		static_cast<u32>(size),
		BCRYPT_USE_SYSTEM_PREFERRED_RNG
	);
}

auto dk::abort_self(s32 code) noexcept -> void {
	ExitProcess(static_cast<u32>(code));
}

auto dk::now_time_us() noexcept -> u64 {
	u64 result = 0;
	LARGE_INTEGER ticks = {};
	if (QueryPerformanceCounter(&ticks)) {
		result = (ticks.QuadPart * 1'000'000) / w32_context.perf_frequency;
	}
	return result;
}

auto dk::sleep(u64 milliseconds) noexcept -> void {
	Sleep(static_cast<DWORD>(milliseconds));
}

auto dk::process_launch(ProcessLaunchParams const *params) noexcept -> Process {
	Process result = {};
	TempArena const scratch = scratch_begin(nullptr, 0);

	String8 cmd = {};
	{
		String8JoinParams join_params = {};
		join_params.prefix = "\""_str8;
		join_params.postfix = "\""_str8;
		join_params.separator = "\" \""_str8;
		cmd = str8_list_join(scratch.arena, &params->cmd_line, &join_params);
	}

	String16 const cmd16 = str16_from_8(scratch.arena, cmd);
	String16 const dir16 = str16_from_8(scratch.arena, params->working_dir);

	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	if ((params->flags & PROCESS_LAUNCH_FLAG_NO_WINDOW) != 0) {
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

auto dk::process_join(Process process, u64 end_time_us, u64 *out_exit_code) noexcept -> b8 {
	HANDLE const handle = reinterpret_cast<HANDLE>(process.v);
	DWORD const sleep_ms = w32_sleep_ms_from_end_time_us(end_time_us);
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

auto dk::process_kill(Process process) noexcept -> b8 {
	HANDLE const handle = reinterpret_cast<HANDLE>(process.v);
	b8 const terminated = TerminateProcess(handle, 999);
	return terminated;
}


//~ Dedrick: @entry_point

auto dk::w32_thread_entry_caller(void *params) noexcept -> DWORD {
	W32_Entity *const entity = static_cast<W32_Entity *>(params);
	DK_ASSERT(entity->kind == W32_ENTITY_THREAD);
	ThreadFunction *const func = entity->thread.func;
	void *const func_params = entity->thread.params;
	thread_entry_point(func, func_params);
	return 0;
}

auto dk::w32_main_thread_entry_caller(int argc, WCHAR **wargv) noexcept -> int {
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		std::FILE *fp = nullptr;
		(void)freopen_s(&fp, "CONOUT$", "w", stdout);
		(void)freopen_s(&fp, "CONOUT$", "w", stderr);
		(void)freopen_s(&fp, "CONIN$", "r", stdin);
	}

	{
		w32_context.perf_frequency = 1;
		LARGE_INTEGER freq{};
		if (QueryPerformanceFrequency(&freq)) {
			w32_context.perf_frequency = freq.QuadPart;
		}
	}
	{
		SYSTEM_INFO sys_info = {};
		GetSystemInfo(&sys_info);

		SystemInfo *info = &w32_context.system_info;
		info->logical_processor_count = static_cast<u32>(sys_info.dwNumberOfProcessors);
		info->page_size = sys_info.dwPageSize;
	}
	{
		ProcessInfo *info = &w32_context.process_info;
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
	w32_context.arena = arena;
	{
		ProcessInfo *info = &w32_context.process_info;
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
				info->user_program_data_dir = str8_from_16(arena, str16_cstring(buffer));
			}
			scratch_end(scratch);
		}
	}

	InitializeCriticalSection(&w32_context.entity_mutex);
	w32_context.entity_arena = arena_alloc();

	int const result = main_thread_entry_point(argc, argv);

	arena_release(w32_context.entity_arena);
	DeleteCriticalSection(&w32_context.entity_mutex);

	arena_release(args_arena);

	thread_context_select(nullptr);
	thread_context_release(thread_context);

	return result;
}

#ifdef DK_BUILD_GRAPHICAL
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR lp_cmd_line, int n_show_cmd) {
	(void)instance;
	(void)prev_instance;
	(void)lp_cmd_line;
	(void)n_show_cmd;
	CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	int const result = dk::w32_main_thread_entry_caller(__argc, __wargv);
	CoUninitialize();
	return result;
}
#else
int wmain(int argc, WCHAR **argv) {
	int const result = dk::w32_main_thread_entry_caller(argc, argv);
	return result;
}
#endif
