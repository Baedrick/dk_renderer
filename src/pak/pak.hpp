// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	// "dkr_pak\0"
	u64 constexpr PAK_MAGIC_CONSTANT = 0x006B61705F726B64;
	u32 constexpr PAK_VERSION = 1;

	using PAK_SectionKind = u32;
	enum : u32 {
		PAK_SECTION_KIND_NULL = 0,
		PAK_SECTION_KIND_STRING_DATA,
		PAK_SECTION_KIND_STRING_TABLE,
		PAK_SECTION_KIND_SHADER,
		PAK_SECTION_KIND_GPU_SHADER,
		PAK_SECTION_KIND_COUNT
	};

	struct PAK_Header {
		u64 magic;
		u32 version;
		u32 section_offset;
		u32 section_count;
		u32 pad;
		u64 gpu_offset;
	};

	struct PAK_Section {
		u64 offset;
		u64 size;
	};

	struct PAK_StringTable {
		u64 offset;
		u64 size;
	};

	using PAK_ShaderKind = u32;
	enum : u32 {
		PAK_SHADER_KIND_VERTEX = 0,
		PAK_SHADER_KIND_FRAGMENT,
		PAK_SHADER_KIND_COMPUTE,
		PAK_SHADER_KIND_COUNT
	};

	struct PAK_Shader {
		u64 name_hash;
		u32 name_string_idx;
		PAK_ShaderKind kind;
		u64 gpu_offset;
		u64 gpu_size;
	};

	using PAK_SectionElementType_StringData = u8;
	using PAK_SectionElementType_StringTable = PAK_StringTable;
	using PAK_SectionElementType_Shader = PAK_Shader;
	using PAK_SectionElementType_GpuShader = u8;

	using PAK_ParseStatus = u32;
	enum : u32 {
		PAK_PARSE_STATUS_GOOD = 0,
		PAK_PARSE_STATUS_HEADER_DOES_NOT_MATCH,
		PAK_PARSE_STATUS_UNSUPPORTED_VERSION,
		PAK_PARSE_STATUS_INVALID_SECTIONS
	};

	struct PAK_Parsed {
		u8 *raw_data;
		u64 raw_data_size;
		PAK_Section *sections;
		u64 section_count;
	};

	auto pak_parse(u8 *data, u64 size, PAK_Parsed *out) noexcept -> PAK_ParseStatus;

	auto pak_find_shader_index(PAK_Parsed const *pak, String8 name) noexcept -> u32;
}
