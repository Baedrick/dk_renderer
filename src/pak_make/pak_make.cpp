// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::PAKM_ShaderArray::operator[](u64 index) noexcept -> PAKM_Shader & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::PAKM_ShaderArray::operator[](u64 index) const noexcept -> PAKM_Shader const & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::PAKM_TextureArray::operator[](u64 index) noexcept -> PAKM_Texture & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::PAKM_TextureArray::operator[](u64 index) const noexcept -> PAKM_Texture const & {
	DK_ASSERT(index < count);
	return data[index];
}

auto dk::pakm_find_string_index(String8 str, String8Array arr) noexcept -> u32 {
	// NOTE(Dedrick): Invariance: arr.count <= 32.
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

auto dk::pakm_strings_sorted_from_unsorted_in_place(String8Array arr) noexcept -> String8Array {
	insertion_sort(
		arr.data,
		arr.count,
		[](String8 a, String8 b) {
			return str8_compare(a, b, STRING_MATCH_FLAG_NONE) < 0;
		}
	);
	return arr;
}
