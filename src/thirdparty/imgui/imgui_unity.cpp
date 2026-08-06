// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#define RGFWDEF
#define RGFW_NATIVE
#define RGFW_OPENGL
#include "thirdparty/rgfw/RGFW.h"
#include "thirdparty/glad/gl.h"

#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui_impl_opengl3.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#define RGFW_IMGUI_IMPLEMENTATION
#include "imgui_impl_rgfw.h"
#undef RGFW_IMGUI_IMPLEMENTATION
