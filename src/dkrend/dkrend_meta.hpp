// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_EventKind : u32 {
		DKR_EVENT_KIND_NULL = 0,
		DKR_EVENT_KIND_QUIT,
		DKR_EVENT_KIND_UPDATE_TARGET_FRAME_RATE,
		DKR_EVENT_KIND_RELOAD_PAK,
		DKR_EVENT_KIND_OPEN_CONSOLE,
		DKR_EVENT_KIND_COUNT
	};

	enum DKR_ShaderModule : u32 {
		DKR_SHADER_MODULE_HELLO_TRIANGLE_VERT,
		DKR_SHADER_MODULE_HELLO_TRIANGLE_FRAG,
		DKR_SHADER_MODULE_DUMMY_COMP,
		DKR_SHADER_MODULE_COUNT
	};

	enum DKR_ShaderKind : u32 {
		DKR_SHADER_KIND_HELLO_TRIANGLE,
		DKR_SHADER_KIND_DUMMY,
		DKR_SHADER_KIND_COUNT
	};

	enum DKR_TextureKind : u32 {
		DKR_TEXTURE_KIND_TONY_MC_MAPFACE,
		DKR_TEXTURE_KIND_COUNT,
	};

	extern GLenum const dkr_shader_module_kind_table[];
	extern String8 const dkr_shader_module_name_table[];
	extern String8 const dkr_shader_name_table[];
	extern String8 const dkr_texture_name_table[];
}
