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
	(void)cmd_line;

	ArenaParams constexpr pm_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *const pm_arena = arena_alloc(&pm_arena_params);

	//~ Dedrick: Extract paths.
	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 const res_dir = str8f(pm_arena, "%.*s/res", DK_STR8_VARG(project_dir));
	String8 const spirv_dir = str8f(pm_arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
	String8 const out_path = str8f(pm_arena, "%.*s/%.*s", binary_dir, DK_STR8_VARG("dkrend.pak"_str8));

	//~ Dedrick: Search shader directory, gather paths, load binaries.
	PAKM_ShaderArray shaders = {};
	{
		struct ShaderTaskNode {
			ShaderTaskNode *next;
			PAK_ShaderKind shader_kind;
			String8 shader_name;
			String8 binary_path;
		};
		ShaderTaskNode *first_shader_task = nullptr;
		ShaderTaskNode *last_shader_task = nullptr;
		u64 shader_task_count = 0;

		//~ Dedrick: Search shader build directory for files to consider.
		std::printf("searching shaders in %.*s...", DK_STR8_VARG(spirv_dir));
		PLT_Handle const iter = plt_dir_iter_begin(spirv_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pm_arena, iter, &file); ) {
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
					ShaderTaskNode *task = arena_push<ShaderTaskNode>(pm_arena);
					task->binary_path = str8f(pm_arena, "%.*s/%.*s", DK_STR8_VARG(spirv_dir), DK_STR8_VARG(file.name));
					task->shader_name = shader_name;
					task->shader_kind = filters[i].kind;
					forward_list_queue_push(&first_shader_task, &last_shader_task, task);
					shader_task_count += 1;
					break;
				}
			}
		}
		plt_dir_iter_end(iter);
		std::printf(" found %llu\n", shader_task_count);

		//~ Dedrick: Load shaders.
		std::printf("processing shaders...");
		PAKM_ShaderList shaders_list = {};
		for (ShaderTaskNode const *task = first_shader_task; task != nullptr; task = task->next) {
			Buffer8 const binary = plt_read_bytes_from_file_path(pm_arena, task->binary_path);
			PAKM_ShaderNode *node = arena_push<PAKM_ShaderNode>(pm_arena);
			node->shader.name = task->shader_name;
			node->shader.binary = binary;
			node->shader.kind = task->shader_kind;
			forward_list_queue_push(&shaders_list.first, &shaders_list.last, node);
			shaders_list.count += 1;
		}
		std::printf(" %llu shader files processed\n", shaders_list.count);

		//~ Dedrick: Join all shaders.
		shaders.data = arena_push_array<PAKM_Shader>(pm_arena, shaders_list.count);
		shaders.count = shaders_list.count;
		u64 shader_idx = 0;
		for (PAKM_ShaderNode const *node = shaders_list.first; node != nullptr; node = node->next) {
			shaders[shader_idx++] = node->shader;
		}
	}

	//~ Dedrick: Recursively search resource directory for all files to consider.
	String8List res_file_paths = {};
	{
		std::printf("searching %.*s...", DK_STR8_VARG(res_dir));
		PLT_Handle const iter = plt_dir_iter_begin(res_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pm_arena, iter, &file); ) {
			String8 const file_path = str8f(pm_arena, "%.*s/%.*s", DK_STR8_VARG(res_dir), DK_STR8_VARG(file.name));
			if ((file.attributes.flags & PLT_FILE_FLAG_DIRECTORY) == 0) {
				str8_list_push(pm_arena, &res_file_paths, file_path);
			}
		}
		plt_dir_iter_end(iter);
		std::printf(" %llu files found\n", res_file_paths.node_count);
	}

	//~ Dedrick: Parse all texture files in resource directory.
	PAKM_TextureArray textures = {};
	{
		PAKM_TextureList textures_list = {};
		struct DDS_PIXELFORMAT {
			u32 dwSize;
			u32 dwFlags;
			u32 dwFourCC;
			u32 dwRGBBitCount;
			u32 dwRBitMask;
			u32 dwGBitMask;
			u32 dwBBitMask;
			u32 dwABitMask;
		};
		struct DDS_HEADER {
			u32 dwSize;
			u32 dwFlags;
			u32 dwHeight;
			u32 dwWidth;
			u32 dwLinearSize;
			u32 dwDepth;
			u32 dwMipMapCount;
			u32 dwReserved1[11];
			DDS_PIXELFORMAT ddspf;
			u32 dwCaps;
			u32 dwCaps2;
			u32 dwCaps3;
			u32 dwCaps4;
			u32 dwReserved2;
		};

		//~ Dedrick: Parse textures.
		std::printf("parsing textures...");
		for (String8Node const *node = res_file_paths.first; node != nullptr; node = node->next) {
			String8 const file_path = node->string;
			String8 const file_ext = path_skip_last_period(file_path);
			if (str8_equals(file_ext, "dds"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				String8 const texture_name = path_skip_last_slash(file_path);


			}
		}
		std::printf(" %llu textuers parsed\n", textures_list.count);


		//~ Dedrick: Join all textures.
		textures.data = arena_push_array<PAKM_Texture>(pm_arena, textures_list.count);
		textures.count = textures_list.count;
		u64 texture_idx = 0;
		for (PAKM_TextureNode const *node = textures_list.first; node != nullptr; node = node->next) {
			textures[texture_idx++] = node->texture;
		}
	}

	//~ Dedrick: Bake strings.


	//~ Dedrick: Bake GPU data.


	//~ Dedrick: Bake CPU data.


	//~ Dedrick: Package results.


	//~ Dedrick: Serialize bundles.


	//~ Dedrick: Write blobs.
	{
		ZoneScopedN("write output")
		std::printf("write output to file\n");
		b8 const is_written = plt_write_bytes_list_to_file_path(out_path, &output_blobs);
		if (is_written) {
			std::printf("written to %.*s\n", DK_STR8_VARG(out_path));
		}
		else {
			std::printf("ERROR: failed to write file %.*s\n", DK_STR8_VARG(out_path));
		}
	}

	// NOTE(Dedrick): Intentional leak because this is a short-lived application.
}

#if 0
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

	//~ Dedrick: Extract paths.
	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 out_path = "resource.pak"_str8;
	if (cmd_line_has_argument(cmd_line, "output"_str8)) {
		out_path = cmd_line_value(cmd_line, "output"_str8);
	}

	//~ Dedrick: Gather and process spir-v shaders.
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

	// TODO(Dedrick): Gather and process textures.
	{
		ZoneScopedN("textures");

		//~ Dedrick: Read Header.
		struct DDS_PIXELFORMAT {
			u32 dwSize;
			u32 dwFlags;
			u32 dwFourCC;
			u32 dwRGBBitCount;
			u32 dwRBitMask;
			u32 dwGBitMask;
			u32 dwBBitMask;
			u32 dwABitMask;
		};
		struct DDS_HEADER {
			u32 dwSize;
			u32 dwFlags;
			u32 dwHeight;
			u32 dwWidth;
			u32 dwLinearSize;
			u32 dwDepth;
			u32 dwMipMapCount;
			u32 dwReserved1[11];
			DDS_PIXELFORMAT ddspf;
			u32 dwCaps;
			u32 dwCaps2;
			u32 dwCaps3;
			u32 dwCaps4;
			u32 dwReserved2;
		};
	}

	//~ Dedrick: Bake Strings.
	PAK_SectionElementType_StringTable *string_table = nullptr;
	PAK_SectionElementType_StringData *string_data = nullptr;
	u64 string_table_size = 0;
	u64 string_data_size = 0;
	String8Array sorted_names = {};
	{
		ZoneScopedN("bake strings");
		DK_LOG_INFOF("baking strings...");
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
		DK_LOG_INFOF(" done\n");
	}

	//~ Dedrick: Bake GPU Data.
	Buffer8 gpu_data_blob = {};
	u64 *shader_gpu_offsets = nullptr;
	{
		ZoneScopedN("bake gpu data");
		DK_LOG_INFOF("baking gpu data...");
		Buffer8List gpu_data_list = {};

		// ~ Dedrick: Shader Data.
		shader_gpu_offsets = arena_push_array<u64>(pg_arena, shaders.count);
		u64 idx = 0;
		for (PAKM_ShaderNode *n = shaders.first; n != nullptr; n = n->next, ++idx) {
			buf8_list_push_align(pg_arena, &gpu_data_list, 16);
			shader_gpu_offsets[idx] = gpu_data_list.total_size;
			buf8_list_push(pg_arena, &gpu_data_list, n->binary);
		}
		// TODO(Dedrick): Texture Data.

		gpu_data_blob = buf8_list_join(pg_arena, &gpu_data_list);
		DK_LOG_INFOF(" done\n");
	}

	//~ Dedrick: Bake Metadata.
	PAK_SectionElementType_Shader *baked_shaders = nullptr;
	u64 baked_shaders_size = 0;
	{
		ZoneScopedN("bake metadata");
		DK_LOG_INFOF("baking metadata...");
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
		DK_LOG_INFOF(" done\n");
	}

	//~ Dedrick: Package results.
	PAKM_BakeBundle bundle = {};
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE] = { string_table, string_table_size };
	bundle.sections[PAK_SECTION_KIND_STRING_DATA] = { string_data, string_data_size };
	bundle.sections[PAK_SECTION_KIND_GPU_SHADER] = { gpu_data_blob.data, gpu_data_blob.size };
	bundle.sections[PAK_SECTION_KIND_SHADER] = { baked_shaders, baked_shaders_size };

	//~ Dedrick: Serialize bundles.
	Buffer8List output_blobs = {};
	{
		ZoneScopedN("serialize bundles");
		DK_LOG_INFOF("serializing bundles...");
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
		DK_LOG_INFOF(" done\n");
	}

	//~ Dedrick: Write blobs.
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

	//~ Dedrick: Write all logs.
	LogFrameResult const log_frame = log_frame_end(pg_arena);
	if (log_frame.list.count > 0) {
		for (LogEntry *node = log_frame.list.first; node != nullptr; node = node->next) {
			std::fprintf(stdout, "%.*s", DK_STR8_VARG(node->string));
		}
	}

	//~ Dedrick: Clean up.
	log_release(log);
	arena_release(pg_arena);
	return 0;
}
#endif
