// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct PAKG_StringChunkNode {
		PAKG_StringChunkNode *next;
		String8 *v;
		u64 count;
		u64 cap;
	};

	struct PAKG_StringChunkList {
		PAKG_StringChunkNode *first;
		PAKG_StringChunkNode *last;
		u64 chunk_count;
		u64 total_count;
		u64 total_size;
	};

	struct PAKG_ShaderChunkNode {
		PAKG_ShaderChunkNode *next;
		PAK_Shader *v;
		u64 count;
		u64 cap;
	};

	struct PAKG_ShaderChunkList {
		PAKG_ShaderChunkNode *first;
		PAKG_ShaderChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct PAKG_BakeParams {
		PAKG_StringChunkList strings;
		PAKG_ShaderChunkList shaders;
		Buffer8List shader_binaries;
	};

	struct PAKG_BakeSection {
		void *data;
		u64 size;
	};

	struct PAKG_BakeBundle {
		PAKG_BakeSection sections[PAK_SECTION_KIND_COUNT];
	};

	auto pakg_string_chunk_list_push(Arena *arena, PAKG_StringChunkList *list, u64 cap) noexcept -> String8 *;
	auto pakg_shader_chunk_list_push(Arena *arena, PAKG_ShaderChunkList *list, u64 cap) noexcept -> PAK_Shader *;

	auto pakg_bake(Arena *arena, PAKG_BakeParams *params) noexcept -> PAKG_BakeBundle;
	auto pakg_serialize(Arena *arena, PAKG_BakeBundle *bundle) noexcept -> Buffer8List;
}
