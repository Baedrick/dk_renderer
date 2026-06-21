// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_ASSET_ENGINE_INCLUDED
#	define DK_ASSET_ENGINE_INCLUDED
#endif

namespace dk {
	auto ase_init(CmdLine *cmd_line) noexcept -> void;
	auto ase_shutdown() noexcept -> void;
}
