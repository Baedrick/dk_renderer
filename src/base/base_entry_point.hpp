// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.


namespace dk {
	// NOTE(Dedrick): To be defined by application.
	extern auto entry_point(dk::CmdLine *cmd_line) noexcept -> int;

	auto main_thread_entry_point(int argc, char **argv) noexcept -> int;
	auto thread_entry_point(void (*func)(void *params), void *func_params) noexcept -> void;
}
