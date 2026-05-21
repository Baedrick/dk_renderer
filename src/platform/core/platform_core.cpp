// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator==(PLT_Handle a, PLT_Handle b) noexcept -> b8 {
	return a.v == b.v;
}

auto dk::operator!=(PLT_Handle a, PLT_Handle b) noexcept -> b8 {
	return !(a == b);
}

auto dk::plt_read_data_from_file_path(Arena *arena, String8 path) noexcept -> Array<u8> {
	PLT_Handle const file = plt_file_open(path, PLT_ACCESS_FLAG_READ);
	PLT_FileAttributes const attr = plt_attributes_from_file(file);
	Array<u8> result = {};
	result.count = attr.size;
	result.v = arena_push_array<u8>(arena, result.count);
	u64 const actual_read_size = plt_file_read(file, 0, result.count, result.v);
	if (actual_read_size < result.count) {
		arena_pop(arena, result.count - actual_read_size);
		result.count = actual_read_size;
	}
	return result;
}

auto dk::plt_write_data_to_file_path(String8 path, Array<u8> data) noexcept -> b8 {
	b8 good = false;
	PLT_Handle const file = plt_file_open(path, PLT_ACCESS_FLAG_WRITE);
	if (file != plt_handle_invalid()) {
		u64 const bytes_written = plt_file_write(file, 0, data.count, data.v);
		good = bytes_written == data.count;
		plt_file_close(file);
	}
	return good;
}

auto dk::plt_append_data_to_file_path(String8 path, Array<u8> data) noexcept -> b8 {
	b8 good = false;
	if (data.count != 0) {
		PLT_Handle const file = plt_file_open(path, PLT_ACCESS_FLAG_APPEND);
		if (file != plt_handle_invalid()) {
			u64 const file_pos = plt_attributes_from_file(file).size;
			u64 const bytes_written = plt_file_write(file, file_pos, file_pos + data.count, data.v);
			good = bytes_written == data.count;
			plt_file_close(file);
		}
	}
	return good;
}
