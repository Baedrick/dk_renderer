// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::operator[](u64 index) noexcept -> PAKM_Token & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::operator[](u64 index) const noexcept -> PAKM_Token const & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::pakm_token_make(u64 begin, u64 end, PAKM_TokenKind kind) noexcept -> PAKM_Token {
	PAKM_Token token = {};
	token.kind = kind;
	token.begin = begin;
	token.end = end;
	return token;
}

auto dk::pakm_token_chunk_list_push(Arena *arena, PAKM_TokenChunkList *list, u64 capacity, PAKM_Token token) noexcept -> void {
	PAKM_TokenChunkNode *node = list->last;
	if (node == nullptr || node->count >= node->capacity) {
		node = arena_push<PAKM_TokenChunkNode>(arena);
		node->capacity = capacity;
		node->data = arena_push<PAKM_Token>(arena, node->capacity);
		forward_list_queue_push(&list->first, &list->last, node);
		list->chunk_count += 1;
	}
	node->data[node->count] = token;
	node->count += 1;
	list->total_count += 1;
}

auto dk::pakm_token_array_from_chunk_list(Arena *arena, PAKM_TokenChunkList const *list) noexcept -> PAKM_TokenArray {
	PAKM_TokenArray result = {};
	result.count = list->total_count;
	result.data = arena_push_array<PAKM_Token>(arena, result.count);
	u64 write_idx = 0;
	for (PAKM_TokenChunkNode const *node = list->first; node != nullptr; node = node->next) {
		std::memcpy(result.data + write_idx, node->data, sizeof(PAKM_Token) * node->count);
		write_idx += node->count;
	}
	return result;
}

auto dk::pakm_token_array_from_text(Arena *arena, String8 text) noexcept -> PAKM_TonkenArray {
	TempArena const scratch = scratch_begin(&arena, 1);
	dk_defer(scratch_end(scratch));

	PAKM_TokenChunkList tokens = {};
	u8 const *byte_begin = text.data;
	u8 const *byte_end = byte_begin + text.size;
	u8 const *byte = byte_begin;

	//~ Dedrick: Scan string & produce tokens.
	while (byte < byte_end) {
		PAKM_TokenKind token_kind = PAKM_TokenKind_Null;
		u8 const *token_begin = nullptr;
		u8 const *token_end = nullptr;

		//~ Dedrick: Skip whitespace.
		if (token_kind == PAKM_TokenKind_Null && char_is_whitespace(*byte)) {
			byte += 1;
			for (; byte <= byte_end; byte += 1) {
				if (byte == byte_end || !char_is_whitespace(*byte)) {
					break;
				}
			}
			continue;
		}

		//~ Dedrick: Comments.
		if (token_kind == PAKM_TokenKind_Null && *byte == '#') {
			token_kind = PAKM_TokenKind_Comment;
			token_begin = byte;
			token_end = byte;
			byte += 1;
			for (; byte <= byte_end; byte += 1) {
				token_end += 1;
				if (byte == byte_end || *byte == '\n') {
					break;
				}
			}
		}

		//~ Dedrick: Sections.
		if (token_kind == PAKM_TokenKind_Null && *byte == '[') {
			token_kind = PAKM_TokenKind_Section;
			token_begin = byte + 1;
			token_end = byte + 1;
			byte += 1;
			for (; byte <= byte_end; byte += 1) {
				if (byte == byte_end || *byte == '\n') {
					token_end = byte;
					break;
				}
				if (*byte == ']') {
					token_end = byte;
					byte += 1;
					break;
				}
			}
		}

		//~ Dedrick: Values.
		if (token_kind == PAKM_TokenKind_Null &&
		   (char_is_alpha(*byte) || char_is_digit(*byte, 10) || *byte == '.' || *byte == '/' || *byte == '_')) {
			token_kind  = PAKM_TokenKind_Value;
			token_begin = byte;
			token_end = byte;
			byte += 1;
			for (; byte <= byte_end; byte += 1) {
				token_end += 1;
				if (byte == byte_end || char_is_whitespace(*byte) || *byte == '#') {
					break;
				}
			}
		}

		//~ Dedrick: Fallthrough any bad inputs.
		if (token_kind == PAKM_TokenKind_Null) {
			byte += 1;
		}

		//~ Dedrick: Push token if formed.
		if (token_kind != PAKM_TokenKind_Null && token_begin != nullptr && token_end > token_begin) {
			PAKM_Token const token = pakm_token_make(
				static_cast<u64>(token_begin - byte_begin),
				static_cast<u64>(token_end - byte_begin),
				token_kind
			);
			pakm_token_chunk_list_push(scratch.arena, &tokens, 4096, token);
		}
	}

	//~ Dedrick: Fill result.
	PAKM_TokenArray const array = pakm_token_array_from_chunk_list(arena, &tokens);
	return array;
}
