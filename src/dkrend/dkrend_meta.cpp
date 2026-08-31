// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	GLenum const dkr_shader_module_kind_table[] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_COMPUTE_SHADER
	};
	static_assert(array_count(dkr_shader_module_kind_table) == DKR_SHADER_MODULE_COUNT, "Mismatch shader modules count");

	String8 const dkr_shader_module_name_table[] = {
		"hello_triangle.vert"_str8,
		"hello_triangle.frag"_str8,
		"dummy.comp"_str8
	};
	static_assert(array_count(dkr_shader_module_name_table) == DKR_SHADER_MODULE_COUNT, "Mismatch shader modules count");

	String8 const dkr_shader_name_table[] = {
		"hello_triangle"_str8,
		"dummy"_str8
	};
	static_assert(array_count(dkr_shader_name_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader count");

	struct { u32 count; DKR_ShaderModule modules[2]; } const dkr_shader_recipe_table[] = {
		{ 2, { DKR_SHADER_MODULE_HELLO_TRIANGLE_VERT, DKR_SHADER_MODULE_HELLO_TRIANGLE_FRAG } },
		{ 1, { DKR_SHADER_MODULE_DUMMY_COMP } }
	};
	static_assert(array_count(dkr_shader_recipe_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader count");

	String8 const dkr_texture_name_table[] = {
		"tony_mc_mapface.dds"_str8
	};
	static_assert(array_count(dkr_texture_name_table) == DKR_TEXTURE_KIND_COUNT, "Mismatch texture count");
}
