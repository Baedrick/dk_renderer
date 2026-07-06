// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	u64 constexpr DKS_MAGIC_CONSTANT = 0x00736B645F726B64; // "dkr_dks\0"
	u32 constexpr DKS_VERSION = 1;

	enum DKS_SectionKind : u32 {
		DKS_SECTION_KIND_NULL = 0,
		DKS_SECTION_KIND_TOP_LEVEL_INFO,
		DKS_SECTION_KIND_STRING_DATA,
		DKS_SECTION_KIND_STRING_TABLE,
		DKS_SECTION_KIND_OBJECTS,
		DKS_SECTION_KIND_GPU_OBJECT_INSTANCES,
		DKS_SECTION_KIND_GPU_VERTICES,
		DKS_SECTION_KIND_GPU_MESHLETS,
		DKS_SECTION_KIND_GPU_MESHLET_VERTICES,
		DKS_SECTION_KIND_GPU_MESHLET_TRIANGLES,
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

	struct DKS_TopLevelInfo {
		u32 model_name_string_idx;
		vec3 dequantization_factor;
		vec3 dequantization_summand;
	};

	struct DKS_Object {
		u32 name_string_idx;
	};

	struct DKS_ObjectInstance {
		f32 center[3];
		f32 extents[3];
		u32 meshlet_offset;
		u32 meshlet_count;
	};

	struct DKS_Meshlet {
		u32 vertex_offset;
		u32 triangle_offset;
		u8 vertex_count;
		u8 triangle_count;
		u16 pad;
	};

	struct DKS_Vertex {
		u64 position; ///< fixed point quantized: [unused:1][x:21][y:21][z:21]
	};

	using DKS_SectionElementType_TopLevelInfo         = DKS_TopLevelInfo;
	using DKS_SectionElementType_StringData           = u8;
	using DKS_SectionElementType_StringTable          = DKS_StringTable;
	using DKS_SectionElementType_Objects              = DKS_Object;
	using DKS_SectionElementType_GPU_ObjectInstances  = DKS_ObjectInstance;
	using DKS_SectionElementType_GPU_Meshlets         = DKS_Meshlet;
	using DKS_SectionElementType_GPU_Vertices         = DKS_Vertex;
	using DKS_SectionElementType_GPU_MeshletVertices  = u32;
	using DKS_SectionElementType_GPU_MeshletTriangles = u32;

	template <DKS_SectionKind Kind> struct DKS_SectionTraits;
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_TOP_LEVEL_INFO>        { using Type = DKS_SectionElementType_TopLevelInfo; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_DATA>           { using Type = DKS_SectionElementType_StringData; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_TABLE>          { using Type = DKS_SectionElementType_StringTable; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_OBJECTS>               { using Type = DKS_SectionElementType_Objects; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_OBJECT_INSTANCES>  { using Type = DKS_SectionElementType_GPU_ObjectInstances; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_VERTICES>          { using Type = DKS_SectionElementType_GPU_Meshlets; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLETS>          { using Type = DKS_SectionElementType_GPU_Vertices; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLET_VERTICES>  { using Type = DKS_SectionElementType_GPU_MeshletVertices; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLET_TRIANGLES> { using Type = DKS_SectionElementType_GPU_MeshletTriangles; };

	extern u16 const dks_section_element_size_table[];
}
