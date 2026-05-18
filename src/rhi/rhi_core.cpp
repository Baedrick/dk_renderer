// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator==(RHI_Handle a, RHI_Handle b) noexcept -> b8 {
	return a.v == b.v;
}

auto dk::operator!=(RHI_Handle a, RHI_Handle b) noexcept -> b8 {
	return !(a == b);
}

auto dk::rhi_handle_invalid() noexcept -> RHI_Handle {
	return { 0 };
}
