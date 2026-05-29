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
		PAK_SECTION_KIND_SHADER_BINARY,
		PAK_SECTION_KIND_SHADER,
		PAK_SECTION_KIND_COUNT
	};

	struct PAK_Header {
		u64 magic;
		u32 version;
		u32 section_offset;
		u32 section_count;
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
		u64 shader_binary_offset;
		u64 shader_binary_size;
	};

	using PAK_SectionElementType_StringData = u8;
	using PAK_SectionElementType_StringTable = PAK_StringTable;
	using PAK_SectionElementType_ShaderBinary = u8;
	using PAK_SectionElementType_Shader = PAK_Shader;
}
