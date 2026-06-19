// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_GRAPHICAL

#include "base/base.hpp"
#include "compiler/compiler.hpp"
#include "desktop/desktop.hpp"
#include "rhi/rhi.hpp"
#include "pak/pak.hpp"
#include "ui/ui.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "compiler/compiler.cpp"
#include "desktop/desktop.cpp"
#include "rhi/rhi.cpp"
#include "pak/pak.cpp"
#include "ui/ui.cpp"
#include "dkrend/dkrend.cpp"

using namespace dk;

enum ExecMode {
	EXEC_MODE_NORMAL = 0,
	EXEC_MODE_COMPILER
};

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	ExecMode exec_mode = EXEC_MODE_NORMAL;
	if (cmd_line_has_flag(cmd_line, "compiler"_str8)) {
		exec_mode = EXEC_MODE_COMPILER;
	}

	switch (exec_mode) {
		default:
		case EXEC_MODE_NORMAL: {
			dk::dkr_init(cmd_line);
			for (dk::b8 quit = false; !quit; ) {
				quit = dk::dkr_frame();
				FrameMark;
			}
			dk::dkr_shutdown();
			break;
		}
		case EXEC_MODE_COMPILER: {
			cpl_entry_point(cmd_line);
			break;
		}
	}
	return 0;
}
