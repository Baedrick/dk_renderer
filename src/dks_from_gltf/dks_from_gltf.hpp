// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct G2D_ConvertParams {
		String8 file_path;
		Buffer file_data;
	};

	auto g2d_cgltf_file_read(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, char const *path, cgltf_size *out_size, void **out_data) noexcept -> cgltf_result;
	auto g2d_cgltf_file_release(cgltf_memory_options const *mem_opts, cgltf_file_options const *file_opts, void *data, cgltf_size size) noexcept -> void;

	auto g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams;
}
