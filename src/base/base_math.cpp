// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

//~ Dedrick: @vec3

auto dk::vec3::operator[](u64 index) noexcept -> f32 & {
	DK_ASSERT(index < sizeof(vec3)/sizeof(f32));
	return (&x)[index];
}

auto dk::vec3::operator[](u64 index) const noexcept -> f32 const & {
	DK_ASSERT(index < sizeof(vec3)/sizeof(f32));
	return (&x)[index];
}

auto dk::operator+(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

auto dk::operator-(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

auto dk::operator*(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

auto dk::operator/(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.x / b.x, a.y / b.y, a.z / b.z };
}

auto dk::operator*(vec3 v, f32 s) noexcept -> vec3 {
	return { v.x * s, v.y * s, v.z * s };
}

auto dk::operator*(f32 s, vec3 v) noexcept -> vec3 {
	return { v.x * s, v.y * s, v.z * s };
}

auto dk::min(vec3 a, vec3 b) noexcept -> vec3 {
	return { min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) };
}

auto dk::max(vec3 a, vec3 b) noexcept -> vec3 {
	return { max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) };
}

auto dk::dot(vec3 a, vec3 b) noexcept -> f32 {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

auto dk::length2(vec3 v) noexcept -> f32 {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

auto dk::length(vec3 v) noexcept -> f32 {
	return sqrt(length2(v));
}

auto dk::normalize(vec3 v) noexcept -> vec3 {
	return (1.0f / length(v)) * v;
}

auto dk::mix(vec3 a, vec3 b, f32 t) noexcept -> vec3 {
	return a + (b - a) * clamp(t, 0.0f, 1.0f);
}

auto dk::cross(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

//~ Dedrick: @uvec2

auto dk::uvec2::operator[](u64 index) noexcept -> u32 & {
	DK_ASSERT(index < sizeof(uvec2)/sizeof(u32));
	return (&x)[index];
}

auto dk::uvec2::operator[](u64 index) const noexcept -> u32 const & {
	DK_ASSERT(index < sizeof(uvec2)/sizeof(u32));
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

auto dk::operator*(u32 s, uvec2 v) noexcept -> uvec2 {
	return { v.x * s, v.y * s };
}
