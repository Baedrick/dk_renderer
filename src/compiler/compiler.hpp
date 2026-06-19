// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	auto cpl_entry_point(CmdLine *cmd_line) noexcept -> void;
	auto cpl_thread_entry_point(void *params) noexcept -> void;
}
