// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/stb/stb_sprintf.h"

namespace dk {
	struct String8 {
		u8 const *data;
		u64 size;

		auto operator[](u64 index) const noexcept -> u8 const &;
	};
	
	struct String16 {
		u16 const *data;
		u64 size;
		
		auto operator[](u64 index) const noexcept -> u16 const &;
	};

	struct String8Node {
		String8Node *next;
		String8 string;
	};

	struct String8List {
		String8Node *first;
		String8Node *last;
		u64 node_count;
		u64 total_size;
	};

	struct String8Array {
		String8 *data;
		u64 count;
		u64 total_size;

		auto operator[](u64 index) noexcept -> String8 &;
		auto operator[](u64 index) const noexcept -> String8 const &;
	};

	struct String8JoinParams {
		String8 prefix;
		String8 postfix;
		String8 separator;
	};

	using StringMatchFlags = u32;
	enum : u32 {
		STRING_MATCH_FLAG_NONE = 0,
		STRING_MATCH_FLAG_CASE_INSENSITIVE = 1u << 0,
		STRING_MATCH_FLAG_SLASH_INSENSITIVE = 1u << 1
	};

	using StringSplitFlags = u32;
	enum : u32 {
		STRING_SPLIT_FLAG_NONE = 0,
		STRING_SPLIT_FLAG_KEEP_EMPTIES
	};

	struct StringDecode {
		u32 codepoint;
		u32 advance;
	};

	auto char_is_upper(u8 c) noexcept -> b8;
	auto char_is_lower(u8 c) noexcept -> b8;
	auto char_is_whitespace(u8 c) noexcept -> b8;
	auto char_is_digit(u8 c) noexcept -> b8;
	auto char_to_upper(u8 c) noexcept -> u8;
	auto char_to_lower(u8 c) noexcept -> u8;
	auto char_to_forward_slash(u8 c) noexcept -> u8;

	auto cstring8_length(u8 const *cstr) noexcept -> u64;
	auto cstring16_length(u16 const *cstr) noexcept -> u64;

	auto operator ""_str8(char const *cstr, u64 size) noexcept -> String8;

	// NOTE(Dedrick): Usage printf("My String: %.*s\n", DK_STR8_VARG(my_string));
	#define DK_STR8_VARG(s) static_cast<int>((s).size), reinterpret_cast<char const *>((s).data)

	auto str8(u8 const *str, u64 size) noexcept -> String8;
	auto str8_range(u8 const *begin, u8 const *end) noexcept -> String8;
	auto str8_cstring(u8 const *cstr) noexcept -> String8;

	auto str16(u16 const *str, u64 size) noexcept -> String16;
	auto str16_range(u16 const *begin, u16 const *end) noexcept -> String16;
	auto str16_cstring(u16 const *cstr) noexcept -> String16;

	auto str8_substr(String8 str, u64 begin, u64 end) noexcept -> String8;
	auto str8_substr_size(String8 str, u64 begin, u64 size) noexcept -> String8;
	auto str8_skip(String8 str, u64 amount) noexcept -> String8;
	auto str8_trim_whitespace(String8 str) noexcept -> String8;

	auto str8_equals(String8 s1, String8 s2, StringMatchFlags flags) noexcept -> b8;
	auto str8_compare(String8 s1, String8 s2, StringMatchFlags flags) noexcept -> s32;
	auto str8_find_needle(String8 str, u64 start_pos, String8 needle, StringMatchFlags flags) noexcept -> u64;
	auto str8_find_needle_reverse(String8 str, u64 start_pos, String8 needle, StringMatchFlags flags) noexcept -> u64;

	auto str8_to_upper(Arena *arena, String8 str) noexcept -> String8;
	auto str8_to_lower(Arena *arena, String8 str) noexcept -> String8;

	auto str8_indent(Arena *arena, String8 str) noexcept -> String8;

	auto str8_cat(Arena *arena, String8 s1, String8 s2) noexcept -> String8;
	auto str8_copy(Arena *arena, String8 str) noexcept -> String8;
	auto str8fv(Arena *arena, char const *fmt, va_list args) noexcept -> String8;
	auto str8f(Arena *arena, char const *fmt, ...) noexcept -> String8;

	auto sign_from_str8(String8 str, String8 *out_tail) noexcept -> s32;
	auto u64_from_str8(String8 str, u32 radix) noexcept -> u64;
	auto s64_from_str8(String8 str, u32 radix) noexcept -> s64;
	auto u32_from_str8(String8 str, u32 radix) noexcept -> u32;
	auto s32_from_str8(String8 str, u32 radix) noexcept -> s32;
	// TODO(Dedrick): f64 from string.

	auto str8_list_push_node(String8List *list, String8Node *node) noexcept -> void;
	auto str8_list_push_node_front(String8List *list, String8Node *node) noexcept -> void;
	auto str8_list_push(Arena *arena, String8List *list, String8 str) noexcept -> void;
	auto str8_list_pushf(Arena *arena, String8List *list, char const *fmt, ...) noexcept -> void;
	auto str8_list_push_front(Arena *arena, String8List *list, String8 str) noexcept -> void;

	auto str8_list_split_by_char(Arena *arena, String8 str, String8 delims, StringSplitFlags flags) noexcept -> String8List;
	auto str8_list_split_by_substr(Arena *arena, String8 str, String8 const *delims, u64 delims_count, StringSplitFlags flags) noexcept -> String8List;
	auto str8_list_join(Arena *arena, String8List list, String8JoinParams const *optional_params) noexcept -> String8;

	auto str8_array_from_list(Arena *arena, String8List list) noexcept -> String8Array;
	
	auto path_chop_last_slash(String8 path) noexcept -> String8;
	auto path_skip_last_slash(String8 path) noexcept -> String8;

	auto utf8_decode(u8 const *str, u64 max) noexcept -> StringDecode;
	auto utf16_decode(u16 const *str, u64 max) noexcept -> StringDecode;
	auto utf8_encode(u8 *out, u32 codepoint) noexcept -> u32;
	auto utf16_encode(u16 *out, u32 codepoint) noexcept -> u32;

	auto str8_from_16(Arena *arena, String16 str) noexcept -> String8;
	auto str16_from_8(Arena *arena, String8 str) noexcept -> String16;

	auto u64_hash_from_seed_str8(u64 seed, String8 str) noexcept -> u64;
	auto u64_hash_from_str8(String8 str) noexcept -> u64;
}
