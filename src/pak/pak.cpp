// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::pak_parse(u8 *data, u64 size, PAK_Parsed *out) noexcept -> PAK_ParseStatus {
	PAK_ParseStatus result = PAK_PARSE_STATUS_GOOD;

	//~ Dedrick: Extract header.
	PAK_Header *header = nullptr;
	if (result == PAK_PARSE_STATUS_GOOD) {
		if (sizeof(*header) <= size) {
			header = reinterpret_cast<PAK_Header *>(data);
		}
		if (header == nullptr || header->magic != PAK_MAGIC_CONSTANT) {
			header = nullptr;
			result = PAK_PARSE_STATUS_HEADER_DOES_NOT_MATCH;
		}
		if (header != nullptr && header->version != PAK_VERSION) {
			header = nullptr;
			result = PAK_PARSE_STATUS_UNSUPPORTED_VERSION;
		}
	}

	//~ Dedrick: Extract sections.
	PAK_Section *sections = nullptr;
	u32 section_count = 0;
	if (result == PAK_PARSE_STATUS_GOOD) {
		u64 const opl = static_cast<u64>(header->section_offset) + static_cast<u64>(header->section_count) * sizeof(PAK_Section);
		if (opl <= size) {
			sections = reinterpret_cast<PAK_Section *>(data + header->section_offset);
			section_count = header->section_count;
		}
		if (sections == nullptr) {
			result = PAK_PARSE_STATUS_INVALID_SECTIONS;
		}
	}

	//~ Dedrick: Fill results.
	if (result == PAK_PARSE_STATUS_GOOD) {
		out->raw_data = data;
		out->raw_data_size = size;
		out->sections = sections;
		out->section_count = section_count;
	}
	return result;
}

auto dk::pak_find_shader_index(PAK_Parsed const *pak, String8 name) noexcept -> u32 {
	(void)pak;
	(void)name;
	u32 result = 0;
	return result;
}
