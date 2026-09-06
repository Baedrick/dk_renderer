// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct File {
		u64 v;
	};

	struct FileMap {
		u64 v;
	};

	using FileAccessFlags = u32;
	enum : u32 {
		FileAccessFlag_Read        = 1u << 0,
		FileAccessFlag_Write       = 1u << 1,
		FileAccessFlag_Append      = 1u << 2,
		FileAccessFlag_ShareRead   = 1u << 3,
		FileAccessFlag_ShareWrite  = 1u << 4
	};

	using FileFlags = u32;
	enum : u32 {
		FileFlag_None       = 0,
		FileFlag_Directory  = 1u << 0
	};

	struct FileAttributes {
		u64 size;
		FileFlags flags;
	};

	using DirIterFlags = u32;
	enum : u32 {
		DirIterFlag_None             = 0,
		DirIterFlag_SkipFolders      = 1u << 0,
		DirIterFlag_SkipFiles        = 1u << 1,
		DirIterFlag_SkipHiddenFiles  = 1u << 2,
		DirIterFlag_Done             = 1u << 31,
	};

	struct DirIter {
		u64 v;
	};

	struct DirIterResult {
		String8 name;
		FileAttributes attributes;
	};

	auto operator==(File a, File b) noexcept -> b8;
	auto operator!=(File a, File b) noexcept -> b8;
	auto is_valid(File file) noexcept -> b8;

	auto read_bytes_from_file_path(Arena *arena, String8 path) noexcept -> Buffer;
	auto write_bytes_to_file_path(String8 path, Buffer bytes) noexcept -> b8;
	auto write_bytes_list_to_file_path(String8 path, BufferList const *list) noexcept -> b8;
	auto append_bytes_to_file_path(String8 path, Buffer bytes) noexcept -> b8;

	auto read_string_from_file_path(Arena *arena, String8 path) noexcept -> String8;
	auto write_string_to_file_path(String8 path, String8 str) noexcept -> b8;
	auto append_string_to_file_path(String8 path, String8 str) noexcept -> b8;

	auto file_open(String8 path, FileAccessFlags flags) noexcept -> File;
	auto file_close(File file) noexcept -> void;
	auto file_read(File file, u64 begin, u64 end, void *out_data) noexcept -> u64;
	auto file_write(File file, u64 begin, u64 end, void const *data) noexcept -> u64;
	auto full_path_from_path(Arena *arena, String8 path) noexcept -> String8;
	auto file_path_exists(String8 path) noexcept -> b8;
	auto folder_path_exists(String8 path) noexcept -> b8;
	auto attributes_from_file(File file) noexcept -> FileAttributes;

	auto file_map_open(File file, FileAccessFlags flags) noexcept -> FileMap;
	auto file_map_close(FileMap map) noexcept -> void;
	auto file_map_view_open(FileMap map, FileAccessFlags flags, u64 begin, u64 end) noexcept -> void *;
	auto file_map_view_close(FileMap map, void *ptr, u64 begin, u64 end) noexcept -> void;

	auto dir_iter_begin(String8 dir, DirIterFlags flags) noexcept -> DirIter;
	auto dir_iter_next(Arena *arena, DirIter dir_iter, DirIterResult *out_result) noexcept -> b8;
	auto dir_iter_end(DirIter dir_iter) noexcept -> void;

	auto make_directory(String8 path) noexcept -> b8;
}
