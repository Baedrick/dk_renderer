// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "pak/pak.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "pak/pak.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	using namespace dk;
	(void)cmd_line;
	ArenaParams constexpr pg_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *pg_arena = arena_alloc(&pg_arena_params);

	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 const code_dir = str8f(pg_arena, "%.*s/src/shaders", DK_STR8_VARG(project_dir));

	// NOTE(Dedrick): Recursively search directories for all files to consider.
	// TODO(Dedrick); Spir-V binaries will only be built to a specific folder, only need to iterate that.
	String8List file_paths = {};
	struct Task {
		Task *next;
		String8 path;
	};
	Task start_task = { nullptr, code_dir };
	Task *first_task = &start_task;
	Task *last_task = &start_task;
	std::printf("searching %.*s...", DK_STR8_VARG(code_dir));
	for (Task *task = first_task; task != nullptr; task = task->next) {
		PLT_Handle const iter = plt_dir_iter_begin(task->path, PLT_DIR_ITER_FLAG_NONE);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pg_arena, iter, &file); ) {
			String8 const file_path = str8f(pg_arena, "%.*s/%.*s", DK_STR8_VARG(task->path), DK_STR8_VARG(file.name));
			if ((file.attributes.flags & PLT_FILE_FLAG_DIRECTORY) != 0) {
				Task *next_task = arena_push<Task>(pg_arena);
				forward_list_queue_push(&first_task, &last_task, next_task);
				next_task->path = file_path;
			} else {
				str8_list_push(pg_arena, &file_paths, file_path);
			}
		}
		plt_dir_iter_end(iter);
	}
	std::printf("%llu files found\n", file_paths.node_count);
	return 0;
}
