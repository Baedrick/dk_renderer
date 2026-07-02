// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_ShaderKind : u32 {
		DKR_SHADER_KIND_HELLO_TRIANGLE,
		DKR_SHADER_KIND_DUMMY,
		DKR_SHADER_KIND_COUNT
	};

	enum DKR_TextureKind : u32 {
		DKR_TEXTURE_KIND_TONY_MC_MAPFACE,
		DKR_TEXTURE_KIND_COUNT,
	};

	struct DKR_RenderAssets {
		GLuint shaders[DKR_SHADER_KIND_COUNT];
		GLuint textures[DKR_TEXTURE_KIND_COUNT];
	};

	struct DKR_RenderContext {
		GLsync stage_sync;
		GLuint stage_buffer;
		GPU_Arena *stage_arena;
	};

	auto dkr_pak_path(Arena *arena) noexcept -> String8;
	auto dkr_pak_read_metadata(Arena *arena, File file, PAK_Parsed *out_parsed) noexcept -> b8;

	auto dkr_render_assets_load(File file, PAK_Parsed const *pak, DKR_RenderAssets *out_assets) noexcept -> b8;
}
