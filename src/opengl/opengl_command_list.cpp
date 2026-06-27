// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::OGL_CmdArray::operator[](u64 index) noexcept -> OGL_Cmd & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::OGL_CmdArray::operator[](u64 index) const noexcept -> OGL_Cmd const & {
	DK_ASSERT(index < count);
	return data[index];
}
