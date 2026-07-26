// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::g2d_convert(Arena *arena, G2D_ConvertParams const *params) noexcept -> DKSM_BakeParams {
	ZoneScoped;

	//~ Dedrick:
	cgltf_data *data = nullptr;
	if (lane_idx() == 0) {
		cgltf_options options = {};
		cgltf_parse(&options, params->file_data.data, params->file_data.size, &data);
		if (params->is_glb) {
			cgltf_load_buffers(&options, data, nullptr);
		}
		else {
			//~ Dedrick: Fixup buffer pointers for glTF + bin.
			ZoneScopedN("fixup buffer pointers for glTF + bin");
			for (cgltf_size i = 0; i < data->buffers_count; ++i) {
				String8 const target_uri = str8_cstring(reinterpret_cast<u8 const *>(data->buffers[i].uri));
				if (!str8_starts_with(target_uri, "data:"_str8)))) {

				}

			}
		}
	}
	lane_sync_broadcast(&data);
}
