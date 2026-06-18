// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct PAKM_Shader {
		String8 name;
		Buffer binary;
	};

	struct PAKM_ShaderNode {
		PAKM_ShaderNode *next;
		PAKM_Shader shader;
	};

	struct PAKM_ShaderList {
		PAKM_ShaderNode *first;
		PAKM_ShaderNode *last;
		u64 count;
	};

	struct PAKM_ShaderArray {
		PAKM_Shader *data;
		u64 count;

		auto operator[](u64 index) noexcept -> PAKM_Shader &;
		auto operator[](u64 index) const noexcept -> PAKM_Shader const &;
	};

	struct PAKM_Texture {
		PAK_TextureKind kind;
		PAK_TextureFormat format;
		u32 width;
		u32 height;
		u32 depth;
		u32 mip_count;
		String8 name;
		Buffer pixels;
	};

	struct PAKM_TextureNode {
		PAKM_TextureNode *next;
		PAKM_Texture texture;
	};

	struct PAKM_TextureList {
		PAKM_TextureNode *first;
		PAKM_TextureNode *last;
		u64 count;
	};

	struct PAKM_TextureArray {
		PAKM_Texture *data;
		u64 count;

		auto operator[](u64 index) noexcept -> PAKM_Texture &;
		auto operator[](u64 index) const noexcept -> PAKM_Texture const &;
	};

	struct PAKM_BakeSection {
		void *data;
		u64 size;
	};

	struct PAKM_BakeBundle {
		PAKM_BakeSection sections[PAK_SECTION_KIND_COUNT];
	};

	auto pakm_pak_texture_format_kind_from_dx10_dxgi_format(s32 format) noexcept -> PAK_TextureFormat;

	auto pakm_find_string_index(String8 str, String8Array arr) noexcept -> u32;
	auto pakm_strings_sorted_from_unsorted_in_place(String8Array arr) noexcept -> String8Array;
}
