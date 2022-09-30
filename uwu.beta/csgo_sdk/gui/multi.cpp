#include "multi.h"

static auto StoredIndex = -1;
static bool m_State = false;
static int m_Time;

Multi::Multi( std::string m_Name, float m_MultiComboSize )
{
	this->m_Name = m_Name;
	this->m_Items = m_Items;
	this->m_MultiComboSize = m_MultiComboSize;
}

void Multi::Update( )
{
	if ( m_State && StoredIndex == Gui::m_Control.GetIndex( ) && Gui::m_Input.KeyPressed( VK_LBUTTON )
		&& !Gui::m_Input.MousePointer( { m_Pos.x + 5, m_Pos.y + 22 }, { m_MultiComboSize, ( float )( m_Items.size( ) * 15 ) } ) )
	{
		m_State = !m_State;
		StoredIndex = -1;
	}

	if ( !Gui::m_Control.ColorPickerOpened && Gui::m_Control.m_Opened[ Gui::m_Control.ControlType::COMBO ] == -1 && m_Time == -1 && Gui::m_Input.KeyPressed( VK_LBUTTON )
		&& Gui::m_Input.MousePointer( { m_Pos.x + 5, m_Pos.y + 3 }, { m_MultiComboSize, 15 } ) )
	{
		m_State = !m_State;
		StoredIndex = Gui::m_Control.GetIndex( );
	}

	if ( m_Time >= 12 )
		m_Time = 12;

	else if ( m_Time <= 0 )
		m_Time = 0;

	if ( m_State )
		m_Time++;
	else
		m_Time--;

	Gui::m_Control.m_Opened[ Gui::m_Control.ControlType::MULTICOMBO ] = m_Time;

	if ( StoredIndex == Gui::m_Control.GetIndex( ) )
		m_Focused = m_State;

	Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::MULTICOMBO ][ Gui::m_Control.GetIndex( ) ] = m_Focused;
}

void Multi::Draw( )
{
	g_Render->DrawString(m_Pos.x + m_MultiComboSize + 15, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name.c_str());
	
	g_Render->FilledRect(m_Pos.x + 5, m_Pos.y + 3, m_MultiComboSize, 15, g_Options.GUI_CLRS.SelectorOuter_Clr, 2 );
	g_Render->Rect(m_Pos.x + 5, m_Pos.y + 3, m_MultiComboSize, 15, g_Options.GUI_CLRS.SelectorHovered_Clr, 2 );

	if (Gui::m_Input.MousePointer(Vector2D(m_Pos.x + 5, m_Pos.y + 3), Vector2D(m_MultiComboSize, 15)))
		g_Render->Rect(m_Pos.x + 6, m_Pos.y + 4, m_MultiComboSize - 2, 13, g_Options.GUI_CLRS.SelectorSelectedText_Clr, 2 );

	if (m_Focused)
	{
		for (int i = 0; i < m_Items.size(); i++)
		{
			if (Gui::m_Input.MousePointer(Vector2D(m_Pos.x + 5, m_Pos.y + 22 + (i * 15)), Vector2D(m_MultiComboSize, 15)))
				Gui::m_External.RectDraw.insert(Gui::m_External.RectDraw.begin(), Gui::External::EDrawRect(m_Pos.x + 6, m_Pos.y + 23 + (i * 15), m_MultiComboSize - 2, 17, Color(25, 25, 25, 130)));
		}

		Gui::m_External.RectDraw.insert(Gui::m_External.RectDraw.begin(), Gui::External::EDrawRect(m_Pos.x + 6, m_Pos.y + 22 + 1, m_MultiComboSize - 2, (this->m_Items.size() * 15) + 4 - 2, g_Options.GUI_CLRS.SelectorOuter_Clr));
		Gui::m_External.RectDraw.insert(Gui::m_External.RectDraw.begin(), Gui::External::EDrawRect(m_Pos.x + 5, m_Pos.y + 22, m_MultiComboSize, this->m_Items.size() * 15 + 4, g_Options.GUI_CLRS.SelectorHovered_Clr));

		for (int i = 0; i < m_Items.size(); i++)
		{
			if (Gui::m_Input.KeyPressed(VK_LBUTTON) && Gui::m_Input.MousePointer(Vector2D(m_Pos.x + 5, m_Pos.y + 22 + (i * 15)), Vector2D(m_MultiComboSize, 15)))
				*this->m_Items[i].m_Value = !*this->m_Items[i].m_Value;

			Color m = *this->m_Items[i].m_Value ? Gui::m_Details.m_DefaultColor : g_Options.GUI_CLRS.ElementsText_Clr;
			Gui::m_External.TextDraw.insert(Gui::m_External.TextDraw.begin(), Gui::External::EDrawText(m_Pos.x + 8, m_Pos.y + 25 + (i * 15), m_Items[i].m_Name, m));
		}
	}

	std::string ToDisplay;

	for ( int i = 0; i < m_Items.size( ); i++ )
	{
		if ( *m_Items[ i ].m_Value )
		{
			if ( ToDisplay != "" )
				ToDisplay.append( _(", ") );

			ToDisplay.append( m_Items[ i ].m_Name );
		}
	}

	if ( ToDisplay == "" )
		ToDisplay.append( _("none") );

	auto TextSize = g_Render->GetSize(RenderImFonts::DefaultFont, ToDisplay.c_str() );

	if ( m_Pos.x + 2 + TextSize.x > m_Pos.x + 140 )
	{
		ToDisplay.resize( 10 );
		ToDisplay.append( _("...") );
	}

	g_Render->DrawString(m_Pos.x + 7, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, ToDisplay.c_str());
}

void Multi::AddItem( std::string m_Name, bool* m_Value )
{
	m_Items.push_back( MultiConstructor( m_Name, m_Value ) );
}