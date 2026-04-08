// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#if defined(DK_COMPILER_MSVC)
auto dk::atomic_u64_load(u64 const volatile *ptr) noexcept -> u64 {
	u64 const result = *ptr;
	_ReadBarrier();
	return result;
}

auto dk::atomic_u64_store(u64 volatile *ptr, u64 val) noexcept -> void {
	*ptr = val;
	_WriteBarrier();
}

auto dk::atomic_u64_inc_fetch(u64 volatile *ptr) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedIncrement64(reinterpret_cast<__int64 volatile *>(ptr))
	);
}

auto dk::atomic_u64_dec_fetch(u64 volatile *ptr) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedDecrement64(reinterpret_cast<__int64 volatile *>(ptr))
	);
}

auto dk::atomic_u64_add_fetch(u64 volatile *ptr, u64 val) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedAdd64(
			reinterpret_cast<__int64 volatile *>(ptr),
			static_cast<__int64>(val)
		)
	);
}

auto dk::atomic_u64_fetch_exchange(u64 volatile *ptr, u64 val) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedExchange64(
			reinterpret_cast<__int64 volatile *>(ptr),
			static_cast<__int64>(val)
		)
	);
}

auto dk::atomic_u64_fetch_compare_exchange(u64 volatile *ptr, u64 expected, u64 desired) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedCompareExchange64(
			reinterpret_cast<__int64 volatile *>(ptr),
			static_cast<__int64>(desired),
			static_cast<__int64>(expected)
		)
	);
}

auto dk::atomic_u32_load(u32 const volatile *ptr) noexcept -> u32 {
	u32 const result = *ptr;
	_ReadBarrier();
	return result;
}

auto dk::atomic_u32_store(u32 volatile *ptr, u32 val) noexcept -> void {
	*ptr = val;
	_WriteBarrier();
}

auto dk::atomic_u32_inc_fetch(u32 volatile *ptr) noexcept -> u32 {
	return static_cast<u32>(
		InterlockedIncrement(reinterpret_cast<LONG volatile *>(ptr))
	);
}

auto dk::atomic_u32_dec_fetch(u32 volatile *ptr) noexcept -> u32 {
	return static_cast<u32>(
		InterlockedDecrement(reinterpret_cast<LONG volatile *>(ptr))
	);
}

auto dk::atomic_u32_add_fetch(u32 volatile *ptr, u32 val) noexcept -> u32 {
	return static_cast<u32>(
		InterlockedAdd(
			reinterpret_cast<LONG volatile *>(ptr),
			static_cast<LONG>(val)
		)
	);
}

auto dk::atomic_u32_fetch_exchange(u32 volatile *ptr, u32 val) noexcept -> u32 {
	return static_cast<u32>(
		InterlockedExchange(
			reinterpret_cast<LONG volatile *>(ptr),
			static_cast<LONG>(val)
		)
	);
}

auto dk::atomic_u32_fetch_compare_exchange(u32 volatile *ptr, u32 expected, u32 desired) noexcept -> u32 {
	return static_cast<u32>(
		InterlockedCompareExchange(
			reinterpret_cast<LONG volatile *>(ptr),
			static_cast<LONG>(desired),
			static_cast<LONG>(expected)
		)
	);
}

auto dk::atomic_ptr_load(void *const volatile *ptr) noexcept -> void * {
	void *const result = *ptr;
	_ReadBarrier();
	return result;
}

auto dk::atomic_ptr_store(void *volatile *ptr, void *val) noexcept -> void {
	*ptr = val;
	_WriteBarrier();
}

auto dk::atomic_ptr_exchange(void *volatile *ptr, void *val) noexcept -> void * {
	return InterlockedExchangePointer(ptr, val);
}

auto dk::atomic_ptr_fetch_compare_exchange(void *volatile *ptr, void *expected, void *desired) noexcept -> void * {
	return InterlockedCompareExchangePointer(ptr, desired, expected);
}
#else
#	error "Unknown atomic operations for this compiler/architecture"
#endif
