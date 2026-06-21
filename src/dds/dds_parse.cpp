// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::dds_parsed_from_bytes(Buffer bytes) noexcept -> DDS_Parsed {
	DDS_Parsed result = {};

	if (dds_check_magic(bytes)) {
		//~ Dedrick: Extract header.
		result.header = reinterpret_cast<DDS_Header *>(bytes.data);
		u64 cursor = sizeof(DDS_Header);

		//~ Dedrick: Extract dxt10 header.
		if (dds_has_dxt10_header(result.header)) {
			if (cursor + sizeof(DDS_DXT10Header) <= bytes.size) {
				result.dxt10_header = reinterpret_cast<DDS_DXT10Header *>(bytes.data + cursor);
				cursor += sizeof(DDS_DXT10Header);
			}
		}

		//~ Dedrick: Extract image data.
		if (bytes.size > cursor) {
			result.image_data = buf(bytes.data + cursor, bytes.size - cursor);
		}
	}

	return result;
}
