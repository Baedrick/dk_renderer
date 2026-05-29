// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::pakg_string_chunk_list_push(Arena *arena, PAKG_StringChunkList *list, u64 cap) noexcept -> String8 * {
	PAKG_StringChunkNode *n = list->last;
	if (n == nullptr || n->count >= n->cap) {
		n = arena_push<PAKG_StringChunkNode>(arena);
		n->cap = cap;
		n->v = arena_push_array<String8>(arena, cap);
		forward_list_queue_push(&list->first, &list->last, n);
		list->chunk_count += 1;
	}
	String8 *result = &n->v[n->count++];
	list->total_count += 1;
	return result;
}

auto dk::pakg_shader_chunk_list_push(Arena *arena, PAKG_ShaderChunkList *list, u64 cap) noexcept -> PAK_Shader * {
	PAKG_ShaderChunkNode *n = list->last;
	if (n == nullptr || n->count >= n->cap) {
		n = arena_push<PAKG_ShaderChunkNode>(arena);
		n->cap = cap;
		n->v = arena_push_array<PAK_Shader>(arena, cap);
		forward_list_queue_push(&list->first, &list->last, n);
		list->chunk_count += 1;
	}
	PAK_Shader *result = &n->v[n->count++];
	list->total_count += 1;
	return result;
}

auto dk::pakg_bake(Arena *arena, PAKG_BakeParams *params) noexcept -> PAKG_BakeBundle {
	PAKG_BakeBundle bundle = {};

	// Dedrick: Bake strings.
	struct BakedStrings {
		PAK_SectionElementType_StringTable *string_table;
		PAK_SectionElementType_StringData *string_data;
	};
	BakedStrings baked_strings = {};
	{
		baked_strings.string_table = arena_push_array<PAK_SectionElementType_StringTable>(arena, params->strings.total_count);
		baked_strings.string_data = arena_push_array<PAK_SectionElementType_StringData>(arena, params->strings.total_size);
		u64 str_idx = 0;
		u64 str_offset = 0;
		for (PAKG_StringChunkNode *n = params->strings.first; n != nullptr; n = n->next) {
			for (u64 i = 0; i < n->count; ++i) {
				String8 str = n->v[i];
				std::memcpy(baked_strings.string_data + str_offset, str.data, str.size);
				baked_strings.string_table[str_idx].offset = str_offset;
				baked_strings.string_table[str_idx].size = str.size;
				str_offset += str.size;
				str_idx += 1;
			}
		}
	}

	// Dedrick: Bake shader binary.
	struct BakedShaderBinary {
		PAK_SectionElementType_ShaderBinary *binaries;
	};
	BakedShaderBinary baked_shader_binaries = {};
	{
		baked_shader_binaries.binaries = arena_push_array<PAK_SectionElementType_ShaderBinary>(arena, params->shader_binaries.total_size);
		u64 binary_offset = 0;
		for (Buffer8Node *n = params->shader_binaries.first; n != nullptr; n = n->next) {
			std::memcpy(baked_shader_binaries.binaries + binary_offset, n->buffer.data, n->buffer.size);
			binary_offset += n->buffer.size;
		}
	}

	// Dedrick: Bake shaders.
	struct BakedShaders {
		PAK_SectionElementType_Shader *shaders;
	};
	BakedShaders baked_shaders = {};
	{
		baked_shaders.shaders = arena_push_array<PAK_SectionElementType_Shader>(arena, params->shaders.total_count);
		u64 shader_idx = 0;
		for (PAKG_ShaderChunkNode *n = params->shaders.first; n != nullptr; n = n->next) {
			std::memcpy(baked_shaders.shaders + shader_idx, n->v, n->count * sizeof(PAK_SectionElementType_Shader));
			shader_idx += n->count;
		}
	}

	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].data = baked_strings.string_table;
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].size = params->strings.total_count * sizeof(PAK_SectionElementType_StringTable);
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].data = baked_strings.string_data;
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].size = params->strings.total_size;
	bundle.sections[PAK_SECTION_KIND_SHADER_BINARY].data = baked_shader_binaries.binaries;
	bundle.sections[PAK_SECTION_KIND_SHADER_BINARY].size = params->shader_binaries.total_size;
	bundle.sections[PAK_SECTION_KIND_SHADER].data = baked_shaders.shaders;
	bundle.sections[PAK_SECTION_KIND_SHADER].size = params->shaders.total_count * sizeof(PAK_SectionElementType_Shader);
	return bundle;
}

auto dk::pakg_serialize(Arena *arena, PAKG_BakeBundle *bundle) noexcept -> Buffer8List {
	Buffer8List blobs = {};

	PAK_Header *header = arena_push<PAK_Header>(arena);
	header->magic = PAK_MAGIC_CONSTANT;
	header->version = PAK_VERSION;
	header->section_count = PAK_SECTION_KIND_COUNT;
	header->section_offset = sizeof(PAK_Header);

	PAK_Section *sections = arena_push_array<PAK_Section>(arena, PAK_SECTION_KIND_COUNT);
	buf8_list_push(arena, &blobs, Buffer8{ reinterpret_cast<u8 *>(header), sizeof(PAK_Header) });
	buf8_list_push(arena, &blobs, Buffer8{ reinterpret_cast<u8 *>(sections), PAK_SECTION_KIND_COUNT * sizeof(PAK_Section) });

	for (u32 i = 0; i < PAK_SECTION_KIND_COUNT; ++i) {
		buf8_list_push_align(arena, &blobs, 16);
		u64 const file_offset = blobs.total_size;
		sections[i].offset = file_offset;
		sections[i].size = bundle->sections[i].size;
		if (bundle->sections[i].size > 0) {
			buf8_list_push(arena, &blobs, Buffer8{ reinterpret_cast<u8 *>(bundle->sections[i].data), bundle->sections[i].size });
		}
	}
	return blobs;
}
