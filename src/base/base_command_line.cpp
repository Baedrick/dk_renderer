// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::cmd_line_slot_from_string(CmdLine *cmd_line, String8 name) noexcept -> CmdLineOptionSlot * {
	CmdLineOptionSlot *slot = nullptr;
	if (cmd_line->slots_count != 0) {
		u64 const hash = u64_hash_from_str8(name);
		u64 const slot_idx = hash % cmd_line->slots_count;
		slot = &cmd_line->slots[slot_idx];
	}
	return slot;
}

auto dk::cmd_line_option_from_slot(CmdLineOptionSlot *slot, String8 name) noexcept -> CmdLineOption * {
	CmdLineOption *result = nullptr;
	for (CmdLineOption *node = slot->first; node != nullptr; node = node->hash_next) {
		if (str8_equals(name, node->name, STRING_MATCH_FLAG_NONE)) {
			result = node;
			break;
		}
	}
	return result;
}

auto dk::cmd_line_option_from_string(CmdLine *cmd_line, String8 name) noexcept -> CmdLineOption * {
	return cmd_line_option_from_slot(cmd_line_slot_from_string(cmd_line, name), name);
}

auto dk::cmd_line_insert_option(Arena *arena, CmdLine *cmd_line, String8 name, String8List values) noexcept -> CmdLineOption * {
	CmdLineOption *option = nullptr;
	CmdLineOptionSlot *slot = cmd_line_slot_from_string(cmd_line, name);
	CmdLineOption *existing_option = cmd_line_option_from_slot(slot, name);
	if (existing_option != nullptr) {
		option = existing_option;
	} else {
		option = arena_push<CmdLineOption>(arena);
		forward_list_stack_push<CmdLineOption, &CmdLineOption::hash_next>(&slot->first, option);
		option->name = str8_copy(arena, name);
		option->value_strings = values;
		String8JoinParams join_params = {};
		join_params.prefix = ""_str8;
		join_params.postfix = ""_str8;
		join_params.separator = ","_str8;
		option->value_string = str8_list_join(arena, &option->value_strings, &join_params);
	}
	return option;
}

// NOTE(Dedrick): Logic shamefully taken from RADDBG.
auto dk::cmd_line_from_string_list(Arena *arena, String8List command_line) noexcept -> CmdLine {
	CmdLine parsed = {};
	parsed.binary_name = command_line.first->string;
	parsed.slots_count = 64;
	parsed.slots = arena_push_array<CmdLineOptionSlot>(arena, parsed.slots_count);

	b8 after_passthrough_option = false;
	b8 first_passthrough = true;
	for (String8Node const *node = command_line.first->next, *next = nullptr; node != nullptr; node = next) {
		next = node->next;

		// NOTE(Dedrick): Look at -- and - at start of argument to determine if
		// it's a flag option. All arguments after a single "--" with no trailing
		// string on the command line will be considered as passthrough input.
		b8 is_option = false;
		String8 option_name = node->string;
		if (!after_passthrough_option) {
			is_option = true;
			if (str8_equals(node->string, "--"_str8, STRING_MATCH_FLAG_NONE)) {
				after_passthrough_option = true;
				is_option = false;
			} else if (str8_equals(str8_substr_size(node->string, 0, 2), "--"_str8, STRING_MATCH_FLAG_NONE)) {
				option_name = str8_skip(option_name, 2);
			} else if (str8_equals(str8_substr_size(node->string, 0, 1), "-"_str8, STRING_MATCH_FLAG_NONE)) {
				option_name = str8_skip(option_name, 1);
			} else {
				is_option = false;
			}
		}

		if (is_option) {
			b8 has_values = false;
			u64 const value_signifier_position = str8_find_needle(option_name, 0, "="_str8, STRING_MATCH_FLAG_NONE);
			String8 const value_portion_this_string = str8_skip(option_name, value_signifier_position + 1);
			if (value_signifier_position < option_name.size) {
				has_values = true;
			}
			option_name = str8_substr_size(option_name, 0, value_signifier_position);

			// NOTE(Dedrick): Parse option values.
			String8List values = {};
			if (has_values) {
				for (String8Node const *n = node; n != nullptr; n = n->next) {
					next = n->next;
					String8 string = n->string;
					if (n == node) {
						string = value_portion_this_string;
					}
					String8List const values_in_this_string = str8_list_split_by_char(arena, string, ","_str8, STRING_SPLIT_FLAG_NONE);
					for (String8Node const *sub_val = values_in_this_string.first; sub_val != nullptr; sub_val = sub_val->next) {
						str8_list_push(arena, &values, sub_val->string);
					}
					if (!str8_equals(str8_skip(n->string, n->string.size - 1), ","_str8, STRING_MATCH_FLAG_NONE) &&
						(n != node || value_portion_this_string.size != 0)) {
						break;
					}
				}
			}
			cmd_line_insert_option(arena, &parsed, option_name, values);
		}
		// NOTE(Dedrick): Treat as passthrough input.
		else if (!str8_equals(node->string, "--"_str8, STRING_MATCH_FLAG_NONE) || !first_passthrough) {
			str8_list_push(arena, &parsed.inputs, node->string);
			first_passthrough = false;
		}
	}
	return parsed;
}

auto dk::cmd_line_values(CmdLine *cmd_line, String8 name) noexcept -> String8List {
	String8List result = {};
	CmdLineOption *option = cmd_line_option_from_string(cmd_line, name);
	if (option != nullptr) {
		result = option->value_strings;
	}
	return result;
}

auto dk::cmd_line_value(CmdLine *cmd_line, String8 name) noexcept -> String8 {
	String8 result = {};
	CmdLineOption *option = cmd_line_option_from_string(cmd_line, name);
	if (option != nullptr) {
		result = option->value_string;
	}
	return result;
}

auto dk::cmd_line_has_flag(CmdLine *cmd_line, String8 name) noexcept -> b8 {
	CmdLineOption *option = cmd_line_option_from_string(cmd_line, name);
	return option != nullptr;
}

auto dk::cmd_line_has_argument(CmdLine *cmd_line, String8 name) noexcept -> b8 {
	CmdLineOption *option = cmd_line_option_from_string(cmd_line, name);
	return option != nullptr && option->value_strings.node_count > 0;
}
