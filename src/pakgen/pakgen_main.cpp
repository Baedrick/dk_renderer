// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "pak/pak.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "pak/pak.cpp"

using namespace dk;

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

auto pakg_string_chunk_list_push(Arena *arena, PAKG_StringChunkList *list, u64 cap) noexcept -> String8 * {
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

auto pakg_shader_chunk_list_push(Arena *arena, PAKG_ShaderChunkList *list, u64 cap) noexcept -> PAK_Shader * {
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

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	ArenaParams constexpr pg_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *pg_arena = arena_alloc(&pg_arena_params);
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	// Dedrick: Extract paths.
	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 out_path = "resource.pak"_str8;
	if (cmd_line_has_argument(cmd_line, "output"_str8)) {
		out_path = cmd_line_value(cmd_line, "output"_str8);
	}

	PAKG_BakeParams bake_params = {};

	// Dedrick: Search spir-v directory for shaders.
	{
		ZoneScopedN("gather shaders");
		struct ShaderTask {
			ShaderTask *next;
			String8 binary_path;
			String8 shader_name;
			PAK_ShaderKind shader_kind;
		};
		ShaderTask *first_task = nullptr;
		ShaderTask *last_task = nullptr;
		String8 const spirv_dir = str8f(pg_arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
		DK_LOG_INFOF("gathering shaders %.*s...", DK_STR8_VARG(spirv_dir));
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
					break;
				}
			}
		}
		plt_dir_iter_end(iter);
		for (ShaderTask const *task = first_task; task != nullptr; task = task->next) {
			Buffer8 const binary = plt_read_bytes_from_file_path(pg_arena, task->binary_path);
			u32 const str_idx = static_cast<u32>(bake_params.strings.total_count);

			String8 *str = pakg_string_chunk_list_push(pg_arena, &bake_params.strings, 64);
			*str = task->shader_name;
			bake_params.strings.total_size += task->shader_name.size;

			buf8_list_push_align(pg_arena, &bake_params.shader_binaries, 16);
			u64 shader_binary_offset = bake_params.shader_binaries.total_size;
			buf8_list_push(pg_arena, &bake_params.shader_binaries, binary);

			PAK_Shader *shader = pakg_shader_chunk_list_push(pg_arena, &bake_params.shaders, 64);
			shader->name_hash = u64_hash_from_str8(task->shader_name);
			shader->name_string_idx = str_idx;
			shader->shader_binary_offset = shader_binary_offset;
			shader->shader_binary_size = binary.size;
			shader->kind = task->shader_kind;
		}
		DK_LOG_INFOF(" %llu shaders processed\n", bake_params.shaders.total_count);
	}

	// Dedrick: Bake Strings.
	struct BakedStrings {
		PAK_SectionElementType_StringTable *string_table;
		PAK_SectionElementType_StringData *string_data;
	};
	BakedStrings *baked_strings = nullptr;
	{
		ZoneScopedN("bake strings");
		baked_strings = arena_push<BakedStrings>(pg_arena);
		baked_strings->string_table = arena_push_array<PAK_SectionElementType_StringTable>(pg_arena, bake_params.strings.total_count + 1);
		baked_strings->string_data = arena_push_array<PAK_SectionElementType_StringData>(pg_arena, bake_params.strings.total_size);
		{
			ZoneScopedN("layout strings");
			u64 str_idx = 1;
			u64 str_offset = 0;
			// TODO(Dedrick): Convert chunk nodes to linked list of 1 element per node.
			for (PAKG_StringChunkNode *node = bake_params.strings.first; node != nullptr; node = node->next) {
				for (u64 i = 0; i < node->count; ++i) {
					String8 const str = node->v[i];
					baked_strings->string_table[str_idx].offset = str_offset;
					baked_strings->string_table[str_idx].size = str.size;
					str_idx += 1;
					str_offset += str.size;
				}
			}
		}
		{
			ZoneScopedN("fill string data");
			u64 str_idx = 1;
			// TODO(Dedrick): Convert to linked list of 1 element per node.
			for (PAKG_StringChunkNode *node = bake_params.strings.first; node != nullptr; node = node->next) {
				for (u64 i = 0; i < node->count; ++i) {
					String8 const str = node->v[i];
					u64 const dst_offset = baked_strings->string_table[str_idx].offset;
					std::memcpy(baked_strings->string_data + dst_offset, str.data, str.size);
					str_idx += 1;
				}
			}
		}
	}

	// Dedrick: Bake Shader binaries.
	struct BakedShaderBinary {
		PAK_SectionElementType_ShaderBinary *binaries;
	};
	BakedShaderBinary *baked_shader_binaries = nullptr;
	{
		ZoneScopedN("bake shader binaries");
		baked_shader_binaries = arena_push<BakedShaderBinary>(pg_arena);
		baked_shader_binaries->binaries = arena_push_array<PAK_SectionElementType_ShaderBinary>(pg_arena, bake_params.shader_binaries.total_size);
		u64 binary_offset = 0;
		for (Buffer8Node *n = bake_params.shader_binaries.first; n != nullptr; n = n->next) {
			std::memcpy(baked_shader_binaries->binaries + binary_offset, n->buffer.data, n->buffer.size);
			binary_offset += n->buffer.size;
		}
	}

	// Dedrick: Bake Shaders.
	struct BakedShaders {
		PAK_SectionElementType_Shader *shaders;
	};
	BakedShaders *baked_shaders = nullptr;
	{
		ZoneScopedN("bake shaders");
		baked_shaders = arena_push<BakedShaders>(pg_arena);
		baked_shaders->shaders = arena_push_array<PAK_SectionElementType_Shader>(pg_arena, bake_params.shaders.total_count);
		u64 shader_idx = 0;
		for (PAKG_ShaderChunkNode *n = bake_params.shaders.first; n != nullptr; n = n->next) {
			std::memcpy(baked_shaders->shaders + shader_idx, n->v, n->count * sizeof(PAK_SectionElementType_Shader));
			shader_idx += n->count;
		}
	}

	// Dedrick: Package results.
	PAKG_BakeBundle bundle = {};
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].data = baked_strings->string_table;
	bundle.sections[PAK_SECTION_KIND_STRING_TABLE].size = bake_params.strings.total_count * sizeof(PAK_SectionElementType_StringTable);
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].data = baked_strings->string_data;
	bundle.sections[PAK_SECTION_KIND_STRING_DATA].size = bake_params.strings.total_size;
	bundle.sections[PAK_SECTION_KIND_SHADER_BINARY].data = baked_shader_binaries->binaries;
	bundle.sections[PAK_SECTION_KIND_SHADER_BINARY].size = bake_params.shader_binaries.total_size;
	bundle.sections[PAK_SECTION_KIND_SHADER].data = baked_shaders->shaders;
	bundle.sections[PAK_SECTION_KIND_SHADER].size = bake_params.shaders.total_count * sizeof(PAK_SectionElementType_Shader);

	// Dedrick: Serialize bundles.
	Buffer8List output_blobs = {};
	{
		ZoneScopedN("serialize bundles");
		PAK_Header *header = arena_push<PAK_Header>(pg_arena);
		header->magic = PAK_MAGIC_CONSTANT;
		header->version = PAK_VERSION;
		header->section_count = PAK_SECTION_KIND_COUNT;
		header->section_offset = sizeof(PAK_Header);

		PAK_Section *sections = arena_push_array<PAK_Section>(pg_arena, PAK_SECTION_KIND_COUNT);
		buf8_list_push(pg_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(header), sizeof(PAK_Header) });
		buf8_list_push(pg_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(sections), PAK_SECTION_KIND_COUNT * sizeof(PAK_Section) });

		for (u32 i = 0; i < PAK_SECTION_KIND_COUNT; ++i) {
			buf8_list_push_align(pg_arena, &output_blobs, 16);
			u64 const file_offset = output_blobs.total_size;
			sections[i].offset = file_offset;
			sections[i].size = bundle.sections[i].size;
			if (bundle.sections[i].size > 0) {
				buf8_list_push(pg_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(bundle.sections[i].data), bundle.sections[i].size });
			}
		}
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
