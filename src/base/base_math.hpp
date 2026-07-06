// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct vec3 {
		f32 x;
		f32 y;
		f32 z;

		auto operator[](u64 index) noexcept -> f32 &;
		auto operator[](u64 index) const noexcept -> f32 const &;
	};

	struct uvec2 {
		u32 x;
		u32 y;

		auto operator[](u64 index) noexcept -> u32 &;
		auto operator[](u64 index) const noexcept -> u32 const &;
	};

	//~ Dedrick: Scalar ops.
	inline auto sqrt(f32 v) noexcept -> f32 { return std::sqrtf(v); }
	inline auto pow(f32 b, f32 e) noexcept -> f32 { return std::powf(b, e); }
	inline auto sin(f32 v) noexcept -> f32 { return std::sinf(v); }
	inline auto cos(f32 v) noexcept -> f32 { return std::cosf(v); }
	inline auto mix(f32 a, f32 b, f32 t) noexcept -> f32 { return a + (b - a) * clamp(t, 0.0f, 1.0f); }

	//~ Dedrick: vec3 ops.
	auto operator+(vec3 a, vec3 b) noexcept -> vec3;
	auto operator-(vec3 a, vec3 b) noexcept -> vec3;
	auto operator*(vec3 a, vec3 b) noexcept -> vec3;
	auto operator/(vec3 a, vec3 b) noexcept -> vec3;
	auto operator*(vec3 v, f32 s) noexcept -> vec3;
	auto operator*(f32 s, vec3 v) noexcept -> vec3;
	auto dot(vec3 a, vec3 b) noexcept -> f32;
	auto length2(vec3 v) noexcept -> f32;
	auto length(vec3 v) noexcept -> f32;
	auto normalize(vec3 v) noexcept -> vec3;
	auto mix(vec3 a, vec3 b, f32 t) noexcept -> vec3;
	auto cross(vec3 a, vec3 b) noexcept -> vec3;

	//~ Dedrick: uvec2 ops.
	auto operator+(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator-(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator/(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 v, u32 s) noexcept -> uvec2;
	auto operator*(u32 s, uvec2 v) noexcept -> uvec2;
}
