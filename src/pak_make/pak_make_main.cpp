// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "pak/pak.hpp"
#include "pak_make/pak_make.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "pak/pak.cpp"
#include "pak_make/pak_make.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	using namespace dk;

	ArenaParams constexpr pg_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *pg_arena = arena_alloc(&pg_arena_params);
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	// ~ Dedrick: Extract paths.
	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 out_path = "resource.pak"_str8;
	if (cmd_line_has_argument(cmd_line, "output"_str8)) {
		out_path = cmd_line_value(cmd_line, "output"_str8);
	}

	// ~ Dedrick: Gather and process spir-v shaders.
	PAKM_ShaderList shaders = {};
	{
		ZoneScopedN("shaders");
		struct ShaderTask {
			ShaderTask *next;
			String8 binary_path;
			String8 shader_name;
			PAK_ShaderKind shader_kind;
		};
		ShaderTask *first_task = nullptr;
		ShaderTask *last_task = nullptr;
		{
			ZoneScopedN("gather");
			String8 const spirv_dir = str8f(pg_arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
			DK_LOG_INFOF("gathering shaders %.*s...", DK_STR8_VARG(spirv_dir));
			u64 task_count = 0;
			PLT_Handle const iter = plt_dir_iter_begin(spirv_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
			for (PLT_DirIterResult file = {}; plt_dir_iter_next(pg_arena, iter, &file); ) {
				if (!str8_equals(path_skip_last_period(file.name), "spv"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
					continue;
				}
				String8 const shader_name = path_chop_last_period(file.name);
				String8 const shader_ext = path_skip_last_period(shader_name);
				struct { String8 ext; PAK_ShaderKind kind; } const filters[] = {
					{ "vert"_str8, PAK_SHADER_KIND_VERTEX },
					{ "frag"_str8, PAK_SHADER_KIND_FRAGMENT },
					{ "comp"_str8, PAK_SHADER_KIND_COMPUTE }
				};
				for (u64 i = 0; i < array_count(filters); ++i) {
					if (str8_equals(shader_ext, filters[i].ext, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
						ShaderTask *task = arena_push<ShaderTask>(pg_arena);
						task->binary_path = str8f(pg_arena, "%.*s/%.*s", DK_STR8_VARG(spirv_dir), DK_STR8_VARG(file.name));
						task->shader_name = shader_name;
						task->shader_kind = filters[i].kind;
						forward_list_queue_push(&first_task, &last_task, task);
						task_count += 1;
						break;
					}
				}
			}
			plt_dir_iter_end(iter);
			DK_LOG_INFOF(" found %llu\n", task_count);
		}
		{
			ZoneScopedN("process");
			DK_LOG_INFOF("processing shaders...");
			for (ShaderTask const *task = first_task; task != nullptr; task = task->next) {
				Buffer8 const binary = plt_read_bytes_from_file_path(pg_arena, task->binary_path);
				PAKM_ShaderNode *node = arena_push<PAKM_ShaderNode>(pg_arena);
				node->name = task->shader_name;
				node->binary = binary;
				node->kind = task->shader_kind;
				forward_list_queue_push(&shaders.first, &shaders.last, node);
				shaders.count += 1;
			}
			DK_LOG_INFOF(" done\n");
		}
	}

	// ~ Dedrick: Bake Strings.
	PAK_SectionElementType_StringTable *string_table = nullptr;
	PAK_SectionElementType_StringData *string_data = nullptr;
	u64 string_table_size = 0;
	u64 string_data_size = 0;
	String8Array sorted_names = {};
	{
		ZoneScopedN("bake strings");
		String8List names = {};
		for (PAKM_ShaderNode *n = shaders.first; n != nullptr; n = n->next) {
			str8_list_push(pg_arena, &names, n->name);
		}

		sorted_names = str8_array_from_list(pg_arena, &names);
		insertion_sort(sorted_names.data, sorted_names.count,
			[](String8 a, String8 b) {
				return str8_compare(a, b, STRING_MATCH_FLAG_NONE) < 0;
			}
		);

		string_table_size = (sorted_names.count + 1) * sizeof(PAK_SectionElementType_StringTable);
		string_table = arena_push_array<PAK_SectionElementType_StringTable>(pg_arena, sorted_names.count + 1);

		string_data_size = names.total_size;
		string_data = arena_push_array<PAK_SectionElementType_StringData>(pg_arena, string_data_size);

		u64 offset = 0;
		for (u64 i = 0; i < sorted_names.count; ++i) {
			String8 const name = sorted_names[i];
			string_table[i + 1].offset = offset;
			string_table[i + 1].size = name.size;
			std::memcpy(string_data + offset, name.data, name.size);
			offset += name.size;
		}
	}

	// ~ Dedrick: Bake GPU Data.
	Buffer8 gpu_data_blob = {0};
	u64 *shader_gpu_offsets = nullptr;
	{
		ZoneScopedN("bake gpu data");
		Buffer8List gpu_data_list = {0};

		// 1. Shaders
		shader_gpu_offsets = arena_push_array<u64>(pg_arena, shaders.count);
		u64 idx = 0;
		for (PAKM_ShaderNode *n = shaders.first; n != nullptr; n = n->next, ++idx) {
			buf8_list_push_align(pg_arena, &gpu_data_list, 16);
			shader_gpu_offsets[idx] = gpu_data_list.total_size;
			buf8_list_push(pg_arena, &gpu_data_list, n->binary);
		}

		gpu_data_blob = buf8_list_join(pg_arena, &gpu_data_list);
	}

	// ~ Dedrick: Bake Metadata.
	PAK_SectionElementType_Shader *baked_shaders = nullptr;
	u64 baked_shaders_size = 0;
	{
		ZoneScopedN("bake metadata");

		// 1. Shaders
		baked_shaders_size = shaders.count * sizeof(PAK_SectionElementType_Shader);
		baked_shaders = arena_push_array<PAK_SectionElementType_Shader>(pg_arena, shaders.count);

		u64 idx = 0;
		for (PAKM_ShaderNode *n = shaders.first; n != nullptr; n = n->next, ++idx) {
			PAK_Shader *s = &baked_shaders[idx];
			s->name_hash = u64_hash_from_str8(n->name);
			s->name_string_idx = pakm_find_string_index(n->name, sorted_names);
			s->kind = n->kind;
			s->gpu_offset = shader_gpu_offsets[idx];
			s->gpu_size = n->binary.size;
		}
	}

	// ~ Dedrick: Package results.
	PAKM_BakeBundle bundle = {0};
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].data = string_table;
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].size = string_table_size;
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].data = string_data;
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].size = string_data_size;
	bundle.sections[PAK_SECTION_KIND_GPU_SHADER].data = gpu_data_blob.data;
	bundle.sections[PAK_SECTION_KIND_GPU_SHADER].size = gpu_data_blob.size;
	bundle.sections[PAK_SECTION_KIND_SHADER].data = baked_shaders;
	bundle.sections[PAK_SECTION_KIND_SHADER].size = baked_shaders_size;

	// Dedrick: Serialize bundles.
	Buffer8List output_blobs = {};
	{
		ZoneScopedN("serialize bundles");
		PAK_Header *header = arena_push<PAK_Header>(pg_arena);
		header->magic = PAK_MAGIC_CONSTANT;
		header->version = PAK_VERSION;
		header->section_count = PAK_SECTION_KIND_COUNT;
		header->section_offset = sizeof(PAK_Header);

		PAK_Section *pak_sections = arena_push_array<PAK_Section>(pg_arena, PAK_SECTION_KIND_COUNT);
		buf8_list_push(pg_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(header), sizeof(PAK_Header) });
		buf8_list_push(pg_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(pak_sections), PAK_SECTION_KIND_COUNT * sizeof(PAK_Section) });

		for (u32 i = 0; i < PAK_SECTION_KIND_COUNT; ++i) {
			buf8_list_push_align(pg_arena, &output_blobs, 16);
			pak_sections[i].offset = output_blobs.total_size;
			pak_sections[i].size = bundle.sections[i].size;
			buf8_list_push(pg_arena, &output_blobs, Buffer8{ static_cast<u8 *>(bundle.sections[i].data), bundle.sections[i].size });
		}

		header->gpu_offset = pak_sections[PAK_SECTION_KIND_GPU_SHADER].offset;
	}

	// Dedrick: Write blobs.
	{
		ZoneScopedN("write output");
		b8 const is_written = plt_write_bytes_list_to_file_path(out_path, &output_blobs);
		if (is_written) {
			DK_LOG_INFOF("written to %.*s\n", DK_STR8_VARG(out_path));
		}
		else {
			DK_LOG_ERRORF("ERROR: failed to write file %.*s\n", DK_STR8_VARG(out_path));
		}
	}

	// Dedrick: Write info & errors.
	LogFrameResult const log_frame = log_frame_end(pg_arena);
	if (cmd_line_has_flag(cmd_line, "verbose"_str8) && log_frame.kind_lists[LOG_KIND_INFO].count > 0) {
		for (LogEntry *node = log_frame.kind_lists[LOG_KIND_INFO].first; node != nullptr; node = node->next) {
			std::fprintf(stdout, "%.*s", DK_STR8_VARG(node->string));
		}
	}
	if (log_frame.kind_lists[LOG_KIND_ERROR].count > 0) {
		for (LogEntry *node = log_frame.kind_lists[LOG_KIND_ERROR].first; node != nullptr; node = node->next) {
			std::fprintf(stdout, "%.*s", DK_STR8_VARG(node->string));
		}
	}

	// Dedrick: Clean up.
	log_release(log);
	arena_release(pg_arena);
	return 0;
}
