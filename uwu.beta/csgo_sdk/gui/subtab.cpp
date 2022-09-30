#include "subtab.h"

SubTab::SubTab() { };

SubTab::SubTab(Vector2D m_Size, int* m_Value)
{
	this->m_Size = m_Size;
	this->m_Value = m_Value;
}

void SubTab::Update()
{
	if (m_Items.size() > 0)
	{
		float tab_size = this->m_Size.x / this->m_Items.size();

		for (int i = 0; i < this->m_Items.size(); i++)
		{
			if (Gui::m_Input.MousePointer(Vector2D(this->m_Pos.x + (tab_size * i), this->m_Pos.y + 2), Vector2D(tab_size, 20)) 
				&& Gui::m_Input.KeyPressed(VK_LBUTTON) && Gui::m_Control.IsPossible() && !Gui::m_Control.ColorPickerOpened )
				*this->m_Value = i;
		}
	}
}

void SubTab::Draw()
{
	g_Render->FilledRectGradient(this->m_Pos.x - 1, this->m_Pos.y + 19, this->m_Size.x + 2, 10, g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr);
	g_Render->Rect(this->m_Pos.x - 1, this->m_Pos.y + 18, this->m_Size.x + 1, 1, Gui::m_Details.m_DefaultColor);

	if (m_Items.size() > 0)
	{
		float tab_size = this->m_Size.x / this->m_Items.size();

		for (int i = 0; i < this->m_Items.size(); i++)
		{
			Color color = i == *this->m_Value ? Gui::m_Details.m_DefaultColor : g_Options.GUI_CLRS.ElementsText_Clr;

			int text_x = this->m_Pos.x + (tab_size * i) + (tab_size / 2);
			g_Render->DrawString((float)text_x, m_Pos.y + 1, color, RenderImFonts::centered_x | RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Items[i].m_Name.c_str());
		}
	}
}

void SubTab::AddTab(std::string name)
{
	this->m_Items.push_back(Tab_T(name));
}