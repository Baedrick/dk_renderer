// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum OGL_CmdKind : u32 {
		OGL_CMD_KIND_NULL = 0,
		OGL_CMD_KIND_COPY_BUFFER_TO_IMAGE,
		OGL_CMD_KIND_COUNT
	};

	struct OGL_CmdCopyBufferToImage {
		OGL_CmdKind kind;
		GLuint src;
		GLuint dst;
	};

	union OGL_Cmd {
		OGL_CmdKind kind;
		OGL_CmdCopyBufferToImage copy_buffer_to_image;
	};

	struct OGL_CmdChunkNode {
		OGL_CmdChunkNode *next;
		OGL_Cmd *data;
		u64 count;
		u64 capacity;
	};

	struct OGL_CmdList {
		OGL_CmdChunkNode *first;
		OGL_CmdChunkNode *last;
		u64 chunk_count;
		u64 total_count;
	};

	struct OGL_CmdArray {
		OGL_Cmd *data;
		u64 count;

		auto operator[](u64 index) noexcept -> OGL_Cmd &;
		auto operator[](u64 index) const noexcept -> OGL_Cmd const &;
	};

	auto ogl_cmd_list_push(Arena *arena, OGL_CmdChunkList *list, u64 capacity, OGL_Cmd const *cmd) noexcept -> void;

	auto ogl_cmd_arrays_submit(u64 count, OGL_CmdArray const *cmd_arrays) noexcept -> GLsync;
	auto ogl_cmd_arrays_wait_idle(GLsync fence) noexcept -> b8;
}
