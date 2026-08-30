// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define DK_BUILD_GRAPHICAL
#define DK_ASSET_SERVER_INIT_MANUAL
#define DK_DESKTOP_INIT_MANUAL
#define DK_OPENGL_INIT_MANUAL

#include "base/base.hpp"
#include "dks/dks.hpp"
#include "dks_make/dks_make.hpp"
#include "exr/exr.hpp"
#include "gltf/gltf.hpp"
#include "dks_from_gltf/dks_from_gltf.hpp"
#include "asset_compiler/asset_compiler.hpp"
#include "asset_server/asset_server.hpp"
#include "desktop/desktop.hpp"
#include "opengl/opengl.hpp"
#include "gpu_allocator/gpu_allocator.hpp"
#include "pak/pak.hpp"
#include "ui/ui.hpp"
#include "dkrend/dkrend.hpp"

#include "base/base.cpp"
#include "dks/dks.cpp"
#include "dks_make/dks_make.cpp"
#include "exr/exr.cpp"
#include "gltf/gltf.cpp"
#include "dks_from_gltf/dks_from_gltf.cpp"
#include "asset_compiler/asset_compiler.cpp"
#include "asset_server/asset_server.cpp"
#include "desktop/desktop.cpp"
#include "opengl/opengl.cpp"
#include "gpu_allocator/gpu_allocator.cpp"
#include "pak/pak.cpp"
#include "ui/ui.cpp"
#include "dkrend/dkrend.cpp"

namespace {
	enum class ExecMode { Normal, AssetCompiler };
}

auto dk::entry_point(CmdLine *cmd_line) noexcept -> int {
	ExecMode exec_mode = ExecMode::Normal;
	if (cmd_line_has_flag(cmd_line, "compiler"_str8)) {
		exec_mode = ExecMode::AssetCompiler;
	}

	//~ Dedrick: Dispatch based on execution mode.
	switch (exec_mode) {
		default: [[fallthrough]];
		case ExecMode::Normal: {
			//~ Dedrick: Manual layer initialization.
			assv_init(cmd_line);
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
			assv_shutdown();
			break;
		}
		case ExecMode::AssetCompiler: {
			asc_entry_point(cmd_line);
			break;
		}
	}
	return 0;
}
