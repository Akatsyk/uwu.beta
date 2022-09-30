#include "bind.h"

static short int StoredIndex = -1;
static bool m_State = false;
static int m_Key = -1;

std::string m_Name[ Gui::m_Maximum ];
static bool isGood = false;

char m_Buffer[ 128 ];
extern const char* m_Keys[ 254 ];

Bind::Bind( int* m_Value )
{
	this->m_Value = m_Value;
}

void Bind::Update( )
{
	if ( !Gui::m_Control.ColorPickerOpened && Gui::m_Control.IsPossible( ) && GetAsyncKeyState( VK_LBUTTON ) && Gui::m_Input.MousePointer( { m_Pos.x + 280, m_Pos.y + 2 }, { 35, 15 } ) )
	{
		if ( !m_State )
			m_State = true;

		StoredIndex = Gui::m_Control.GetIndex( );
	}

	if ( StoredIndex == Gui::m_Control.GetIndex( ) )
		m_Focused = m_State;

	if ( m_Focused )
	{
		for ( int i = 0; i < 255; i++ )
		{
			if ( Gui::m_Input.KeyPressed( i ) )
			{
				if ( i == VK_ESCAPE )
				{
					*m_Value = -1;
					StoredIndex = -1;
					return;
				}

				*m_Value = i;
				StoredIndex = -1;
				return;
			}
		}
	}

	std::string Container = "[";
	std::string NextContainer = "]";

	if ( m_Focused ) {
		m_Name[ Gui::m_Control.GetIndex( ) ] = "[-]";
	}
	else
	{
		if ( *m_Value >= 0 ) {
			m_Name[ Gui::m_Control.GetIndex( ) ] = Container + m_Keys[ *m_Value ] + NextContainer;

			if ( m_Name[ Gui::m_Control.GetIndex( ) ].c_str( ) ) {
				isGood = true;
			}
			else
			{
				/*if ( GetKeyNameText( *m_Value << 16, m_Buffer, 127 ) )
				{
					m_Name[ Gui::m_Control.GetIndex( ) ] = m_Buffer;
					isGood = true;
				}*/
			}
		}

		if ( !isGood ) {
			m_Name[ Gui::m_Control.GetIndex( ) ] = "[-]";
		}
	}
}

void Bind::Draw( )
{
	if ( m_Name[ Gui::m_Control.GetIndex( ) ] == "" )
		m_Name[ Gui::m_Control.GetIndex( ) ] = "[-]";

	auto TextSize = g_Render->GetSize( RenderImFonts::DefaultFont, m_Name[ Gui::m_Control.GetIndex( ) ].c_str() );
	g_Render->DrawString(m_Pos.x + 310 - TextSize.x, m_Pos.y + 8, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Name[Gui::m_Control.GetIndex()].c_str());
}

const char* m_Keys[254] = {
	"invld", "m1", "m2", "brk", "m3", "m4", "m5",
	"invld", "bspc", "tab", "invld", "invld", "invld", "enter", "invld", "invld", "shi",
	"ctrl", "alt", "pau", "caps", "invld", "invld", "invld", "invld", "invld", "invld",
	"esc", "invld", "invld", "invld", "invld", "space", "pgup", "pgdown", "end", "home", "left",
	"up", "right", "down", "invld", "prnt", "invld", "prtscr", "ins", "del", "invld", "0", "1",
	"2", "3", "4", "5", "6", "7", "8", "9", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u",
	"v", "w", "x", "y", "z", "lftwin", "rghtwin", "invld", "invld", "invld", "num0", "num1",
	"num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9", "*", "+", "_", "-", ".", "/", "f1", "f2", "f3",
	"f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20",
	"f21",
	"f22", "f23", "f24", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"num lock", "scroll lock", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "lshft", "rshft", "lctrl",
	"rctrl", "lmenu", "rmenu", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "ntrk", "ptrk", "stop", "play", "invld", "invld",
	"invld", "invld", "invld", "invld", ";", "+", ",", "-", ".", "/?", "~", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "{", "\\|", "}", "'\"", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld", "invld",
	"invld", "invld"
};