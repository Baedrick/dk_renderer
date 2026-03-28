// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#if defined(DK_PROFILE_ENABLE)
#	define TRACY_ENABLE
#	if defined(DK_COMPILER_MSVC)
#		define TracyFunction __FUNCSIG__
#	endif
#	include "thirdparty/tracy/tracy/Tracy.hpp"
#endif
