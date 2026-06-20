// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum ASC_FileFormat : u32 {
		ASC_FILE_FORMAT_NULL = 0,
		ASC_FILE_FORMAT_COUNT
	};

	struct ASC_ThreadParams {
		CmdLine *cmd_line;
		LaneContext lane_context;
	};

	auto asc_entry_point(CmdLine *cmd_line) noexcept -> void;
	auto asc_thread_entry_point(void *p) noexcept -> void;
}
