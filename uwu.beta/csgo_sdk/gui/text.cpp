#include "text.h"

Text::Text() 
{ 
}

Text::Text(std::string m_Name)
{
	this->m_Name = m_Name;
}

void Text::Draw()
{
	g_Render->DrawString(m_Pos.x + 7, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name.c_str());
}