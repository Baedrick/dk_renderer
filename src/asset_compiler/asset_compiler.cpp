// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::ASC_Shared *dk::asc_shared;

auto dk::asc_entry_point(CmdLine *cmd_line) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	u64 threads_count = get_system_info()->logical_processor_count;
	String8 const threads_count_from_cmd_line_str = cmd_line_value(cmd_line, "thread_count"_str8);
	if (threads_count_from_cmd_line_str.size > 0) {
		u64 threads_count_from_cmd_line = u64_from_str8(threads_count_from_cmd_line_str, 10);
		if (threads_count_from_cmd_line > 0) {
			threads_count = threads_count_from_cmd_line;
		}
	}
	Thread *threads = arena_push_array<Thread>(scratch.arena, threads_count);
	ASC_ThreadParams *threads_params = arena_push_array<ASC_ThreadParams>(scratch.arena, threads_count);
	Barrier const barrier = barrier_alloc(threads_count);
	u64 broadcast_value = 0;
	for (u64 idx = 0; idx < threads_count; ++idx) {
		threads_params[idx].cmd_line = cmd_line;
		threads_params[idx].lane_context.lane_idx = idx;
		threads_params[idx].lane_context.lane_count = threads_count;
		threads_params[idx].lane_context.barrier = barrier;
		threads_params[idx].lane_context.broadcast_memory = &broadcast_value;
		threads[idx] = thread_launch(asc_thread_entry_point, &threads_params[idx]);
	}
	for (u64 i = 0; i < threads_count; ++i) {
		thread_join(threads[i]);
	}
	scratch_end(scratch);
}

auto dk::asc_thread_entry_point(void *p) noexcept -> void {
	//~ Dedrick: Set up thread state.
	ASC_ThreadParams *params = static_cast<ASC_ThreadParams *>(p);
	CmdLine *cmd_line = params->cmd_line;
	LaneContext lane_context = params->lane_context;
	set_thread_namef("compiler_thread_%llu", lane_context.lane_idx);
	lane_context_select(lane_context);
	Arena *arena = arena_alloc();
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	//~ Dedrick: Set up shared state.
	if (lane_idx == 0) {
		asc_shared = arena_push<ASC_Shared>(arena);
	}
	lane_sync();

	//~ Dedrick: Analyze and load command line asset input files.
	{
		ZoneScopedN("analyze and load command line input files");
		if (lane_idx() == 0) {
			String8List input_file_path_tasks = str8_list_copy(arena, &cmd_line->inputs);
			for (String8Node const *node = input_file_path_tasks.first; node != nullptr; node = node->next) {
				ZoneScopedN("analysis of file");

				ASC_FileFormat file_format = ASC_FILE_FORMAT_NULL;
				ASC_FileFormatFlags file_format_flags = ASC_FILE_FORMAT_FLAG_NONE;
				File const file = file_open(node->string, FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_SHARE_READ);
				FileAttributes const file_attr = attributes_from_file(file);
				(void)file_format;
				(void)file_format_flags;
				(void)file;
				(void)file_attr;

				//~ Dedrick: GLB magic -> GLB input.

				//~ Dedrick: EXR magic -> HDRI input.

				//~ Dedrick: PNG/JPEG magic -> Texture input.

				//~ Dedrick: GLTF ext -> GLTF input.

				//~ Dedrick: Load recognized file?

				//~ Dedrick: Mesh buffer?

				//~ Dedrick: GLB format -> generate new tasks for textures, meshes.

				//~ Dedrick: GLTF format -> generate new tasks for textures, meshes.

				//~ Dedrick: Bucket input file by format.
			}
		}
		lane_sync();
	}

	//~ Dedrick: Unpack output and to where.
	enum OutputKind {
		OUTPUT_KIND_NULL,
		OUTPUT_KIND_DKS,
		OUTPUT_KIND_COUNT
	};
	struct { String8 flag; String8 title; } const output_kind_info[] = {
		{ ""_str8, ""_str8 },
		{ "dks"_str8, "DK Scene (.dks) Conversion"_str8 },
	};
	OutputKind output_kind = OUTPUT_KIND_NULL;
	String8 output_path = cmd_line_value(cmd_line, "out"_str8);
	{
		//~ Dedrick: User manually specified output kind.
		if (output_kind == OUTPUT_KIND_NULL) {
			for (u64 k = OUTPUT_KIND_NULL + 1; k < OUTPUT_KIND_COUNT; ++k) {
				if (cmd_line_has_flag(cmd_line, output_kind_info[k].flag)) {
					output_kind = static_cast<OutputKind>(k);
					DK_LOG_INFOF("Specified --%.*s; performing `%.*s`\n", DK_STR8_VARG(output_kind_info[k].flag), DK_STR8_VARG(output_kind_info[k].title));
					break;
				}
			}
		}

		//~ Dedrick: Infer output kind from user specified path.
		if (output_kind == OUTPUT_KIND_NULL) {
			if (str8_equals(path_skip_last_period(output_path), "dks"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				output_kind = OUTPUT_KIND_DKS;
				DK_LOG_INFOF("Output path has .dks extension; performing `%.*s`\n", DK_STR8_VARG(output_kind_info[output_kind].title));
			}
		}
	}

	// TODO(Dedrick): Handle case when no output path is specified. Use input file and format.

	//~ Dedrick: Collect logs
	LogFrameResult const log_frame = log_frame_end(arena);
	if (lane_idx() == 0) {
		// TODO(Dedrick): Write to shared memory so the renderer can display in its logs.
		std::fwrite(log_frame.string.data, log_frame.string.size, 1, stdout);
	}
}
