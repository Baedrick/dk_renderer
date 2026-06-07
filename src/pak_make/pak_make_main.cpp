// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#include "base/base.hpp"
#include "platform/platform.hpp"
#include "pak/pak.hpp"
#include "pak_make/pak_make.hpp"

#include "base/base.cpp"
#include "platform/platform.cpp"
#include "pak/pak.cpp"
#include "pak_make/pak_make.cpp"

auto entry_point(dk::CmdLine *cmd_line) noexcept -> int {
	using namespace dk;
	(void)cmd_line;

	ArenaParams constexpr pm_arena_params = {
		.reserve_size = giga_bytes(1),
		.commit_size = mega_bytes(64),
		.flags = ARENA_DEFAULT_FLAGS
	};
	Arena *const pm_arena = arena_alloc(&pm_arena_params);

	//~ Dedrick: Extract paths.
	String8 const binary_dir = plt_get_process_info()->binary_dir;
	String8 const project_dir = path_chop_last_slash(binary_dir);
	String8 const res_dir = str8f(pm_arena, "%.*s/res", DK_STR8_VARG(project_dir));
	String8 const spirv_dir = str8f(pm_arena, "%.*s/src/shaders/.spirv", DK_STR8_VARG(project_dir));
	String8 const out_path = str8f(pm_arena, "%.*s/%.*s", DK_STR8_VARG(binary_dir), DK_STR8_VARG("dkrend.pak"_str8));

	//~ Dedrick: Search shader directory, gather paths, load binaries.
	PAKM_ShaderArray shaders = {};
	{
		struct ShaderTaskNode {
			ShaderTaskNode *next;
			String8 shader_name;
			String8 binary_path;
		};
		ShaderTaskNode *first_shader_task = nullptr;
		ShaderTaskNode *last_shader_task = nullptr;
		u64 shader_task_count = 0;

		//~ Dedrick: Search shader build directory for files to consider.
		std::printf("searching shaders in %.*s...", DK_STR8_VARG(spirv_dir));
		PLT_Handle const iter = plt_dir_iter_begin(spirv_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pm_arena, iter, &file); ) {
			if (!str8_equals(path_skip_last_period(file.name), "spv"_str8, STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
				continue;
			}
			String8 const shader_name = path_chop_last_period(file.name);
			String8 const shader_ext = path_skip_last_period(shader_name);
			String8 const filters[] = {
				{ "vert"_str8 },
				{ "frag"_str8 },
				{ "comp"_str8 }
			};
			for (u64 i = 0; i < array_count(filters); ++i) {
				if (str8_equals(shader_ext, filters[i], STRING_MATCH_FLAG_CASE_INSENSITIVE)) {
					ShaderTaskNode *task = arena_push<ShaderTaskNode>(pm_arena);
					task->binary_path = str8f(pm_arena, "%.*s/%.*s", DK_STR8_VARG(spirv_dir), DK_STR8_VARG(file.name));
					task->shader_name = shader_name;
					forward_list_queue_push(&first_shader_task, &last_shader_task, task);
					shader_task_count += 1;
					break;
				}
			}
		}
		plt_dir_iter_end(iter);
		std::printf(" found %llu\n", shader_task_count);

		//~ Dedrick: Load shaders.
		std::printf("processing shaders...");
		PAKM_ShaderList shaders_list = {};
		for (ShaderTaskNode const *task = first_shader_task; task != nullptr; task = task->next) {
			Buffer8 const binary = plt_read_bytes_from_file_path(pm_arena, task->binary_path);
			PAKM_ShaderNode *node = arena_push<PAKM_ShaderNode>(pm_arena);
			node->shader.name = task->shader_name;
			node->shader.binary = binary;
			forward_list_queue_push(&shaders_list.first, &shaders_list.last, node);
			shaders_list.count += 1;
		}
		std::printf(" %llu shader files processed\n", shaders_list.count);

		//~ Dedrick: Join all shaders.
		shaders.data = arena_push_array<PAKM_Shader>(pm_arena, shaders_list.count);
		shaders.count = shaders_list.count;
		u64 shader_idx = 0;
		for (PAKM_ShaderNode const *node = shaders_list.first; node != nullptr; node = node->next) {
			shaders[shader_idx++] = node->shader;
		}
	}

	//~ Dedrick: Gather files in resource directory to consider.
	String8List dds_file_paths = {};
	{
		std::printf("searching %.*s...", DK_STR8_VARG(res_dir));
		PLT_Handle const iter = plt_dir_iter_begin(res_dir, PLT_DIR_ITER_FLAG_SKIP_FOLDERS);
		for (PLT_DirIterResult file = {}; plt_dir_iter_next(pm_arena, iter, &file); ) {
			String8 const file_path = str8f(pm_arena, "%.*s/%.*s", DK_STR8_VARG(res_dir), DK_STR8_VARG(file.name));
			String8 const file_ext = path_skip_last_period(file_path);
			struct { String8 ext; String8List *file_paths_list; } const file_path_table[] = {
				{ "dds"_str8, &dds_file_paths }
			};
			for (u64 i = 0; i < array_count(file_path_table); ++i) {
				if (str8_equals(file_ext, file_path_table[i].ext, STRING_MATCH_FLAG_NONE)) {
					str8_list_push(pm_arena, file_path_table[i].file_paths_list, file_path);
				}
			}
		}
		plt_dir_iter_end(iter);
		std::printf(" %llu textures found\n", dds_file_paths.node_count);
	}

	//~ Dedrick: Parse all texture files in resource directory.
	PAKM_TextureArray textures = {};
	{
		PAKM_TextureList textures_list = {};
		struct DDS_PixelFormat {
			u32 size;
			u32 flags;
			u32 four_cc;
			u32 rgb_bit_count;
			u32 r_bit_mask;
			u32 g_bit_mask;
			u32 b_bit_mask;
			u32 a_bit_mask;
		};
		struct DDS_Header {
			u32 size;
			u32 flags;
			u32 height;
			u32 width;
			u32 linear_size;
			u32 depth;
			u32 mip_map_count;
			u32 reserved1[11];
			DDS_PixelFormat pixel_format;
			u32 caps;
			u32 caps2;
			u32 caps3;
			u32 caps4;
			u32 reserved2;
		};
		struct DDS_HeaderDx10 {
			u32 dxgi_format;
			u32 resource_dimension;
			u32 misc_flag;
			u32 array_size;
			u32 misc_flags2;
		};
		u32 constexpr DDS_MAGIC = 0x20534444; // "DDS "
		u32 constexpr DDS_DX10_EXT_MAGIC = 0x30315844; // "DX10"
		u32 constexpr DDS_DDPF_FOURCC = 0x4;

		//~ Dedrick: Parse textures.
		std::printf("parsing textures...");
		for (String8Node const *path = dds_file_paths.first; path != nullptr; path = path->next) {
			String8 const file_path = path->string;
			Buffer8 const file_data = plt_read_bytes_from_file_path(pm_arena, file_path);
			u8 *ptr = file_data.data;
			u8 *const opl = file_data.data + file_data.size;

			if (ptr + sizeof(u32) + sizeof(DDS_Header) <= opl) {
				u32 *const magic = reinterpret_cast<u32 *>(file_data.data);
				ptr += sizeof(u32);
				DDS_Header *const header = reinterpret_cast<DDS_Header *>(ptr);
				ptr += sizeof(DDS_Header);

				if (*magic == DDS_MAGIC && header->size == 124) {
					PAK_TextureKind tex_kind = PAK_TEXTURE_KIND_2D;
					if (header->depth > 1) {
						tex_kind = PAK_TEXTURE_KIND_3D;
					}
					PAK_TextureFormat tex_format = PAK_TEXTURE_FORMAT_NULL;
					b8 const has_dx10 = (header->pixel_format.flags & DDS_DDPF_FOURCC) != 0 && header->pixel_format.four_cc == DDS_DX10_EXT_MAGIC;
					if (has_dx10 && ptr + sizeof(DDS_HeaderDx10) <= opl) {
						DDS_HeaderDx10 *dx10_header = reinterpret_cast<DDS_HeaderDx10 *>(ptr);
						ptr += sizeof(DDS_HeaderDx10);
						// NOTE(Dedrick): Values taken from here:
						// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
						switch (dx10_header->dxgi_format) {
							case 67: tex_format = PAK_TEXTURE_FORMAT_RGB9E5; break;
							default: break;
						}
					}
					if (tex_format != PAK_TEXTURE_FORMAT_NULL) {
						u64 const pixels_size = static_cast<u64>(opl - ptr);
						String8 const tex_name = path_skip_last_slash(file_path);
						PAKM_TextureNode *const node = arena_push<PAKM_TextureNode>(pm_arena);
						node->texture.kind = tex_kind;
						node->texture.format = tex_format;
						node->texture.width = header->width;
						node->texture.height = header->height;
						node->texture.depth = header->depth > 0 ? header->depth : 1;
						node->texture.mip_count = header->mip_map_count > 0 ? header->mip_map_count : 1;
						node->texture.name = tex_name;
						node->texture.pixels = buf8(ptr, pixels_size);
						forward_list_queue_push(&textures_list.first, &textures_list.last, node);
						textures_list.count += 1;
					}
				}
			}
		}
		std::printf(" %llu textures parsed\n", textures_list.count);

		//~ Dedrick: Join all textures.
		textures.data = arena_push_array<PAKM_Texture>(pm_arena, textures_list.count);
		textures.count = textures_list.count;
		u64 texture_idx = 0;
		for (PAKM_TextureNode const *node = textures_list.first; node != nullptr; node = node->next) {
			textures[texture_idx++] = node->texture;
		}
	}

	//~ Dedrick: Build strings.
	String8Array strings = {};
	{
		String8List strings_list = {};

		//~ Dedrick: Push strings from shaders.
		for (u64 shader_idx = 0; shader_idx < shaders.count; ++shader_idx) {
			str8_list_push(pm_arena, &strings_list, shaders[shader_idx].name);
		}

		//~ Dedrick: Push strings from textures.
		for (u64 texture_idx = 0; texture_idx < textures.count; ++texture_idx) {
			str8_list_push(pm_arena, &strings_list, textures[texture_idx].name);
		}

		//~ Dedrick: Join all strings.
		strings.data = arena_push_array<String8>(pm_arena, strings_list.node_count);
		strings.count = strings_list.node_count;
		u64 string_idx = 0;
		for (String8Node const *node = strings_list.first; node != nullptr; node = node->next) {
			strings[string_idx++] = node->string;
		}

		//~ Dedrick: Sort.
		strings = pakm_strings_sorted_from_unsorted_in_place(strings);
	}

	//~ Dedrick: Bake strings.
	struct BakedStrings {
		PAK_SectionElementType_StringTable *string_tables;
		u64 string_tables_count;
		PAK_SectionElementType_StringData *string_data;
		u64 string_data_size;
	};
	BakedStrings baked_strings = {};
	{
		//~ Dedrick: Set up.
		baked_strings.string_tables_count = strings.count + 1;
		baked_strings.string_tables = arena_push_array<PAK_SectionElementType_StringTable>(pm_arena, baked_strings.string_tables_count);
		u64 offset_cursor = 0;
		for (u64 str_idx = 0; str_idx < strings.count; ++str_idx) {
			String8 const str = strings[str_idx];
			baked_strings.string_tables[str_idx + 1] = { offset_cursor, str.size };
			offset_cursor += str.size;
		}

		//~ Dedrick: Fill.
		baked_strings.string_data_size = offset_cursor;
		baked_strings.string_data = arena_push_array<PAK_SectionElementType_StringData>(pm_arena, baked_strings.string_data_size);
		for (u64 str_idx = 0; str_idx < strings.count; ++str_idx) {
			String8 const str = strings[str_idx];
			u64 const dst_offset = baked_strings.string_tables[str_idx + 1].offset;
			std::memcpy(baked_strings.string_data + dst_offset, str.data, str.size);
		}
	}

	//~ Dedrick: Build GPU data.
	Buffer8Array gpu_data = {};
	{
		Buffer8List gpu_data_list = {};

		//~ Dedrick: Push data from shaders.
		for (u64 shader_idx = 0; shader_idx < shaders.count; ++shader_idx) {
			buf8_list_push(pm_arena, &gpu_data_list, shaders[shader_idx].binary);
		}

		//~ Dedrick: Push data from textures.
		for (u64 texture_idx = 0; texture_idx < textures.count; ++texture_idx) {
			buf8_list_push(pm_arena, &gpu_data_list, textures[texture_idx].pixels);
		}

		//~ Dedrick: Join all buffers.
		gpu_data.data = arena_push_array<Buffer8>(pm_arena, gpu_data_list.node_count);
		gpu_data.count = gpu_data_list.node_count;
		u64 buffer_idx = 0;
		for (Buffer8Node const *node = gpu_data_list.first; node != nullptr; node = node->next) {
			gpu_data[buffer_idx++] = node->buffer;
		}
	}

	//~ Dedrick: Bake GPU data.
	struct GpuDataEntry {
		u64 offset;
		u64 size;
	};
	struct BakedGpuData {
		GpuDataEntry *gpu_entries;
		u64 gpu_entries_count;
		PAK_SectionElementType_GpuData *gpu_data;
		u64 gpu_data_size;
	};
	BakedGpuData baked_gpu_data = {};
	{
		//~ Dedrick: Set up.
		baked_gpu_data.gpu_entries_count = gpu_data.count + 1;
		baked_gpu_data.gpu_entries = arena_push_array<GpuDataEntry>(pm_arena, baked_gpu_data.gpu_entries_count);
		u64 offset_cursor = 0;
		for (u64 buf_idx = 0; buf_idx < gpu_data.count; ++buf_idx) {
			Buffer8 const buf = gpu_data[buf_idx];
			offset_cursor = align_pow2(offset_cursor, 16);
			baked_gpu_data.gpu_entries[buf_idx + 1] = { offset_cursor, buf.size };
			offset_cursor += buf.size;
		}

		//~ Dedrick: Fill.
		baked_gpu_data.gpu_data_size = offset_cursor;
		baked_gpu_data.gpu_data = arena_push_array<PAK_SectionElementType_GpuData>(pm_arena, baked_gpu_data.gpu_data_size);
		for (u64 buf_idx = 0; buf_idx < gpu_data.count; ++buf_idx) {
			Buffer8 const buf = gpu_data[buf_idx];
			u64 const dst_offset = baked_gpu_data.gpu_entries[buf_idx + 1].offset;
			std::memcpy(reinterpret_cast<u8 *>(baked_gpu_data.gpu_data) + dst_offset, buf.data, buf.size);
		}
	}

	//~ Dedrick: Bake CPU data.
	struct BakedCpuData {
		PAK_SectionElementType_Shader *shaders;
		u64 shaders_size;
		PAK_SectionElementType_Texture *textures;
		u64 textures_size;
		PAK_SectionElementType_GpuHeader *gpu_header;
		u64 gpu_header_size;
	};
	BakedCpuData baked_cpu_data = {};
	{
		//~ Dedrick: Set up.
		baked_cpu_data.shaders_size = shaders.count * sizeof(PAK_SectionElementType_Shader);
		baked_cpu_data.shaders = arena_push_array<PAK_SectionElementType_Shader>(pm_arena, shaders.count);
		baked_cpu_data.textures_size = textures.count * sizeof(PAK_SectionElementType_Texture);
		baked_cpu_data.textures = arena_push_array<PAK_SectionElementType_Texture>(pm_arena, textures.count);
		baked_cpu_data.gpu_header_size = sizeof(PAK_SectionElementType_GpuHeader);
		baked_cpu_data.gpu_header = arena_push<PAK_SectionElementType_GpuHeader>(pm_arena);

		//~ Dedrick: Fill.
		u64 total_count = 0;
		for (u64 shader_idx = 0; shader_idx < shaders.count; ++shader_idx) {
			PAKM_Shader const *src = &shaders[shader_idx];
			PAK_SectionElementType_Shader *dst = baked_cpu_data.shaders + shader_idx;
			dst->name_hash = u64_hash_from_str8(src->name);
			dst->name_string_idx = pakm_find_string_index(src->name, strings);
			dst->gpu_offset = baked_gpu_data.gpu_entries[shader_idx + 1].offset;
			dst->gpu_size = src->binary.size;
		}
		total_count += shaders.count;
		for (u64 texture_idx = 0; texture_idx < textures.count; ++texture_idx) {
			PAKM_Texture const *src = &textures[texture_idx];
			PAK_SectionElementType_Texture *dst = baked_cpu_data.textures + texture_idx;
			dst->name_hash = u64_hash_from_str8(src->name);
			dst->name_string_idx = pakm_find_string_index(src->name, strings);
			dst->kind = src->kind;
			dst->format = src->format;
			dst->width = src->width;
			dst->height = src->height;
			dst->depth = src->depth;
			dst->mip_count = src->mip_count;
			dst->gpu_offset = baked_gpu_data.gpu_entries[total_count + texture_idx + 1].offset;
			dst->gpu_size = src->pixels.size;
		}
	}

	//~ Dedrick: Package results.
	PAKM_BakeBundle bundle = {};
	{
		bundle.sections[PAK_SECTION_KIND_STRING_TABLE] = { baked_strings.string_tables, baked_strings.string_tables_count * sizeof(PAK_SectionElementType_StringTable) };
		bundle.sections[PAK_SECTION_KIND_STRING_DATA]  = { baked_strings.string_data, baked_strings.string_data_size };
		bundle.sections[PAK_SECTION_KIND_SHADER]       = { baked_cpu_data.shaders, baked_cpu_data.shaders_size };
		bundle.sections[PAK_SECTION_KIND_TEXTURE]      = { baked_cpu_data.textures, baked_cpu_data.textures_size };
		bundle.sections[PAK_SECTION_KIND_GPU_HEADER]   = { baked_cpu_data.gpu_header, baked_cpu_data.gpu_header_size };
		bundle.sections[PAK_SECTION_KIND_GPU_DATA]     = { baked_gpu_data.gpu_data, baked_gpu_data.gpu_data_size };
	}

	//~ Dedrick: Serialize bundles.
	Buffer8List output_blobs = {};
	{
		PAK_Header *header = arena_push<PAK_Header>(pm_arena);
		header->magic = PAK_MAGIC_CONSTANT;
		header->version = PAK_VERSION;
		header->section_count = PAK_SECTION_KIND_COUNT;
		header->section_offset = sizeof(PAK_Header);

		PAK_Section *pak_sections = arena_push_array<PAK_Section>(pm_arena, PAK_SECTION_KIND_COUNT);
		buf8_list_push(pm_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(header), sizeof(PAK_Header) });
		buf8_list_push(pm_arena, &output_blobs, Buffer8{ reinterpret_cast<u8 *>(pak_sections), PAK_SECTION_KIND_COUNT * sizeof(PAK_Section) });

		for (u32 i = 0; i < PAK_SECTION_KIND_COUNT; ++i) {
			buf8_list_push_align(pm_arena, &output_blobs, 16);
			pak_sections[i].offset = output_blobs.total_size;
			pak_sections[i].size = bundle.sections[i].size;
			if (bundle.sections[i].size > 0) {
				buf8_list_push(pm_arena, &output_blobs, Buffer8{ static_cast<u8 *>(bundle.sections[i].data), bundle.sections[i].size });
			}
		}

		baked_cpu_data.gpu_header->gpu_offset = pak_sections[PAK_SECTION_KIND_GPU_DATA].offset;
	}

	//~ Dedrick: Write blobs.
	{
		b8 const is_written = plt_write_bytes_list_to_file_path(out_path, &output_blobs);
		if (is_written) {
			std::printf("written to %.*s\n", DK_STR8_VARG(out_path));
		}
		else {
			std::printf("ERROR: failed to write file %.*s\n", DK_STR8_VARG(out_path));
		}
	}

	// NOTE(Dedrick): Intentional leak because this is a short-lived application.
	return 0;
}
