#pragma once
#include "window.h"
#include "../config/config.h"

class Checkbox;
class Slider;
class Combo;
class Multi;
class ColorPicker;
class Bind;
class TextInput;
class Button;
class ListBox;
class SubTab;
class Text;
class ListBoxAlternative;
class LuaList;

class Groupbox : public Base {
public:

	// Some groupbox flags.
	enum Flags
	{
		GROUP_NO_DRAW = 1 << 0,
		GROUP_NO_DRAW_TEXT = 1 << 1
	};

	// Constructor.
	Groupbox( std::string m_Name, int m_GroupNum, float X, float  Y, float  m_Width, float  m_Height, int m_CurrentTab = 0, uint32_t m_Flags = 0,
		bool m_AllowLimit = false );

	void Run( );
	void Draw( );

	// Functions.
	void SetVisible( bool m_Vis );
	bool GetVisible( );

	void SetLimit();
	void InitScroll();

	void SetPos( Vector2D m_Pos );
	Vector2D GetPos( );

	bool m_Visible;

	std::string m_Name;
	uint32_t m_Flags;
	bool m_AllowLimit;

	Vector2D m_Size;
	Vector2D m_Pos;

	int m_Tab;
	int m_GroupNum;

	bool IsHovered( );

	struct
	{

		float m_OffsetX;
		float m_OffsetY[ Gui::m_Maximum ];

	} m_Elements;

public:

	void AddElement( Checkbox* m_Check );
	void AddElement( Slider* m_Slider );
	void AddElement( Combo* m_Combo );
	void AddElement( Multi* m_Multi );
	void AddElement( ColorPicker* m_Color, bool m_Swap = false );
	void AddElement( Bind* m_Bind );
	void AddElement( TextInput* m_Input );
	void AddElement( Button* m_Button );
	void AddElement( ListBox* m_List );
	void AddElement( Text* m_Text );
	void AddElement( SubTab* m_Subtab );
	void AddElement( ListBoxAlternative* m_List );
	void AddElement( LuaList* m_List );

	void AddEmpty( int m_EmptyPlace, int m_Slide );
};