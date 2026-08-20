// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

//~ Dedrick: @vec3

auto dk::vec3::operator[](u64 index) noexcept -> f32 & {
	DK_ASSERT(index < 3);
	return (&x)[index];
}

auto dk::vec3::operator[](u64 index) const noexcept -> f32 const & {
	DK_ASSERT(index < 3);
	return (&x)[index];
}

auto dk::operator-(vec3 v) noexcept -> vec3 {
	return { -v.x, -v.y, -v.z };
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
	return v * s;
}

auto dk::operator/(vec3 v, f32 s) noexcept -> vec3 {
	return { v.x / s, v.y / s, v.z / s };
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

auto dk::length_sq(vec3 v) noexcept -> f32 {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

auto dk::length(vec3 v) noexcept -> f32 {
	return sqrt(length_sq(v));
}

auto dk::normalize(vec3 v) noexcept -> vec3 {
	return (1.0f / length(v)) * v;
}

auto dk::mix(vec3 a, vec3 b, f32 t) noexcept -> vec3 {
	return a + (b - a) * t;
}

auto dk::cross(vec3 a, vec3 b) noexcept -> vec3 {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

// https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
auto dk::rotate(vec3 v, quat q) noexcept -> vec3 {
	vec3 const qv = vec3(q.x, q.y, q.z);
	vec3 const t = 2.0f * cross(qv, v);
	vec3 const result = v + q.w * t + cross(qv, t);
	return result;
}

auto dk::rotate_axis_angle(vec3 v, vec3 axis, f32 radians) noexcept -> vec3 {
	return rotate(v, quat_from_axis_angle(axis, radians));
}

//~ Dedrick: @vec4

auto dk::vec4::operator[](u64 index) noexcept -> f32 & {
	DK_ASSERT(index < 4);
	return (&x)[index];
}

auto dk::vec4::operator[](u64 index) const noexcept -> f32 const & {
	DK_ASSERT(index < 4);
	return (&x)[index];
}

auto dk::operator-(vec4 v) noexcept -> vec4 {
	return { -v.x, -v.y, -v.z, -v.w };
}

auto dk::operator+(vec4 a, vec4 b) noexcept -> vec4 {
	return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

auto dk::operator-(vec4 a, vec4 b) noexcept -> vec4 {
	return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

auto dk::operator*(vec4 a, vec4 b) noexcept -> vec4 {
	return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

auto dk::operator/(vec4 a, vec4 b) noexcept -> vec4 {
	return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

auto dk::operator*(vec4 v, f32 s) noexcept -> vec4 {
	return { v.x * s, v.y * s, v.z * s, v.w * s };
}

auto dk::operator*(f32 s, vec4 v) noexcept -> vec4 {
	return v * s;
}

auto dk::operator/(vec4 v, f32 s) noexcept -> vec4 {
	return { v.x / s, v.y / s, v.z / s, v.w / s };
}

auto dk::min(vec4 a, vec4 b) noexcept -> vec4 {
	return { min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w) };
}

auto dk::max(vec4 a, vec4 b) noexcept -> vec4 {
	return { max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w) };
}

auto dk::dot(vec4 a, vec4 b) noexcept -> f32 {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

auto dk::length_sq(vec4 v) noexcept -> f32 {
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

auto dk::length(vec4 v) noexcept -> f32 {
	return sqrt(length_sq(v));
}

auto dk::normalize(vec4 v) noexcept -> vec4 {
	return (1.0f / length(v)) * v;
}

auto dk::mix(vec4 a, vec4 b, f32 t) noexcept -> vec4 {
	return a + (b - a) * t;
}

//~ Dedrick: @uvec2

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

auto dk::operator*(u32 s, uvec2 v) noexcept -> uvec2 {
	return v * s;
}

//~ Dedrick: @quat

auto dk::quat::operator[](u64 index) noexcept -> f32 & {
	DK_ASSERT(index < 4);
	return (&x)[index];
}

auto dk::quat::operator[](u64 index) const noexcept -> f32 const & {
	DK_ASSERT(index < 4);
	return (&x)[index];
}

auto dk::operator-(quat q) noexcept -> quat {
	return { -q.x, -q.y, -q.z, -q.w };
}

auto dk::operator+(quat a, quat b) noexcept -> quat {
	return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

auto dk::operator-(quat a, quat b) noexcept -> quat {
	return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

auto dk::operator*(quat a, quat b) noexcept -> quat {
	quat result = {};
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return result;
}

auto dk::operator*(quat q, f32 s) noexcept -> quat {
	return { q.x * s, q.y * s, q.z * s, q.w * s };
}

auto dk::operator*(f32 s, quat q) noexcept -> quat {
	return q * s;
}

auto dk::operator/(quat q, f32 s) noexcept -> quat {
	return { q.x / s, q.y / s, q.z / s, q.w / s };
}

auto dk::quat_identity() noexcept -> quat {
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

auto dk::dot(quat a, quat b) noexcept -> f32 {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

auto dk::inverse(quat q) noexcept -> quat {
	quat result = {};
	result.x = -q.x;
	result.y = -q.y;
	result.z = -q.z;
	result.w = q.w;
	result = result / dot(q, q);
	return result;
}

auto dk::normalize(quat q) noexcept -> quat {
	vec4 const v = normalize(vec4(q.x, q.y, q.z, q.w));
	return { v.x, v.y, v.z, v.w };
}

auto dk::lerp_(quat a, f32 a_t, quat b, f32 b_t) noexcept -> quat {
	quat result = {};
	result.x = a.x * a_t + b.x * b_t;
	result.y = a.y * a_t + b.y * b_t;
	result.z = a.z * a_t + b.z * b_t;
	result.w = a.w * a_t + b.w * b_t;
	return result;
}

auto dk::nlerp(quat a, quat b, f32 t) noexcept -> quat {
	if (dot(a, b) < 0.0f) {
		b = -b;
	}
	quat const result = normalize(lerp_(a, 1.0f - t, b, t));
	return result;
}

auto dk::slerp(quat a, quat b, f32 t) noexcept -> quat {
	quat result = {};

	// NOTE(Dedrick): Take shortest path on hyper-sphere.
	f32 cos_theta = dot(a, b);
	if (cos_theta < 0.0f) {
		cos_theta = -cos_theta;
		b = -b;
	}

	// NOTE(Dedrick): Use linear interpolation when quaternions are near-parallel.
	if (cos_theta > 0.9995f) {
		result = nlerp(a, b, t);
	}
	else {
		f32 const theta = acos(cos_theta);
		f32 const inv_sin_theta = 1.0f / sin(theta);
		f32 const a_t = sin((1.0f - t) * theta) * inv_sin_theta;
		f32 const b_t = sin(t * theta) * inv_sin_theta;
		result = lerp_(a, a_t, b, b_t);
	}

	return result;
}

auto dk::quat_from_axis_angle(vec3 axis, f32 radians) noexcept -> quat {
	quat result = {};
	axis = normalize(axis);
	f32 const sin_half_theta = sin(radians / 2.0f);
	f32 const cos_half_theta = cos(radians / 2.0f);
	result.x = axis.x * sin_half_theta;
	result.y = axis.y * sin_half_theta;
	result.z = axis.z * sin_half_theta;
	result.w = cos_half_theta;
	return result;
}

//~ Dedrick: @mat4

auto dk::mat4::operator[](u64 index) noexcept -> vec4 & {
	DK_ASSERT(index < 4);
	return (&c0)[index];
}

auto dk::mat4::operator[](u64 index) const noexcept -> vec4 const & {
	DK_ASSERT(index < 4);
	return (&c0)[index];
}

auto dk::operator+(mat4 const &a, mat4 const &b) noexcept -> mat4 {
	mat4 result = {};
	for (u32 c = 0; c < 4; ++c) {
		for (u32 r = 0; r < 4; ++r) {
			result[c][r] = a[c][r] + b[c][r];
		}
	}
	return result;
}

auto dk::operator-(mat4 const &a, mat4 const &b) noexcept -> mat4 {
	mat4 result = {};
	for (u32 c = 0; c < 4; ++c) {
		for (u32 r = 0; r < 4; ++r) {
			result[c][r] = a[c][r] - b[c][r];
		}
	}
	return result;
}

auto dk::operator*(mat4 const &a, mat4 const &b) noexcept -> mat4 {
	mat4 result = {};
	for (u32 c = 0; c < 4; ++c) {
		for (u32 r = 0; r < 4; ++r) {
			result[c][r] =
				a[0][r] * b[c][0] +
				a[1][r] * b[c][1] +
				a[2][r] * b[c][2] +
				a[3][r] * b[c][3];
		}
	}
	return result;
}

auto dk::operator*(mat4 const &m, f32 s) noexcept -> mat4 {
	mat4 result = {};
	for (u32 c = 0; c < 4; ++c) {
		for (u32 r = 0; r < 4; ++r) {
			result[c][r] = m[c][r] * s;
		}
	}
	return result;
}

auto dk::operator*(f32 s, mat4 const &m) noexcept -> mat4 {
	mat4 result = {};
	for (u32 c = 0; c < 4; ++c) {
		for (u32 r = 0; r < 4; ++r) {
			result[c][r] = m[c][r] * s;
		}
	}
	return result;
}

auto dk::operator*(mat4 const &m, vec4 v) noexcept -> vec4 {
	vec4 result = {};
	result.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0];
	result.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1];
	result.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2];
	result.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3];
	return result;
}

auto dk::mat4_identity() noexcept -> mat4 {
	mat4 result = {};
	result[0][0] = 1.0f;
	result[1][1] = 1.0f;
	result[2][2] = 1.0f;
	result[3][3] = 1.0f;
	return result;
}

auto dk::mat4_from_quat(quat q) noexcept -> mat4 {
	mat4 result = {};

	q = normalize(q);
	f32 const xx = q.x * q.x;
	f32 const yy = q.y * q.y;
	f32 const zz = q.z * q.z;
	f32 const xy = q.x * q.y;
	f32 const xz = q.x * q.z;
	f32 const yz = q.y * q.z;
	f32 const wx = q.w * q.x;
	f32 const wy = q.w * q.y;
	f32 const wz = q.w * q.z;

	result[0][0] = 1.0f - 2.0f * (yy + zz);
	result[0][1] = 2.0f * (xy + wz);
	result[0][2] = 2.0f * (xz - wy);
	result[0][3] = 0.0f;

	result[1][0] = 2.0f * (xy - wz);
	result[1][1] = 1.0f - 2.0f * (xx + zz);
	result[1][2] = 2.0f * (yz + wx);
	result[1][3] = 0.0f;

	result[2][0] = 2.0f * (xz + wy);
	result[2][1] = 2.0f * (yz - wx);
	result[2][2] = 1.0f - 2.0f * (xx + yy);
	result[2][3] = 0.0f;

	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;

	return result;
}

auto dk::make_scale(vec3 scale) noexcept -> mat4 {
	mat4 result = mat4_identity();
	result[0][0] = scale.x;
	result[1][1] = scale.y;
	result[2][2] = scale.z;
	return result;
}

auto dk::make_translate(vec3 translation) noexcept -> mat4 {
	mat4 result = mat4_identity();
	result[3][0] = translation.x;
	result[3][1] = translation.y;
	result[3][2] = translation.z;
	return result;
}

auto dk::make_transform(vec3 translation, quat rotation, vec3 scale) noexcept -> mat4 {
	mat4 const rotate = mat4_from_quat(rotation);
	mat4 result = {};
	result[0][0] = rotate[0][0] * scale.x;
	result[0][1] = rotate[0][1] * scale.x;
	result[0][2] = rotate[0][2] * scale.x;
	result[0][3] = 0.0f;
	result[1][0] = rotate[1][0] * scale.y;
	result[1][1] = rotate[1][1] * scale.y;
	result[1][2] = rotate[1][2] * scale.y;
	result[1][3] = 0.0f;
	result[2][0] = rotate[2][0] * scale.z;
	result[2][1] = rotate[2][1] * scale.z;
	result[2][2] = rotate[2][2] * scale.z;
	result[2][3] = 0.0f;
	result[3][0] = translation.x;
	result[3][1] = translation.y;
	result[3][2] = translation.z;
	result[3][3] = 1.0f;
	return result;
}

auto dk::transpose(mat4 const &m) noexcept -> mat4 {
	mat4 result = {};
	result[0][0] = m[0][0];
	result[0][1] = m[1][0];
	result[0][2] = m[2][0];
	result[0][3] = m[3][0];
	result[1][0] = m[0][1];
	result[1][1] = m[1][1];
	result[1][2] = m[2][1];
	result[1][3] = m[3][1];
	result[2][0] = m[0][2];
	result[2][1] = m[1][2];
	result[2][2] = m[2][2];
	result[2][3] = m[3][2];
	result[3][0] = m[0][3];
	result[3][1] = m[1][3];
	result[3][2] = m[2][3];
	result[3][3] = m[3][3];
	return result;
}
