// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base_core.cpp"
#include "base_hash.cpp"
#include "base_profile.cpp"
#include "base_memory.cpp"
#include "base_arena.cpp"
#include "base_string.cpp"
#include "base_buffer.cpp"
#include "base_ring_buffer.cpp"
#include "base_thread.cpp"
#include "base_thread_context.cpp"
#include "base_file.cpp"
#include "base_system.cpp"
#include "base_shared_memory.cpp"
#include "base_log.cpp"
#include "base_command_line.cpp"
#include "base_entry_point.cpp"

#if defined(DK_PLATFORM_WIN32)
#	include "win32/win32_base.cpp"
#else
#	error "Base layer platform backend not implemented."
#endif
