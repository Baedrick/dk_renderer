// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	(void)cmd_line;
	return 0;
}
