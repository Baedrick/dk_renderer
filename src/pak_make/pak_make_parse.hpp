// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum PAKM_TokenKind {
		PAKM_TokenKind_Null = 0,
		PAKM_TokenKind_Section,
		PAKM_TokenKind_Value,
		PAKM_TokenKind_Comment,
		PAKM_TokenKind_COUNT
	};

	struct PAKM_Token {
		PAKM_TokenKind kind;
		u64 begin;
		u64 end;
	};

	struct PAKM_TokenChunkNode {
		PAKM_TokenChunkNode *next;
		PAKM_Token *data;
		u64 count;
		u64 capacity;
	};

	struct PAKM_TokenChunkList {
		PAKM_TokenChunkNode *first;
		PAKM_TokenChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct PAKM_TokenArray {
		PAKM_Token *data;
		u64 count;

		auto operator[](u64 index) noexcept -> PAKM_Token &;
		auto operator[](u64 index) const noexcept -> PAKM_Token const &;
	};

	auto pakm_token_make(u64 begin, u64 end, PAKM_TokenKind kind) noexcept -> PAKM_Token;
	auto pakm_token_chunk_list_push(Arena *arena, PAKM_TokenChunkList *list, u64 capacity, PAKM_Token token) noexcept -> void;
	auto pakm_token_array_from_chunk_list(Arena *arena, PAKM_TokenChunkList const *list) noexcept -> PAKM_TokenArray;

	auto pakm_token_array_from_text(Arena *arena, String8 text) noexcept -> PAKM_TokenArray;
}
