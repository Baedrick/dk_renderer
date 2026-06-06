// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct PAKM_ShaderNode {
		PAKM_ShaderNode *next;
		PAK_ShaderKind kind;
		String8 name;
		Buffer8 binary;
	};

	struct PAKM_ShaderList {
		PAKM_ShaderNode *first;
		PAKM_ShaderNode *last;
		u64 count;
	};

	struct PAKM_TextureNode {

	};

	struct PAKM_TextureList {

	};

	struct PAKM_BakeSection {
		void *data;
		u64 size;
	};

	struct PAKM_BakeBundle {
		PAKM_BakeSection sections[PAK_SECTION_KIND_COUNT];
	};

	auto pakm_strings_sorted_from_unsorted_in_place(String8Array arr) noexcept -> String8Array;
	auto pakm_find_string_index(String8 str, String8Array arr) noexcept -> u32;
}
