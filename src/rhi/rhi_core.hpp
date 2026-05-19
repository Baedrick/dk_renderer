// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct RHI_Handle {
		u64 v;
	};

	auto operator==(RHI_Handle a, RHI_Handle b) noexcept -> b8;
	auto operator!=(RHI_Handle a, RHI_Handle b) noexcept -> b8;

	auto rhi_handle_invalid() noexcept -> RHI_Handle;

	auto rhi_init(CmdLine *cmd_line) noexcept -> void;
	auto rhi_shutdown() noexcept -> void;

	auto rhi_window_equip(RGFW_window *window) noexcept -> void;
	auto rhi_window_unequip(RGFW_window *window) noexcept -> void;

	// TODO(Dedrick): Needs surface and texture structures.
	auto rhi_surface_current_texture(RGFW_window *window) noexcept -> RHI_Handle;
	auto rhi_surface_present(RGFW_window *window) noexcept -> void;
}
