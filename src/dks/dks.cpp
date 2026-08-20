// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

dk::u16 const dk::dks_section_element_size_table[] = {
	sizeof(u8),
	sizeof(DKS_SectionElementType_TopLevelInfo),
	sizeof(DKS_SectionElementType_StringData),
	sizeof(DKS_SectionElementType_StringTable),
	sizeof(DKS_SectionElementType_Nodes),
	sizeof(DKS_SectionElementType_GPU_Transforms),
	sizeof(DKS_SectionElementType_GPU_Instances),
	sizeof(DKS_SectionElementType_GPU_Meshes),
	sizeof(DKS_SectionElementType_GPU_Vertices),
	sizeof(DKS_SectionElementType_GPU_Meshlets),
	sizeof(DKS_SectionElementType_GPU_MeshletBounds),
	sizeof(DKS_SectionElementType_GPU_MeshletVertices),
	sizeof(DKS_SectionElementType_GPU_MeshletTriangles),
};
static_assert(dk::array_count(dk::dks_section_element_size_table) == dk::DKS_SECTION_KIND_COUNT);
