#include "listbox_alternate.h"
#include <cctype>

static int m_ScrollPos[ Gui::m_Maximum ];
static bool m_ScrollState[ Gui::m_Maximum ];

ListBoxAlternative::ListBoxAlternative( std::string m_Name, int* m_Selec, int* m_SelectPlus, std::vector<std::string>& m_Items, int m_Num, int m_Addition )
{
	this->m_Name = m_Name;
	this->m_Select = m_Select;
	this->m_Items = &m_Items;
	this->m_Num = m_Num;
	this->m_Addition = m_Addition;
	this->m_SelectPlus = m_SelectPlus;
}

ListBoxAlternative::ListBoxAlternative( std::string m_Name, int* m_Select, int* m_SelectPlus, std::vector<std::string>& m_Items, std::string& m_Filter, int m_Num, int m_Addition )
{
	this->m_Name = m_Name;
	this->m_Select = m_Select;
	this->m_Items = &m_Items;
	this->m_Filter = &m_Filter;
	this->m_Num = m_Num;
	this->m_Addition = m_Addition;
	this->m_SelectPlus = m_SelectPlus;
}

bool ListBoxAlternative::IsHold( )
{
	if ( !GetAsyncKeyState( VK_LBUTTON ) )
		return false;

	return true;
}

void ListBoxAlternative::Update( )
{
	GetItems( );
	Scroll( 0 );
}

void ListBoxAlternative::Scroll( int m_Limit )
{
	static size_t m_Lost[ Gui::m_Maximum ];
	if ( m_Lost[ m_Num ] != m_Temp.size( ) ) {
		m_ScrollPos[ m_Num ] = 0;
		m_Lost[ m_Num ] = m_Temp.size( );
	}

	int ItemsToDraw = 16;
	bool isHovered = Gui::m_Input.MousePointer
	(
		{ m_Pos.x + 5, m_Pos.y + 5 },
		{ 250, 362 }
	);

	if ( !m_ScrollState[ m_Num ] && isHovered && m_Temp.size( ) > 16 )
	{
		float ratio = ItemsToDraw / float( m_Temp.size( ) );
		float some_shit = m_Pos.y;

		float size_ratio = ItemsToDraw / float( m_Temp.size( ) );
		size_ratio *= 363;

		float height_delta = some_shit + size_ratio - (363);
		if ( height_delta > 0 )
			some_shit -= height_delta;

		float pos_ratio = float( some_shit ) / float( 363 );
		float m_prikol = pos_ratio * m_Temp.size( );

		if ( Gui::m_Input.GetMouseWheel( ) != 0 && isHovered )
		{
			m_ScrollPos[ m_Num ] += Gui::m_Input.GetMouseWheel( ) * (-1);
			Gui::m_Input.SetMouseWheel( 0 );

			if ( m_ScrollPos[ m_Num ] < 0 )
				m_ScrollPos[ m_Num ] = 0;

			if ( m_ScrollPos[ m_Num ] > ( int )m_prikol )
				m_ScrollPos[ m_Num ] = ( int )m_prikol;
		}
	}
}

void ListBoxAlternative::Draw( )
{
	int DrawnItems = 0;
	int ItemsToDraw = 16;

	if ( !m_Temp.empty( ) )
	{
		for ( int i{ m_ScrollPos[ m_Num ] }; (i < m_Temp.size( ) && DrawnItems < ItemsToDraw); i++ )
		{
			bool isHovered = Gui::m_Input.MousePointer
			(
				{ m_Pos.x + 6, (m_Pos.y + 10 + DrawnItems * 23) },
				{ 200, 15 }
			);

			if ( isHovered && Gui::m_Input.KeyPressed( VK_LBUTTON ) && Gui::m_Control.IsPossible( ) && !Gui::m_Control.ColorPickerOpened )
			{
				*m_Select = m_Temp[ i ].second;
				*m_SelectPlus = m_Temp[ i ].second;
			}

			Color m_Color = isHovered || *m_Select == m_Temp[ i ].second ? Gui::m_Details.m_DefaultColor : g_Options.GUI_CLRS.ElementsText_Clr;
			g_Render->DrawString( m_Pos.x + 10, (m_Pos.y + 10 + DrawnItems * 23), m_Color, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, m_Temp[ i ].first.c_str( ) );

			DrawnItems++;
		}
	}

	g_Render->Rect( m_Pos.x + 3, m_Pos.y + 5, 265, 365, g_Options.GUI_CLRS.ListBoxLine_Clr, 2 );
	g_Render->Rect( m_Pos.x + 4, m_Pos.y + 6, 263, 363, g_Options.GUI_CLRS.ListBoxLine_2_Clr, 2 );
}

std::string ListBoxAlternative::ToLower( std::string String )
{
	std::transform( String.begin( ), String.end( ), String.begin( ), (int(*)(int))::tolower );
	return String;
}

void ListBoxAlternative::GetItems( )
{
	if ( m_LowerItems.empty( ) || m_LowerItems.size( ) != m_Items->size( ) )
	{
		m_LowerItems.clear( );
		for ( int i = 0; i < m_Items->size( ); i++ )
		{
			std::string temp_s = (*m_Items)[ i ];
			m_LowerItems.push_back( temp_s );
		}
	}

	if ( !m_Filter && m_Temp.empty( ) || m_Temp.size( ) != m_Items->size( ) && (!m_Filter || m_Filter->empty( )) )
	{
		m_Temp.clear( );
		for ( int i = 0; i < m_Items->size( ); i++ ) {
			m_Temp.emplace_back( (*m_Items)[ i ], i );
		}
	}

	if ( !m_Filter )
		return;

	if ( m_PreviousFilter == *m_Filter )
		return;

	m_PreviousFilter = *m_Filter;
	const auto l_filter = ToLower( *m_Filter );

	m_Temp.clear( );

	for ( int i = 0; i < m_Items->size( ); i++ )
	{
		if ( m_Filter != nullptr )
		{
			std::string temp_s = m_LowerItems[ i ];
			if ( !(temp_s.find( l_filter ) != std::string::npos) )
				continue;
		}

		m_Temp.emplace_back( m_Items->at( i ), i );
	}
}