#pragma once
#include "font.h"

struct VerticeT
{
	float x, y, z, rhw;
	int c;
};

struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};

namespace Fonts
{
	extern std::shared_ptr<c_font> Main;
	extern std::shared_ptr<c_font> Icon;
}

class DirectRender
{
private:

	static void StringPrototype(Vector2D pos, std::wstring string, Color color, std::shared_ptr<c_font>& font, uint8_t flags = 0);
	static Vector2D TextSizeProtype(std::wstring string, std::shared_ptr<c_font>& font);
	void GradientHorizontalPrototype(Vector2D a, Vector2D b, Color c_a, Color c_b);
	void GradientVerticalPrototype(Vector2D a, Vector2D b, Color c_a, Color c_b);
	void FilledPrototype(Vector2D a, Vector2D b, Color c);
	void RectPrototype(Vector2D a, Vector2D b, Color c);
	void LinePrototype(Vector2D a, Vector2D b, Color c);

public:

	void SetupRenderStates();
	void Init(IDirect3DDevice9* dev);

	void InitDevice(IDirect3DDevice9* dev);
	void InvalidateDevice();

	// ------------------------------------------ //

	D3DVIEWPORT9 GetViewport();
	void SetViewport(D3DVIEWPORT9 vp);

	// ------------------------------------------ //

	void DrawString(Vector2D pos, std::string string, Color color, std::shared_ptr<c_font>& font, uint8_t flags = 0) const;
	Vector2D GetTextSize(std::string string, std::shared_ptr<c_font>& font) const;
	void GradientHorizontal(float x, float y, float w, float h, Color c_a, Color c_b);
	void GradientVertical(float x, float y, float w, float h, Color c_a, Color c_b);
	void FilledRect(float x, float y, float w, float h, Color c);
	void Rect(float x, float y, float w, float h, Color c);
	void Line(float x, float y, float x1, float x2, Color c);
	void CircleDualColor(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device);
	void CircleDualColor(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2, IDirect3DDevice9* m_device);
	void RoundedRect(int x, int y, int w, int h, int iSmooth, Color Color);
private:

	IDirect3DDevice9* dev;
	bool bReady = false;
	std::vector<std::shared_ptr<c_font>> m_Fonts;
};

extern DirectRender DrawList;