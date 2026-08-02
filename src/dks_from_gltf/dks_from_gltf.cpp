// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::g2d_cgltf_file_read(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, char const *path, cgltf_size *out_size, void **out_data) noexcept -> cgltf_result {
	(void)mem_opts;

	cgltf_result result = cgltf_result_file_not_found;
	Arena *arena = static_cast<Arena *>(file_opts->user_data);
	String8 const file_path = str8_cstring(reinterpret_cast<u8 const *>(path));
	File const file = file_open(file_path, FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_SHARE_READ);
	if (is_valid(file)) {
		FileAttributes const attr = attributes_from_file(file);
		u8 *const data = arena_push_array<u8>(arena, attr.size);
		u64 read_size = file_read(file, 0, attr.size, data);
		if (read_size < attr.size) {
			arena_pop(arena, attr.size - read_size);
		}
		*out_data = reinterpret_cast<void *>(data);
		*out_size = static_cast<cgltf_size>(read_size);
		result = cgltf_result_success;
		file_close(file);
	}
	return result;
}

auto dk::g2d_cgltf_file_release(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, void *data, cgltf_size size) noexcept -> void {
	(void)mem_opts;
	(void)file_opts;
	(void)data;
	(void)size;
}

auto dk::g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams {
	ZoneScopedN;

	cgltf_data *gltf = nullptr;
	{
		ZoneScopedN("load and parse gltf file(s)");
		if (lane_idx() == 0) {
			cgltf_options options = {};
			options.file.read = g2d_cgltf_file_read;
			options.file.release = g2d_cgltf_file_release;
			options.file.user_data = arena;
			cgltf_parse(&options, params->file_data.data, static_cast<cgltf_size>(params->file_data.size), &gltf);
			cgltf_load_buffers(&options, data, reinterpret_cast<char const *>(params->file_path.data));
		}
		lane_sync_broadcast(&gltf, 0);
	}

	// TODO(Dedrick): Multilane compile.

	if (lane_idx() == 0) {
		cgltf_free(gltf);
	}
	lane_sync();

	DKSM_BakeParams result = {};
	return result;
}
