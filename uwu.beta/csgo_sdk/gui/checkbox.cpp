#include "checkbox.h"

Checkbox::Checkbox( std::string m_Name, bool* m_Value )
{
	this->m_Name = m_Name;
	this->m_Value = m_Value;
}

void Checkbox::Update( )
{
	if ( Gui::m_Control.IsPossible( ) && !Gui::m_Control.ColorPickerOpened ) {
		if ( Gui::m_Input.MousePointer( { m_Pos.x + 6, m_Pos.y + 4 }, { 13, 13 } ) && Gui::m_Input.KeyPressed( VK_LBUTTON ) )
			*this->m_Value = !*this->m_Value;
	}
}

void Checkbox::Draw( )
{
	g_Render->FilledRect(m_Pos.x + 6, m_Pos.y + 4, 13, 13, g_Options.GUI_CLRS.SelectorHovered_Clr, 2 );
	g_Render->FilledRect(m_Pos.x + 7, m_Pos.y + 5, 11, 11, g_Options.GUI_CLRS.SelectorOuter_Clr, 2 );

	if (*this->m_Value)
		g_Render->FilledRect(m_Pos.x + 7, m_Pos.y + 5, 11, 11, Gui::m_Details.m_DefaultColor, 2 );

	if (Gui::m_Input.MousePointer(Vector2D(m_Pos.x + 5, m_Pos.y + 3), Vector2D(13, 13)))
		g_Render->Rect(m_Pos.x + 7, m_Pos.y + 5, 11, 11, g_Options.GUI_CLRS.SelectorSelectedText_Clr, 2 );

	g_Render->DrawString(m_Pos.x + 25, m_Pos.y + 3, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name.c_str());
}