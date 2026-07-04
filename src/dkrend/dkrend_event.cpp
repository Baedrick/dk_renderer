// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::dkr_event_list_push(Arena *arena, DKR_EventList *events, DKR_Event const *event) noexcept -> void {
	DKR_EventNode *node = arena_push<DKR_EventNode>(arena);
	node->event.kind = event->kind;
	// NOTE(Dedrick): Attach payload, if required.
	switch (event->kind) {
		case DKR_EVENT_KIND_RELOAD_PAK : {
			node->event.reload_pak.file_path = str8_copy(arena, event->reload_pak.file_path);
			break;
		}
	}
	list_push_back(&events->first, &events->last, node);
	events->count += 1;
}

auto dk::dkr_push_event(DKR_Event const *event) noexcept -> void {
	dkr_event_list_push(dkr_frame_arena(), &dkr_context->events[0], event);
}

auto dk::dkr_push_event_kind(DKR_EventKind kind) noexcept -> void {
	DKR_Event const event = { kind };
	dkr_event_list_push(dkr_frame_arena(), &dkr_context->events[0], &event);
}

auto dk::dkr_next_event(DKR_Event **event) noexcept -> b8 {
	DKR_EventNode *node = dkr_context->events[1].first;
	if (*event != nullptr) {
		node = DK_CAST_FROM_MEMBER(DKR_EventNode, event, *event);
		node = node->next;
	}
	*event = nullptr;
	if (node != nullptr) {
		*event = &node->event;
	}
	return *event != nullptr;
}
