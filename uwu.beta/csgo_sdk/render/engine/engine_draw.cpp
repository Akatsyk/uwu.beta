#include "engine_draw.h"
#include "../../config/config.h"
#include "../../sdk/utils/math.hpp"

void EngineDraw::DrawTextA(int X, int Y, Color Color, int Font, bool Center, const char* _Input, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, _Input);
	_vsnprintf(buf, sizeof(buf), _Input, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int Width = 0, Height = 0;

	if (Center)
		g_VGuiSurface->GetTextSize(Font, wbuf, Width, Height);

	g_VGuiSurface->DrawSetTextColor(Color.r(), Color.g(), Color.b(), Color.a());
	g_VGuiSurface->DrawSetTextFont(Font);
	g_VGuiSurface->DrawSetTextPos(X - (Width / 2), Y);
	g_VGuiSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void EngineDraw::OutlineRect(int x, int y, int w, int h, Color color)
{
	g_VGuiSurface->DrawSetColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void EngineDraw::FilledRect(int x, int y, int w, int h, Color color)
{
	g_VGuiSurface->DrawSetColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
}

RECT EngineDraw::GetTextSize(DWORD font, const char* text)
{
	va_list va_alist;
	char buf[ 1024 ];
	va_start( va_alist, text );
	_vsnprintf( buf, sizeof( buf ), text, va_alist );
	va_end( va_alist );
	wchar_t wbuf[ 1024 ];
	MultiByteToWideChar( CP_UTF8, 0, buf, 256, wbuf, 256 );

	RECT rect; int x, y;
	g_VGuiSurface->GetTextSize(font, wbuf, x, y);
	rect.left = x; rect.bottom = y;
	rect.right = x;
	return rect;
}

void EngineDraw::Line(int x, int y, int x2, int y2, Color color)
{
	g_VGuiSurface->DrawSetColor(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->DrawLine(x, y, x2, y2);
}

void EngineDraw::AddTexturedPolygon(int n, Vertex_t* vertice, int r, int g, int b, int a)
{
	static int texture_id = g_VGuiSurface->CreateNewTextureID(true);
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	g_VGuiSurface->DrawSetColor(r, g, b, a);
	g_VGuiSurface->DrawSetTexture(texture_id);
	g_VGuiSurface->DrawTexturedPolygon(n, vertice);
}

void EngineDraw::DrawTriangle(std::array< Vector2D, 3 >points, Color color)
{
	std::array< Vertex_t, 3 >vertices{ Vertex_t(points.at(0)), Vertex_t(points.at(1)), Vertex_t(points.at(2)) };
	AddTexturedPolygon(3, vertices.data(), color.r(), color.g(), color.b(), color.a());
}

void EngineDraw::InitFonts()
{
	if (!m_init) {
		m_init = true;

		m_LastFontName = "Verdana";
		fonts.default_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(fonts.default_font, "Verdana", 12, 700, 0, 0, FONTFLAG_DROPSHADOW );

		m_LastFontName = "Small Fonts";
		fonts.pixel_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(fonts.pixel_font, "Small Fonts", 8, 400, 0, 0, FONTFLAG_OUTLINE);

		m_LastFontName = "undefeated";
		fonts.weapon_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(fonts.weapon_font, "undefeated", 13, 500, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

		m_LastFontName = "Lucida Console";
		fonts.logs_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(fonts.logs_font, "Lucida Console", 10, 0, 0, 0, FONTFLAG_DROPSHADOW);

		m_LastFontName = "Verdana";
		fonts.indicators_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(fonts.indicators_font, "Verdana", 12, 200, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

		m_LastFontName.clear();
	}
}

void EngineDraw::Gradient(int x, int y, int w, int h, Color first, Color second, GradientType type)
{
	auto filled_rect_fade = [&](bool reversed, float alpha) {
		using Fn = void(__thiscall*)(VOID*, int, int, int, int, unsigned int, unsigned int, bool);
		CallVFunction< Fn >(g_VGuiSurface, 123) (
			g_VGuiSurface, x, y, x + w, y + h,
			reversed ? alpha : 0,
			reversed ? 0 : alpha,
			type == GRADIENT_HORIZONTAL);
	};

	static auto blend = [](const Color& first, const Color& second, float t) -> Color {
		return Color(
			first.r() + t * (second.r() - first.r()),
			first.g() + t * (second.g() - first.g()),
			first.b() + t * (second.b() - first.b()),
			first.a() + t * (second.a() - first.a()));
	};

	if (first.a() == 255 || second.a() == 255) {
		g_VGuiSurface->DrawSetColor(blend(first, second, 0.5f));
		g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
	}

	g_VGuiSurface->DrawSetColor(first);
	filled_rect_fade(true, first.a());

	g_VGuiSurface->DrawSetColor(second);
	filled_rect_fade(false, second.a());
}

bool IsZeroVec(Vector m_Rec)
{
	return m_Rec.x == 0.0f && m_Rec.y == 0.0f
		&& m_Rec.z == 0.0f;
}

void EngineDraw::Draw3DCircle(const Vector& origin, float radius, Color color)
{
	static auto prevScreenPos = Vector(0, 0, 0);

	auto step = M_PI * 2.0f / 72.0f;
	auto screenPos = Vector(0, 0, 0);

	for (auto rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step) //-V1034
	{
		Vector pos(radius * cos(rotation) + origin.x, radius * sin(rotation) + origin.y, origin.z);

		if (Math::WorldToScreen(pos, screenPos))
		{
			if (!IsZeroVec(prevScreenPos))
				Line(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);

			prevScreenPos = screenPos;
		}
	}
}

void EngineDraw::CreateFontMem(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
{
	DWORD flags = FONTFLAG_NONE;

	if (antialias.value_or(false))
		flags |= FONTFLAG_ANTIALIAS;

	if (dropshadow.value_or(false))
		flags |= FONTFLAG_DROPSHADOW;

	if (outline.value_or(false))
		flags |= FONTFLAG_OUTLINE;
	
	m_LastFontName = name.c_str();
	g_Options.CustomFonts.m_Font[ g_Options.CustomFonts.m_CurrentSlot ] = g_VGuiSurface->CreateFont_();
	g_VGuiSurface->SetFontGlyphSet(g_Options.CustomFonts.m_Font[ g_Options.CustomFonts.m_CurrentSlot], name.c_str(), (int)size, (int)weight, 0, 0, flags);
	m_LastFontName.clear();
}

void EngineDraw::CreateFontRea(int i, const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
{
	DWORD flags = FONTFLAG_NONE;

	if (antialias.value_or(false))
		flags |= FONTFLAG_ANTIALIAS;

	if (dropshadow.value_or(false))
		flags |= FONTFLAG_DROPSHADOW;

	if (outline.value_or(false))
		flags |= FONTFLAG_OUTLINE;

	m_LastFontName = name.c_str();
	g_Options.CustomFonts.m_Font[i] = g_VGuiSurface->CreateFont_();
	g_VGuiSurface->SetFontGlyphSet(g_Options.CustomFonts.m_Font[i], name.c_str(), (int)size, (int)weight, 0, 0, flags);
	m_LastFontName.clear();
}

void EngineDraw::Draw3DBox( Vector origin, int width, int height, Color outline )
{
	float difw = float( width / 2 );
	float difh = float( height / 2 );
	Vector boxVectors[ 8 ] =
	{
		Vector( origin.x - difw, origin.y - difh, origin.z - difw ),
		Vector( origin.x - difw, origin.y - difh, origin.z + difw ),
		Vector( origin.x + difw, origin.y - difh, origin.z + difw ),
		Vector( origin.x + difw, origin.y - difh, origin.z - difw ),
		Vector( origin.x - difw, origin.y + difh, origin.z - difw ),
		Vector( origin.x - difw, origin.y + difh, origin.z + difw ),
		Vector( origin.x + difw, origin.y + difh, origin.z + difw ),
		Vector( origin.x + difw, origin.y + difh, origin.z - difw ),
	};

	static Vector vec0, vec1, vec2, vec3,
		vec4, vec5, vec6, vec7;

	if ( Math::WorldToScreen( boxVectors[ 0 ], vec0 ) &&
		Math::WorldToScreen( boxVectors[ 1 ], vec1 ) &&
		Math::WorldToScreen( boxVectors[ 2 ], vec2 ) &&
		Math::WorldToScreen( boxVectors[ 3 ], vec3 ) &&
		Math::WorldToScreen( boxVectors[ 4 ], vec4 ) &&
		Math::WorldToScreen( boxVectors[ 5 ], vec5 ) &&
		Math::WorldToScreen( boxVectors[ 6 ], vec6 ) &&
		Math::WorldToScreen( boxVectors[ 7 ], vec7 ) )
	{
		Vector lines[ 12 ][ 2 ];
		lines[ 0 ][ 0 ] = vec0;
		lines[ 0 ][ 1 ] = vec1;
		lines[ 1 ][ 0 ] = vec1;
		lines[ 1 ][ 1 ] = vec2;
		lines[ 2 ][ 0 ] = vec2;
		lines[ 2 ][ 1 ] = vec3;
		lines[ 3 ][ 0 ] = vec3;
		lines[ 3 ][ 1 ] = vec0;

		lines[ 4 ][ 0 ] = vec4;
		lines[ 4 ][ 1 ] = vec5;
		lines[ 5 ][ 0 ] = vec5;
		lines[ 5 ][ 1 ] = vec6;
		lines[ 6 ][ 0 ] = vec6;
		lines[ 6 ][ 1 ] = vec7;
		lines[ 7 ][ 0 ] = vec7;
		lines[ 7 ][ 1 ] = vec4;

		lines[ 8 ][ 0 ] = vec0;
		lines[ 8 ][ 1 ] = vec4;

		lines[ 9 ][ 0 ] = vec1;
		lines[ 9 ][ 1 ] = vec5;

		lines[ 10 ][ 0 ] = vec2;
		lines[ 10 ][ 1 ] = vec6;

		lines[ 11 ][ 0 ] = vec3;
		lines[ 11 ][ 1 ] = vec7;

		for ( int i = 0; i < 12; i++ )
			Line( lines[ i ][ 0 ].x, lines[ i ][ 0 ].y, lines[ i ][ 1 ].x, lines[ i ][ 1 ].y, outline );
	}
}