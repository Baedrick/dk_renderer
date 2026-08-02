// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::String8 const dk::asc_file_format_display_name_table[] {
	""_str8
	"GLB"_str8,
	"GLTF"_str8,
	"EXR"_str8,
	""_str8
};
static_assert(dk::array_count(dk::asc_file_format_display_name_table) == dk::ASC_FILE_FORMAT_COUNT);

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
	ArenaParams const arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(2),
	};
	Arena *arena = arena_alloc(&arena_params);
	LogContext *log = log_alloc();
	log_select(log);
	log_frame_begin();

	//~ Dedrick: Set up shared state.
	if (lane_idx() == 0) {
		asc_shared = arena_push<ASC_Shared>(arena);
	}
	lane_sync();

	//~ Dedrick: Analyze and load command line asset input files.
	{
		ZoneScopedN("analyze and load command line input files");
		if (lane_idx() == 0) {
			String8 const working_dir = get_current_dir(arena);
			String8List input_file_path_tasks = str8_list_copy(arena, &cmd_line->inputs);
			for (String8Node const *node = input_file_path_tasks.first; node != nullptr; node = node->next) {

				//~ Dedrick: Possibly relative -> absolute path.
				String8 input_file_path = node->string;
				{
					PathStyle path_style = path_style_from_str8(input_file_path);
					if (path_style == PathStyle::Relative) {
						String8 const abs_path = str8f(arena, "%.*s/%.*s", DK_STR8_VARG(working_dir), DK_STR8_VARG(node->string));
						input_file_path = path_normalized_from_path(arena, abs_path);
					}
				}

				//~ Dedrick: Thin analysis of file.
				ASC_FileFormat file_format = ASC_FILE_FORMAT_NULL;
				{
					ZoneScopedN("thin analysis of file");
					File const file = file_open(node->string, FILE_ACCESS_FLAG_READ | FILE_ACCESS_FLAG_SHARE_READ);

					//~ Dedrick: GLB magic -> GLB input.
					if (file_format == ASC_FILE_FORMAT_NULL) {
						u32 glb_magic_maybe = 0;
						file_read(file, 0, sizeof(glb_magic_maybe), &glb_magic_maybe);
						if (glb_magic_maybe == GLB_MAGIC_CONSTANT) {
							file_format = ASC_FILE_FORMAT_GLB;
						}
					}

					//~ Dedrick: EXR magic -> EXR input.
					if (file_format == ASC_FILE_FORMAT_NULL) {
						u32 exr_magic_maybe = 0;
						file_read(file, 0, sizeof(exr_magic_maybe), &exr_magic_maybe);
						if (exr_magic_maybe == EXR_MAGIC_CONSTANT) {
							file_format = ASC_FILE_FORMAT_EXR;
						}
					}

					//~ Dedrick: GLTF ext -> GLTF input.
					if (file_format == ASC_FILE_FORMAT_NULL) {
						String8 const ext = path_skip_last_period(node->string);
						if (str8_equals(ext, "gltf"_str8, STRING_MATCH_FLAG_NONE)) {
							file_format = ASC_FILE_FORMAT_GLTF;
						}
					}

					file_close(file);
				}

				//~ Dedrick: Log file recognition.
				if (file_format != ASC_FILE_FORMAT_NULL) {
					DK_LOG_INFOF("%.*s recognized as %.*s\n", DK_STR8_VARG(node->string), DK_STR8_VARG(asc_file_format_display_name_table[file_format]));
				}
				else {
					DK_LOG_INFOF("%.*s was not recognized as a supported format.\n", DK_STR8_VARG(node->string));
				}

				//~ Dedrick: Load recognized file.
				Buffer file_data = {};
				if (file_format != ASC_FILE_FORMAT_NULL) {
					file_data = read_bytes_from_file_path(arena, node->string);
				}

				//~ Dedrick: Save to list.
				ASC_File *const file = arena_push<ASC_File>(arena);
				file->format = file_format;
				file->path = input_file_path;
				file->data = file_data;
				ASC_FileNode *const file_node = arena_push<ASC_FileNode>(arena);
				file_node->file = file;
				forward_list_queue_push(&asc_shared->input_files.first, &asc_shared->input_files.last, file_node);
				asc_shared->input_files.count += 1;
			}
		}
		lane_sync();
	}
	ASC_FileList input_files = asc_shared->input_files;

	{
		ZoneScopedN("bucket input files by format");
		if (lane_idx() == 0) {
			for (ASC_FileNode *node = input_files.first; node != nullptr; node = node->next) {
				ASC_FileNode *file_node = arena_push<ASC_FileNode>(arena);
				file_node->file = node->file;
				ASC_FileList *const input_files_from_format = &asc_shared->input_files_from_format[node->file->format];
				forward_list_queue_push(&input_files_from_format->first, &input_files_from_format->last, file_node);
				input_files_from_format->count += 1;
			}
		}
	}
	ASC_FileList *const input_files_from_format = asc_shared->input_files_from_format;

	//~ Dedrick: Unpack output and to where.
	enum OutputKind {
		OUTPUT_KIND_NULL,
		OUTPUT_KIND_DKS,
		OUTPUT_KIND_CUBE,
		OUTPUT_KIND_COUNT
	};
	struct { String8 flag; String8 title; } const output_kind_info[] = {
		{ ""_str8, ""_str8 },
		{ "dks"_str8, "DK Scene (.dks) Conversion"_str8 },
		{ "cube"_str8, "Equirectangular to Cubemap Conversion"_str8 }
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
			else if (str8_equals(path_skip_last_period(output_path), "cube"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				output_kind = OUTPUT_KIND_CUBE;
				DK_LOG_INFOF("Output path has .cube extension; performing `%.*s`\n", DK_STR8_VARG(output_kind_info[output_kind].title));
			}
		}
	}

	//~ Dedrick: No output path specified, build from input files.
	if (output_path.size == 0) {
		String8 output_path_no_ext = {};
		if (input_files_from_format[ASC_FILE_FORMAT_GLB].first != nullptr) {
			output_path_no_ext = path_chop_last_period(input_files_from_format[ASC_FILE_FORMAT_GLB].first->file->path);
		}
		else if (input_files_from_format[ASC_FILE_FORMAT_GLTF].first != nullptr) {
			output_path_no_ext = path_chop_last_period(input_files_from_format[ASC_FILE_FORMAT_GLTF].first->file->path);
		}
		if (output_path_no_ext.size > 0) {
			output_path = str8f(arena, "%.*s.%.*s", DK_STR8_VARG(output_path_no_ext), DK_STR8_VARG(output_kind_info[output_kind].flag));
		}
	}

	// TODO(Dedrick): Switch on output_kind.
	BufferList output_blobs = {};
	switch (output_kind) {
		default: [[fallthrough]];
		case OUTPUT_KIND_NULL: {
			break;
		}
		case OUTPUT_KIND_DKS: {
			break;
		}
	}

	// TODO(Dedrick): Write output.
	if (lane_idx() == 0) {

	}
	lane_sync();

	//~ Dedrick: Collect logs
	LogFrameResult const log_frame = log_frame_end(arena);
	if (lane_idx() == 0) {
		// TODO(Dedrick): Write to shared memory so the renderer can display in its logs.
		std::fwrite(log_frame.string.data, log_frame.string.size, 1, stdout);
	}
}
