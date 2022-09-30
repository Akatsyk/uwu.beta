#include "window.h"
#include "groupbox.h"
#include "../render/imgui render/im_shadow.h"

Window::Window( std::string m_Name, Vector2D* m_Pos, Vector2D m_Size, std::vector<std::string> m_WindowTabs, int* m_Selected )
{
	this->m_Name = m_Name;
	this->m_WindowPosition = m_Pos;
	this->m_Size = m_Size;

	this->m_WindowTabs = m_WindowTabs;
	this->m_Selected = m_Selected;

	this->Run( );
}

void Window::Run( )
{
	InitPosition( );
	Draw( );
	InitTabs( );
}

void Window::Draw()
{
	g_Render->FilledRect(this->m_WindowPosition->x, this->m_WindowPosition->y, this->m_Size.x, this->m_Size.y, g_Options.GUI_CLRS.Frame_1_Clr, 2);
	g_Render->FilledRect(this->m_WindowPosition->x + 1, this->m_WindowPosition->y + 1, this->m_Size.x - 2, this->m_Size.y - 2, g_Options.GUI_CLRS.Frame_2_Clr, 2 );

	g_Render->FilledRect(this->m_WindowPosition->x, this->m_WindowPosition->y + 20, this->m_Size.x, 1, Gui::m_Details.m_DefaultColor);
	g_Render->FilledRect(this->m_WindowPosition->x, this->m_WindowPosition->y + this->m_Size.y - 20, this->m_Size.x, 1, Gui::m_Details.m_DefaultColor);

	g_Render->FilledRectGradient(this->m_WindowPosition->x, this->m_WindowPosition->y + 21, this->m_Size.x, 10, g_Options.GUI_CLRS.ElementsGradient_1_Clr,
		g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr);

	g_Render->FilledRectGradient(this->m_WindowPosition->x, this->m_WindowPosition->y + this->m_Size.y - 31, this->m_Size.x, 10,
		g_Options.GUI_CLRS.ElementsGradient_2_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr, g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_1_Clr);

	g_Render->FilledRect(this->m_WindowPosition->x + 160, this->m_WindowPosition->y + 40, 650, 450, g_Options.GUI_CLRS.Frame_3_Clr, 2 );
	g_Render->FilledRect(this->m_WindowPosition->x + 161, this->m_WindowPosition->y + 41, 648, 448, g_Options.GUI_CLRS.Frame_4_Clr, 2 );

	g_Render->FilledRect(this->m_WindowPosition->x + 20, this->m_WindowPosition->y + 390, 120, 80, g_Options.GUI_CLRS.Frame_5_Clr, 2 );
	g_Render->FilledRect(this->m_WindowPosition->x + 21, this->m_WindowPosition->y + 391, 118, 78, g_Options.GUI_CLRS.Frame_6_Clr, 2 );

	g_Render->DrawString(this->m_WindowPosition->x + 10, this->m_WindowPosition->y + 4, g_Options.GUI_CLRS.NameWater_Clr,
		RenderImFonts::make_shadow, RenderImFonts::DefaultFont, _( "uwu [ beta ]" ) );

	std::string m_Build = _("build: jul 17 2022");
	g_Render->DrawString(this->m_WindowPosition->x + this->m_Size.x - 90, this->m_WindowPosition->y + 4, g_Options.GUI_CLRS.NameWater_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Build.c_str());
	g_Render->DrawString(this->m_WindowPosition->x + (this->m_Size.x / 2), this->m_WindowPosition->y + this->m_Size.y - 15,
		g_Options.GUI_CLRS.NameWater_Clr, RenderImFonts::centered_x | RenderImFonts::make_shadow, RenderImFonts::DefaultFont, _( "counter-strike:global offensive" ) );

	RectangleShadowSettings DrawShadow;

	DrawShadow.rectPos.x = this->m_WindowPosition->x - 68;
	DrawShadow.rectPos.y = this->m_WindowPosition->y - 86;
	DrawShadow.rectSize.x = this->m_Size.x;
	DrawShadow.rectSize.y = this->m_Size.y;
	DrawShadow.sigma = 7;
	
	shadow::Get().drawRectangleShadowVerticesAdaptive( DrawShadow );
}

void Window::InitPosition()
{
	static bool m_Drag{ false };
	static Vector2D m_Offset{ };

	Vector2D m_Delta = Gui::m_Input.GetMousePos() - m_Offset;

	if (m_Drag && !GetAsyncKeyState(VK_LBUTTON))
		m_Drag = false;

	if (m_Drag && GetAsyncKeyState(VK_LBUTTON))
		*this->m_WindowPosition = m_Delta;

	if (Gui::m_Input.MousePointer(Vector2D(this->m_WindowPosition->x, this->m_WindowPosition->y), Vector2D(this->m_Size.x, 20))) {
		m_Drag = true;
		m_Offset = Gui::m_Input.GetMousePos() - *this->m_WindowPosition;
	}

	if (this->m_WindowPosition->x < 0)
		this->m_WindowPosition->x = 0;

	if (this->m_WindowPosition->y < 0)
		this->m_WindowPosition->y = 0;

	static int size_w, size_h;
	g_EngineClient->GetScreenSize(size_w, size_h);

	if ((this->m_WindowPosition->x + this->m_Size.x) > size_w)
		this->m_WindowPosition->x = size_w - this->m_Size.x;

	if ((this->m_WindowPosition->y + this->m_Size.y) > size_h)
		this->m_WindowPosition->y = size_h - this->m_Size.y;
}

static bool m_TabSwitched{ true };

void Window::InitTabs()
{
	if (this->m_WindowTabs.empty())
		return;

	static bool isSwitched{ true };
	static int m_AddValue = 20;
	static int m_AddX = 25;
	static int m_Alpha{ 255 };

	if (isSwitched != m_TabSwitched)
	{
		m_Alpha = 0;
		isSwitched = m_TabSwitched;
	}

	if (m_Alpha != 255)
		m_Alpha += 5;

	if (m_Alpha > 255)
		m_Alpha = 255;

	if (m_Alpha < 0)
		m_Alpha = 0;

	std::clamp<int>(m_Alpha, 0, 255);

	if (m_TabSwitched)
	{
		for (int i = 0; i < this->m_WindowTabs.size() - 3; i++)
		{
			if (Gui::m_Input.MousePointer({ this->m_WindowPosition->x + 28 + m_AddX, this->m_WindowPosition->y + m_AddValue + 35 + (i * 60) }, { 50, 50 }) && Gui::m_Input.KeyPressed(VK_LBUTTON))
				*this->m_Selected = i;

			Color clr = i == *this->m_Selected ? Color(g_Options.GUI_CLRS.TabText_Clr.r(), g_Options.GUI_CLRS.TabText_Clr.g(), g_Options.GUI_CLRS.TabText_Clr.b()
				, m_Alpha) : Color(g_Options.GUI_CLRS.TabTextHovered_Clr.r(), g_Options.GUI_CLRS.TabTextHovered_Clr.g(), g_Options.GUI_CLRS.TabTextHovered_Clr.b()
					, m_Alpha);

			if (i != *this->m_Selected && Gui::m_Input.MousePointer({ this->m_WindowPosition->x + 28 + m_AddX, this->m_WindowPosition->y + m_AddValue + 35 + (i * 60) }, { 50, 50 }))
				clr = Color(g_Options.GUI_CLRS.SubtabFilled_Clr.r(), g_Options.GUI_CLRS.SubtabFilled_Clr.g(), g_Options.GUI_CLRS.SubtabFilled_Clr.b(), m_Alpha);

			g_Render->DrawString(this->m_WindowPosition->x + 35 + m_AddX, this->m_WindowPosition->y + m_AddValue + 40 + (i * 62), clr, 
				RenderImFonts::make_shadow, RenderImFonts::IconFont, this->m_WindowTabs[i].c_str());
		}
	}
	else
	{
		for (int i = 4; i < this->m_WindowTabs.size(); i++)
		{
			if (Gui::m_Input.MousePointer({ this->m_WindowPosition->x + 28 + m_AddX, this->m_WindowPosition->y + m_AddValue + 35 + ((i - 4) * 60) }, { 50, 50 }) && Gui::m_Input.KeyPressed(VK_LBUTTON))
				*this->m_Selected = i;

			Color clr = i == *this->m_Selected ? Color(g_Options.GUI_CLRS.TabText_Clr.r(), g_Options.GUI_CLRS.TabText_Clr.g(), g_Options.GUI_CLRS.TabText_Clr.b()
				, m_Alpha) : Color(g_Options.GUI_CLRS.TabTextHovered_Clr.r(), g_Options.GUI_CLRS.TabTextHovered_Clr.g(), g_Options.GUI_CLRS.TabTextHovered_Clr.b()
					, m_Alpha);

			if (i != *this->m_Selected && Gui::m_Input.MousePointer({ this->m_WindowPosition->x + 28 + m_AddX, this->m_WindowPosition->y + m_AddValue + 35 + ((i - 4) * 60) }, { 50, 50 }))
				clr = Color(g_Options.GUI_CLRS.SubtabFilled_Clr.r(), g_Options.GUI_CLRS.SubtabFilled_Clr.g(), g_Options.GUI_CLRS.SubtabFilled_Clr.b(), m_Alpha);

			g_Render->DrawString(this->m_WindowPosition->x + 35 + m_AddX, this->m_WindowPosition->y + m_AddValue + 40 + ((i - 4) * 62), clr,
				RenderImFonts::make_shadow, RenderImFonts::IconFont, this->m_WindowTabs[i].c_str());
		}
	}

	ControlCenter();
}

void Window::ControlCenter()
{
	static int m_Alpha[2]{ };
	static int m_AddValue = 50;

	bool isHovered = Gui::m_Input.MousePointer
	(
		{ this->m_WindowPosition->x + 75, this->m_WindowPosition->y + 315 },
		{ 10, 10 }
	);

	auto DrawArrow = [](int x, int y, bool turn) {
		if (turn)
		{
			for (auto i = 10; i >= 4; --i) {
				auto offset = 10 - i;
				g_Render->DrawLine(x + offset, y + offset, x + offset + std::clamp(i - offset, 0, 10), y + offset, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
			}
		}
		else
		{
			g_Render->DrawLine(x + 4, y, x + 6, y, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
			g_Render->DrawLine(x + 3, y + 1, x + 7, y + 1, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
			g_Render->DrawLine(x + 2, y + 2, x + 8, y + 2, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
			g_Render->DrawLine(x + 1, y + 3, x + 9, y + 3, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
			g_Render->DrawLine(x, y + 4, x + 10, y + 4, g_Options.GUI_CLRS.ColorPickerFrame_3_Clr);
		}
	};

	if (isHovered && Gui::m_Input.KeyPressed(VK_LBUTTON))
		m_TabSwitched = !m_TabSwitched;

	m_SetSwitch = m_TabSwitched;
	DrawArrow(this->m_WindowPosition->x + 75, this->m_WindowPosition->y + 315, m_TabSwitched);

	if (isHovered) {
		m_Alpha[0] += 5;
		m_Alpha[1] += 15;
	}

	else {
		m_Alpha[0] -= 5;
		m_Alpha[1] -= 15;
	}

	if (m_Alpha[0] > 100)
		m_Alpha[0] = 100;

	if (m_Alpha[0] < 0)
		m_Alpha[0] = 0;

	if (m_Alpha[1] > 200)
		m_Alpha[1] = 200;

	if (m_Alpha[1] < 0)
		m_Alpha[1] = 0;

	std::clamp<int>(m_Alpha[0], 0, 100);
	std::clamp<int>(m_Alpha[1], 0, 200);

	std::string m_Buf = m_TabSwitched ? _("next") : _("previous");
	auto m_Size = g_Render->GetSize(RenderImFonts::DefaultFont, m_Buf.c_str());

	g_Render->FilledRect(Gui::m_Input.GetMousePos().x - m_Size.x / 2, Gui::m_Input.GetMousePos().y - 25, m_Size.x + 5, 15, Color(100, 100, 100, m_Alpha[0]));
	g_Render->Rect(Gui::m_Input.GetMousePos().x - m_Size.x / 2, Gui::m_Input.GetMousePos().y - 25, m_Size.x + 5, 15, Color(17, 17, 17, m_Alpha[0]));
	
	g_Render->DrawString(Gui::m_Input.GetMousePos().x + 2, Gui::m_Input.GetMousePos().y - 24, 
		Color(200, 200, 200, m_Alpha[1]), RenderImFonts::make_shadow | RenderImFonts::centered_x, RenderImFonts::DefaultFont, m_Buf.c_str());
}

void Window::AddGroup( Groupbox* Groupbox )
{
	Groupbox->SetVisible( Groupbox->m_Tab == *this->m_Selected );

	if ( Groupbox->GetVisible( ) )
	{
		Groupbox->SetPos( *this->m_WindowPosition );
		Groupbox->Run( );
	}
}