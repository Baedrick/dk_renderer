// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct PAKG_Shader {
		String8 name;
		Buffer8 binary;
		PAK_ShaderKind kind;
	};

	struct PAKG_ShaderNode {
		PAKG_ShaderNode *next;
		PAKG_Shader shader;
	};

	struct PAKG_ShaderList {
		PAKG_ShaderNode *first;
		PAKG_ShaderNode *last;
		u64 node_count;
		u64 total_size;
	};

	struct PAKG_BakeSection {
		void *data;
		u64 size;
	};

	struct PAKG_BakeBundle {
		PAK_BakeSection sections[PAK_SECTION_KIND_COUNT];
	};
}
