// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator==(Process a, Process b) noexcept -> b8 {
	return a.v == b.v;
}

auto dk::operator!=(Process a, Process b) noexcept -> b8 {
	return !(a == b);
}

auto dk::is_valid(Process process) noexcept -> b8 {
	return process != Process{};
}
