// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::PLT_W32_GfxContext *dk::plt_w32_gfx_context;

auto dk::plt_w32_window_alloc() noexcept -> RGFW_window * {
	RGFW_window *result = plt_w32_gfx_context->free_window;
	if (result != nullptr) {
		forward_list_stack_pop(&plt_w32_gfx_context->free_window);
		std::memset(result, 0, sizeof(RGFW_window));
	}
	else {
		result = arena_push<RGFW_window>(plt_w32_gfx_context->arena);
	}
	list_push_back(
		&plt_w32_gfx_context->first_window,
		&plt_w32_gfx_context->last_window,
		result
	);
	return result;
}

auto dk::plt_w32_window_release(RGFW_window *window) noexcept -> void {
	list_remove(&plt_w32_gfx_context->first_window, &plt_w32_gfx_context->last_window, window);
	forward_list_stack_push(&plt_w32_gfx_context->free_window, window);
}

auto dk::plt_gfx_init() noexcept -> void {
	Arena *arena = arena_alloc();
	plt_w32_gfx_context = arena_push<PLT_W32_GfxContext>(arena);
	plt_w32_gfx_context->arena = arena;

	// NOTE(Dedrick): Set DPI awareness. Should be set in application manifest. Sob :(
	using w32_SetProcessDpiAwarenessContext = BOOL (void *value);
	w32_SetProcessDpiAwarenessContext *SetProcessDpiAwarenessContext_func = nullptr;
	HMODULE const user32 = LoadLibraryA("user32.dll");
	if (user32 != nullptr) {
		SetProcessDpiAwarenessContext_func =
			reinterpret_cast<w32_SetProcessDpiAwarenessContext *>(
				GetProcAddress(user32, "SetProcessDpiAwarenessContext")
			);
		FreeLibrary(user32);
	}
	if (SetProcessDpiAwarenessContext_func != nullptr) {
		void *const w32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = reinterpret_cast<void *>(-4);
		SetProcessDpiAwarenessContext_func(w32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}

auto dk::plt_gfx_shutdown() noexcept -> void {
	arena_release(plt_w32_gfx_context->arena);
}

auto dk::plt_window_open(String8 title, s32 x, s32 y, s32 w, s32 h, RGFW_windowFlags flags) noexcept -> RGFW_window * {
	RGFW_window *window = plt_w32_window_alloc();
	window = RGFW_createWindowPtr(reinterpret_cast<char const *>(title.data), x, y, w, h, flags, window);
	return window;
}

auto dk::plt_window_close(RGFW_window *window) noexcept -> void {
	if (window == nullptr) {
		return;
	}
	RGFW_window_closePtr(window);
	plt_w32_window_release(window);
}

auto dk::plt_show_dialog(RGFW_window const *parent, String8 title, String8 message, b8 error) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String16 const title16 = str16_from_8(scratch.arena, title);
	String16 const message16 = str16_from_8(scratch.arena, message);
	HWND const parent_hwnd = parent != nullptr
		? static_cast<HWND>(RGFW_window_getHWND(const_cast<RGFW_window *>(parent)))
		: nullptr;
	MessageBoxW(
		parent_hwnd,
		reinterpret_cast<WCHAR const *>(message16.data),
		reinterpret_cast<WCHAR const *>(title16.data),
		MB_OK | (error ? MB_ICONERROR : 0)
	);
	scratch_end(scratch);
}

auto dk::plt_show_in_file_browser(String8 path) noexcept -> void {
	TempArena const scratch = scratch_begin(nullptr, 0);
	String8 const path_copy = str8_copy(scratch.arena, path);
	for (u64 i = 0; i < path_copy.size; ++i) {
		if (path_copy[i] == '/') {
			const_cast<u8 *>(path_copy.data)[i] = '\\';
		}
	}
	String16 const path16 = str16_from_8(scratch.arena, path_copy);
	SFGAOF flags = 0;
	PIDLIST_ABSOLUTE list = nullptr;
	if (path16.size > 0 &&
		SUCCEEDED(SHParseDisplayName(reinterpret_cast<WCHAR const *>(path16.data), nullptr, &list, 0, &flags))
	) {
		HRESULT const hr = SHOpenFolderAndSelectItems(list, 0, nullptr, 0);
		CoTaskMemFree(list);
		(void)hr;
	}
	scratch_end(scratch);
}

auto dk::plt_w32_create_filter_specs(
	Arena *arena, PLT_FileDialogFilter const *filters, u64 filter_count, UINT *out_count
) -> COMDLG_FILTERSPEC * {
	if (filter_count == 0) {
		*out_count = 0;
		return nullptr;
	}

	TempArena const scratch = scratch_begin(&arena, 1);
	COMDLG_FILTERSPEC *const filter_spec = arena_push_array<COMDLG_FILTERSPEC>(arena, filter_count + 1);
	String8 const delims = ","_str8;
	String8JoinParams const join_params = { .separator = ";"_str8 };

	// NOTE(Dedrick): IFileDialog expects extension filters in the format: "*.txt;*.text".
	for (u64 i = 0; i < filter_count; ++i) {
		String8List const exts = str8_list_split_by_char(scratch.arena, filters[i].extensions, delims, STRING_SPLIT_FLAG_NONE);
		String8List fmt_exts = {};
		for (String8Node const *node = exts.first; node != nullptr; node = node->next) {
			str8_list_pushf(scratch.arena, &fmt_exts, "*.%.*s", static_cast<s32>(node->string.size), node->string.data);
		}
		String8 const ext_filter = str8_list_join(scratch.arena, fmt_exts, &join_params);
		String8 const display_name = str8f(
			scratch.arena,
			"%.*s (%.*s)",
			static_cast<s32>(filters[i].display_name.size), filters[i].display_name.data,
			static_cast<s32>(ext_filter.size), ext_filter.data
		);
		filter_spec[i].pszName = reinterpret_cast<WCHAR const *>(str16_from_8(arena, display_name).data);
		filter_spec[i].pszSpec = reinterpret_cast<WCHAR const *>(str16_from_8(arena, ext_filter).data);
	}
	filter_spec[filter_count].pszName = L"All Files (*.*)";
	filter_spec[filter_count].pszSpec = L"*.*";

	*out_count = static_cast<UINT>(filter_count + 1);
	scratch_end(scratch);
	return filter_spec;
}

auto dk::plt_file_dialog_pick_file(
	Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count
) noexcept -> String8 {
	using Microsoft::WRL::ComPtr;

	String8 result = {};
	TempArena const scratch = scratch_begin(&arena, 1);
	ComPtr<IFileOpenDialog> dialog = {};
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dialog)))) {
		FILEOPENDIALOGOPTIONS options = {};
		dialog->GetOptions(&options);
		dialog->SetOptions(options | PLT_W32_FILE_DIALOG_COMMON_FLAGS | FOS_FILEMUSTEXIST);

		UINT filter_spec_count = 0;
		COMDLG_FILTERSPEC const *const filter_spec = plt_w32_create_filter_specs(
			scratch.arena, filters, filter_count, &filter_spec_count
		);
		if (filter_spec_count > 0) {
			dialog->SetFileTypes(filter_spec_count, filter_spec);
		}

		HWND const parent_hwnd = parent != nullptr
			? static_cast<HWND>(RGFW_window_getHWND(const_cast<RGFW_window *>(parent)))
			: nullptr;
		if (SUCCEEDED(dialog->Show(parent_hwnd))) {
			ComPtr<IShellItem> selected_item = {};
			if (SUCCEEDED(dialog->GetResult(&selected_item))) {
				PWSTR path = nullptr;
				if (SUCCEEDED(selected_item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
					if (path != nullptr) {
						String16 const path16 = {
							.data = reinterpret_cast<u16 *>(path),
							.size = static_cast<u64>(lstrlenW(path))
						};
						// TODO(Dedrick): Path normalization.
						result = str8_from_16(arena, path16);
						CoTaskMemFree(path);
					}
				}
			}
		}
	}
	scratch_end(scratch);
	return result;
}

auto dk::plt_file_dialog_pick_multiple_files(
	Arena *arena, RGFW_window const *parent, PLT_FileDialogFilter const *filters, u64 filter_count
) noexcept -> String8List {
	using Microsoft::WRL::ComPtr;

	String8List result = {};
	TempArena const scratch = scratch_begin(&arena, 1);
	ComPtr<IFileOpenDialog> dialog = {};
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dialog)))) {
		FILEOPENDIALOGOPTIONS options = {};
		dialog->GetOptions(&options);
		dialog->SetOptions(options | PLT_W32_FILE_DIALOG_COMMON_FLAGS | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST);

		UINT filter_spec_count = 0;
		COMDLG_FILTERSPEC const *const filter_spec = plt_w32_create_filter_specs(
			scratch.arena, filters, filter_count, &filter_spec_count
		);
		if (filter_spec_count > 0) {
			dialog->SetFileTypes(filter_spec_count, filter_spec);
		}

		HWND const parent_hwnd = parent != nullptr
			? static_cast<HWND>(RGFW_window_getHWND(const_cast<RGFW_window *>(parent)))
			: nullptr;
		if (SUCCEEDED(dialog->Show(parent_hwnd))) {
			ComPtr<IShellItemArray> items = {};
			if (SUCCEEDED(dialog->GetResults(&items))) {
				DWORD count = 0;
				items->GetCount(&count);
				for (DWORD i = 0; i < count; ++i) {
					ComPtr<IShellItem> item = {};
					if (SUCCEEDED(items->GetItemAt(i, &item))) {
						SFGAOF attribs{};
						if (SUCCEEDED(item->GetAttributes(SFGAO_FILESYSTEM, &attribs))) {
							if ((attribs & SFGAO_FILESYSTEM) == 0) {
								continue;
							}
						}
						PWSTR path = nullptr;
						if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
							if (path != nullptr) {
								String16 const path16 = {
									.data = reinterpret_cast<u16 *>(path),
									.size = static_cast<u64>(lstrlenW(path))
								};
								// TODO(Dedrick): Path normalization.
								str8_list_push(arena, &result, str8_from_16(arena, path16));
								CoTaskMemFree(path);
							}
						}
					}
				}
			}
		}
	}
	scratch_end(scratch);
	return result;
}

auto dk::plt_file_dialog_save(
	Arena *arena, RGFW_window const *parent, String8 default_name, PLT_FileDialogFilter const *filters, u64 filter_count, u64 *out_filter_index
) noexcept -> String8 {
	using Microsoft::WRL::ComPtr;

	String8 result = {};
	TempArena const scratch = scratch_begin(&arena, 1);
	ComPtr<IFileSaveDialog> dialog = {};
	if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dialog)))) {
		FILEOPENDIALOGOPTIONS options = {};
		dialog->GetOptions(&options);
		dialog->SetOptions(options | PLT_W32_FILE_DIALOG_COMMON_FLAGS | FOS_OVERWRITEPROMPT);

		UINT filter_spec_count = 0;
		COMDLG_FILTERSPEC const *const filter_spec = plt_w32_create_filter_specs(
			scratch.arena, filters, filter_count, &filter_spec_count
		);
		if (filter_spec_count > 0) {
			dialog->SetFileTypes(filter_spec_count, filter_spec);
			String8 const delims = ","_str8;
			String8List const exts = str8_list_split_by_char(scratch.arena, filters[0].extensions, delims, STRING_SPLIT_FLAG_NONE);
			if (exts.first != nullptr) {
				String16 const ext16 = str16_from_8(scratch.arena, exts.first->string);
				dialog->SetDefaultExtension(reinterpret_cast<WCHAR const *>(ext16.data));
			}
		}
		if (default_name.size > 0) {
			String16 const default_name16 = str16_from_8(scratch.arena, default_name);
			dialog->SetFileName(reinterpret_cast<WCHAR const *>(default_name16.data));
		}

		HWND const parent_hwnd = parent != nullptr
			? static_cast<HWND>(RGFW_window_getHWND(const_cast<RGFW_window *>(parent)))
			: nullptr;
		if (SUCCEEDED(dialog->Show(parent_hwnd))) {
			ComPtr<IShellItem> selected_item = {};
			if (SUCCEEDED(dialog->GetResult(&selected_item))) {
				UINT file_type_index = 0;
				if (out_filter_index != nullptr && SUCCEEDED(dialog->GetFileTypeIndex(&file_type_index))) {
					// NOTE(Dedrick): IFileSaveDialog::GetFileTypeIndex is one-based.
					*out_filter_index = file_type_index - 1;
				}

				PWSTR path = nullptr;
				if (SUCCEEDED(selected_item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
					if (path != nullptr) {
						String16 const path16 = {
							.data = reinterpret_cast<u16 *>(path),
							.size = static_cast<u64>(lstrlenW(path))
						};
						// TODO(Dedrick): Path normalization.
						result = str8_from_16(arena, path16);
						CoTaskMemFree(path);
					}
				}
			}
		}
	}
	scratch_end(scratch);
	return result;
}

auto dk::plt_file_dialog_pick_folder(Arena *arena, RGFW_window const *parent) noexcept -> String8 {
	using Microsoft::WRL::ComPtr;

	String8 result = {};
	TempArena const scratch = scratch_begin(&arena, 1);
	ComPtr<IFileOpenDialog> dialog = {};
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dialog)))) {
		FILEOPENDIALOGOPTIONS options{};
		dialog->GetOptions(&options);
		dialog->SetOptions(options | PLT_W32_FILE_DIALOG_COMMON_FLAGS | FOS_PICKFOLDERS);

		HWND const parent_hwnd = parent != nullptr
			? static_cast<HWND>(RGFW_window_getHWND(const_cast<RGFW_window *>(parent)))
			: nullptr;
		if (SUCCEEDED(dialog->Show(parent_hwnd))) {
			ComPtr<IShellItem> selected_item = {};
			if (SUCCEEDED(dialog->GetResult(&selected_item))) {
				PWSTR path = nullptr;
				if (SUCCEEDED(selected_item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
					if (path != nullptr) {
						String16 const path16 = {
							.data = reinterpret_cast<u16 *>(path),
							.size = static_cast<u64>(lstrlenW(path))
						};
						// TODO(Dedrick): Path normalization.
						result = str8_from_16(arena, path16);
						CoTaskMemFree(path);
					}
				}
			}
		}
	}
	scratch_end(scratch);
	return result;
}
