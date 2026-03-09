// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
#if !defined(NDEBUG)
#	define DK_ASSERT(x) DK_ASSERT_ALWAYS(x)
#else
#	define DK_ASSERT(x) (void)(x)
#endif

#if defined(DK_COMPILER_MSVC)
#	if defined(__SANITIZE_ADDRESS__)
#		define DK_ASAN_ENABLED
#		define DK_ASAN_EXCLUDE __declspec(no_sanitize_address)
#	else
#		define DK_ASAN_EXCLUDE
#	endif
#endif

#if defined(DK_ASAN_ENABLED)
#	include <sanitizer/asan_interface.h>
#	define DK_ASAN_POISON_MEMORY_REGION(addr, size) __asan_poison_memory_region((addr), (size))
#	define DK_ASAN_UNPOISON_MEMORY_REGION(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#	define DK_ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#	define DK_ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
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

	template <typename T>
	constexpr auto min(T a, T b) noexcept -> T {
		return a < b ? a : b;
	}

	template <typename T>
	constexpr auto max(T a, T b) noexcept -> T {
		return a < b ? b : a;
	}

	template <typename T>
	constexpr auto clamp(T x, T min_x, T max_x) noexcept -> T {
		return min(max_x, max(min_x, x));
	}

	template <typename T>
	auto forward_list_stack_push(T **first, T *node) noexcept -> void {
		node->next = *first;
		*first = node;
	}

	template <typename T>
	auto forward_list_stack_pop(T **first) noexcept -> void {
		if (*first != nullptr) {
			*first = (*first)->next;
		}
	}
}
