// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct CmdLineOption {
		CmdLineOption *next;
		CmdLineOption *hash_next;
		String8 name;
		String8List value_strings;
		String8 value_string; ///< Concatenated values
	};

	struct CmdLineOptionSlot {
		CmdLineOption *first;
		CmdLineOption *last;
	};

	struct CmdLine {
		String8 binary_name;
		String8List inputs;
		CmdLineOptionSlot *slots;
		u64 slots_count;
	};

	auto cmd_line_slot_from_string(CmdLine *cmd_line, String8 name) noexcept -> CmdLineOptionSlot *;
	auto cmd_line_option_from_slot(CmdLineOptionSlot *slot, String8 name) noexcept -> CmdLineOption *;
	auto cmd_line_option_from_string(CmdLine *cmd_line, String8 name) noexcept -> CmdLineOption *;

	auto cmd_line_insert_option(Arena *arena, CmdLine *cmd_line, String8 name, String8List values) noexcept -> CmdLineOption *;
	auto cmd_line_from_string_list(Arena *arena, String8List command_line) noexcept -> CmdLine;

	auto cmd_line_values(CmdLine *cmd_line, String8 name) noexcept -> String8List;
	auto cmd_line_value(CmdLine *cmd_line, String8 name) noexcept -> String8;
	auto cmd_line_has_flag(CmdLine *cmd_line, String8 name) noexcept -> b8;
	auto cmd_line_has_argument(CmdLine *cmd_line, String8 name) noexcept -> b8;
}
