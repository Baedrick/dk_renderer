// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct AC_ThreadParams {
		CmdLine *cmd_line;
		LaneContext lane_context;
	};

	auto ac_entry_point(CmdLine *cmd_line) noexcept -> void;
	auto ac_thread_entry_point(void *p) noexcept -> void;
}
