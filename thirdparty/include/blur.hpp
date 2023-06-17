#pragma once
#include<string>
#include<sstream>
#include<vector>
#include<math.h>

#include <d3dx9.h>
#include <d3d9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

extern void DrawRectBlur(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, int round);
extern void DrawBackgroundBlur(ImDrawList* drawList);
extern void SetDevice(IDirect3DDevice9* setdevice);
