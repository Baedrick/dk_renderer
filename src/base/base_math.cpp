// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::uvec2::operator[](u64 index) noexcept -> u32 & {
	DK_ASSERT(index < 2);
	return (&x)[index];
}

auto dk::uvec2::operator[](u64 index) const noexcept -> u32 const & {
	DK_ASSERT(index < 2);
	return (&x)[index];
}

auto dk::operator+(uvec2 a, uvec2 b) noexcept -> uvec2 {
	return { a.x + b.x, a.y + b.y };
}

auto dk::operator-(uvec2 a, uvec2 b) noexcept -> uvec2 {
	return { a.x - b.x, a.y - b.y };
}

auto dk::operator*(uvec2 a, uvec2 b) noexcept -> uvec2 {
	return { a.x * b.x, a.y * b.y };
}

auto dk::operator/(uvec2 a, uvec2 b) noexcept -> uvec2 {
	return { a.x / b.x, a.y / b.y };
}

auto dk::operator*(uvec2 v, u32 s) noexcept -> uvec2 {
	return { v.x * s, v.y * s };
}

auto dk::operator*(u32 v, uvec2 s) noexcept -> uvec2 {
	return { v.x * s, v.y * s };
}
