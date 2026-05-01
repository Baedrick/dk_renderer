// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

// NOTE(Dedrick): To be defined by application.
extern auto entry_point(int argc, char **argv) noexcept -> int;

namespace dk {
	auto main_thread_entry_point(int argc, char **argv) noexcept -> int;
	auto thread_entry_point(PLT_ThreadFunction *func, void *func_params) noexcept -> void;
}
