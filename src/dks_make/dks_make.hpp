// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct DKSM_TopLevelInfo {
		String8 model_name;
		vec3 dequantization_factor;
		vec3 dequantization_summand;
	};

	struct DKSM_Object {
		String8 name;
		vec3 center;
		vec3 extents;

	};

	struct DKSM_ObjectChunkNode {
		DKSM_ObjectChunkNode *next;
		DKSM_Object *data;
		u64 count;
		u64 capacity;
		u64 base_idx;
	};

	struct DKS_ObjectChunkList {
		DKSM_ObjectChunkNode *first;
		DKSM_ObjectChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct DKSM_BakeParams {
		DKSM_TopLevelInfo top_level_info;
		DKS_ObjectChunkList objects;
	};

	struct DKSM_TopLevelInfoBakeResult {
		DKSM_TopLevelInfo *top_level_info;
	};

	struct DKSM_ObjectBakeResult {

	};

	struct DKSM_MeshBakeResult {

	};

	struct DKSM_BakeResults {

	};

	struct DKSM_SerializedSection {
		void *data;
		u64 size;
	};

	struct DKSM_SerializedSectionBundle {
		DKSM_SerializedSection sections[DKS_SECTION_KIND_COUNT];
	};


	auto dksm_serialized_section_bundle_from_bake_results() noexcept -> DKSM_SerializedSectionBundle;
	auto dksm_buffer_blobs_from_section_bundle() noexcept -> BufferList;
}
