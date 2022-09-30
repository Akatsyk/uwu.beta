#include "button.h"

static short int StoredIndex = -1;
static bool m_State = false;

Button::Button( std::string m_Name, std::function< void( ) > Function, Vector2D m_CustomPos, Vector2D m_CustomSize )
{
	this->m_Name = m_Name;
	this->Function = Function;
	this->m_CustomPos = m_CustomPos;
	this->m_CustomSize = m_CustomSize;
}

void Button::Update( )
{
	if ( !Gui::m_Input.KeyPressed( VK_LBUTTON ) && Gui::m_Input.MousePointer( { m_Pos.x, m_Pos.y }, { m_CustomSize.x, m_CustomSize.y } ) )
	{
		if ( m_State )
			Function( );

		m_State = false;
	}

	if ( !Gui::m_Control.ColorPickerOpened && Gui::m_Control.IsPossible( ) && Gui::m_Input.KeyPressed( VK_LBUTTON ) && Gui::m_Input.MousePointer( { m_Pos.x, m_Pos.y }, { m_CustomSize.x, m_CustomSize.y } ) )
	{
		m_State = true;
		StoredIndex = Gui::m_Control.GetIndex( );
	}

	if ( StoredIndex == Gui::m_Control.GetIndex( ) )
		m_Focused = m_State;
}

void Button::Draw( )
{
	g_Render->FilledRect(m_Pos.x, m_Pos.y, m_CustomSize.x, m_CustomSize.y, g_Options.GUI_CLRS.SelectorOuter_Clr, 2 );
	g_Render->Rect(m_Pos.x, m_Pos.y, m_CustomSize.x, m_CustomSize.y, g_Options.GUI_CLRS.SelectorHovered_Clr, 2 );

	if ( !Gui::m_Control.ColorPickerOpened && Gui::m_Control.IsPossible() &&
		Gui::m_Input.MousePointer(Vector2D(m_Pos.x, m_Pos.y), Vector2D(m_CustomSize.x, m_CustomSize.y)))
		g_Render->Rect(m_Pos.x + 1, m_Pos.y + 1, m_CustomSize.x - 2, m_CustomSize.y - 2, g_Options.GUI_CLRS.SelectorSelectedText_Clr, 2 );

	g_Render->DrawString(m_Pos.x + (m_CustomSize.x / 2), (m_Pos.y + (m_CustomSize.y / 2)) - 1, g_Options.GUI_CLRS.ElementsText_Clr, 
		RenderImFonts::make_shadow | RenderImFonts::centered_x | RenderImFonts::centered_y, RenderImFonts::DefaultFont, m_Name.c_str());
}