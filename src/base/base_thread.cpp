// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::set_thread_name(String8 name) noexcept -> void {
#if defined(DK_PROFILE_ENABLE)
	// NOTE(Dedrick): Tracy expects a null-terminated string. We copy to make
	// sure that the string slice is terminated where we want it to be.
	TempArena const scratch = scratch_begin(nullptr, 0);
	String8 const name_copy = str8_copy(scratch.arena, name);
	tracy::SetThreadName(reinterpret_cast<char const *>(name_copy.data));
	scratch_end(scratch);
#endif
	plt_set_thread_name(name);
}

auto dk::set_thread_namef(char const *fmt, ...) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	va_list args;
	va_start(args, fmt);
	String8 const name = str8fv(scratch.arena, fmt, args);
	set_thread_name(name);
	va_end(args);
	scratch_end(scratch);
}
