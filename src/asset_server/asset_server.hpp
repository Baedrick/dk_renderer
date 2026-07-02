// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#ifndef DK_ASSET_SERVER_INCLUDED
#	define DK_ASSET_SERVER_INCLUDED
#endif

namespace dk {
	auto assv_init(CmdLine *cmd_line) noexcept -> void;
	auto assv_shutdown() noexcept -> void;
}
