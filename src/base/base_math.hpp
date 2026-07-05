// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct uvec2 {
		u32 x;
		u32 y;

		auto operator[](u64 index) noexcept -> u32 &;
		auto operator[](u64 index) const noexcept -> u32 const &;
	};

	//~ Dedrick: uvec2 ops.
	auto operator+(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator-(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator/(uvec2 a, uvec2 b) noexcept -> uvec2;
	auto operator*(uvec2 v, u32 s) noexcept -> uvec2;
	auto operator*(u32 v, uvec2 s) noexcept -> uvec2;
}
