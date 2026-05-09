// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::VW_Context *dk::vw_context;

auto dk::vw_init(int /*argc*/, char **/*argv*/) noexcept -> void {
	ZoneScoped;
	Arena *arena = arena_alloc();
	vw_context = arena_push<VW_Context>(arena);
	vw_context->arena = arena;
	vw_context->window = plt_window_open("RGFW"_str8, 0, 0, 800, 600, RGFW_windowCenter);
}

auto dk::vw_frame() noexcept -> b8 {
	ZoneScoped;
	// TODO(Dedrick): Process platform (window) events, then process application events.
	for (RGFW_event event; RGFW_window_checkEvent(vw_context->window, &event); ) {
		if (event.type == RGFW_windowClose) {
			vw_context->quit = true;
		}
	}
	return vw_context->quit;
}

auto dk::vw_shutdown() noexcept -> void {
	ZoneScoped;
	plt_window_close(vw_context->window);
}
