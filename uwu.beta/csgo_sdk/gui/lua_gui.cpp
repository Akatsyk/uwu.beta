#include "gui.h"

#include "window.h"
#include "groupbox.h"
#include "checkbox.h"
#include "slider.h"
#include "combo.h"
#include "multi.h"
#include "color_selector.h"
#include "bind.h"
#include "textbox.h"
#include "button.h"
#include "listbox.h"
#include "listbox_alternate.h"
#include "text.h"
#include "subtab.h"
#include "lua_list.h"

#include "../config/config.h"
#include "../functions/logs.h"
#include "../lua/Clua.h"

void LoadScript( ) {

	if ( Gui::m_Control.m_LuaExecute.m_Scripts.empty( ) )
		return;

	c_lua::Get( ).load_script( Gui::m_Control.m_LuaExecute.m_SelectedScript );
	c_lua::Get( ).refresh_scripts( );

	Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

	if ( Gui::m_Control.m_LuaExecute.m_SelectedScript >= Gui::m_Control.m_LuaExecute.m_Scripts.size( ) )
		Gui::m_Control.m_LuaExecute.m_SelectedScript = Gui::m_Control.m_LuaExecute.m_Scripts.size( ) - 1;

	for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
	{
		if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
			current.erase( current.size( ) - 5, 5 );

		else if ( current.size( ) >= 4 )
			current.erase( current.size( ) - 4, 4 );
	}
}

void UnloadScript( )
{
	if ( Gui::m_Control.m_LuaExecute.m_Scripts.empty( ) )
		return;

	c_lua::Get( ).unload_script( Gui::m_Control.m_LuaExecute.m_SelectedScript );
	c_lua::Get( ).refresh_scripts( );

	Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

	if ( Gui::m_Control.m_LuaExecute.m_SelectedScript >= Gui::m_Control.m_LuaExecute.m_Scripts.size( ) )
		Gui::m_Control.m_LuaExecute.m_SelectedScript = Gui::m_Control.m_LuaExecute.m_Scripts.size( ) - 1;

	for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
	{
		if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
			current.erase( current.size( ) - 5, 5 );
		else if ( current.size( ) >= 4 )
			current.erase( current.size( ) - 4, 4 );
	}
}

void UnloadAllScripts( )
{
	c_lua::Get( ).unload_all_scripts( );
	c_lua::Get( ).refresh_scripts( );

	Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

	if ( Gui::m_Control.m_LuaExecute.m_SelectedScript >= Gui::m_Control.m_LuaExecute.m_Scripts.size( ) )
		Gui::m_Control.m_LuaExecute.m_SelectedScript = Gui::m_Control.m_LuaExecute.m_Scripts.size( ) - 1;

	for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
	{
		if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
			current.erase( current.size( ) - 5, 5 );
		else if ( current.size( ) >= 4 )
			current.erase( current.size( ) - 4, 4 );
	}
}

void ReloadAllScripts( )
{
	c_lua::Get( ).reload_all_scripts( );
	c_lua::Get( ).refresh_scripts( );

	Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

	if ( Gui::m_Control.m_LuaExecute.m_SelectedScript >= Gui::m_Control.m_LuaExecute.m_Scripts.size( ) )
		Gui::m_Control.m_LuaExecute.m_SelectedScript = Gui::m_Control.m_LuaExecute.m_Scripts.size( ) - 1;

	for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
	{
		if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
			current.erase( current.size( ) - 5, 5 );
		else if ( current.size( ) >= 4 )
			current.erase( current.size( ) - 4, 4 );
	}
}

void RefreshScripts( )
{
	c_lua::Get( ).refresh_scripts( );
	Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

	if ( Gui::m_Control.m_LuaExecute.m_SelectedScript >= Gui::m_Control.m_LuaExecute.m_Scripts.size( ) )
		Gui::m_Control.m_LuaExecute.m_SelectedScript = Gui::m_Control.m_LuaExecute.m_Scripts.size( ) - 1;

	for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
	{
		if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
			current.erase( current.size( ) - 5, 5 );
		else if ( current.size( ) >= 4 )
			current.erase( current.size( ) - 4, 4 );
	}
}

void Gui::Tabs::SetupLua( Window* m_Window, int m_Tab )
{
	static auto should_update = true;
	static int useless_pointer;

	if ( should_update )
	{
		Gui::m_Control.m_LuaExecute.m_Scripts = c_lua::Get( ).scripts;

		for ( auto& current : Gui::m_Control.m_LuaExecute.m_Scripts )
		{
			if ( current.size( ) >= 5 && current.at( current.size( ) - 1 ) == 'c' )
				current.erase( current.size( ) - 5, 5 );
			else if ( current.size( ) >= 4 )
				current.erase( current.size( ) - 4, 4 );
		}

		should_update = false;
	}

	auto GroupScripts = new Groupbox( _( "scripts" ), 701, 180, 60, 290, 415, m_Tab );
	{
		m_Window->AddGroup( GroupScripts );

		GroupScripts->AddEmpty( 5, 5 );

		if ( Gui::m_Control.m_LuaExecute.m_Scripts.empty( ) )
			GroupScripts->AddElement( new LuaList( "", &Gui::m_Control.m_LuaExecute.m_SelectedScript, Gui::m_Control.m_LuaExecute.m_Scripts, 15, 0 ) );
		else
		{
			auto backup_scripts = Gui::m_Control.m_LuaExecute.m_Scripts;

			for ( auto& script : Gui::m_Control.m_LuaExecute.m_Scripts )
			{
				auto script_id = c_lua::Get( ).get_script_id( script + (".lua") );

				if ( script_id == -1 )
					continue;

				if ( c_lua::Get( ).loaded.at( script_id ) )
					Gui::m_Control.m_LuaExecute.m_Scripts.at( script_id ) += (" - [loaded]");
			}

			GroupScripts->AddElement( new LuaList( "", &Gui::m_Control.m_LuaExecute.m_SelectedScript, Gui::m_Control.m_LuaExecute.m_Scripts, 15, 0 ) );
			Gui::m_Control.m_LuaExecute.m_Scripts = std::move( backup_scripts );
		}
	}

	auto GroupFunctions = new Groupbox( _( "functions" ), 702, 496, 60, 290, 105, m_Tab );
	{
		m_Window->AddGroup( GroupFunctions );

		GroupFunctions->AddEmpty( 5, 5 );

		GroupFunctions->AddElement( new Button( _( "load" ), [ ]( ) { LoadScript( ); }, { 30, 0 }, { 105, 18 } ) );
		GroupFunctions->AddElement( new Button( _( "unload" ), [ ]( ) { UnloadScript( ); }, { 30, 22 }, { 105, 18 } ) );

		GroupFunctions->AddElement( new Button( _( "reload all" ), [ ]( ) { ReloadAllScripts( ); }, { 140, 0 }, { 105, 18 } ) );
		GroupFunctions->AddElement( new Button( _( "unload all" ), [ ]( ) { UnloadAllScripts( ); }, { 140, 22 }, { 105, 18 } ) );

		GroupFunctions->AddElement( new Button( _( "refresh" ), [ ]( ) { RefreshScripts( ); }, { 85, 44 }, { 105, 18 } ) );
	}

	auto GroupElements = new Groupbox( _( "elements" ), 703, 496, 190, 290, 285, m_Tab, { 0 }, true );
	{
		m_Window->AddGroup( GroupElements );

		for ( auto& current : c_lua::Get( ).scripts )
		{
			auto& items = c_lua::Get( ).items.at( c_lua::Get( ).get_script_id( current ) );

			for ( auto& item : items )
			{
				std::string item_name;

				auto first_point = false;
				auto item_str = false;

				for ( auto& c : item.first )
				{
					if ( c == '.' )
					{
						if ( first_point )
						{
							item_str = true;
							continue;
						}
						else
							first_point = true;
					}

					if ( item_str ) {

						item_name.push_back( c );
					}
				}

				switch ( item.second.type )
				{
				case NEXT_LINE:
					break;
				case CHECK_BOX:
					GroupElements->AddElement( new Checkbox( item_name, &item.second.check_box_value ) );
					break;
				case COMBO_BOX:
					GroupElements->AddElement( new Combo( item_name, &item.second.combo_box_value, item.second.combo_box_labels, 140 ) );
					break;
				case SLIDER_FLOAT:
					GroupElements->AddElement( new Slider( item_name, &item.second.slider_float_value, Sld_Defaut, item.second.slider_float_min, item.second.slider_float_max ) );
					break;
				case COLOR_PICKER:
					GroupElements->AddElement( new Text( item_name ) );
					GroupElements->AddElement( new ColorPicker( &item.second.color_picker_value, &item.second.color_picker_hue, true ), true );
					break;
				}
			}
		}
	}

	GroupElements->InitScroll( );

	delete GroupScripts; delete GroupFunctions;
	delete GroupElements;
}