// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct DKSM_TopLevelInfo {
		String8 model_name;
	};

	struct DKSM_Instance {
		struct DKSM_InstanceChunkNode *chunk;
		String8 name;
		DKSM_Instance *parent;
		DKSM_Instance *first_child;
		DKSM_Instance *next_sibling;
		DKSM_Instance *prev_sibling;
		struct DKSM_GPU_Instance *first_gpu_instance;
		struct DKSM_GPU_Instance *last_gpu_instance;
		u64 gpu_instance_count;
	};

	struct DKSM_InstanceChunkNode {
		DKSM_InstanceChunkNode *next;
		DKSM_Instance *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKSM_InstanceChunkList {
		DKSM_InstanceChunkNode *first;
		DKSM_InstanceChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct DKSM_GPU_Instance {
		DKSM_GPU_Instance *next;
		struct DKSM_GPU_InstanceChunkNode *chunk;
		struct DKSM_GPU_Mesh *mesh;
		f32 world_from_object[12]; ///< mat4x3, implicit vec4(0,0,0,1)
	};

	struct DKSM_GPU_InstanceChunkNode {
		DKSM_GPU_InstanceChunkNode *next;
		DKSM_GPU_Instance *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKSM_GPU_InstanceChunkList {
		DKSM_GPU_InstanceChunkNode *first;
		DKSM_GPU_InstanceChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct DKSM_GPU_Mesh {
		struct DKSM_GPU_MeshChunkNode *chunk;
		f32 sphere_center[3];
		f32 sphere_radius;
		f32 dequantization_factor[3];
		f32 dequantization_summand[3];
		DKS_GPU_Vertex *vertices;
		u64 vertex_count;
		DKS_GPU_Meshlet *meshlets;
		DKS_GPU_MeshletBounds *meshlet_bounds;
		u32 meshlet_count;
		u32 *meshlet_vertices;
		u32 *meshlet_triangles;
		u32 meshlet_vertex_count;
		u32 meshlet_triangle_count;
	};

	struct DKSM_GPU_MeshChunkNode {
		DKSM_GPU_MeshChunkNode *next;
		DKSM_GPU_Mesh *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKSM_GPU_MeshChunkList {
		DKSM_GPU_MeshChunkNode *first;
		DKSM_GPU_MeshChunkNode *last;
		u64 chunk_count;
		u64 total_count;
		u64 total_vertex_count;
		u64 total_meshlet_count;
		u64 total_meshlet_vertex_count;
		u64 total_meshlet_triangle_count;
	};

	struct DKSM_BakeParams {
		DKSM_TopLevelInfo top_level_info;
		DKSM_InstanceChunkList instances;
		DKSM_GPU_InstanceChunkList gpu_instances;
		DKSM_GPU_MeshChunkList gpu_meshes;
	};

	struct DKSM_BakeString {
		String8 string;
		u64 hash;
	};

	struct DKSM_BakeStringChunkNode {
		DKSM_BakeStringChunkNode *next;
		DKSM_BakeString *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKSM_BakeStringChunkList {
		DKSM_BakeStringChunkNode *first;
		DKSM_BakeStringChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct DKSM_BakeStringMapTopology {
		u64 slots_count;
	};

	struct DKSM_BakeStringMapBaseIndices {
		u64 *slots_base_idxs;
	};

	struct DKSM_BakeStringMapLoose {
		DKSM_BakeStringChunkList **slots;
	};

	struct DKSM_BakeStringMapTight {
		DKSM_BakeStringChunkList *slots;
		u64 *slots_base_idxs;
		u64 slots_count;
		u64 total_count;
	};

	struct DKSM_TopLevelInfoBakeResult {
		DKSM_TopLevelInfo *top_level_info;
	};

	struct DKSM_StringBakeResult {
		DKS_StringTable *strings_table;
		u64 strings_table_count;
		u8 *string_data;
		u64 string_data_size;
	};

	struct DKSM_InstanceBakeResult {
		DKS_Instance *instances;
		u64 instances_count;
	};

	struct DKSM_GPU_InstanceBakeResult {
		DKS_GPU_Instance *gpu_instances;
		u64 gpu_instances_count;
	};

	struct DKSM_GPU_MeshBakeResult {
		DKS_GPU_Mesh *gpu_meshes;
		u64 gpu_meshes_count;
		DKS_GPU_Vertex *vertices;
		u64 vertices_count;
		DKS_GPU_Meshlet *meshlets;
		u64 meshlets_count;
		DKS_GPU_MeshletBounds *meshlet_bounds;
		u64 meshlet_bounds_count;
		u32 *meshlet_vertices;
		u64 meshlet_vertices_count;
		u32 *meshlet_triangles;
		u64 meshlet_triangles_count;
	};

	struct DKSM_BakeResults {
		DKSM_TopLevelInfoBakeResult top_level_info;
		DKSM_StringBakeResult strings;
		DKSM_InstanceBakeResult instances;
		DKSM_GPU_InstanceBakeResult gpu_instances;
		DKSM_GPU_MeshBakeResult gpu_meshes;
	};

	struct DKSM_SerializedSection {
		void *data;
		u64 size;
	};

	struct DKSM_SerializedSectionBundle {
		DKSM_SerializedSection sections[DKS_SECTION_KIND_COUNT];
	};

	auto dksm_instance_chunk_list_push(Arena *arena, DKSM_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_Instance *;
	auto dksm_instance_chunk_list_concat_in_place(DKSM_InstanceChunkList *dst, DKSM_InstanceChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_instance(DKSM_Instance const *instance) noexcept -> u64;

	auto dksm_gpu_instance_chunk_list_push(Arena *arena, DKSM_GPU_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Instance *;
	auto dksm_gpu_instance_chunk_list_concat_in_place(DKSM_GPU_InstanceChunkList *dst, DKSM_GPU_InstanceChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_gpu_instance(DKSM_GPU_Instance const *gpu_instance) noexcept -> u64;

	auto dksm_gpu_mesh_chunk_list_push(Arena *arena, DKSM_GPU_MeshChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Mesh *;
	auto dksm_gpu_mesh_chunk_list_concat_in_place(DKSM_GPU_MeshChunkList *dst, DKSM_GPU_MeshChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_gpu_mesh(DKSM_GPU_Mesh const *gpu_mesh) noexcept -> u64;

	auto dksm_quantize_vertex_position(f32 const position[3], f32 const dequant_summand[3], f32 const dequant_factor[3]) noexcept -> u64;
	auto dksm_gpu_meshlet_triangle_from_indices(u32 i0, u32 i1, u32 i2) noexcept -> u32;

	auto dksm_bake_string_chunk_list_push(Arena *arena, DKSM_BakeStringChunkList *list, u64 capacity) noexcept -> DKSM_BakeString *;
	auto dksm_bake_string_chunk_list_concat_in_place(DKSM_BakeStringChunkList *dst, DKSM_BakeStringChunkList *to_push) noexcept -> void;
	auto dksm_bake_string_chunk_list_sorted_from_unsorted(Arena *arena, DKSM_BakeStringChunkList const *list) noexcept -> DKSM_BakeStringChunkList;

	auto dksm_bake_string_map_loose_make(Arena *arena, DKSM_BakeStringMapTopology *topology) noexcept -> DKSM_BakeStringMapLoose *;
	auto dksm_bake_string_map_loose_insert(Arena *arena, DKSM_BakeStringMapTopology *map_topology, DKSM_BakeStringMapLoose *map, u64 chunk_cap, String8 str) noexcept -> DKSM_BakeString *;
	auto dksm_bake_string_map_base_indices_from_map_loose(Arena *arena, DKSM_BakeStringMapTopology const *map_topology, DKSM_BakeStringMapLoose const *map) noexcept -> DKSM_BakeStringMapBaseIndices;
	auto dksm_bake_idx_from_string(DKSM_BakeStringMapTight const *map, String8 str) noexcept -> u32;

	auto dksm_bake(Arena *arena, DKSM_BakeParams const *params) noexcept -> DKSM_BakeResults;

	auto dksm_serialized_section_bundle_from_bake_results(DKSM_BakeResults const *bake_results) noexcept -> DKSM_SerializedSectionBundle;
	auto dksm_buffer_blobs_from_section_bundle(Arena *arena, DKSM_SerializedSectionBundle const *bundle) noexcept -> BufferList;
}
