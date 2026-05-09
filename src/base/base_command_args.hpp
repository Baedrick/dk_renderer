// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct CmdArgs {

	};

	auto cmd_args_from_string_list(Arena *arena, String8List arguments) noexcept -> CmdArgs;
}
