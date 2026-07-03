// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	u64 constexpr DKS_MAGIC_CONSTANT = 0x00736B645F726B64; // "dkr_dks\0"
	u32 constexpr DKS_VERSION = 1;

	enum DKS_SectionKind : u32 {
		DKS_SECTION_KIND_NULL = 0,
		DKS_SECTION_KIND_STRING_DATA,
		DKS_SECTION_KIND_STRING_TABLE,
		DKS_SECTION_KIND_COUNT
	};

	struct DKS_Header {
		u64 magic;
		u32 version;
		u32 section_offset;
		u32 section_count;
		u32 pad;
		u64 metadata_size; //< CPU data size.
	};

	struct DKS_Section {
		u64 offset;
		u64 size;
	};

	struct DKS_StringTable {
		u64 offset;
		u64 size;
	};

	using DKS_SectionElementType_StringData = u8;
	using DKS_SectionElementType_StringTable = DKS_StringTable;

	template <DKS_SectionKind Kind> struct DKS_SectionTraits;
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_DATA>  { using Type = DKS_SectionElementType_StringData; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_TABLE> { using Type = DKS_SectionElementType_StringTable; };

	extern u16 const dks_section_element_size_table[];
}
