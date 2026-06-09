// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#	define DK_PLATFORM_WIN32
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
#endif

#if defined(DK_COMPILER_MSVC)
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

#define DK_CAST_FROM_MEMBER(T, m, ptr) reinterpret_cast<T *>(reinterpret_cast<u8 *>(ptr) - offsetof(T, m))

#define DK_STRINGIFY_(x) #x
#define DK_STRINGIFY(x) DK_STRINGIFY_(x)

#define DK_GLUE_(a, b) a##b
#define DK_GLUE(a, b) DK_GLUE_(a, b)

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

	u32 constexpr U32_MAX = 0xFFFFFFFF;
	u64 constexpr U32_MIN = 0;

	u64 constexpr U64_MAX = 0xFFFFFFFFFFFFFFFF;
	u64 constexpr U64_MIN = 0;

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

	template <typename Dst, typename Src>
	auto bit_cast(Src src) noexcept -> Dst {
		static_assert(sizeof(Src) == sizeof(Dst));
		Dst dst = {};
		std::memcpy(&dst, &src, sizeof(Src));
		return dst;
	}

	auto atomic_u64_load(u64 const volatile *ptr) noexcept -> u64;
	auto atomic_u64_store(u64 volatile *ptr, u64 val) noexcept -> void;
	auto atomic_u64_inc_fetch(u64 volatile *ptr) noexcept -> u64;
	auto atomic_u64_dec_fetch(u64 volatile *ptr) noexcept -> u64;
	auto atomic_u64_add_fetch(u64 volatile *ptr, u64 val) noexcept -> u64;
	auto atomic_u64_fetch_exchange(u64 volatile *ptr, u64 val) noexcept -> u64;
	auto atomic_u64_fetch_compare_exchange(u64 volatile *ptr, u64 expected, u64 desired) noexcept -> u64;

	auto atomic_u32_load(u32 const volatile *ptr) noexcept -> u32;
	auto atomic_u32_store(u32 volatile *ptr, u32 val) noexcept -> void;
	auto atomic_u32_inc_fetch(u32 volatile *ptr) noexcept -> u32;
	auto atomic_u32_dec_fetch(u32 volatile *ptr) noexcept -> u32;
	auto atomic_u32_add_fetch(u32 volatile *ptr, u32 val) noexcept -> u32;
	auto atomic_u32_fetch_exchange(u32 volatile *ptr, u32 val) noexcept -> u32;
	auto atomic_u32_fetch_compare_exchange(u32 volatile *ptr, u32 expected, u32 desired) noexcept -> u32;

	auto atomic_ptr_load(void *const volatile *ptr) noexcept -> void *;
	auto atomic_ptr_store(void *volatile *ptr, void *val) noexcept -> void;
	auto atomic_ptr_exchange(void *volatile *ptr, void *val) noexcept -> void *;
	auto atomic_ptr_fetch_compare_exchange(void *volatile *ptr, void *expected, void *desired) noexcept -> void *;

	template <typename T>
	constexpr auto is_pow2(T x) noexcept -> b8 {
		return x > 0 && (x & (x - 1)) == 0;
	}

	template <typename T>
	constexpr auto is_pow2_or_zero(T x) noexcept -> b8 {
		return (x & (x - 1)) == 0;
	}

	constexpr auto align_pow2(u64 value, u64 align) noexcept -> u64 {
		DK_ASSERT(is_pow2(align));
		return (value + align - 1) & ~(align - 1);
	}

	constexpr auto align_down_pow2(u64 value, u64 align) noexcept -> u64 {
		DK_ASSERT(is_pow2(align));
		return value & ~(align - 1);
	}

	constexpr auto align_pad_pow2(u64 value, u64 align) noexcept -> u64 {
		DK_ASSERT(is_pow2(align));
		return (0 - value) & (align - 1);
	}

	template <typename T>
	constexpr auto abs(T x) noexcept -> T {
		return x < 0 ? -x : x;
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

	template <typename T, T *T::*Next = &T::next>
	auto forward_list_stack_push(T **top, T *node) noexcept -> void {
		node->*Next = *top;
		*top = node;
	}

	template <typename T, T *T::*Next = &T::next>
	auto forward_list_stack_pop(T **top) noexcept -> void {
		if (*top != nullptr) {
			*top = (*top)->*Next;
		}
	}

	template <typename T, T *T::*Next = &T::next>
	auto forward_list_queue_push(T **first, T **last, T *node) noexcept -> void {
		node->*Next = nullptr;
		if (*first == nullptr) {
			*first = *last = node;
		}
		else {
			(*last)->*Next = node;
			*last = node;
		}
	}

	template <typename T, T *T::*Next = &T::next>
	auto forward_list_queue_push_front(T **first, T **last, T *node) noexcept -> void {
		node->*Next = *first;
		*first = node;
		if (*last == nullptr) {
			*last = node;
		}
	}

	template <typename T, T *T::*Next = &T::next>
	auto forward_list_queue_pop(T **first, T **last) noexcept -> void {
		if (*first == *last) {
			*first = *last = nullptr;
		}
		else {
			*first = (*first)->*Next;
		}
	}

	template <typename T, T *T::*Next = &T::next, T *T::*Prev = &T::prev>
	auto list_insert_after(T **first, T **last, T*pos, T *node) noexcept -> void {
		if (*first == nullptr) {
			*first = *last = node;
			node->*Next = nullptr;
			node->*Prev = nullptr;
		}
		else if (pos == nullptr) {
			node->*Next = *first;
			(*first)->*Prev = node;
			*first = node;
			node->*Prev = nullptr;
		}
		else if (pos == *last) {
			(*last)->*Next = node;
			node->*Prev = *last;
			*last = node;
			node->*Next = nullptr;
		}
		else {
			if (pos->*Next != nullptr) {
				(pos->*Next)->*Prev = node;
			}
			node->*Next = pos->*Next;
			pos->*Next = node;
			node->*Prev = pos;
		}
	}

	template <typename T, T *T::*Next = &T::next, T *T::*Prev = &T::prev>
	auto list_push_front(T **first, T **last, T *node) noexcept -> void {
		list_insert_after<T, Next, Prev>(first, last, nullptr, node);
	}

	template <typename T, T *T::*Next = &T::next, T *T::*Prev = &T::prev>
	auto list_push_back(T **first, T **last, T *node) noexcept -> void {
		list_insert_after<T, Next, Prev>(first, last, *last, node);
	}

	template <typename T, T *T::*Next = &T::next, T *T::*Prev = &T::prev>
	auto list_remove(T **first, T **last, T *node) noexcept -> void {
		if (node == *first) {
			*first = node->*Next;
		}
		if (node == *last) {
			*last = node->*Prev;
		}
		if (node->prev != nullptr) {
			(node->*Prev)->*Next = node->*Next;
		}
		if (node->next != nullptr) {
			(node->*Next)->*Prev = node->*Prev;
		}
		node->*Next = nullptr;
		node->*Prev = nullptr;
	}

	template <typename T, typename Compare>
	auto insertion_sort(T *arr, u64 count, Compare comp) {
		for (u64 i = 0; i < count; ++i) {
			T const key = arr[i];
			u64 j = i;
			while (j > 0 && comp(key, arr[j - 1])) {
				arr[j] = arr[j - 1];
				j -= 1;
			}
			arr[j] = key;
		}
	}
}
