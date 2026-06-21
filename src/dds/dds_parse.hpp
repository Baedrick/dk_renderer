// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct DDS_Parsed {
		DDS_Header *header;
		DDS_DXT10Header *dxt10_header;
		Buffer image_data;
	};

	auto dds_parsed_from_bytes(Buffer bytes) noexcept -> DDS_Parsed;
}
