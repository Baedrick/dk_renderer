/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick.
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#   define DK_PLATFORM_WIN32
#else
#	error "Unknown platform"
#endif

#if defined(_MSC_VER)
#	define DK_COMPILER_MSVC
#else
#	error "Unknown compiler"
#endif

#if defined(DK_COMPILER_MSVC)
#	include <intrin.h>
#	define DK_TRAP() __debugbreak()
#else
#	error "Unknown trap intrinsic for this compiler"
#endif

#define DK_ASSERT_ALWAYS(x) do { if (!(x)) { DK_TRAP(); } } while(false) /* NOLINT */
#ifndef NDEBUG
#	define DK_ASSERT(x) DK_ASSERT_ALWAYS(x)
#else
#	define DK_ASSERT(x) (void)(x)
#endif

namespace dk {
	using b8 = bool;
	using s8 = std::int8_t;
	using s16 = std::int16_t;
	using s32 = std::int32_t;
	using s64 = std::int64_t;
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using f32 = float;
	using f64 = double;
	static_assert(sizeof(f32) == 4);
	static_assert(sizeof(f64) == 8);

	constexpr auto kilo_bytes(u64 x) noexcept -> u64 { return x << 10; }
	constexpr auto mega_bytes(u64 x) noexcept -> u64 { return x << 20; }
	constexpr auto giga_bytes(u64 x) noexcept -> u64 { return x << 30; }

	template <typename T, u64 N>
	constexpr auto array_count(T const (&)[N]) noexcept -> u64 { return N; }

	template <typename T>
	auto swap(T *a, T *b) noexcept -> void {
		T const tmp = *a;
		*a = *b;
		*b = tmp;
	}

	template <typename T>
	constexpr auto is_pow2(T x) noexcept -> b8 {
		return x > 0 && (x & (x - 1)) == 0;
	}

	constexpr auto align_forward_pow2(u64 value, u64 align) noexcept -> u64 {
		DK_ASSERT(is_pow2(align));
		return (value + align - 1) & ~(align - 1);
	}
	
	constexpr auto align_down_pow2(u64 value, u64 align) noexcept -> u64 {
		DK_ASSERT(is_pow2(align));
		return value & ~(align - 1);
	}
}
