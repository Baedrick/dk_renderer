// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/meshoptimizer/meshoptimizer.h"

namespace dk {
	struct DKSM_TopLevelInfo {
		String8 model_name;
	};

	struct DKSM_Node {
		struct DKSM_NodeChunkNode *chunk;
		String8 name;
		DKSM_Node *parent;
		DKSM_Node *first_child;
		DKSM_Node *next_sibling;
		DKSM_Node *prev_sibling;
		struct DKSM_GPU_Instance *first_gpu_instance;
		struct DKSM_GPU_Instance *last_gpu_instance;
		u64 gpu_instance_count;
		vec3 local_translation;
		quat local_rotation;
		vec3 local_scale;
	};

	struct DKSM_NodeChunkNode {
		DKSM_NodeChunkNode *next;
		DKSM_Node *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKSM_NodeChunkList {
		DKSM_NodeChunkNode *first;
		DKSM_NodeChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct DKSM_GPU_Instance {
		DKSM_GPU_Instance *next;
		struct DKSM_GPU_InstanceChunkNode *chunk;
		struct DKSM_GPU_Mesh *mesh;
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

	struct DKSM_GPU_Vertex {
		f32 position[3];
		// f32 normal[3]; ///< TODO(Dedrick)
		// f32 uv0[2]; ///< TODO(Dedrick)
	};

	struct DKSM_GPU_Mesh {
		struct DKSM_GPU_MeshChunkNode *chunk;
		DKSM_GPU_Vertex *vertices;
		u64 vertex_count;
		u32 *indices;
		u64 index_count;
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
	};

	struct DKSM_BakeParams {
		DKSM_TopLevelInfo top_level_info;
		DKSM_NodeChunkList nodes;
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
		DKS_TopLevelInfo *top_level_info;
	};

	struct DKSM_StringBakeResult {
		DKS_StringTable *strings_table;
		u64 strings_table_count;
		u8 *string_data;
		u64 string_data_size;
	};

	struct DKSM_NodeBakeResult {
		DKS_Node *nodes;
		u64 nodes_count;
	};

	struct DKSM_GPU_TransformBakeResult {
		DKS_GPU_Transform *gpu_transforms;
		u64 gpu_transforms_count;
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
		DKSM_NodeBakeResult nodes;
		DKSM_GPU_InstanceBakeResult gpu_instances;
		DKSM_GPU_TransformBakeResult gpu_transforms;
		DKSM_GPU_MeshBakeResult gpu_meshes;
	};

	struct DKSM_SerializedSection {
		void *data;
		u64 size;
	};

	struct DKSM_SerializedSectionBundle {
		DKSM_SerializedSection sections[DKS_SECTION_KIND_COUNT];
	};

	auto dksm_node_chunk_list_push(Arena *arena, DKSM_NodeChunkList *list, u64 capacity) noexcept -> DKSM_Node *;
	auto dksm_node_chunk_list_concat_in_place(DKSM_NodeChunkList *dst, DKSM_NodeChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_node(DKSM_Node const *node) noexcept -> u64;

	auto dksm_gpu_instance_chunk_list_push(Arena *arena, DKSM_GPU_InstanceChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Instance *;
	auto dksm_gpu_instance_chunk_list_concat_in_place(DKSM_GPU_InstanceChunkList *dst, DKSM_GPU_InstanceChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_gpu_instance(DKSM_GPU_Instance const *gpu_instance) noexcept -> u64;

	auto dksm_gpu_mesh_chunk_list_push(Arena *arena, DKSM_GPU_MeshChunkList *list, u64 capacity) noexcept -> DKSM_GPU_Mesh *;
	auto dksm_gpu_mesh_chunk_list_concat_in_place(DKSM_GPU_MeshChunkList *dst, DKSM_GPU_MeshChunkList *to_push) noexcept -> void;
	auto dksm_idx_from_gpu_mesh(DKSM_GPU_Mesh const *gpu_mesh) noexcept -> u64;

	auto dksm_bake_string_chunk_list_push(Arena *arena, DKSM_BakeStringChunkList *list, u64 capacity) noexcept -> DKSM_BakeString *;
	auto dksm_bake_string_chunk_list_concat_in_place(DKSM_BakeStringChunkList *dst, DKSM_BakeStringChunkList *to_push) noexcept -> void;
	auto dksm_bake_string_chunk_list_sorted_from_unsorted(Arena *arena, DKSM_BakeStringChunkList const *list) noexcept -> DKSM_BakeStringChunkList;

	auto dksm_bake_string_map_loose_make(Arena *arena, DKSM_BakeStringMapTopology *topology) noexcept -> DKSM_BakeStringMapLoose *;
	auto dksm_bake_string_map_loose_insert(Arena *arena, DKSM_BakeStringMapTopology *map_topology, DKSM_BakeStringMapLoose *map, u64 chunk_cap, String8 str) noexcept -> DKSM_BakeString *;
	auto dksm_bake_string_map_base_indices_from_map_loose(Arena *arena, DKSM_BakeStringMapTopology const *map_topology, DKSM_BakeStringMapLoose const *map) noexcept -> DKSM_BakeStringMapBaseIndices;
	auto dksm_bake_idx_from_string(DKSM_BakeStringMapTight const *map, String8 str) noexcept -> u32;

	auto dksm_mat4x3_from_mat4(mat4 const &src, f32 dst[12]) noexcept -> void;

	auto dksm_quantize_vertex_position(f32 const position[3], f32 const quant_factor[3], f32 const quant_offset[3]) noexcept -> u64;

	auto dksm_gpu_meshlet_triangle_from_indices(u32 i0, u32 i1, u32 i2) noexcept -> u32;

	auto dksm_bake(Arena *arena, DKSM_BakeParams const *params) noexcept -> DKSM_BakeResults;

	auto dksm_serialized_section_bundle_from_bake_results(DKSM_BakeResults const *bake_results) noexcept -> DKSM_SerializedSectionBundle;
	auto dksm_buffer_blobs_from_section_bundle(Arena *arena, DKSM_SerializedSectionBundle const *bundle) noexcept -> BufferList;
}
