// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

namespace dk {
	struct RHI_Fence {
		u64 v;
	};

	auto rhi_init(CmdLine *cmd_line) noexcept -> void;
	auto rhi_shutdown() noexcept -> void;

	auto rhi_queue_wait_idle(RHI_Fence fence) noexcept -> b8;

	auto rhi_window_equip(RGFW_window *window) noexcept -> void;
	auto rhi_window_unequip(RGFW_window *window) noexcept -> void;

	// TODO(Dedrick): Needs surface and texture structures.
	auto rhi_surface_present(RGFW_window *window) noexcept -> void;
}
