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
		DKS_SECTION_KIND_INSTANCES,
		DKS_SECTION_KIND_GPU_INSTANCES,
		DKS_SECTION_KIND_GPU_MESHES,
		DKS_SECTION_KIND_GPU_VERTICES,
		DKS_SECTION_KIND_GPU_MESHLETS,
		DKS_SECTION_KIND_GPU_MESHLET_BOUNDS,
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
	};

	struct DKS_Instance {
		u32 name_string_idx;
		u32 parent;
		u32 first_child;
		u32 prev_sibling;
		u32 next_sibling;
	};

	struct DKS_GPU_Instance {
		f32 world_from_object[12]; ///< mat4x3, implicit vec4(0,0,0,1)
		u32 mesh_idx;
	};

	struct DKS_GPU_Mesh {
		f32 sphere_center[3];
		f32 sphere_radius;
		f32 dequantization_factor[3];
		f32 dequantization_summand[3];
		u32 meshlet_offset;
		u32 meshlet_count;
	};

	struct DKS_GPU_MeshletBounds {
		f32 sphere_center[3];
		f32 sphere_radius;
		f32 cone_apex[3];
		f32 cone_axis[3];
		f32 cone_cutoff;
	};

	struct DKS_GPU_Meshlet {
		u32 vertex_offset;
		u32 triangle_offset;
		u8 vertex_count;
		u8 triangle_count;
		u16 pad;
	};

	struct DKS_GPU_Vertex {
		u64 position; ///< fixed point quantized: [unused:1][x:21][y:21][z:21]
	};

	using DKS_SectionElementType_TopLevelInfo         = DKS_TopLevelInfo;
	using DKS_SectionElementType_StringData           = u8;
	using DKS_SectionElementType_StringTable          = DKS_StringTable;
	using DKS_SectionElementType_Instances            = DKS_Instance;
	using DKS_SectionElementType_GPU_Instances        = DKS_GPU_Instance;
	using DKS_SectionElementType_GPU_Meshes           = DKS_GPU_Mesh;
	using DKS_SectionElementType_GPU_Vertices         = DKS_GPU_Vertex;
	using DKS_SectionElementType_GPU_Meshlets         = DKS_GPU_Meshlet;
	using DKS_SectionElementType_GPU_MeshletBounds    = DKS_GPU_MeshletBounds;
	using DKS_SectionElementType_GPU_MeshletVertices  = u32;
	using DKS_SectionElementType_GPU_MeshletTriangles = u32; // packed: [unused:8][i2:8][i1:8][i0:8]

	template <DKS_SectionKind Kind> struct DKS_SectionTraits;
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_TOP_LEVEL_INFO>        { using Type = DKS_SectionElementType_TopLevelInfo; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_DATA>           { using Type = DKS_SectionElementType_StringData; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_STRING_TABLE>          { using Type = DKS_SectionElementType_StringTable; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_INSTANCES>             { using Type = DKS_SectionElementType_Instances; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_INSTANCES>         { using Type = DKS_SectionElementType_GPU_Instances; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHES>            { using Type = DKS_SectionElementType_GPU_Meshes; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_VERTICES>          { using Type = DKS_SectionElementType_GPU_Vertices; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLETS>          { using Type = DKS_SectionElementType_GPU_Meshlets; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLET_BOUNDS>    { using Type = DKS_SectionElementType_GPU_MeshletBounds; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLET_VERTICES>  { using Type = DKS_SectionElementType_GPU_MeshletVertices; };
	template <> struct DKS_SectionTraits<DKS_SECTION_KIND_GPU_MESHLET_TRIANGLES> { using Type = DKS_SectionElementType_GPU_MeshletTriangles; };

	extern u16 const dks_section_element_size_table[];
}
