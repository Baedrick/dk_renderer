// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::PLT_Handle::operator==(PLT_Handle o) const noexcept -> b8 {
	return v == o.v;
}

auto dk::PLT_Handle::operator!=(PLT_Handle o) const noexcept -> b8 {
	return !(*this == o);
}

#ifdef DK_PLATFORM_WIN32
#	include "platform_win32.cpp"
#endif
