// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "dds/dds.hpp"
#include "dds/dds_parse.hpp"
#include "pak/pak.hpp"
#include "pak_make/pak_make.hpp"

#include "base/base.cpp"
#include "dds/dds.cpp"
#include "dds/dds_parse.cpp"
#include "pak/pak.cpp"
#include "pak_make/pak_make.cpp"

auto entry_point(dk::CmdLine */* cmd_line */) noexcept -> int {
	using namespace dk;

	ArenaParams constexpr arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *const arena = arena_alloc(&arena_params);
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	//~ Dedrick: Extract paths.
	String8 const binary_dir = get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 const resource_dir = str8f(arena, "%.*s/res", DK_STR8_VARG(project_dir));
	String8 const spirv_dir = str8f(arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
	String8 const output_path = str8f(arena, "%.*s/%.*s", DK_STR8_VARG(binary_dir), DK_STR8_VARG("dkrend.pak"_str8));

	//~ Dedrick: Search shader directory, gather paths, load binaries.
	PAKM_ShaderArray shaders = {};
	{
		struct ShaderTaskNode {
			ShaderTaskNode *next;
			String8 shader_name;
			String8 binary_path;
		};
		ShaderTaskNode *first_shader_task = nullptr;
		ShaderTaskNode *last_shader_task = nullptr;
		u64 shader_task_count = 0;

		//~ Dedrick: Search shader build directory for files to consider.
		DK_LOG_INFOF("searching shaders in %.*s...", DK_STR8_VARG(spirv_dir));
		DirIter const iter = dir_iter_begin(spirv_dir, DIR_ITER_FLAG_SKIP_FOLDERS);
		for (DirIterResult file = {}; dir_iter_next(arena, iter, &file); ) {
			if (!str8_equals(path_skip_last_period(file.name), "spv"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				continue;
			}
			String8 const shader_name = path_chop_last_period(file.name);
			String8 const shader_ext = path_skip_last_period(shader_name);
			String8 const filters[] = {
				{ "vert"_str8 },
				{ "frag"_str8 },
				{ "comp"_str8 }
			};
			for (u64 i = 0; i < array_count(filters); ++i) {
				if (str8_equals(shader_ext, filters[i], STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
					ShaderTaskNode *task = arena_push<ShaderTaskNode>(arena);
					task->binary_path = str8f(arena, "%.*s/%.*s", DK_STR8_VARG(spirv_dir), DK_STR8_VARG(file.name));
					task->shader_name = shader_name;
					forward_list_queue_push(&first_shader_task, &last_shader_task, task);
					shader_task_count += 1;
					break;
				}
			}
		}
		dir_iter_end(iter);
		DK_LOG_INFOF(" found %llu\n", shader_task_count);

		//~ Dedrick: Load shaders.
		DK_LOG_INFOF("processing shaders...");
		PAKM_ShaderList shaders_list = {};
		for (ShaderTaskNode const *task = first_shader_task; task != nullptr; task = task->next) {
			Buffer const binary = read_bytes_from_file_path(arena, task->binary_path);
			PAKM_ShaderNode *node = arena_push<PAKM_ShaderNode>(arena);
			node->shader.name = task->shader_name;
			node->shader.binary = binary;
			forward_list_queue_push(&shaders_list.first, &shaders_list.last, node);
			shaders_list.count += 1;
		}

		//~ Dedrick: Join all shaders.
		shaders.data = arena_push_array<PAKM_Shader>(arena, shaders_list.count);
		shaders.count = shaders_list.count;
		u64 idx = 0;
		for (PAKM_ShaderNode const *node = shaders_list.first; node != nullptr; node = node->next) {
			shaders[idx++] = node->shader;
		}
		DK_LOG_INFOF(" %llu processed\n", shaders.count);
	}

	//~ Dedrick: Gather files in resource directory to consider.
	String8List textures_path_list = {};
	{
		DK_LOG_INFOF("searching %.*s...\n", DK_STR8_VARG(resource_dir));
		DirIter const iter = dir_iter_begin(resource_dir, DIR_ITER_FLAG_SKIP_FOLDERS);
		for (DirIterResult file = {}; dir_iter_next(arena, iter, &file); ) {
			String8 const file_path = str8f(arena, "%.*s/%.*s", DK_STR8_VARG(resource_dir), DK_STR8_VARG(file.name));
			String8 const file_ext = path_skip_last_period(file_path);
			struct { String8 ext; String8List *file_paths_list; } const file_path_table[] = {
				{ "dds"_str8, &textures_path_list }
			};
			for (u64 i = 0; i < array_count(file_path_table); ++i) {
				if (str8_equals(file_ext, file_path_table[i].ext, STRING_MATCH_FLAG_NONE)) {
					str8_list_push(arena, file_path_table[i].file_paths_list, file_path);
				}
			}
		}
		dir_iter_end(iter);
		DK_LOG_INFOF("  found %llu textures\n", textures_path_list.node_count);
	}

	//~ Dedrick: Parse all texture files in resource directory.
	PAKM_TextureArray textures = {};
	{
		PAKM_TextureList textures_list = {};

		//~ Dedrick: Parse textures.
		DK_LOG_INFOF("processing textures...");
		for (String8Node const *path = textures_path_list.first; path != nullptr; path = path->next) {
			String8 const file_path = path->string;
			Buffer const file_data = read_bytes_from_file_path(arena, file_path);

			//~ Dedrick: Parse DDS file.
			DDS_Parsed parsed = {};
			if (dds_parse(file_data, &parsed)) {
				DDS_Header const *header = parsed.header;
				PAK_TextureKind tex_kind = PAK_TEXTURE_KIND_2D;
				if (header->depth > 1) {
					tex_kind = PAK_TEXTURE_KIND_3D;
				}
				PAK_TextureFormat tex_format = PAK_TEXTURE_FORMAT_NULL;
				if (parsed.dxt10_header != nullptr) {
					DDS_DXT10Header const *dxt10_header = parsed.dxt10_header;
					switch (dxt10_header->dxgi_format) {
						case DDS_DXGI_FORMAT_R9G9B9E5: tex_format = PAK_TEXTURE_FORMAT_RGB9E5; break;
						default: break;
					}
				}
				if (tex_format != PAK_TEXTURE_FORMAT_NULL) {
					String8 const tex_name = path_skip_last_slash(file_path);
					PAKM_TextureNode *const node = arena_push<PAKM_TextureNode>(arena);
					node->texture.kind = tex_kind;
					node->texture.format = tex_format;
					node->texture.width = header->width;
					node->texture.height = header->height;
					node->texture.depth = header->depth > 0 ? header->depth : 1;
					node->texture.mip_count = header->mip_map_count > 0 ? header->mip_map_count : 1;
					node->texture.name = tex_name;
					node->texture.pixels = parsed.image_data;
					forward_list_queue_push(&textures_list.first, &textures_list.last, node);
					textures_list.count += 1;
				}
			}
		}

		//~ Dedrick: Join all textures.
		textures.data = arena_push_array<PAKM_Texture>(arena, textures_list.count);
		textures.count = textures_list.count;
		u64 idx = 0;
		for (PAKM_TextureNode const *node = textures_list.first; node != nullptr; node = node->next) {
			textures[idx++] = node->texture;
		}
		DK_LOG_INFOF(" %llu processed\n", textures.count);
	}

	//~ Dedrick: Build strings.
	String8Array strings = {};
	{
		String8List strings_list = {};

		//~ Dedrick: Push strings from shaders.
		for (u64 idx = 0; idx < shaders.count; ++idx) {
			str8_list_push(arena, &strings_list, shaders[idx].name);
		}

		//~ Dedrick: Push strings from textures.
		for (u64 idx = 0; idx < textures.count; ++idx) {
			str8_list_push(arena, &strings_list, textures[idx].name);
		}

		//~ Dedrick: Join all strings.
		strings.data = arena_push_array<String8>(arena, strings_list.node_count);
		strings.count = strings_list.node_count;
		u64 idx = 0;
		for (String8Node const *node = strings_list.first; node != nullptr; node = node->next) {
			strings[idx++] = node->string;
		}

		//~ Dedrick: Sort.
		strings = pakm_strings_sorted_from_unsorted_in_place(strings);
	}

	//~ Dedrick: Bake strings.
	struct PAKM_StringBakeResult {
		PAK_SectionElementType_StringTable *string_tables;
		u64 string_tables_count;
		PAK_SectionElementType_StringData *string_data;
		u64 string_data_size;
	};
	PAKM_StringBakeResult baked_strings = {};
	{
		//~ Dedrick: Set up.
		baked_strings.string_tables_count = strings.count + 1;
		baked_strings.string_tables = arena_push_array<PAK_SectionElementType_StringTable>(arena, baked_strings.string_tables_count);
		u64 offset_cursor = 0;
		for (u64 idx = 0; idx < strings.count; ++idx) {
			String8 const str = strings[idx];
			baked_strings.string_tables[idx + 1] = { offset_cursor, str.size };
			offset_cursor += str.size;
		}

		//~ Dedrick: Fill.
		baked_strings.string_data_size = offset_cursor;
		baked_strings.string_data = arena_push_array<PAK_SectionElementType_StringData>(arena, baked_strings.string_data_size);
		for (u64 idx = 0; idx < strings.count; ++idx) {
			String8 const str = strings[idx];
			u64 const dst_offset = baked_strings.string_tables[idx + 1].offset;
			std::memcpy(baked_strings.string_data + dst_offset, str.data, str.size);
		}
	}

	//~ Dedrick: Bake Shaders.
	PAKM_ShaderBakeResult baked_shaders = {};
	{
		//~ Dedrick: Set up.
		baked_shaders.metadata_size = shaders.count * sizeof(PAK_SectionElementType_Shader);
		baked_shaders.metadata = arena_push_array<PAK_SectionElementType_Shader>(arena, shaders.count);
		u64 offset_cursor = 0;
		for (u64 idx = 0; idx < shaders.count; ++idx) {
			PAKM_Shader const *src = &shaders[idx];
			PAK_SectionElementType_Shader *dst = baked_shaders.metadata + idx;
			dst->name_hash = u64_hash_from_str8(src->name);
			dst->name_string_idx = pakm_find_string_index(src->name, strings);
			dst->pad = 0;
			dst->offset = offset_cursor;
			dst->size = src->binary.size;
			offset_cursor += src->binary.size;
		}

		//~ Dedrick: Fill.
		baked_shaders.data_size = offset_cursor;
		baked_shaders.data = arena_push_array<PAK_SectionElementType_ShaderData>(arena, baked_shaders.data_size);
		for (u64 idx = 0; idx < shaders.count; ++idx) {
			PAKM_Shader const *src = &shaders[idx];
			u64 const dst_offset = baked_shaders.metadata[idx].offset;
			std::memcpy(baked_shaders.data + dst_offset, src->binary.data, src->binary.size);
		}
	}

	//~ Dedrick: Bake Textures.
	PAKM_TextureBakeResult baked_textures = {};
	{
		//~ Dedrick: Set up.
		baked_textures.metadata_size = textures.count * sizeof(PAK_SectionElementType_Texture);
		baked_textures.metadata = arena_push_array<PAK_SectionElementType_Texture>(arena, textures.count);
		u64 offset_cursor = 0;
		for (u64 idx = 0; idx < textures.count; ++idx) {
			PAKM_Texture const *src = &textures[idx];
			PAK_SectionElementType_Texture *dst = baked_textures.metadata + idx;
			dst->name_hash = u64_hash_from_str8(src->name);
			dst->name_string_idx = pakm_find_string_index(src->name, strings);
			dst->kind = src->kind;
			dst->format = src->format;
			dst->width = src->width;
			dst->height = src->height;
			dst->depth = src->depth;
			dst->mip_count = src->mip_count;
			dst->offset = offset_cursor;
			dst->size = src->pixels.size;
			offset_cursor += src->pixels.size;
		}

		//~ Dedrick: Fill.
		baked_textures.data_size = offset_cursor;
		baked_textures.data = arena_push_array<PAK_SectionElementType_TextureData>(arena, baked_textures.data_size);
		for (u64 idx = 0; idx < textures.count; ++idx) {
			PAKM_Texture const *src = &textures[idx];
			u64 const dst_offset = baked_textures.metadata[idx].offset;
			std::memcpy(baked_textures.data + dst_offset, src->pixels.data, src->pixels.size);
		}
	}

	//~ Dedrick: Package results.
	PAKM_BakeBundle bundle = {};
	{
		bundle.sections[PAK_SECTION_KIND_NULL]         = { nullptr, 0 };
		bundle.sections[PAK_SECTION_KIND_STRING_TABLE] = { baked_strings.string_tables, baked_strings.string_tables_count * sizeof(PAK_SectionElementType_StringTable) };
		bundle.sections[PAK_SECTION_KIND_STRING_DATA]  = { baked_strings.string_data, baked_strings.string_data_size };
		bundle.sections[PAK_SECTION_KIND_SHADER]       = { baked_shaders.metadata, baked_shaders.metadata_size };
		bundle.sections[PAK_SECTION_KIND_TEXTURE]      = { baked_textures.metadata, baked_textures.metadata_size };
		bundle.sections[PAK_SECTION_KIND_SHADER_DATA]  = { baked_shaders.data, baked_shaders.data_size };
		bundle.sections[PAK_SECTION_KIND_TEXTURE_DATA] = { baked_textures.data, baked_textures.data_size };
	}

	//~ Dedrick: Serialize bundles.
	BufferList output_blobs = {};
	{
		PAK_Header *header = arena_push<PAK_Header>(arena);
		header->magic = PAK_MAGIC_CONSTANT;
		header->version = PAK_VERSION;
		header->section_count = PAK_SECTION_KIND_COUNT;
		header->section_offset = sizeof(PAK_Header);

		PAK_Section *pak_sections = arena_push_array<PAK_Section>(arena, PAK_SECTION_KIND_COUNT);
		buf_list_push(arena, &output_blobs, buf(header, sizeof(PAK_Header)));
		buf_list_push(arena, &output_blobs, buf(pak_sections, PAK_SECTION_KIND_COUNT * sizeof(PAK_Section)));

		for (u32 k = 0; k< PAK_SECTION_KIND_COUNT; ++k) {
			buf_list_push_align(arena, &output_blobs, 16);
			pak_sections[k].offset = output_blobs.total_size;
			pak_sections[k].size = bundle.sections[k].size;
			if (bundle.sections[k].size > 0) {
				buf_list_push(arena, &output_blobs, buf(bundle.sections[k].data, bundle.sections[k].size));
			}
		}

		header->metadata_size = pak_sections[PAK_SECTION_KIND_SHADER_DATA].offset;
	}

	//~ Dedrick: Write blobs.
	{
		b8 const is_written = write_bytes_list_to_file_path(output_path, &output_blobs);
		if (is_written) {
			DK_LOG_INFOF("written to %.*s\n", DK_STR8_VARG(output_path));
		}
		else {
			DK_LOG_ERRORF("ERROR: failed to write file %.*s\n", DK_STR8_VARG(output_path));
		}
	}

	//~ Dedrick: Collect logs.
	LogFrameResult const log_frame = log_frame_end(arena);
	std::fwrite(log_frame.string.data, log_frame.string.size, 1, stdout);

	// NOTE(Dedrick): Intentional leak because this is a short-lived application.
	return 0;
}
