// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator==(File a, File b) noexcept -> b8 {
	return a.v == b.v;
}

auto dk::operator!=(File a, File b) noexcept -> b8 {
	return !(a == b);
}

auto dk::is_valid(File file) noexcept -> b8 {
	return file != File{};
}

auto dk::read_bytes_from_file_path(Arena *arena, String8 path) noexcept -> Buffer {
	File const file = file_open(path, FILE_ACCESS_FLAG_READ);
	FileAttributes const attr = attributes_from_file(file);
	Buffer result = {};
	result.size = attr.size;
	result.data = arena_push_array<u8>(arena, result.size);
	u64 const actual_read_size = file_read(file, 0, result.size, result.data);
	if (actual_read_size < result.size) {
		arena_pop(arena, result.size - actual_read_size);
		result.size = actual_read_size;
	}
	return result;
}

auto dk::write_bytes_to_file_path(String8 path, Buffer bytes) noexcept -> b8 {
	b8 good = false;
	File const file = file_open(path, FILE_ACCESS_FLAG_WRITE);
	if (is_valid(file)) {
		u64 const bytes_written = file_write(file, 0, bytes.size, bytes.data);
		good = bytes_written == bytes.size;
		file_close(file);
	}
	return good;
}

auto dk::write_bytes_list_to_file_path(String8 path, BufferList const *list) noexcept -> b8 {
	// TODO(Dedrick): This isn't performant because writing each node directly
	// to disk creates a massive bottleneck when handling lists of small bytes.
	// Every node forces a new system call, causing constant, expensive CPU
	// context switches between user and kernel modes. Buffer the writing to
	// disk with 64KB temporary memory space. Use a ring buffer to align writes.
	b8 good = false;
	File const file = file_open(path, FILE_ACCESS_FLAG_WRITE);
	if (is_valid(file)) {
		u64 file_pos = 0;
		for (BufferNode const *node = list->first; node != nullptr; node = node->next) {
			u64 const bytes_written = file_write(file, file_pos, file_pos + node->buffer.size, node->buffer.data);
			if (bytes_written != node->buffer.size) {
				break;
			}
			file_pos += bytes_written;
		}
		good = file_pos == list->total_size;
		file_close(file);
	}
	return good;
}

auto dk::append_bytes_to_file_path(String8 path, Buffer bytes) noexcept -> b8 {
	b8 good = false;
	if (bytes.size != 0) {
		File const file = file_open(path, FILE_ACCESS_FLAG_APPEND);
		if (is_valid(file)) {
			u64 const file_pos = attributes_from_file(file).size;
			u64 const bytes_written = file_write(file, file_pos, file_pos + bytes.size, bytes.data);
			good = bytes_written == bytes.size;
			file_close(file);
		}
	}
	return good;
}

auto dk::read_string_from_file_path(Arena *arena, String8 path) noexcept -> String8 {
	File const file = file_open(path, FILE_ACCESS_FLAG_READ);
	FileAttributes const attr = attributes_from_file(file);
	String8 result = {};
	result.size = attr.size;
	result.data = arena_push_array<u8>(arena, result.size);
	u64 const actual_read_size = file_read(file, 0, result.size, const_cast<u8 *>(result.data));
	if (actual_read_size < result.size) {
		arena_pop(arena, result.size - actual_read_size);
		result.size = actual_read_size;
	}
	return result;
}

auto dk::write_string_to_file_path(String8 path, String8 str) noexcept -> b8 {
	b8 good = false;
	File const file = file_open(path, FILE_ACCESS_FLAG_WRITE);
	if (is_valid(file)) {
		u64 const bytes_written = file_write(file, 0, str.size, str.data);
		good = bytes_written == str.size;
		file_close(file);
	}
	return good;
}

auto dk::append_string_to_file_path(String8 path, String8 str) noexcept -> b8 {
	b8 good = false;
	if (str.size != 0) {
		File const file = file_open(path, FILE_ACCESS_FLAG_APPEND);
		if (is_valid(file)) {
			u64 const file_pos = attributes_from_file(file).size;
			u64 const bytes_written = file_write(file, file_pos, file_pos + str.size, str.data);
			good = bytes_written == str.size;
			file_close(file);
		}
	}
	return good;
}
