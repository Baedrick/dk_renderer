// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

#include "thirdparty/cgltf/cgltf.h"

namespace dk {
	u32 constexpr GLB_MAGIC_CONSTANT = 0x46546C67; // "glTF"

	struct GLTF_Buffer {
		String8 uri;
		Buffer data;
	};

	struct GLTF_BufferArray {
		GLTF_Buffer *data;
		u64 count;
	};

	auto gltf_buffer_uri_list_from_json(Arena *arena, String8 json) noexcept -> String8List;
}
