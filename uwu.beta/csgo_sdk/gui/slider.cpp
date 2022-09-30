#include "slider.h"

float m_SliderSize = 140;

Slider::Slider( std::string m_Name, float* m_Value, int m_Type, int m_Min, int m_Max, std::string m_Suffix )
{
	this->m_Name = m_Name;
	this->m_Value = m_Value;
	this->m_Min = m_Min;
	this->m_Max = m_Max;
	this->m_Suffix = m_Suffix;
	this->m_Type = m_Type;
}

void Slider::Update( )
{
	static auto StoredIndex = -1;
	auto Delta = m_Max - m_Min;

	if ( Gui::m_Control.IsPossible( ) && !Gui::m_Control.ColorPickerOpened ) {
		if ( Gui::m_Input.KeyPressed( VK_LBUTTON ) & 1 && Gui::m_Input.MousePointer( Vector2D( m_Pos.x, m_Pos.y + 3 ), Vector2D(m_SliderSize, 9 ) ) )
			StoredIndex = Gui::m_Control.GetIndex( );
	}

	if ( GetAsyncKeyState( VK_LBUTTON ) && StoredIndex == Gui::m_Control.GetIndex( ) )
	{
		*m_Value = m_Min + Delta * ( Gui::m_Input.GetMousePos( ).x - ( m_Pos.x ) ) / m_SliderSize;

		if ( *m_Value < m_Min )
			*m_Value = m_Min;

		if ( *m_Value > m_Max )
			*m_Value = m_Max;
	}

	if ( !GetAsyncKeyState( VK_LBUTTON ) )
		StoredIndex = -1;
}

void Slider::Draw( )
{
	auto m_MinDelta = *m_Value - m_Min;
	auto Delta = m_Max - m_Min;
	auto m_Total = ( m_MinDelta / Delta ) * m_SliderSize;

	g_Render->FilledRect(m_Pos.x + 5, m_Pos.y + 3, m_SliderSize, 9, g_Options.GUI_CLRS.SelectorOuter_Clr, 2 );

	if ( *m_Value > 0.3 || ( m_Min < 0 ) && *m_Value < 0.3 )
		g_Render->FilledRect(m_Pos.x + 5, m_Pos.y + 3, m_Total, 9, Gui::m_Details.m_DefaultColor, 2 );

	g_Render->Rect(m_Pos.x + 5, m_Pos.y + 3, m_SliderSize, 9, g_Options.GUI_CLRS.SelectorHovered_Clr, 2 );

	if (Gui::m_Input.MousePointer(Vector2D(m_Pos.x, m_Pos.y + 3), Vector2D(m_SliderSize, 9)))
		g_Render->Rect(m_Pos.x + 6, m_Pos.y + 4, m_SliderSize - 2, 7, g_Options.GUI_CLRS.SelectorSelectedText_Clr, 2 );

	float m_Ratio = (*m_Value - m_Min) / (m_Max - m_Min);
	float m_Pixel = m_Ratio * m_SliderSize;

	std::string m_ToDisp = std::to_string((int)*m_Value) + m_Suffix;
	g_Render->DrawString(m_Pixel + 3 + m_Pos.x, this->m_Pos.y + 6, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::outline | RenderImFonts::centered_x, RenderImFonts::DefaultFont, m_ToDisp.c_str());
	g_Render->DrawString(m_Pos.x + m_SliderSize + 15, m_Pos.y + 1, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name.c_str());
}