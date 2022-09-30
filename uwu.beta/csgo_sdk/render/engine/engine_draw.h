#pragma once
#include "../directx render/font.h"

#include <array>
#include <utility>
#include <optional>

enum GradientType
{
	GRADIENT_HORIZONTAL,
	GRADIENT_VERTICAL
};

class EngineDraw : public Singleton<EngineDraw>
{
public:

	void OutlineRect(int x, int y, int w, int h, Color color);
	void FilledRect(int x, int y, int w, int h, Color color);
	void Line(int x, int y, int x2, int y2, Color color);
	void AddTexturedPolygon(int n, Vertex_t* vertice, int r, int g, int b, int a);
	void DrawTriangle(std::array< Vector2D, 3 >points, Color colour);
	RECT GetTextSize(DWORD font, const char* text);
	void DrawTextA(int X, int Y, Color Color, int Font, bool Center, const char* _Input, ...);
	void CreateFontMem(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline);
	void CreateFontRea(int i, const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline);
	void Gradient(int x, int y, int w, int h, Color first, Color second, GradientType type);
	void Draw3DCircle(const Vector& origin, float radius, Color color);
	void Draw3DBox( Vector origin, int width, int height, Color outline );

	struct
	{
		DWORD default_font;
		DWORD pixel_font;
		DWORD weapon_font;
		DWORD logs_font;
		DWORD indicators_font;
		
	} fonts;

	bool m_init = false;
	void InitFonts();

};