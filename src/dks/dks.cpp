// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::u16 const dk::dks_section_element_size_table[] = {
	sizeof(u8),
	sizeof(DKS_SectionElementType_TopLevelInfo),
	sizeof(DKS_SectionElementType_StringData),
	sizeof(DKS_SectionElementType_StringTable)
};
static_assert(dk::array_count(dk::dks_section_element_size_table) == dk::DKS_SECTION_KIND_COUNT);
