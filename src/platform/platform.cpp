// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator==(PLT_Handle a, PLT_Handle b) noexcept -> b8 {
	return a.v == b.v;
}

auto dk::operator!=(PLT_Handle a, PLT_Handle b) noexcept -> b8 {
	return !(a == b);
}

#ifdef DK_PLATFORM_WIN32
#	include "platform_win32.cpp"
#endif
