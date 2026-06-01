// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::pakm_find_string_index(String8 str, String8Array arr) noexcept -> u32 {
	u32 result = 0;
	if (str.size > 0) {
		for (u32 string_idx = 0; string_idx < arr.count; ++string_idx) {
			if (str8_equals(str, arr[string_idx], STRING_MATCH_FLAG_NONE)) {
				result = string_idx + 1;
				break;
			}
		}
	}
	return result;
}
