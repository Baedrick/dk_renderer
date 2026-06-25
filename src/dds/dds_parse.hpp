// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct DDS_Parsed {
		DDS_Header *header;
		DDS_DXT10Header *dxt10_header;
		Buffer image_data;
	};

	auto dds_parse(Buffer bytes, DDS_Parsed *out) noexcept -> b8;
}
