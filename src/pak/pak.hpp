// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	// "dkr_pak\0"
	u64 constexpr PAK_MAGIC_CONSTANT = 0x006B61705F726B64;
	u32 constexpr PAK_VERSION = 1;

	enum PAK_SectionKind : u32 {
		PAK_SECTION_KIND_NULL = 0,
		PAK_SECTION_KIND_STRING_DATA,
		PAK_SECTION_KIND_STRING_TABLE,
		PAK_SECTION_KIND_SHADER,
		PAK_SECTION_KIND_TEXTURE,
		PAK_SECTION_KIND_GPU_HEADER,
		PAK_SECTION_KIND_GPU_DATA,
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

	struct PAK_Shader {
		u64 name_hash;
		u32 name_string_idx;
		u64 gpu_offset;
		u64 gpu_size;
	};

	enum PAK_TextureKind : u16 {
		PAK_TEXTURE_KIND_2D,
		PAK_TEXTURE_KIND_3D
	};

	enum PAK_TextureFormat : u16 {
		PAK_TEXTURE_FORMAT_NULL = 0,
		PAK_TEXTURE_FORMAT_RGB9E5
	};

	struct PAK_Texture {
		u64 name_hash;
		u32 name_string_idx;
		PAK_TextureKind kind;
		PAK_TextureFormat format;
		u32 width;
		u32 height;
		u32 depth;
		u32 mip_count;
		u64 gpu_offset;
		u64 gpu_size;
	};

	struct PAK_GpuHeader {
		u64 gpu_offset;
	};

	using PAK_SectionElementType_StringData = u8;
	using PAK_SectionElementType_StringTable = PAK_StringTable;
	using PAK_SectionElementType_Shader = PAK_Shader;
	using PAK_SectionElementType_Texture = PAK_Texture;
	using PAK_SectionElementType_GpuHeader = PAK_GpuHeader;
	using PAK_SectionElementType_GpuData = u8;

	template <PAK_SectionKind Kind> struct PAK_SectionTraits;
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_STRING_DATA>  { using Type = PAK_SectionElementType_StringData; };
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_STRING_TABLE> { using Type = PAK_SectionElementType_StringTable; };
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_SHADER>       { using Type = PAK_SectionElementType_Shader; };
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_TEXTURE>      { using Type = PAK_SectionElementType_Texture; };
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_GPU_HEADER>   { using Type = PAK_SectionElementType_GpuHeader; };
	template <> struct PAK_SectionTraits<PAK_SECTION_KIND_GPU_DATA>     { using Type = PAK_SectionElementType_GpuData; };

	extern u16 const pak_section_element_size_table[];

	enum PAK_ParseStatus : u32 {
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

	auto pak_section_raw_data_from_kind(PAK_Parsed const *pak, PAK_SectionKind kind, u64 *out_size) noexcept -> void *;
	auto pak_section_raw_table_from_kind(PAK_Parsed const *pak, PAK_SectionKind kind, u64 *out_count) noexcept -> void *;
	auto pak_section_raw_element_from_kind_idx(PAK_Parsed const *pak, PAK_SectionKind kind, u64 idx) noexcept -> void *;

	template <PAK_SectionKind Kind>
	auto pak_table_from_kind(PAK_Parsed const *pak, u64 *out_count) noexcept -> typename PAK_SectionTraits<Kind>::Type * {
		return static_cast<typename PAK_SectionTraits<Kind>::Type *>(
			pak_section_raw_table_from_kind(pak, Kind, out_count)
		);
	}
	template <PAK_SectionKind Kind>
	auto pak_element_from_kind_idx(PAK_Parsed const *pak, u64 idx) noexcept -> typename PAK_SectionTraits<Kind>::Type * {
		return static_cast<typename PAK_SectionTraits<Kind>::Type *>(
			pak_section_raw_element_from_kind_idx(pak, Kind, idx)
		);
	}

	auto pak_string_from_idx(PAK_Parsed const *pak, u64 idx) noexcept -> String8;

	auto pak_shader_from_name(PAK_Parsed const *pak, String8 name) noexcept -> PAK_Shader *;
	auto pak_texture_from_name(PAK_Parsed const *pak, String8 name) noexcept -> PAK_Texture *;
}
