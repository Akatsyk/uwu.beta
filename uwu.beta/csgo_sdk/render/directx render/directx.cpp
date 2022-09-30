#include "directx.h"

DirectRender DrawList;

namespace Fonts
{
	std::shared_ptr<c_font> Main = std::make_shared<c_font>("Tahoma", 7, 400);
	std::shared_ptr<c_font> Icon = std::make_shared<c_font>("astrix", 30, 500);
}

void DirectRender::Init(IDirect3DDevice9* dev)
{
	if (bReady)
		return;

	m_Fonts.push_back(Fonts::Main);
	m_Fonts.push_back(Fonts::Icon);

	InitDevice(dev);
	bReady = true;
}

void DirectRender::InitDevice(IDirect3DDevice9* dev)
{
	this->dev = dev;

	for (auto& font : m_Fonts)
	{
		font->init_device_objects(dev);
		font->restore_device_objects();
	}
}

void DirectRender::InvalidateDevice()
{
	dev = nullptr;

	for (auto& font : m_Fonts)
		font->invalidate_device_objects();
}

void DirectRender::SetupRenderStates()
{
	dev->SetVertexShader(nullptr);
	dev->SetPixelShader(nullptr);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	dev->SetRenderState(D3DRS_LIGHTING, false);
	dev->SetRenderState(D3DRS_FOGENABLE, false);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	dev->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	dev->SetRenderState(D3DRS_STENCILENABLE, false);

	dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);

	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	dev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	dev->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dev->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

	dev->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	dev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
}

D3DVIEWPORT9 DirectRender::GetViewport()
{
	D3DVIEWPORT9 vp;
	dev->GetViewport(&vp);
	return vp;
}

void DirectRender::SetViewport(D3DVIEWPORT9 vp)
{
	dev->SetViewport(&vp);
}

void DirectRender::StringPrototype(Vector2D pos, std::wstring string, Color c, std::shared_ptr<c_font>& font, uint8_t flags)
{
	font->draw_string(std::roundf(pos.x), std::roundf(pos.y),
		D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()), string.c_str(), flags);
}

void DirectRender::DrawString(Vector2D pos, std::string string, Color color, std::shared_ptr<c_font>& font, uint8_t flags) const
{
	StringPrototype(pos, std::wstring(string.begin(), string.end()),
		color, font, flags);
}

Vector2D DirectRender::TextSizeProtype(std::wstring string, std::shared_ptr<c_font>& font)
{
	SIZE size;
	font->get_text_extent(string.c_str(), &size);
	return Vector2D(static_cast<float>(size.cx), static_cast<float>(size.cy));
}

Vector2D DirectRender::GetTextSize(std::string string, std::shared_ptr<c_font>& font) const
{
	return TextSizeProtype(std::wstring(string.begin(), string.end()), font);
}

void DirectRender::GradientHorizontalPrototype(Vector2D a, Vector2D b, Color c_a, Color c_b)
{
	b += a;

	VerticeT verts[4] = {
		{ a.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a()) },
		{ b.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a()) },
		{ a.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a()) },
		{ b.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a()) }
	};

	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts, 20);
}

void DirectRender::GradientHorizontal(float x, float y, float w, float h, Color c_a, Color c_b)
{
	GradientHorizontalPrototype({ x, y }, { w, h }, c_a, c_b);
}

void DirectRender::GradientVerticalPrototype(Vector2D a, Vector2D b, Color c_a, Color c_b)
{
	b += a;

	VerticeT verts[4] = {
		{ a.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a()) },
		{ b.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a()) },
		{ a.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a()) },
		{ b.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a()) }
	};

	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts, 20);
}

void DirectRender::GradientVertical(float x, float y, float w, float h, Color c_a, Color c_b)
{
	GradientVerticalPrototype({ x, y }, { w, h }, c_a, c_b);
}

void DirectRender::RectPrototype(Vector2D a, Vector2D b, Color c)
{
	b += a;

	b.x -= 1;
	b.y -= 1;

	VerticeT verts[5] = {
		{ float(a.x), float(a.y), 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ float(b.x), float(a.y), 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ float(b.x), float(b.y), 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ float(a.x), float(b.y), 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ float(a.x), float(a.y), 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) }
	};

	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &verts, 20);
}

void DirectRender::Rect(float x, float y, float w, float h, Color c)
{
	RectPrototype({ x, y }, { w + 1, h + 1 }, c);
}

void DirectRender::FilledPrototype(Vector2D a, Vector2D b, Color c)
{
	b += a;

	VerticeT verts[4] = {
		{ a.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ b.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ a.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ b.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) }
	};

	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts, 20);
}

void DirectRender::FilledRect(float x, float y, float w, float h, Color c)
{
	FilledPrototype({ x, y }, { w, h }, c);
}

void DirectRender::LinePrototype(Vector2D a, Vector2D b, Color c)
{
	VerticeT verts[2] = {
		{ a.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) },
		{ b.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()) }
	};

	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_LINELIST, 1, &verts, 20);
}

void DirectRender::Line(float x, float y, float x1, float x2, Color c)
{
	LinePrototype({ x, y }, { x1, x2 }, c);
}

void DirectRender::CircleDualColor(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;

	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180, pi = D3DX_PI;

	if (type == 1)
		pi = D3DX_PI; // Full circle
	if (type == 2)
		pi = D3DX_PI / 2; // 1/2 circle
	if (type == 3)
		pi = D3DX_PI / 4; // 1/4 circle

	pi = D3DX_PI / type; // 1/4 circle

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);

	float hue = 0.f;

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;

		auto clr = Color::FromHSB(hue, 1.f, 1.f);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), 150);
		hue += 0.02;
	}

	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));

		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}

	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);

	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();

	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}

void DirectRender::CircleDualColor(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;

	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180, pi = D3DX_PI;

	if (type == 1)
		pi = D3DX_PI;

	if (type == 2)
		pi = D3DX_PI / 2;

	if (type == 3)
		pi = D3DX_PI / 4;

	pi = D3DX_PI / type;

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = color2;

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].color = color;
	}

	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(angle) * (circle[i].x - x) - sin(angle) * (circle[i].y - y);
		circle[i].y = y + sin(angle) * (circle[i].x - x) + cos(angle) * (circle[i].y - y);
	}

	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);

	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();

	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}

void DirectRender::RoundedRect(int x, int y, int w, int h, int iSmooth, Color Color)
{
	POINT pt[4];

	pt[0].x = x + (w - iSmooth);
	pt[0].y = y + (h - iSmooth);

	pt[1].x = x + iSmooth;
	pt[1].y = y + (h - iSmooth);

	pt[2].x = x + iSmooth;
	pt[2].y = y + iSmooth;

	pt[3].x = x + w - iSmooth;
	pt[3].y = y + iSmooth;

	DrawList.FilledRect(x, y + iSmooth, w, h - iSmooth * 2, Color);
	DrawList.FilledRect(x + iSmooth, y, w - iSmooth * 2, h, Color);

	float fDegree = 0;

	for (int i = 0; i < 4; i++)
	{
		for (float k = fDegree; k < fDegree + ( D3DX_PI * 2 ) / 4; k += D3DXToRadian(1))
		{
			DrawList.Line(pt[i].x, pt[i].y,
				pt[i].x + (cosf(k) * iSmooth),
				pt[i].y + (sinf(k) * iSmooth),
				Color);
		}

		fDegree += (D3DX_PI * 2) / 4;
	}
}