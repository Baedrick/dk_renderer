// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#if defined(DK_COMPILER_MSVC)
auto dk::atomic_u64_load(u64 *ptr) noexcept -> u64 {
	u64 const result = *ptr;
	_ReadBarrier();
	return result;
}

auto dk::atomic_u64_inc_fetch(u64 *ptr) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedIncrement64(reinterpret_cast<__int64 *>(ptr))
	);
}

auto dk::atomic_u64_dec_fetch(u64 *ptr) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedDecrement64(reinterpret_cast<__int64 *>(ptr))
	);
}

auto dk::atomic_u64_add_fetch(u64 *ptr, u64 val) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedAdd64(
			reinterpret_cast<__int64 *>(ptr),
			static_cast<__int64>(val)
		)
	);
}

auto dk::atomic_u64_fetch_exchange(u64 *ptr, u64 val) noexcept -> u64 {
	return static_cast<u64>(
		InterlockedExchange64(
			reinterpret_cast<__int64 *>(ptr),
			static_cast<__int64>(val)
		)
	);
}

auto dk::atomic_u64_fetch_compare_exchange(u64 *ptr, u64 expected, u64 desired) noexcept -> u64 {
	return static_cast<u64>(
		_InterlockedCompareExchange64(
			reinterpret_cast<__int64 *>(ptr),
			static_cast<__int64>(desired),
			static_cast<__int64>(expected)
		)
	);
}

auto dk::atomic_u32_load(u32 *ptr) noexcept -> u32 {
	u32 const result = *ptr;
	_ReadBarrier();
	return result;
}

auto dk::atomic_u32_inc_fetch(u32 *ptr) noexcept -> u32 {

}

auto dk::atomic_u32_dec_fetch(u32 *ptr) noexcept -> u32 {

}

auto dk::atomic_u32_add_fetch(u32 *ptr, u32 val) noexcept -> u32 {

}

auto dk::atomic_u32_fetch_exchange(u32 *ptr, u32 val) noexcept -> u32 {

}

auto dk::atomic_u32_fetch_compare_exchange(u32 *ptr, u32 expected, u32 desired) noexcept -> u32 {

}

auto dk::atomic_ptr_load(void **ptr) noexcept -> void * {
	
}

auto dk::atomic_ptr_exchange(void **ptr, void *val) noexcept -> void * {

}

auto dk::atomic_ptr_fetch_compare_exchange(void **ptr, void *expected, void *desired) noexcept -> void * {

}
#else
#	error "Unknown atomic operations for this compiler/architecture"
#endif
