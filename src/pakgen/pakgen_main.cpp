// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "pak/pak.hpp"
#include "pakgen/pakgen.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "pak/pak.cpp"
#include "pakgen/pakgen.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	using namespace dk;

	ArenaParams constexpr pg_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *pg_arena = arena_alloc(&pg_arena_params);

	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);

	// Dedrick: Search spriv directory for all files to pack.
	String8 const spriv_dir = str8f(pg_arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
	PAKG_BakeParams bake_params = {};

	std::printf("gathering shaders (.spv) %.*s...", DK_STR8_VARG(spriv_dir));
	{
		PLT_Handle const iter = plt_dir_iter_begin(spriv_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pg_arena, iter, &file); ) {
			// TODO(Dedrick): Protect against stray files.

			String8 const file_path = str8f(pg_arena, "%.*s/%.*s", DK_STR8_VARG(spriv_dir), DK_STR8_VARG(file.name));
			Buffer8 binary = plt_read_bytes_from_file_path(pg_arena, file_path);

			String8 shader_name = file.name;
			String8 ext = path_skip_last_period(shader_name);
			if (str8_equals(ext, "spv"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				shader_name = path_chop_last_period(shader_name);
			}

			u32 str_idx = static_cast<u32>(bake_params.strings.total_count);
			String8 *str_ptr = pakg_string_chunk_list_push(pg_arena, &bake_params.strings, 64);
			*str_ptr = shader_name;
			bake_params.strings.total_size += shader_name.size;

			buf8_list_push_align(pg_arena, &bake_params.shader_binaries, 16);
			u64 shader_binary_offset = bake_params.shader_binaries.total_size;
			buf8_list_push(pg_arena, &bake_params.shader_binaries, binary);

			PAK_Shader *sh = pakg_shader_chunk_list_push(pg_arena, &bake_params.shaders, 64);
			sh->name_hash = u64_hash_from_str8(shader_name);
			sh->name_string_idx = str_idx;
			sh->shader_binary_offset = shader_binary_offset;
			sh->shader_binary_size = binary.size;

			sh->kind = PAK_SHADER_KIND_COUNT;
			String8 shader_ext = path_skip_last_period(shader_name);
			if (str8_equals(shader_ext, "vert"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				sh->kind = PAK_SHADER_KIND_VERTEX;
			}
			else if (str8_equals(shader_ext, "frag"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				sh->kind = PAK_SHADER_KIND_FRAGMENT;
			}
			else if (str8_equals(shader_ext, "comp"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				sh->kind = PAK_SHADER_KIND_COMPUTE;
			}
		}
		plt_dir_iter_end(iter);
	}
	std::printf(" %llu shaders processed\n", bake_params.shaders.total_count);

	PAKG_BakeBundle bundle = pakg_bake(pg_arena, &bake_params);
	Buffer8List blobs = pakg_serialize(pg_arena, &bundle);

	// TODO(Dedrick): Move out_file_path to top of file.
	String8 out_file_name = "resources.pak"_str8;
	if (cmd_line_has_argument(cmd_line, "output"_str8)) {
		out_file_name = cmd_line_value(cmd_line, "output"_str8);
	}
	String8 const out_path = str8f(pg_arena, "%.*s/%.*s", DK_STR8_VARG(binary_dir), DK_STR8_VARG(out_file_name));
	std::printf("writing %.*s...", DK_STR8_VARG(out_path));

	// Dedrick: Write blobs.
	if (blobs.first) {
		plt_write_bytes_to_file_path(out_path, blobs.first->buffer);
		for (Buffer8Node *n = blobs.first->next; n != nullptr; n = n->next) {
			plt_append_bytes_to_file_path(out_path, n->buffer);
		}
	}

	std::printf(" done\n");
	return 0;
}
