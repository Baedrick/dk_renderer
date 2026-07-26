// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define CGLTF_IMPLEMENTATION
#include "thirdparty/cgltf/cgltf.h"
#undef CGLTF_IMPLEMENTATION

auto dk::gltf_buffer_uri_list_from_json(Arena *arena, String8 json) noexcept -> String8List {
	ZoneScoped;
	String8List result = {};

	u64 offset = 0;
	String8 const uri_needle = "\"uri\""_str8;
	while (true) {
		offset = str8_find_needle(json, offset, uri_needle, STRING_MATCH_FLAG_NONE);
		if (offset >= json.size) {
			break;
		}
		offset += uri_needle.size;

		//~ Dedrick: Advance to opening quote.
		while (offset < json.size && (char_is_whitespace(json[offset]) || json[offset] == ':')) {
			offset += 1;
		}

		//~ Dedrick: Extract string between quotes.
		if (offset < json.size && json[offset] == '"') {
			offset += 1;
			u64 const begin = offset;

			//~ Dedrick: Find end quote.
			while (offset < json.size && json[offset] != '"') {
				offset += 1;
			}

			//~ Dedrick: Extract string.
			String8 const uri = str8_substr(json, begin, offset);

			if(!str8_starts_with(uri, "data:"_str8, STRING_MATCH_FLAG_NONE)) {
				str8_list_push(arena, &result, uri);
			} else {
				DK_LOG_ERRORF("base64 embedded glTF buffers are not supported.\n");
				break;
			}
		}
	}

	return result;
}
