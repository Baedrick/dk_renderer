// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

auto dk::dds_parse(Buffer bytes, DDS_Parsed *out) noexcept -> b8 {
	b8 good = false;
	if (dds_check_magic(bytes)) {
		//~ Dedrick: Extract header.
		DDS_Header *header = reinterpret_cast<DDS_Header *>(bytes.data);
		u64 cursor = sizeof(DDS_Header);

		//~ Dedrick: Extract dxt10 header.
		DDS_DXT10Header *dxt10_header = nullptr;
		if (dds_has_dxt10_header(header)) {
			if (cursor + sizeof(DDS_DXT10Header) <= bytes.size) {
				dxt10_header = reinterpret_cast<DDS_DXT10Header *>(bytes.data + cursor);
				cursor += sizeof(DDS_DXT10Header);
			}
		}

		//~ Dedrick: Extract image data.
		Buffer image_data = {};
		if (bytes.size > cursor) {
			image_data = buf(bytes.data + cursor, bytes.size - cursor);
			good = true;
		}

		//~ Dedrick: Write output.
		if (good) {
			out->header = header;
			out->dxt10_header = dxt10_header;
			out->image_data = image_data;
		}
	}
	return good;
}
