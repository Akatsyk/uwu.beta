#include "groupbox.h"
#include "checkbox.h"
#include "slider.h"
#include "combo.h"
#include "multi.h"
#include "color_selector.h"
#include "Bind.h"
#include "Textbox.h"
#include "button.h"
#include "listbox.h"
#include "text.h"
#include "subtab.h"
#include "listbox_alternate.h"
#include "lua_list.h"

float m_AddX = 0;
float m_AddY = 0;

static int m_Slide[ Gui::m_Maximum ];
static float m_SlideOffset[ Gui::m_Maximum ];

Groupbox::Groupbox( std::string m_Name, int m_GroupNum, float  X, float  Y, float  m_Width, float  m_Height, int m_CurrentTab, uint32_t m_Flags, bool m_AllowLimit )
{
	this->m_Name = m_Name;
	this->m_GroupNum = m_GroupNum;
	this->m_Pos = { X, Y };
	this->m_Size = { m_Width, m_Height };
	this->m_Tab = m_CurrentTab;
	this->m_Flags = m_Flags;
	this->m_AllowLimit = m_AllowLimit;
}

bool Groupbox::GetVisible( )
{
	return m_Visible;
}

void Groupbox::SetVisible( bool m_Vis )
{
	m_Visible = m_Vis;
}

Vector2D Groupbox::GetPos( )
{
	return m_Pos;
}

void Groupbox::SetPos( Vector2D m_Position )
{
	m_Pos += m_Position;
}

void Groupbox::Run( )
{
	m_Elements.m_OffsetX = this->m_Pos.x + 7;
	m_Elements.m_OffsetY[ this->m_GroupNum ] = this->m_Pos.y + 27 + m_Slide[ this->m_GroupNum ];;
	m_SlideOffset[ m_GroupNum ] = 0;

	Draw( );
}

void Groupbox::Draw( )
{
	if ( !( m_Flags & Flags::GROUP_NO_DRAW ) )
	{
		g_Render->FilledRect(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, g_Options.GUI_CLRS.GroupBoxOuter_Clr, 2);
		g_Render->FilledRect( m_Pos.x + 1, m_Pos.y + 1, m_Size.x - 2, m_Size.y - 2, g_Options.GUI_CLRS.GroupBoxOuter_2_Clr, 2 );

		g_Render->FilledRect(m_Pos.x + 1, m_Pos.y + 20, m_Size.x - 2, 1, Gui::m_Details.m_DefaultColor );
		g_Render->FilledRectGradient(m_Pos.x + 1, m_Pos.y + 21, m_Size.x - 2, 10, g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_1_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr, g_Options.GUI_CLRS.ElementsGradient_2_Clr);
		g_Render->DrawString(m_Pos.x + 13, m_Pos.y + 4, g_Options.GUI_CLRS.ElementsText_Clr, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, this->m_Name.c_str());
	}
}

void Groupbox::SetLimit()
{
	auto m_ToLockLimits = Vector2D( m_Pos.x + m_Size.x, m_Pos.y + m_Size.y );
	g_Render->GetDrawList( )->PushClipRect(
		ImVec2( m_Pos.x, m_Pos.y + 30 ), ImVec2( m_ToLockLimits.x, m_ToLockLimits.y - 5 ), true
	);
}

bool Groupbox::IsHovered( )
{
	if ( m_AllowLimit )
	{
		return Gui::m_Input.MousePointer(
			{ m_Pos.x, m_Pos.y + 30 },
			{ m_Size.x, m_Size.y - 5 }
		);
	}
	else
		return true;
}

void Groupbox::AddElement( Checkbox* m_Check )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Check->m_Pos = { m_Elements.m_OffsetX, m_Elements.m_OffsetY[ m_GroupNum ] };

		if ( IsHovered( ) )
			m_Check->Update( );

		m_Check->Draw( );

		m_Elements.m_OffsetY[ m_GroupNum ] += 20;
		m_SlideOffset[ m_GroupNum ] += 20;

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Check;
}

void Groupbox::AddElement( Slider* m_Slider )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		int Add = 0;

		if (m_Slider->m_Type == SliderType::Sld_Checkbox)
			Add = 3;

		if ( m_Slider->m_Type == SliderType::Sld_Defaut )
			m_AddX = 0;

		else
			m_AddX = 22;

		m_Slider->m_Pos = { m_Elements.m_OffsetX + m_AddX,
			m_Elements.m_OffsetY[ m_GroupNum ] + Add };

		if ( IsHovered( ) )
			m_Slider->Update( );

		m_Slider->Draw();

		if ( m_Slider->m_Type != SliderType::Sld_Checkbox )
		{
			m_Elements.m_OffsetY[ m_GroupNum ] += 20;
			m_SlideOffset[ m_GroupNum ] += 20;
		}

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Slider;
}

void Groupbox::AddElement( Combo* m_Combo )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Combo->m_Pos = { m_Elements.m_OffsetX, m_Elements.m_OffsetY[ m_GroupNum ] };

		if ( !Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::COMBO ][ Gui::m_Control.GetIndex( ) ] ) {
			if ( IsHovered( ) )
				m_Combo->Update( );
		}
		else if ( Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::COMBO ][ Gui::m_Control.GetIndex( ) ] )
			m_Combo->Update( );

		m_Combo->Draw( );

		m_Elements.m_OffsetY[ m_GroupNum ] += 25;
		m_SlideOffset[ m_GroupNum ] += 25;

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Combo;
}

void Groupbox::AddElement( Multi* m_Multi )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Multi->m_Pos = { m_Elements.m_OffsetX, m_Elements.m_OffsetY[ m_GroupNum ] };

		if ( !Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::MULTICOMBO ][ Gui::m_Control.GetIndex( ) ] ) {
			if ( IsHovered( ) )
				m_Multi->Update( );
		}
		else if ( Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::MULTICOMBO ][ Gui::m_Control.GetIndex( ) ] )
			m_Multi->Update( );

		m_Multi->Draw( );

		m_Elements.m_OffsetY[ m_GroupNum ] += 25;
		m_SlideOffset[ m_GroupNum ] += 25;

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Multi;
}

void Groupbox::AddElement( ColorPicker* m_Color, bool m_Swap )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		if ( !m_Swap )
			m_Color->m_Pos = { m_Elements.m_OffsetX + 252, m_Elements.m_OffsetY[ m_GroupNum ] - 15 };

		else
			m_Color->m_Pos = { m_Elements.m_OffsetX + 252, m_Elements.m_OffsetY[m_GroupNum] - 20 };

		if ( !Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::COLORSELECTOR ][ Gui::m_Control.GetIndex( ) ] ) {
			if ( IsHovered( ) )
				m_Color->Update( );
		}
		else
			m_Color->Update( );

		m_Color->Draw( );

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Color;
}

void Groupbox::AddElement( Bind* m_Bind )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Bind->m_Pos = { m_Elements.m_OffsetX - 40, m_Elements.m_OffsetY[ m_GroupNum ] - 25 };

		if ( IsHovered( ) )
			m_Bind->Update( );

		m_Bind->Draw( );

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Bind;
}

void Groupbox::AddElement( TextInput* m_Input )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Input->m_Pos = { m_Elements.m_OffsetX, m_Elements.m_OffsetY[ m_GroupNum ] };

		if ( !Gui::m_Control.m_OpenedState[ Gui::m_Control.ControlType::TEXTBOX ][ Gui::m_Control.GetIndex( ) ] ) {
			if ( IsHovered( ) )
				m_Input->Update( );
		}
		else
			m_Input->Update( );

		m_Input->Draw( );

		m_Elements.m_OffsetY[ m_GroupNum ] += 25;
		m_SlideOffset[ m_GroupNum ] += 25;

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Input;
}

void Groupbox::AddElement( Button* m_Button )
{
	if ( GetVisible( ) )
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Button->m_Pos = { m_Elements.m_OffsetX + m_Button->m_CustomPos.x,
			m_Elements.m_OffsetY[ m_GroupNum ] + m_Button->m_CustomPos.y
		};

		if ( IsHovered( ) )
			m_Button->Update( );

		m_Button->Draw( );

		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Button;
}

void Groupbox::AddElement( ListBox* m_List )
{
	if ( GetVisible( ) )
	{
		m_List->m_Pos = { m_Elements.m_OffsetX + 2, m_Elements.m_OffsetY[ m_GroupNum ] - 3 };

		m_List->Update( );
		m_List->Draw( );

		m_Elements.m_OffsetY[m_GroupNum] += 110;
		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );
	}

	delete m_List;
}

void Groupbox::AddElement( LuaList* m_List )
{
	if ( GetVisible( ) )
	{
		m_List->m_Pos = { m_Elements.m_OffsetX + 2, m_Elements.m_OffsetY[ m_GroupNum ] - 3 };

		m_List->Update( );
		m_List->Draw( );

		m_Elements.m_OffsetY[ m_GroupNum ] += 110;
		Gui::m_Control.SetIndex( Gui::m_Control.GetIndex( ) + 1 );
	}

	delete m_List;
}

void Groupbox::AddElement(ListBoxAlternative* m_List)
{
	if (GetVisible())
	{
		m_List->m_Pos = { m_Elements.m_OffsetX + 2, m_Elements.m_OffsetY[m_GroupNum] - 3 };

		m_List->Update();
		m_List->Draw();

		m_Elements.m_OffsetY[m_GroupNum] += 110;
		Gui::m_Control.SetIndex(Gui::m_Control.GetIndex() + 1);
	}

	delete m_List;
}

void Groupbox::AddElement(Text* m_Text)
{
	if (GetVisible())
	{
		if ( m_AllowLimit )
			SetLimit( );

		m_Text->m_Pos = { m_Elements.m_OffsetX, m_Elements.m_OffsetY[m_GroupNum] };
		m_Text->Draw();

		m_Elements.m_OffsetY[m_GroupNum] += 25;
		m_SlideOffset[ m_GroupNum ] += 25;

		if ( m_AllowLimit )
			g_Render->GetDrawList( )->PopClipRect( );
	}

	delete m_Text;
}

void Groupbox::AddElement(SubTab* m_SubTab)
{
	if (GetVisible())
	{
		m_SubTab->m_Pos = { m_Elements.m_OffsetX - 5, m_Elements.m_OffsetY[m_GroupNum] };

		m_SubTab->Update();
		m_SubTab->Draw();

		m_Elements.m_OffsetY[m_GroupNum] += 25;
		Gui::m_Control.SetIndex(Gui::m_Control.GetIndex() + 1);
	}

	delete m_SubTab;
}

void Groupbox::AddEmpty( int m_EmptyPlace, int m_Slide )
{
	m_Elements.m_OffsetY[ m_GroupNum ] += m_EmptyPlace;
	m_SlideOffset[ m_GroupNum ] += m_Slide;
}

void Groupbox::InitScroll()
{
	if ( !GetVisible( ) )
		return;

	if ( !(m_SlideOffset[ m_GroupNum ] > m_Size.y - 24) ) {
		m_Slide[ m_GroupNum ] = 0;
		return;
	}

	bool isColorPicker = Gui::m_Control.ColorPickerOpened;
	bool isHovered = Gui::m_Input.MousePointer
	(
		{ m_Pos.x, m_Pos.y },
		{ m_Size.x, m_Size.y }
	);

	auto ScrollProcess = [ this ]( )
	{
		if ( m_Slide[ m_GroupNum ] > 0 )
			m_Slide[ m_GroupNum ] = 0;

		if ( m_Slide[ m_GroupNum ] < (m_Size.y - 24) - m_SlideOffset[ m_GroupNum ] )
			m_Slide[ m_GroupNum ] = (m_Size.y - 24) - m_SlideOffset[ m_GroupNum ];
	};

	if ( !isColorPicker && Gui::m_Control.IsPossible( ) && isHovered && Gui::m_Input.GetMouseWheel( ) != 0 )
	{
		m_Slide[ m_GroupNum ] += Gui::m_Input.GetMouseWheel( ) * 8;
		Gui::m_Input.SetMouseWheel( 0 );
		ScrollProcess( );
	}

	int m_Max = m_SlideOffset[ m_GroupNum ] - m_Size.y + 50.f;
	float m_ScrollPos = (( float )m_Slide[ m_GroupNum ] / ( float )m_SlideOffset[ m_GroupNum ]) * (m_Size.y - 20.f) * ( -1 );
	float m_ScrollPosMax = m_Max / ( float )m_SlideOffset[ m_GroupNum ] * (m_Size.y - 20.f);

	static int m_Old[ Gui::m_Maximum ];
	if ( m_Old[ m_GroupNum ] != m_Max ) {
		ScrollProcess( );
		m_Old[ m_GroupNum ] = m_Max;
	}

	g_Render->FilledRect( m_Pos.x + m_Size.x - 4, m_Pos.y + m_ScrollPos + 21, 3, m_Size.y - m_ScrollPosMax, Gui::m_Details.m_DefaultColor );
}