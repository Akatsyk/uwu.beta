#include "textbox.h"

extern const char* LowerCase[ 254 ];
extern const char* UpperCase[ 254 ];

static short int StoredIndex = -1;
static bool m_State = false;

float m_Blink;

TextInput::TextInput( std::string m_Name, std::string* m_Label )
{
	this->m_Name = m_Name;
	this->m_Label = m_Label;
}

void TextInput::Update( )
{
	if ( m_State && StoredIndex == Gui::m_Control.GetIndex( ) && Gui::m_Input.KeyPressed( VK_LBUTTON ) )
	{
		m_State = !m_State;
		StoredIndex = -1;
	}

	if ( !Gui::m_Control.ColorPickerOpened && Gui::m_Control.IsPossible( ) && Gui::m_Input.KeyPressed( VK_LBUTTON )
		&& Gui::m_Input.MousePointer( { m_Pos.x + 5, m_Pos.y + 3 }, { 140, 15 } ) )
	{
		m_State = !m_State;
		StoredIndex = Gui::m_Control.GetIndex( );
	}

	if ( StoredIndex == Gui::m_Control.GetIndex( ) )
		m_Focused = m_State;

	Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::TEXTBOX ][ Gui::m_Control.GetIndex( ) ] = m_Focused;

	if ( m_Focused )
	{
		std::string m_Str = *m_Label;

		for ( int i = 0; i < 255; i++ ) {
			if ( Gui::m_Input.KeyPressed( i ) ) {
				if ( i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT ) {
					StoredIndex = -1;
					return;
				}

				if ( strlen( m_Str.c_str( ) ) != 0 ) {
					if ( GetAsyncKeyState( VK_BACK ) ) {
						*m_Label = m_Str.substr( 0, strlen( m_Str.c_str( ) ) - 1 );
					}
				}

				if ( strlen( m_Str.c_str( ) ) < 27 && i != NULL && UpperCase[ i ] != nullptr ) {
					if ( GetAsyncKeyState( VK_SHIFT ) ) {
						*m_Label = m_Str + UpperCase[ i ];
					}
					else {
						*m_Label = m_Str + LowerCase[ i ];
					}

					return;
				}

				if ( strlen( m_Str.c_str( ) ) < 27 && i == 32 ) {
					*m_Label = m_Str + " ";
					return;
				}
			}
		}
	}
}

void TextInput::Draw( )
{
	std::string ToDraw = *m_Label;

	if ( GetTickCount64( ) >= m_Blink )
		m_Blink = GetTickCount64( ) + 800;

	if ( m_Focused && GetTickCount64( ) > ( m_Blink - 400 ) )
		ToDraw += _("_");

	g_Render->DrawString(m_Pos.x + 140 + 15, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name.c_str());

	g_Render->FilledRect(m_Pos.x + 5, m_Pos.y + 3, 140, 15, g_Options.GUI_CLRS.SelectorOuter_Clr, 2 );
	g_Render->Rect(m_Pos.x + 5, m_Pos.y + 3, 140, 15, g_Options.GUI_CLRS.SelectorHovered_Clr, 2 );

	if (Gui::m_Input.MousePointer(Vector2D(m_Pos.x + 5, m_Pos.y + 3), Vector2D(160, 15)))
		g_Render->Rect( m_Pos.x + 6, m_Pos.y + 4, 140 - 2, 13, g_Options.GUI_CLRS.SelectorSelectedText_Clr, 2 );

	g_Render->DrawString(m_Pos.x + 7, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, ToDraw.c_str());
}

const char* LowerCase[ 254 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
"y", "z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[", "\\", "]", "'", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

const char* UpperCase[ 254 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "_", ".", "?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "{", "|", "}", "\"", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };