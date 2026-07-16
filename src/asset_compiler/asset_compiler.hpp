// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum ASC_FileFormat : u32 {
		ASC_FILE_FORMAT_NULL = 0,
		ASC_FILE_FORMAT_GLB,
		ASC_FILE_FORMAT_GLTF,
		ASC_FILE_FORMAT_GLTF_BIN,
		ASC_FILE_FORMAT_EXR,
		ASC_FILE_FORMAT_COUNT
	};

	extern String8 const asc_file_format_display_name_table[];

	struct ASC_File {
		ASC_FileFormat format;
		String8 path;
		Buffer data;
	};

	struct ASC_FileNode {
		ASC_FileNode *next;
		ASC_File *file;
	};

	struct ASC_FileList {
		ASC_FileNode *first;
		ASC_FileNode *last;
		u64 count;
	};

	struct ASC_ThreadParams {
		CmdLine *cmd_line;
		LaneContext lane_context;
	};

	struct ASC_Shared {
		ASC_FileList input_files;
		ASC_FileList input_files_from_format[ASC_FILE_FORMAT_COUNT];
	};

	extern ASC_Shared *asc_shared;

	auto asc_entry_point(CmdLine *cmd_line) noexcept -> void;
	auto asc_thread_entry_point(void *p) noexcept -> void;
}
