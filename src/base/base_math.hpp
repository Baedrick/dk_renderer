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

	struct vec4 {
		f32 x;
		f32 y;
		f32 z;
		f32 w;

		auto operator[](u64 index) noexcept -> f32 &;
		auto operator[](u64 index) const noexcept -> f32 const &;
	};

	struct uvec2 {
		u32 x;
		u32 y;

		auto operator[](u64 index) noexcept -> u32 &;
		auto operator[](u64 index) const noexcept -> u32 const &;
	};

	struct quat {
		f32 x;
		f32 y;
		f32 z;
		f32 w;

		auto operator[](u64 index) noexcept -> f32 &;
		auto operator[](u64 index) const noexcept -> f32 const &;
	};

	struct mat4 {
		vec4 c0;
		vec4 c1;
		vec4 c2;
		vec4 c3;

		auto operator[](u64 index) noexcept -> vec4 &;
		auto operator[](u64 index) const noexcept -> vec4 const &;
	};

	//~ Dedrick: Scalar ops.
	inline auto sqrt(f32 v) noexcept -> f32 { return std::sqrtf(v); }
	inline auto pow(f32 b, f32 e) noexcept -> f32 { return std::powf(b, e); }
	inline auto sin(f32 r) noexcept -> f32 { return std::sinf(r); }
	inline auto cos(f32 r) noexcept -> f32 { return std::cosf(r); }
	inline auto asin(f32 r) noexcept -> f32 { return std::asinf(r); }
	inline auto acos(f32 r) noexcept -> f32 { return std::acosf(r); }
	inline auto mix(f32 a, f32 b, f32 t) noexcept -> f32 { return a + (b - a) * t; }

	//~ Dedrick: vec3 ops.
	auto operator-(vec3 v) noexcept -> vec3;
	auto operator+(vec3 a, vec3 b) noexcept -> vec3;
	auto operator-(vec3 a, vec3 b) noexcept -> vec3;
	auto operator*(vec3 a, vec3 b) noexcept -> vec3;
	auto operator/(vec3 a, vec3 b) noexcept -> vec3;
	auto operator*(vec3 v, f32 s) noexcept -> vec3;
	auto operator*(f32 s, vec3 v) noexcept -> vec3;
	auto operator/(vec3 v, f32 s) noexcept -> vec3;
	auto min(vec3 a, vec3 b) noexcept -> vec3;
	auto max(vec3 a, vec3 b) noexcept -> vec3;
	auto dot(vec3 a, vec3 b) noexcept -> f32;
	auto length_sq(vec3 v) noexcept -> f32;
	auto length(vec3 v) noexcept -> f32;
	auto normalize(vec3 v) noexcept -> vec3;
	auto mix(vec3 a, vec3 b, f32 t) noexcept -> vec3;
	auto cross(vec3 a, vec3 b) noexcept -> vec3;
	auto rotate(vec3 v, quat q) noexcept -> vec3;
	auto rotate_axis_angle(vec3 v, vec3 axis, f32 radians) noexcept -> vec3;

	//~ Dedrick: vec4 ops.
	auto operator-(vec4 v) noexcept -> vec4;
	auto operator+(vec4 a, vec4 b) noexcept -> vec4;
	auto operator-(vec4 a, vec4 b) noexcept -> vec4;
	auto operator*(vec4 a, vec4 b) noexcept -> vec4;
	auto operator/(vec4 a, vec4 b) noexcept -> vec4;
	auto operator*(vec4 v, f32 s) noexcept -> vec4;
	auto operator*(f32 s, vec4 v) noexcept -> vec4;
	auto operator/(vec4 v, f32 s) noexcept -> vec4;
	auto min(vec4 a, vec4 b) noexcept -> vec4;
	auto max(vec4 a, vec4 b) noexcept -> vec4;
	auto dot(vec4 a, vec4 b) noexcept -> f32;
	auto length_sq(vec4 v) noexcept -> f32;
	auto length(vec4 v) noexcept -> f32;
	auto normalize(vec4 v) noexcept -> vec4;
	auto mix(vec4 a, vec4 b, f32 t) noexcept -> vec4;

	//~ Dedrick: uvec2 ops.
	auto operator+(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator-(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator/(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 v, u32 s) noexcept -> uvec2;
	auto operator*(u32 s, uvec2 v) noexcept -> uvec2;

	//~ Dedrick: quat ops.
	auto operator-(quat q) noexcept -> quat;
	auto operator+(quat a, quat b) noexcept -> quat;
	auto operator-(quat a, quat b) noexcept -> quat;
	auto operator*(quat a, quat b) noexcept -> quat;
	auto operator*(quat q, f32 s) noexcept -> quat;
	auto operator*(f32 s, quat q) noexcept -> quat;
	auto operator/(quat q, f32 s) noexcept -> quat;
	auto quat_identity() noexcept -> quat;
	auto dot(quat a, quat b) noexcept -> f32;
	auto inverse(quat q) noexcept -> quat;
	auto normalize(quat q) noexcept -> quat;
	auto lerp_(quat a, f32 a_t, quat b, f32 b_t) noexcept -> quat;
	auto nlerp(quat a, quat b, f32 t) noexcept -> quat;
	auto slerp(quat a, quat b, f32 t) noexcept -> quat;
	auto quat_from_axis_angle(vec3 axis, f32 radians) noexcept -> quat;

	//~ Dedrick: mat4 ops.
	auto operator+(mat4 const &a, mat4 const &b) noexcept -> mat4;
	auto operator-(mat4 const &a, mat4 const &b) noexcept -> mat4;
	auto operator*(mat4 const &a, mat4 const &b) noexcept -> mat4;
	auto operator*(mat4 const &m, f32 s) noexcept -> mat4;
	auto operator*(f32 s, mat4 const &m) noexcept -> mat4;
	auto operator*(mat4 const &m, vec4 v) noexcept -> vec4;
	auto mat4_identity() noexcept -> mat4;
	auto mat4_from_quat(quat q) noexcept -> mat4;
	auto make_scale(vec3 scale) noexcept -> mat4;
	auto make_translate(vec3 translation) noexcept -> mat4;
	auto make_transform(vec3 translation, quat rotation, vec3 scale) noexcept -> mat4;
	auto transpose(mat4 const &m) noexcept -> mat4;
}
