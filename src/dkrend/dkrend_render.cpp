// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::dkr_pak_path(Arena *arena) noexcept -> String8 {
	return str8f(arena, "%.*s/dkrend.pak", DK_STR8_VARG(get_process_info()->binary_dir));
}

auto dk::dkr_pak_read_metadata(Arena *arena, File file, PAK_Parsed *out_parsed) noexcept -> b8 {
	b8 good = false;
	PAK_Header header_maybe = {};
	file_read(file, 0, sizeof(PAK_Header), &header_maybe);
	if (header_maybe.magic == PAK_MAGIC_CONSTANT && header_maybe.version == PAK_VERSION) {
		u64 const size = attributes_from_file(file).size;
		u64 const metadata_size = header_maybe.metadata_size;
		if (size >= metadata_size) {
			Buffer buffer = {};
			buffer.size = metadata_size;
			buffer.data = arena_push_array<u8>(arena, buffer.size);
			file_read(file, 0, buffer.size, buffer.data);
			good = pak_parse(buffer, out_parsed);
			if (!good) {
				arena_pop(arena, buffer.size);
			}
		}
	}
	return good;
}

auto dk::dkr_render_assets_load(File file, PAK_Parsed const *pak, DKR_RenderAssets *out_assets) noexcept -> b8 {
	ZoneScoped;
	TempArena const scratch = scratch_begin(nullptr, 0);
	b8 success = true;

	u64 const chunk_size = mega_bytes(4);
	u8 *const buffer = arena_push_array<u8>(scratch.arena, chunk_size);

	//~ Dedrick: Shader tables.
	enum ShaderModule : u32 {
		SHADER_MODULE_HELLO_TRIANGLE_VERT,
		SHADER_MODULE_HELLO_TRIANGLE_FRAG,
		SHADER_MODULE_DUMMY_COMP,
		SHADER_MODULE_COUNT
	};
	struct { GLenum stage; String8 name; } const shader_module_table[] = {
		{ GL_VERTEX_SHADER,   "hello_triangle.vert"_str8 },
		{ GL_FRAGMENT_SHADER, "hello_triangle.frag"_str8 },
		{ GL_COMPUTE_SHADER,  "dummy.comp"_str8 },
	};
	static_assert(array_count(shader_module_table) == SHADER_MODULE_COUNT, "Mismatch shader modules count");
	struct { String8 name; u32 count; ShaderModule modules[2]; } const shader_table[] = {
		{ "hello_triangle"_str8, 2, { SHADER_MODULE_HELLO_TRIANGLE_VERT, SHADER_MODULE_HELLO_TRIANGLE_FRAG } },
		{ "dummy"_str8,          1, { SHADER_MODULE_DUMMY_COMP } },
	};
	static_assert(array_count(shader_table) == DKR_SHADER_KIND_COUNT, "Mismatch shader count");
	u64 const shader_data_offset = pak->sections[PAK_SECTION_KIND_SHADER_DATA].offset;

	//~ Dedrick: Compile shader stages.
	GLuint shader_modules[SHADER_MODULE_COUNT] = {};
	for (u64 m = 0; m < SHADER_MODULE_COUNT; ++m) {
		String8 const name = shader_module_table[m].name;
		PAK_Shader const *pak_shader = pak_shader_from_name(pak, name);
		DK_ASSERT(pak_shader != nullptr);
		DK_ASSERT(pak_shader->size <= chunk_size);
		file_read(
			file,
			shader_data_offset + pak_shader->offset,
			shader_data_offset + pak_shader->offset + pak_shader->size,
			buffer
		);
		Buffer const binary = buf(buffer, pak_shader->size);
		shader_modules[m] = ogl_shader_stage_compile(shader_module_table[m].stage, binary, name);

		// NOTE(Dedrick): Do not stop compiling the rest of the shaders if this
		// module fails to compile. Its more helpful to us if we can get as much
		// logs on all of the errors as possible at once to display.
		if (shader_modules[m] == 0) {
			success = false;
		}
	}

	//~ Dedrick: Link Shaders.
	if (success) {
		for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
			String8 const name = shader_table[s].name;
			u32 const count = shader_table[s].count;
			GLuint modules[array_count(shader_table[0].modules)] = {};
			for (u32 idx = 0; idx < count; ++idx) {
				modules[idx] = shader_modules[shader_table[s].modules[idx]];
			}
			out_assets->shaders[s] = ogl_shader_link(count, modules, name);

			// NOTE(Dedrick): Do not stop linking the rest of the shaders if this
			// shader fails to link. Its more helpful to us if we can get as much
			// logs on all of the errors as possible at once to display.
			if (out_assets->shaders[s] == 0) {
				success = false;
			}
		}
	}

	//~ Dedrick: Clean up intermediate shader objects.
	for (u64 m = 0; m < SHADER_MODULE_COUNT; ++m) {
		glDeleteShader(shader_modules[m]);
	}

	//~ Dedrick: Create textures.
	// TODO(Dedrick): Move ogl helpers to rhi_opengl.
	struct OGL_TextureFormat { GLenum fmt; GLenum pixel_fmt; GLenum type; u32 bytes_per_pixel; }
	const ogl_fmt_table[] = {
		{ GL_NONE, GL_NONE, GL_NONE, 0 },
		{ GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, 4 }
	};
	static_assert(array_count(ogl_fmt_table) == PAK_TEXTURE_FORMAT_COUNT, "Mismatch texture format table");
	String8 const texture_name_table[] = {
		"tony_mc_mapface.dds"_str8
	};
	u64 const texture_data_offset = pak->sections[PAK_SECTION_KIND_TEXTURE_DATA].offset;
	u64 const texture_data_size = pak->sections[PAK_SECTION_KIND_TEXTURE_DATA].size;

	u8 *stage_base = nullptr;
	GPU_AllocResult alloc_result;
	do {
		alloc_result = gpu_arena_try_push(dkr_context->render.stage_arena, texture_data_size, 16, reinterpret_cast<void **>(&stage_base));
		if (alloc_result == GPU_AllocResult::ERROR_OUT_OF_MEMORY) {
			if (dkr_context->render.stage_sync) {
				glClientWaitSync(dkr_context->render.stage_sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
				glDeleteSync(dkr_context->render.stage_sync);
				dkr_context->render.stage_sync = nullptr;
			}
			gpu_arena_clear(dkr_context->render.stage_arena);
		}
	} while (alloc_result != GPU_AllocResult::OK);

	for (u64 cursor = 0; cursor < texture_data_size; ) {
		u64 const read_size = min(chunk_size, texture_data_size - cursor);
		file_read(
			file,
			texture_data_offset + cursor,
			texture_data_offset + cursor + read_size,
			buffer
		);
		std::memcpy(stage_base + cursor, buffer, read_size);
		cursor += read_size;
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dkr_context->render.stage_buffer);
	for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
		//~ Dedrick: Unpack texture from pak.
		String8 const tex_name = texture_name_table[t];
		PAK_Texture const *pak_tex = pak_texture_from_name(pak, tex_name);
		DK_ASSERT(pak_tex != nullptr);
		OGL_TextureFormat const *tex_fmt = &ogl_fmt_table[pak_tex->format];
		GLenum tex_kind = GL_TEXTURE_2D;
		switch (pak_tex->kind) {
			case PAK_TEXTURE_KIND_2D: { tex_kind = GL_TEXTURE_2D; } break;
			case PAK_TEXTURE_KIND_3D: { tex_kind = GL_TEXTURE_3D; } break;
		}

		GLuint tex = 0;
		glCreateTextures(tex_kind, 1, &tex);
		glObjectLabel(GL_TEXTURE, tex, static_cast<GLsizei>(tex_name.size), reinterpret_cast<char const *>(tex_name.data));

		if (tex_kind == GL_TEXTURE_2D) {
			glTextureStorage2D(tex, pak_tex->mip_count, tex_fmt->fmt, pak_tex->width, pak_tex->height);

			//~ Dedrick: Upload mip maps.
			u64 offset = pak_tex->offset;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage2D(tex, mip, 0, 0, mip_w, mip_h, tex_fmt->pixel_fmt, tex_fmt->type, reinterpret_cast<void const *>(offset));
				offset += static_cast<u64>(mip_w) * mip_h * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
			}
		}
		else if (tex_kind == GL_TEXTURE_3D) {
			glTextureStorage3D(tex, pak_tex->mip_count, tex_fmt->fmt, pak_tex->width, pak_tex->height, pak_tex->depth);

			//~ Dedrick: Upload mip maps.
			u64 offset = pak_tex->offset;
			u32 mip_w = pak_tex->width;
			u32 mip_h = pak_tex->height;
			u32 mip_d = pak_tex->depth;
			for (u32 mip = 0; mip < pak_tex->mip_count; ++mip) {
				glTextureSubImage3D(tex, mip, 0, 0, 0, mip_w, mip_h, mip_d, tex_fmt->pixel_fmt, tex_fmt->type, reinterpret_cast<void const *>(offset));
				offset += static_cast<u64>(mip_w) * mip_h * mip_d * tex_fmt->bytes_per_pixel;
				mip_w = mip_w > 1 ? (mip_w >> 1) : 1;
				mip_h = mip_h > 1 ? (mip_h >> 1) : 1;
				mip_d = mip_d > 1 ? (mip_d >> 1) : 1;
			}
		}

		out_assets->textures[t] = tex;
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	//~ Dedrick: Update staging fence.
	if (dkr_context->render.stage_sync) {
		glDeleteSync(dkr_context->render.stage_sync);
	}
	dkr_context->render.stage_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	//~ Dedrick: Clean up on failure.
	if (!success) {
		for (u64 s = 0; s < DKR_SHADER_KIND_COUNT; ++s) {
			glDeleteProgram(out_assets->shaders[s]);
			out_assets->shaders[s] = 0;
		}
		for (u64 t = 0; t < DKR_TEXTURE_KIND_COUNT; ++t) {
			glDeleteTextures(1, &out_assets->textures[t]);
			out_assets->textures[t] = 0;
		}
	}

	return success;
}
