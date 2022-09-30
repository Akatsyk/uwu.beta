#include "im_main.h"

ImGuiRendering* g_Render = new ImGuiRendering();

namespace RenderImFonts {

	ImFont* DefaultFont;
	ImFont* IconFont;
}

void __stdcall ImGuiRendering::CreateObjects(IDirect3DDevice9* pDevice)
{
	ImGui_ImplDX9_CreateDeviceObjects();
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::InvalidateObjects()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	_drawList = nullptr;
}

void __stdcall ImGuiRendering::SetupPresent(IDirect3DDevice9* device)
{
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::EndPresent(IDirect3DDevice9* device)
{
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void __stdcall ImGuiRendering::PreRender(IDirect3DDevice9* device)
{
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetRenderState(D3DRS_LIGHTING, false);
	device->SetRenderState(D3DRS_FOGENABLE, false);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	device->SetRenderState(D3DRS_STENCILENABLE, false);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	m_pDevice = device;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	_drawData.Valid = false;
	_drawList->Clear();
	_drawList->PushClipRectFullScreen();
}

void __stdcall ImGuiRendering::PostRender(IDirect3DDevice9* device)
{
	if (!_drawList->VtxBuffer.empty()) {
		_drawData.Valid = true;
		_drawData.CmdLists = &_drawList;
		_drawData.CmdListsCount = 1;
		_drawData.TotalVtxCount = _drawList->VtxBuffer.Size;
		_drawData.TotalIdxCount = _drawList->IdxBuffer.Size;
	}

	ImGui_ImplDX9_RenderDrawData(&_drawData);
}

ImVec2 ImGuiRendering::GetSize(ImFont* font, const char* message)
{
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);

	_drawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto m_TotalSize = ImGui::CalcTextSize(output);
	ImGui::PopFont();

	return m_TotalSize;
}

void ImGuiRendering::DrawString(float x, float y, Color color, int flags, ImFont* font, const char* message, ...)
{
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);

	_drawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(output);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	Color outline_clr = Color(0, 0, 0, color.a());

	int width = 0, height = 0;

	if (!(flags & RenderImFonts::centered_x))
		size.x = 0;
	if (!(flags & RenderImFonts::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5), coord.y - (size.y * .5));

	if (flags & RenderImFonts::outline)
	{
		pos.y++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.y--;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x--;
		_drawList->AddText(pos, outline_clr.u32(), output);
	}

	if (flags & RenderImFonts::make_shadow)
	{
		pos.y++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.y--;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x--;
		_drawList->AddText(pos, outline_clr.u32(), output);
		/*pos.y = pos.y - 1;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x = pos.x - 1;
		_drawList->AddText(pos, outline_clr.u32(), output);*/
	}
	
	_drawList->AddText(pos, color.u32(), output);
	ImGui::PopFont();
}

void ImGuiRendering::DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, Color clr, float width)
{
	ImVec2 min = ImVec2(leftUpCorn.x, leftUpCorn.y);
	ImVec2 max = ImVec2(rightDownCorn.x, rightDownCorn.y);
	_drawList->AddRect(min, max, clr.u32(), 0.0F, -1, width);
}

void ImGuiRendering::DrawLine(float x1, float y1, float x2, float y2, Color clr, float thickness)
{
	_drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), clr.u32(), thickness);
}

void ImGuiRendering::DrawLineGradient(float x1, float y1, float x2, float y2, Color clr1, Color cl2, float thickness)
{
	_drawList->AddRectFilledMulticolor_t(ImVec2(x1, y2), ImVec2(x2 + thickness, y2 + thickness),
		clr1.u32(), cl2.u32(), cl2.u32(), clr1.u32());
}

void ImGuiRendering::Rect(float x, float y, float w, float h, Color clr, float rounding)
{
	_drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding);
}

void ImGuiRendering::FilledRect(float x, float y, float w, float h, Color clr, float rounding)
{
	_drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding);
}

void ImGuiRendering::FilledRectGradient(float x, float y, float w, float h, Color col_upr_left,
	Color col_upr_right, Color col_bot_right, Color col_bot_left)
{
	_drawList->AddRectFilledMulticolor_t(ImVec2(x, y), ImVec2(x + w, y + h),
		col_upr_left.u32(), col_upr_right.u32(), col_bot_right.u32(), col_bot_left.u32());
}

void ImGuiRendering::FilledRectHorizontal(float x, float y, float w, float h, Color col_upr_left,
	Color col_upr_right, Color col_bot_right, Color col_bot_left)
{
	_drawList->AddRectFilledMultiReversed(ImVec2(x, y), ImVec2(x + w, y + h),
		col_upr_left.u32(), col_upr_right.u32(), col_bot_right.u32(), col_bot_left.u32());
}

void ImGuiRendering::Triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color clr, float thickness)
{
	_drawList->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32(), thickness);
}

void ImGuiRendering::TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, Color clr)
{
	_drawList->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32());
}

void ImGuiRendering::CircleFilled(float x1, float y1, float radius, Color col, int segments)
{
	_drawList->AddCircleFilled(ImVec2(x1, y1), radius, col.u32(), segments);
}


void ImGuiRendering::DrawWave(Vector loc, float radius, Color color, float thickness)
{
	
}