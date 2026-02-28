/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick.
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 */

auto dk::PLT_Handle::operator==(PLT_Handle o) const noexcept -> b8 {
	return v == o.v;
}

auto dk::PLT_Handle::operator!=(PLT_Handle o) const noexcept -> b8 {
	return !(*this == o);
}

auto dk::plt_handle_invalid() noexcept -> PLT_Handle {
	return { 0 };
}

#ifdef DK_PLATFORM_WIN32
#	include "platform_win32.cpp"
#endif
