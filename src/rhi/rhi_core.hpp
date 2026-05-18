// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct RHI_Handle {
		u64 v;
	};

	auto operator==(RHI_Handle a, RHI_Handle b) noexcept -> b8;
	auto operator!=(RHI_Handle a, RHI_Handle b) noexcept -> b8;

	auto rhi_handle_invalid() noexcept -> RHI_Handle;

	auto rhi_init(String8List args) noexcept -> void;
	auto rhi_shutdown() noexcept -> void;

	auto rhi_window_hook(RGFW_window *window) noexcept -> void;
	auto rhi_window_unhook(RGFW_window *window) noexcept -> void;
}
