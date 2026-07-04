// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	enum DKR_EventKind : u32 {
		DKR_EVENT_KIND_NULL = 0,
		DKR_EVENT_KIND_QUIT,
		DKR_EVENT_KIND_UPDATE_TARGET_FRAME_RATE,
		DKR_EVENT_KIND_RELOAD_PAK,
		DKR_EVENT_KIND_OPEN_CONSOLE,
		DKR_EVENT_KIND_COUNT
	};

	struct DKR_EventReloadPak {
		DKR_EventKind kind;
		String8 file_path;
	};

	union DKR_Event {
		DKR_EventKind kind;
		DKR_EventReloadPak reload_pak;
	};

	struct DKR_EventNode {
		DKR_EventNode *next;
		DKR_EventNode *prev;
		DKR_Event event;
	};

	struct DKR_EventList {
		DKR_EventNode *first;
		DKR_EventNode *last;
		u64 count;
	};

	auto dkr_event_list_push(Arena *arena, DKR_EventList *events, DKR_Event const *event) noexcept -> void;
	auto dkr_push_event(DKR_Event const *event) noexcept -> void;
	auto dkr_push_event_kind(DKR_EventKind kind) noexcept -> void;
	auto dkr_next_event(DKR_Event **event) noexcept -> b8;
}
