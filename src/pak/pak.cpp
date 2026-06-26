// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::u16 const dk::pak_section_element_size_table[] = {
	sizeof(u8),
	sizeof(PAK_SectionElementType_StringData),
	sizeof(PAK_SectionElementType_StringTable),
	sizeof(PAK_SectionElementType_Shader),
	sizeof(PAK_SectionElementType_Texture),
	sizeof(PAK_SectionElementType_ShaderData),
	sizeof(PAK_SectionElementType_TextureData)
};
static_assert(dk::array_count(dk::pak_section_element_size_table) == dk::PAK_SECTION_KIND_COUNT);

auto dk::pak_metadata_size_from_bytes(Buffer bytes) noexcept -> u64 {
	u64 result = 0;
	if (bytes.size >= sizeof(PAK_Header)) {
		PAK_Header *const header = reinterpret_cast<PAK_Header *>(bytes.data);
		if (header->magic == PAK_MAGIC_CONSTANT && header->version == PAK_VERSION) {
			result = header->metadata_size;
		}
	}
	return result;
}

auto dk::pak_parse(Buffer bytes, PAK_Parsed *out) noexcept -> b8 {
	b8 good = false;
	if (bytes.size >= sizeof(PAK_Header)) {
		PAK_Header *const header = reinterpret_cast<PAK_Header *>(bytes.data);
		if (header->magic == PAK_MAGIC_CONSTANT && header->version == PAK_VERSION) {
			u64 const opl = static_cast<u64>(header->section_offset) + static_cast<u64>(header->section_count) * sizeof(PAK_Section);
			if (opl <= bytes.size) {
				out->raw_data = bytes.data;
				out->raw_data_size = bytes.size;
				out->sections = reinterpret_cast<PAK_Section *>(bytes.data + header->section_offset);
				out->section_count = header->section_count;
				good = true;
			}
		}
	}
	return good;
}

auto dk::pak_section_raw_data_from_kind(PAK_Parsed const *pak, PAK_SectionKind kind, u64 *out_size) noexcept -> void * {
	void *result = nullptr;
	if (kind < pak->section_count && pak->sections[kind].offset < pak->raw_data_size) {
		result = pak->raw_data + pak->sections[kind].offset;
		*out_size = pak->sections[kind].size;
	}
	return result;
}

auto dk::pak_section_raw_table_from_kind(PAK_Parsed const *pak, PAK_SectionKind kind, u64 *out_count) noexcept -> void * {
	u64 all_elements_size = 0;
	void *const all_elements = pak_section_raw_data_from_kind(pak, kind, &all_elements_size);
	u64 const element_size = pak_section_element_size_table[kind];
	*out_count = all_elements_size / element_size;
	return all_elements;
}

auto dk::pak_section_raw_element_from_kind_idx(PAK_Parsed const *pak, PAK_SectionKind kind, u64 idx) noexcept -> void * {
	u64 count = 0;
	void *const table = pak_section_raw_table_from_kind(pak, kind, &count);
	void *result = table;
	if (idx < count) {
		u64 const element_size = pak_section_element_size_table[kind];
		result = static_cast<u8 *>(table) + element_size * idx;
	}
	return result;
}

auto dk::pak_string_from_idx(PAK_Parsed const *pak, u64 idx) noexcept -> String8 {
	String8 result = {};
	u64 str_count = 0;
	PAK_StringTable *const strings = pak_table_from_kind<PAK_SECTION_KIND_STRING_TABLE>(pak, &str_count);
	if (idx < str_count) {
		u64 str_data_size = 0;
		u8 *const str_data = pak_table_from_kind<PAK_SECTION_KIND_STRING_DATA>(pak, &str_data_size);
		result = str8(str_data + strings[idx].offset, strings[idx].size);
	}
	return result;
}

auto dk::pak_shader_from_name(PAK_Parsed const *pak, String8 name) noexcept -> PAK_Shader * {
	PAK_Shader *result = nullptr;
	if (name.size > 0) {
		u64 const name_hash = u64_hash_from_str8(name);
		u64 shader_count = 0;
		PAK_Shader *const shaders = pak_table_from_kind<PAK_SECTION_KIND_SHADER>(pak, &shader_count);
		for (u64 shader_idx = 0; shader_idx < shader_count; ++shader_idx) {
			PAK_Shader *const shader = shaders + shader_idx;
			if (shader->name_hash == name_hash) {
				String8 const shader_name = pak_string_from_idx(pak, shader->name_string_idx);
				if (str8_equals(shader_name, name, STRING_MATCH_FLAG_NONE)) {
					result = shader;
					break;
				}
			}
		}
	}
	return result;
}

auto dk::pak_texture_from_name(PAK_Parsed const *pak, String8 name) noexcept -> PAK_Texture * {
	PAK_Texture *result = nullptr;
	if (name.size > 0) {
		u64 const name_hash = u64_hash_from_str8(name);
		u64 texture_count = 0;
		PAK_Texture *const textures = pak_table_from_kind<PAK_SECTION_KIND_TEXTURE>(pak, &texture_count);
		for (u64 texture_idx = 0; texture_idx < texture_count; ++texture_idx) {
			PAK_Texture *const texture = textures + texture_idx;
			if (texture->name_hash == name_hash) {
				String8 const texture_name = pak_string_from_idx(pak, texture->name_string_idx);
				if (str8_equals(texture_name, name, STRING_MATCH_FLAG_NONE)) {
					result = texture;
					break;
				}
			}
		}
	}
	return result;
}
