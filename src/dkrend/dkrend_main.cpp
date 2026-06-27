// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_GRAPHICAL
#define DK_ASSET_ENGINE_INIT_MANUAL
#define DK_DESKTOP_INIT_MANUAL
#define DK_OPENGL_INIT_MANUAL

#include "base/base.hpp"
#include "asset_compiler/asset_compiler.hpp"
#include "asset_engine/asset_engine.hpp"
#include "desktop/desktop.hpp"
#include "opengl/opengl.hpp"
#include "pak/pak.hpp"
#include "ui/ui.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "asset_compiler/asset_compiler.cpp"
#include "asset_engine/asset_engine.cpp"
#include "desktop/desktop.cpp"
#include "opengl/opengl.cpp"
#include "pak/pak.cpp"
#include "ui/ui.cpp"
#include "dkrend/dkrend.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	using namespace dk;

	enum ExecMode {
		EXEC_MODE_NORMAL = 0,
		EXEC_MODE_ASSET_COMPILER
	};
	ExecMode exec_mode = EXEC_MODE_NORMAL;
	if (cmd_line_has_flag(cmd_line, "compiler"_str8)) {
		exec_mode = EXEC_MODE_ASSET_COMPILER;
	}

	//~ Dedrick: Dispatch based on execution mode.
	switch (exec_mode) {
		default:
		case EXEC_MODE_NORMAL: {
			//~ Dedrick: Manual layer initialization.
			ase_init(cmd_line);
			dt_init();
			ogl_init(cmd_line);
			dkr_init(cmd_line);

			//~ Dedrick: Main application loop.
			for (b8 quit = false; !quit; ) {
				quit = dkr_frame();
				FrameMark;
			}

			//~ Dedrick: Manual layer shutdown.
			dkr_shutdown();
			ogl_shutdown();
			dt_shutdown();
			ase_shutdown();
			break;
		}
		case EXEC_MODE_ASSET_COMPILER: {
			asc_entry_point(cmd_line);
			break;
		}
	}
	return 0;
}
