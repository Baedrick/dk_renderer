// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define STB_SPRINTF_IMPLEMENTATION
#include "thirdparty/stb/stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

auto dk::String8::operator[](u64 index) const noexcept -> u8 const & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::String16::operator[](u64 index) const noexcept -> u16 const & {
	DK_ASSERT(index < size);
	return data[index];
}

auto dk::String8Array::operator[](u64 index) noexcept -> String8 & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::String8Array::operator[](u64 index) const noexcept -> String8 const & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::char_is_upper(u8 c) noexcept -> b8 {
	return c >= 'A' && c <= 'Z';
}

auto dk::char_is_lower(u8 c) noexcept -> b8 {
	return c >= 'a' && c <= 'z';
}

auto dk::char_is_whitespace(u8 c) noexcept -> b8 {
	return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\n';
}

auto dk::char_is_digit(u8 c) noexcept -> b8 {
	return c >= '0' && c <= '9';
}

auto dk::char_to_upper(u8 c) noexcept -> u8 {
	return char_is_lower(c) ? 'A' + (c - 'a') : c;
}

auto dk::char_to_lower(u8 c) noexcept -> u8 {
	return char_is_upper(c) ? 'a' + (c - 'A') : c;
}

auto dk::char_to_forward_slash(u8 c) noexcept -> u8 {
	return c == '\\' ? '/' : c;
}

auto dk::char16_is_lower(u16 c) noexcept -> b8 {
	return c >= u'A' && c <= u'Z';
}

auto dk::char16_to_lower(u16 c) noexcept -> u16 {
	return char16_is_lower(c) ? u'a' + (c - u'A') : c;
}

auto dk::char16_to_forward_slash(u16 c) noexcept -> u16 {
	return c == '\\' ? u'/' : c;
}

auto dk::cstring8_length(u8 const *cstr) noexcept -> u64 {
	u64 length = 0;
	if (cstr != nullptr) {
		for (; cstr[length] != '\0'; ++length) {}
	}
	return length;
}

auto dk::cstring16_length(u16 const *cstr) noexcept -> u64 {
	u64 length = 0;
	if (cstr != nullptr) {
		for (; cstr[length] != '\0'; ++length) {}
	}
	return length;
}

auto dk::operator""_str8(char const *cstr, u64 size) noexcept -> String8 {
	return { .data = reinterpret_cast<u8 const *>(cstr), .size = size };
}

auto dk::operator""_str16(char16_t const *cstr, u64 size) noexcept -> String16 {
	return { .data = reinterpret_cast<u16 const *>(cstr), .size = size };
}

auto dk::str8(u8 const *str, u64 size) noexcept -> String8 {
	return { .data = str, .size = size };
}

auto dk::str8_range(u8 const *begin, u8 const *end) noexcept -> String8 {
	return { .data = begin, .size = static_cast<u64>(end - begin) };
}

auto dk::str16(u16 const *str, u64 size) noexcept -> String16 {
	return { .data = str, .size = size };
}

auto dk::str16_range(u16 const *begin, u16 const *end) noexcept -> String16 {
	return { .data = begin, .size = static_cast<u64>(end - begin) };
}

auto dk::str8_cstring(u8 const *cstr) noexcept -> String8 {
	return { .data = cstr, .size = cstring8_length(cstr) };
}

auto dk::str16_cstring(u16 const *cstr) noexcept -> String16 {
	return { .data = cstr, .size = cstring16_length(cstr) };
}

auto dk::str8_substr(String8 str, u64 begin, u64 end) noexcept -> String8 {
	end = min(end, str.size);
	begin = min(begin, end);
	str.data += begin;
	str.size = end - begin;
	return str;
}

auto dk::str8_substr_size(String8 str, u64 begin, u64 size) noexcept -> String8 {
	begin = min(begin, str.size);
	size = min(size, str.size - begin);
	str.data += begin;
	str.size = size;
	return str;
}

auto dk::str8_skip(String8 str, u64 amount) noexcept -> String8 {
	amount = min(amount, str.size);
	str.data += amount;
	str.size -= amount;
	return str;
}

auto dk::str8_trim_whitespace(String8 str) noexcept -> String8 {
	u64 begin = 0;
	for (; begin < str.size && char_is_whitespace(str[begin]); ++begin) { }
	u64 end = str.size;
	for (; end > begin && char_is_whitespace(str[end - 1]); --end) { }
	return str8_substr(str, begin, end);
}

auto dk::str8_equals(String8 s1, String8 s2, StringMatchFlags flags) noexcept -> b8 {
	if (s1.size != s2.size) {
		return false;
	}
	for (u64 i = 0; i < s1.size; ++i) {
		u8 c1 = s1[i];
		u8 c2 = s2[i];
		if ((flags & STRING_MATCH_FLAG_CASE_INSENSITIVE) != 0) {
			c1 = char_to_lower(c1);
			c2 = char_to_lower(c2);
		}
		if ((flags & STRING_MATCH_FLAG_SLASH_INSENSITIVE) != 0) {
			c1 = char_to_forward_slash(c1);
			c2 = char_to_forward_slash(c2);
		}
		if (c1 != c2) {
			return false;
		}
	}
	return true;
}

auto dk::str8_compare(String8 s1, String8 s2, StringMatchFlags flags) noexcept -> s32 {
	u64 const size = min(s1.size, s2.size);
	for (u64 i = 0; i < size; ++i) {
		u8 c1 = s1[i];
		u8 c2 = s2[i];
		if ((flags & STRING_MATCH_FLAG_CASE_INSENSITIVE) != 0) {
			c1 = char_to_lower(c1);
			c2 = char_to_lower(c2);
		}
		if ((flags & STRING_MATCH_FLAG_SLASH_INSENSITIVE) != 0) {
			c1 = char_to_forward_slash(c1);
			c2 = char_to_forward_slash(c2);
		}
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	}
	if (s1.size < s2.size) {
		return -1;
	}
	if (s1.size > s2.size) {
		return 1;
	}
	return 0;
}

auto dk::str8_find_needle(String8 str, u64 start_pos, String8 needle, StringMatchFlags flags) noexcept -> u64 {
	if (needle.size == 0) {
		return str.size;
	}
	u64 const stop_index = max(str.size + 1, needle.size) - needle.size;
	b8 const case_insensitive = (flags & STRING_MATCH_FLAG_CASE_INSENSITIVE) != 0;
	u8 const needle_first = case_insensitive ? char_to_lower(needle[0]) : needle[0];
	for (u64 i = start_pos; i < stop_index; ++i) {
		u8 const haystack_char = case_insensitive ? char_to_lower(str[i]) : str[i];
		if (haystack_char == needle_first) {
			String8 const substr = str8_substr(str, i, i + needle.size);
			if (str8_equals(substr, needle, flags)) {
				return i;
			}
		}
	}
	return str.size;
}

auto dk::str8_find_needle_reverse(String8 str, u64 start_pos, String8 needle, StringMatchFlags flags) noexcept -> u64 {
	if (needle.size == 0) {
		return str.size;
	}
	if (str.size < needle.size + start_pos) {
		return str.size;
	}
	s64 const start_index = static_cast<s64>(str.size - needle.size - start_pos);
	for (s64 i = start_index; i >= 0; i -= 1) {
		String8 const sub = str8_substr(str, static_cast<u64>(i), static_cast<u64>(i) + needle.size);
		if (str8_equals(sub, needle, flags)) {
			return static_cast<u64>(i);
		}
	}
	return str.size;
}

auto dk::str16_equals(String16 s1, String16 s2, StringMatchFlags flags) noexcept -> b8 {
	if (s1.size != s2.size) {
		return false;
	}
	for (u64 i = 0; i < s1.size; ++i) {
		u16 c1 = s1[i];
		u16 c2 = s2[i];
		if ((flags & STRING_MATCH_FLAG_CASE_INSENSITIVE) != 0) {
			c1 = char16_to_lower(c1);
			c2 = char16_to_lower(c2);
		}
		if ((flags & STRING_MATCH_FLAG_SLASH_INSENSITIVE) != 0) {
			c1 = char16_to_forward_slash(c1);
			c2 = char16_to_forward_slash(c2);
		}
		if (c1 != c2) {
			return false;
		}
	}
	return true;
}

auto dk::str8_to_upper(Arena *arena, String8 str) noexcept -> String8 {
	u8 *s = arena_push_array<u8>(arena, str.size + 1);
	for (u64 i = 0; i < str.size; ++i) {
		s[i] = char_to_upper(str[i]);
	}
	s[str.size] = '\0';
	return { .data = s, .size = str.size };
}

auto dk::str8_to_lower(Arena *arena, String8 str) noexcept -> String8 {
	u8 *s = arena_push_array<u8>(arena, str.size + 1);
	for (u64 i = 0; i < str.size; ++i) {
		s[i] = char_to_lower(str[i]);
	}
	s[str.size] = '\0';
	return { .data = s, .size = str.size };
}

auto dk::str8_indent(Arena *arena, String8 str) noexcept -> String8 {
	u8 constexpr indentation_bytes[] = "                                                                                                                                ";
	TempArena const scratch = scratch_begin(&arena, 1);
	String8List indented_strings = {};
	s64 depth = 0;
	s64 next_depth = 0;
	u64 line_begin_offset = 0;
	for (u64 offset = 0; offset <= str.size; ++offset) {
		u8 const byte = offset < str.size ? str[offset] : '\0';
		switch (byte) {
			case '{': {
				next_depth += 1;
				next_depth = max<s64>(0, next_depth);
				break;
			}
			case '}': {
				next_depth -= 1;
				next_depth = max<s64>(0, next_depth);
				depth = next_depth;
				break;
			}
			case '\n': case '\0': {
				String8 const line = str8_trim_whitespace(str8_substr(str, line_begin_offset, offset));
				if (line.size > 0) {
					str8_list_pushf(
						scratch.arena,
						&indented_strings,
						"%.*s%.*s\n", static_cast<int>(depth * 2), indentation_bytes, DK_STR8_VARG(line)
					);
				}
				if (line.size == 0 && indented_strings.node_count > 0 && offset < str.size) {
					str8_list_pushf(scratch.arena, &indented_strings, "\n");
				}
				line_begin_offset = offset + 1;
				depth = next_depth;
				break;
			}
			default: break;
		}
	}
	String8 const result = str8_list_join(arena, &indented_strings, nullptr);
	scratch_end(scratch);
	return result;
}

auto dk::str8_cat(Arena *arena, String8 s1, String8 s2) noexcept -> String8 {
	u64 const size = s1.size + s2.size;
	u8 *s = arena_push_array<u8>(arena, size + 1);
	std::memcpy(s, s1.data, s1.size);
	std::memcpy(s + s1.size, s2.data, s2.size);
	s[size] = '\0';
	return { .data = s, .size = size };
}

auto dk::str8_copy(Arena *arena, String8 str) noexcept -> String8 {
	u8 *s = arena_push_array<u8>(arena, str.size + 1);
	std::memcpy(s, str.data, str.size);
	s[str.size] = '\0';
	return { .data = s, .size = str.size };
}

auto dk::str8fv(Arena *arena, char const *fmt, va_list args) noexcept -> String8 {
	va_list args2;
	va_copy(args2, args);
	s32 const needed_bytes = stbsp_vsnprintf(nullptr, 0, fmt, args) + 1; // NOLINT(clang-diagnostic-format-nonliteral)
	u8 *s = arena_push_array<u8>(arena, needed_bytes);
	stbsp_vsnprintf(reinterpret_cast<char *>(s), needed_bytes, fmt, args2); // NOLINT(clang-diagnostic-format-nonliteral)
	return { .data = s, .size = static_cast<u64>(needed_bytes - 1) };
}

auto dk::str8f(Arena *arena, char const *fmt, ...) noexcept -> String8 {
	va_list args;
	va_start(args, fmt);
	String8 const result = str8fv(arena, fmt, args);
	va_end(args);
	return result;
}

auto dk::sign_from_str8(String8 str, String8 *out_tail) noexcept -> s32 {
	u64 negative_count = 0;
	u64 i = 0;
	for (; i < str.size; ++i) {
		if (str[i] == '-') {
			++negative_count;
		}
		else if (str[i] != '+') {
			break;
		}
	}
	if (out_tail != nullptr) {
		*out_tail = str8_skip(str, i);
	}
	return (negative_count & 1) ? -1 : 1;
}

auto dk::u64_from_str8(String8 str, u32 radix) noexcept -> u64 {
	DK_ASSERT(radix >= 2 && radix <= 16);
	u8 constexpr char_to_value[] = {
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	};
	u64 value = 0;
	for (u64 i = 0; i < str.size; i += 1) {
		u8 const c = str[i];
		u8 const digit = char_to_value[(c - 0x30) & 0x1F];
		value = value * radix + digit;
	}
	return value;
}

auto dk::s64_from_str8(String8 str, u32 radix) noexcept -> s64 {
	String8 tail = str;
	s64 const sign = sign_from_str8(str, &tail);
	return sign * static_cast<s64>(u64_from_str8(tail, radix));
}

auto dk::u32_from_str8(String8 str, u32 radix) noexcept -> u32 {
	return static_cast<u32>(u64_from_str8(str, radix));
}

auto dk::s32_from_str8(String8 str, u32 radix) noexcept -> s32 {
	return static_cast<s32>(s64_from_str8(str, radix));
}

auto dk::str8_list_push_node(String8List *list, String8Node *node) noexcept -> String8Node * {
	forward_list_queue_push(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->string.size;
	return node;
}

auto dk::str8_list_push_node_front(String8List *list, String8Node *node) noexcept -> String8Node * {
	forward_list_queue_push_front(&list->first, &list->last, node);
	list->node_count += 1;
	list->total_size += node->string.size;
	return node;
}

auto dk::str8_list_push(Arena *arena, String8List *list, String8 str) noexcept -> String8Node * {
	String8Node *node = arena_push<String8Node>(arena);
	node->string = str;
	str8_list_push_node(list, node);
	return node;
}

auto dk::str8_list_pushf(Arena *arena, String8List *list, char const *fmt, ...) noexcept -> String8Node * {
	va_list args;
	va_start(args, fmt);
	String8 const str = str8fv(arena, fmt, args);
	String8Node *result = str8_list_push(arena, list, str);
	va_end(args);
	return result;
}

auto dk::str8_list_push_front(Arena *arena, String8List *list, String8 str) noexcept -> String8Node * {
	String8Node *node = arena_push<String8Node>(arena);
	node->string = str;
	str8_list_push_node_front(list, node);
	return node;
}

auto dk::str8_list_push_frontf(Arena *arena, String8List *list, char const *fmt, ...) noexcept -> String8Node * {
	va_list args;
	va_start(args, fmt);
	String8 const str = str8fv(arena, fmt, args);
	String8Node *result = str8_list_push_front(arena, list, str);
	va_end(args);
	return result;
}

auto dk::str8_list_split_by_char(Arena *arena, String8 str, String8 delims, StringSplitFlags flags) noexcept -> String8List {
	String8List list = {};
	b8 const keep_empties = (flags & STRING_SPLIT_FLAG_KEEP_EMPTIES) != 0;
	u64 last_split = 0;
	for (u64 i = 0; i < str.size; ++i) {
		b8 is_delim = false;
		for (u64 d = 0; d < delims.size; ++d) {
			if (str[i] == delims[d]) {
				is_delim = true;
				break;
			}
		}
		if (is_delim) {
			if (keep_empties || i > last_split) {
				str8_list_push(arena, &list, str8_substr(str, last_split, i));
			}
			last_split = i + 1;
		}
	}
	if (keep_empties || last_split < str.size) {
		str8_list_push(arena, &list, str8_substr(str, last_split, str.size));
	}
	return list;
}

auto dk::str8_list_split_by_substr(Arena *arena, String8 str, String8 const *delims, u64 delims_count, StringSplitFlags flags) noexcept -> String8List {
	String8List list = {};
	b8 const keep_empties = (flags & STRING_SPLIT_FLAG_KEEP_EMPTIES) != 0;
	u64 last_split = 0;
	for (u64 i = 0; i < str.size; ) {
		b8 is_delim = false;
		u64 delim_size = 0;
		for (u64 d = 0; d < delims_count; ++d) {
			String8 const delim = delims[d];
			if (delim.size > 0 && str.size - i >= delim.size) {
				String8 const sub = str8_substr(str, i, i + delim.size);
				if (str8_equals(sub, delim, STRING_MATCH_FLAG_NONE)) {
					is_delim = true;
					delim_size = delim.size;
					break;
				}
			}
		}
		if (is_delim) {
			if (keep_empties || i > last_split) {
				str8_list_push(arena, &list, str8_substr(str, last_split, i));
			}
			last_split = i + delim_size;
			i += delim_size;
		} else {
			i += 1;
		}
	}
	if (keep_empties || last_split < str.size) {
		str8_list_push(arena, &list, str8_substr(str, last_split, str.size));
	}
	return list;
}

auto dk::str8_list_join(Arena *arena, String8List const *list, String8JoinParams const *optional_params) noexcept -> String8 {
	String8JoinParams params = {};
	if (optional_params != nullptr) {
		params = *optional_params;
	}

	u64 total_size = params.prefix.size + params.postfix.size + list->total_size;
	if (list->node_count > 1) {
		total_size += params.separator.size * (list->node_count - 1);
	}

	u8 *s = arena_push_array<u8>(arena, total_size + 1);
	u8 *p = s;

	std::memcpy(p, params.prefix.data, params.prefix.size);
	p += params.prefix.size;

	for (String8Node *node = list->first; node != nullptr; node = node->next) {
		std::memcpy(p, node->string.data, node->string.size);
		p += node->string.size;
		if (node != list->last) {
			std::memcpy(p, params.separator.data, params.separator.size);
			p += params.separator.size;
		}
	}

	std::memcpy(p, params.postfix.data, params.postfix.size);
	p += params.postfix.size;
	*p = '\0';

	return { .data = s, .size = total_size };
}

auto dk::str8_array_from_list(Arena *arena, String8List const *list) noexcept -> String8Array {
	String8Array array = {};
	array.count = list->node_count;
	array.data = arena_push_array<String8>(arena, array.count);
	u64 idx = 0;
	for (String8Node *node = list->first; node != nullptr; node = node->next) {
		array[idx++] = node->string;
	}
	return array;
}

auto dk::path_chop_last_slash(String8 path) noexcept -> String8 {
	u64 slash_idx = path.size;
	b8 slash_found = false;
	while (slash_idx --> 0) {
		if (path[slash_idx] == '/' || path[slash_idx] == '\\') {
			slash_found = true;
			break;
		}
	}
	path.size = slash_found ? slash_idx : 0;
	return path;
}

auto dk::path_skip_last_slash(String8 path) noexcept -> String8 {
	u64 slash_idx = path.size;
	b8 slash_found = false;
	while (slash_idx --> 0) {
		if (path[slash_idx] == '/' || path[slash_idx] == '\\') {
			slash_found = true;
			break;
		}
	}
	if (slash_found) {
		path.data += slash_idx + 1;
		path.size -= slash_idx + 1;
	}
	return path;
}

auto dk::path_chop_last_period(String8 path) noexcept -> String8 {
	u64 period_idx = path.size;
	b8 period_found = false;
	while (period_idx --> 0) {
		if (path[period_idx] == '.') {
			period_found = true;
			break;
		}
		if (path[period_idx] == '/' || path[period_idx] == '\\') {
			break;
		}
	}
	path.size = period_found ? period_idx : path.size;
	return path;
}

auto dk::path_skip_last_period(String8 path) noexcept -> String8 {
	u64 period_idx = path.size;
	b8 period_found = false;
	while (period_idx --> 0) {
		if (path[period_idx] == '.') {
			period_found = true;
			break;
		}
		if (path[period_idx] == '/' || path[period_idx] == '\\') {
			break;
		}
	}
	if (period_found) {
		path.data += period_idx + 1;
		path.size -= period_idx + 1;
	} else {
		path.data += path.size;
		path.size = 0;
	}
	return path;
}

// NOTE(Dedrick): Based on the following decoder.
// https://nullprogram.com/blog/2017/10/06/
// https://git.mr4th.com/mr4th-public/mr4th/src/branch/main/src/base/base_big_functions.c#L696
auto dk::utf8_decode(u8 const *str, u64 max) noexcept -> StringDecode {
	u8 constexpr lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	u8 constexpr first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
	u8 constexpr final_shift[] = { 0, 18, 12, 6, 0 };

	// NOTE(Dedrick): Replacement character '?'.
	StringDecode result = { 0xFFFD, 1 };
	if (max > 0) {
		u8 const byte = str[0];
		u8 const length = lengths[byte >> 3];
		if (0 < length && length <= max) {
			u32 codepoint = (byte & first_byte_mask[length]) << 18;
			b8 is_valid = true;
			switch (length) {
				case 4: {
					if ((str[3] & 0xC0) != 0x80) { is_valid = false; break; }
					codepoint |= (str[3] & 0x3F);
					[[fallthrough]];
				}
				case 3: {
					if ((str[2] & 0xC0) != 0x80) { is_valid = false; break; }
					codepoint |= (str[2] & 0x3F) << 6;
					[[fallthrough]];
				}
				case 2: {
					if ((str[1] & 0xC0) != 0x80) { is_valid = false; break; }
					codepoint |= (str[1] & 0x3F) << 12;
					[[fallthrough]];
				}
				default: break;
			}
			if (is_valid) {
				result.codepoint = codepoint >> final_shift[length];
				result.advance = length;
			}
		}
	}
	return result;
}

auto dk::utf16_decode(u16 const *str, u64 max) noexcept -> StringDecode {
	// NOTE(Dedrick): Replacement character '?'.
	StringDecode result = { 0xFFFD, 1 };
	u16 const x = str[0];
	if (0xD800 > x || x > 0xDFFF) {
		result.codepoint = x;
	}
	else if (max >= 2) {
		u16 const y = str[1];
		if (0xD800 <= x && x < 0xDC00 &&
			0xDC00 <= y && y < 0xE000) {
				u16 const hi = x - 0xD800;
				u16 const lo = y - 0xDC00;
				result.codepoint = ((hi << 10) | lo) + 0x10000;
				result.advance = 2;
			}
	}
	return result;
}

auto dk::utf8_encode(u8 *out, u32 codepoint) noexcept -> u32 {
	u32 advance = 0;
	if (codepoint <= 0x7F) {
		out[0] = static_cast<u8>(codepoint & 0xFF);
		advance = 1;
	}
	else if (codepoint <= 0x7FF) {
		out[0] = static_cast<u8>(0xC0 | (codepoint >> 6));
		out[1] = static_cast<u8>(0x80 | (codepoint & 0x3F));
		advance = 2;
	}
	else if (codepoint <= 0xFFFF) {
		out[0] = static_cast<u8>(0xE0 | (codepoint >> 12));
		out[1] = static_cast<u8>(0x80 | ((codepoint >> 6) & 0x3F));
		out[2] = static_cast<u8>(0x80 | (codepoint & 0x3F));
		advance = 3;
	}
	else if (codepoint <= 0x10FFFF) {
		out[0] = static_cast<u8>(0xF0 | (codepoint >> 18));
		out[1] = static_cast<u8>(0x80 | ((codepoint >> 12) & 0x3F));
		out[2] = static_cast<u8>(0x80 | ((codepoint >> 6) & 0x3F));
		out[3] = static_cast<u8>(0x80 | (codepoint & 0x3F));
		advance = 4;
	}
	else {
		// NOTE(Dedrick): Replacement character '?'.
		advance = utf8_encode(out, 0xFFFD);
	}
	return advance;
}

auto dk::utf16_encode(u16 *out, u32 codepoint) noexcept -> u32 {
	u32 advance = 0;
	if (codepoint < 0x10000) {
		out[0] = static_cast<u16>(codepoint);
		advance = 1;
	}
	else {
		u32 const v = codepoint - 0x10000;
		out[0] = static_cast<u16>(0xD800 + (v >> 10));
		out[1] = static_cast<u16>(0xDC00 + (v & 0x3FF));
		advance = 2;
	}
	return advance;
}

auto dk::str8_from_16(Arena *arena, String16 str) noexcept -> String8 {
	String8 result = {};
	if (str.size > 0) {
		u64 const max_size = str.size * 3 + 1;
		u8 *const out_begin = arena_push_array<u8>(arena, max_size);
		u8 *out = out_begin;

		u16 const *in = str.data;
		u16 const *const in_end = in + str.size;

		while (in < in_end) {
			StringDecode const decode = utf16_decode(in, in_end - in);
			in += decode.advance;
			out += utf8_encode(out, decode.codepoint);
		}
		*out = '\0';

		u64 const actual_size = out - out_begin;
		arena_pop(arena, max_size - (actual_size + 1));
		result.data = out_begin;
		result.size = actual_size;
	}
	return result;
}

auto dk::str16_from_8(Arena *arena, String8 str) noexcept -> String16 {
	String16 result = {};
	if (str.size > 0) {
		u64 const max_size = str.size * 2 + 1;
		u16 *const out_begin = arena_push_array<u16>(arena, max_size);
		u16 *out = out_begin;

		u8 const *in = str.data;
		u8 const *const in_end = in + str.size;

		while (in < in_end) {
			StringDecode const decode = utf8_decode(in, in_end - in);
			in += decode.advance;
			out += utf16_encode(out, decode.codepoint);
		}
		*out = '\0';

		u64 const actual_size = out - out_begin;
		arena_pop(arena, (max_size - (actual_size + 1)) * sizeof(u16));
		result.data = out_begin;
		result.size = actual_size;
	}
	return result;
}

auto dk::u64_hash_from_seed_str8(u64 seed, String8 str) noexcept -> u64 {
	u64 const result = u64_hash_from_seed_data(seed, str.data, str.size);
	return result;
}

auto dk::u64_hash_from_str8(String8 str) noexcept -> u64 {
	u64 const result = u64_hash_from_seed_str8(5381, str);
	return result;
}
