#pragma once
#include "../directx render/font.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imconfig.h"
#include "../../imgui/imgui_impl_dx9.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../imgui/imgui_freetype.h"

namespace RenderImFonts {

	extern ImFont* DefaultFont;
	extern ImFont* IconFont;

	enum e_textflags {
		none = 0,
		outline = 1 << 0,
		centered_x = 1 << 1,
		centered_y = 1 << 2,
		make_shadow = 1 << 3
	};
}

class ImGuiRendering
{
public:

	void __stdcall CreateObjects(IDirect3DDevice9* pDevice);
	void __stdcall InvalidateObjects();
	void __stdcall PreRender(IDirect3DDevice9* device);
	void __stdcall PostRender(IDirect3DDevice9* deivce);
	void __stdcall EndPresent(IDirect3DDevice9* device);
	void __stdcall SetupPresent(IDirect3DDevice9* device);

	void DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, Color clr, float width);
	void DrawLine(float x1, float y1, float x2, float y2, Color clr, float thickness = 1.f);
	void DrawLineGradient(float x1, float y1, float x2, float y2, Color clr1, Color cl2, float thickness = 1.f);
	void Rect(float x, float y, float w, float h, Color clr, float rounding = 0.f);
	void FilledRect(float x, float y, float w, float h, Color clr, float rounding = 0.f);
	void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color clr, float thickness = 1.f);
	void TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, Color clr);
	void CircleFilled(float x1, float y1, float radius, Color col, int segments);
	void DrawWave(Vector loc, float radius, Color clr, float thickness = 1.f);
	void DrawString(float x, float y, Color color, int flags, ImFont* font, const char* message, ...);
	void FilledRectGradient(float x, float y, float w, float h, Color col_upr_left,
		Color col_upr_right, Color col_bot_right, Color col_bot_left);
	void ImGuiRendering::FilledRectHorizontal(float x, float y, float w, float h, Color col_upr_left,
		Color col_upr_right, Color col_bot_right, Color col_bot_left);
	
	void build_lookup_table();
	IDirect3DDevice9* GetDevice() {
		return m_pDevice;
	}

	ImDrawList* GetDrawList() {
		return _drawList;
	}

	ImVec2 GetSize(ImFont* font, const char* message);

private:
	IDirect3DDevice9* m_pDevice;
	static constexpr auto points = 64;
	std::vector<Vector2D> lookup_table;
	ImDrawData _drawData;
	ImDrawList* _drawList;
	IDirect3DTexture9* _texture;
	ImFontAtlas _fonts;
	DWORD dwOld_D3DRS_COLORWRITEENABLE;
}; extern ImGuiRendering* g_Render;