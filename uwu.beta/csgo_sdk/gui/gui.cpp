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

#include "../config/config.h"
#include "../functions/skins/kit_parser.h"
#include "../functions/skins/skins.h"
#include "../render/engine/engine_draw.h"
#include "../functions/logs.h"
#include "../lua/Clua.h"
#include "../sdk/mem_init.h"

Gui::Details Gui::m_Details;
Gui::Input Gui::m_Input;
Gui::Control Gui::m_Control;
Gui::ExternalRendering Gui::m_External;
Gui::Tabs Gui::m_Tabs;

static int m_CurrentDisplay;
static int m_CurrentDisplayAtLeftSide;

bool Gui::Details::GetMenuState( )
{
	return m_Show;
}

void Gui::Details::SetMenuState( const bool m_State )
{
	m_Show = m_State;
}

Vector2D Gui::Input::GetMousePos( )
{
	return m_MousePos;
}

void Gui::Input::SetMousePos( const Vector2D m_Pos )
{
	m_MousePos = Vector2D{ m_Pos.x, m_Pos.y };
}

void Gui::Input::PollInput( )
{
	for ( size_t i{ 0 }; i < 256; i++ ) {
		Helpers.m_PrevKeyState[ i ] = Helpers.m_KeyState[ i ];
		Helpers.m_KeyState[ i ] = GetAsyncKeyState( i );
	}
}

bool Gui::Input::KeyPressed( const uintptr_t m_Key )
{
	return Helpers.m_KeyState[ m_Key ] && !Helpers.m_PrevKeyState[ m_Key ];
}

bool Gui::Input::MousePointer( const Vector2D m_Pos, const Vector2D m_Size )
{
	std::pair<bool, bool> m_ShouldReturn
	{
		GetMousePos( ).x > m_Pos.x && GetMousePos( ).y > m_Pos.y,
		GetMousePos( ).x < m_Pos.x + m_Size.x && GetMousePos( ).y < m_Pos.y + m_Size.y
	};

	return m_ShouldReturn.first && m_ShouldReturn.second;
}

void Gui::Input::SetMouseWheel( const float m_Wheel )
{
	m_MouseWheel = m_Wheel;
}

float Gui::Input::GetMouseWheel( )
{
	return m_MouseWheel;
}

int Gui::Control::GetIndex( )
{
	return m_Index;
}

void Gui::Control::SetIndex( const uintptr_t m_Last )
{
	m_Index = m_Last;
}

void Gui::Control::AddConfigSkin( )
{
	if ( g_Options.new_config_skin_name.find( _( ".json" ) ) == -1 )
		g_Options.new_config_skin_name += _( ".json" );

	g_ConfigMaster.save_skins_items( g_Options.new_config_skin_name.c_str( ) );
	g_ConfigMaster.skin_config_files( );

	g_Options.new_config_skin_name = "";
	g_Options.selected_config_at_skin = g_ConfigMaster.skin_files.size( ) - 1;
}

void Gui::Control::LoadConfigSkin( )
{
	if ( !g_ConfigMaster.skin_files.size( ) )
		return;

	g_ConfigMaster.load_skins( g_ConfigMaster.skin_files[ g_Options.selected_config_at_skin ] );
	m_Tabs.UpdateSkins( );
}

void Gui::Control::SaveConfigSkin( )
{
	if ( !g_ConfigMaster.skin_files.size( ) )
		return;

	g_ConfigMaster.save_skins_items( g_ConfigMaster.skin_files[ g_Options.selected_config_at_skin ] );
	g_ConfigMaster.skin_config_files( );
}

void Gui::Control::AddConfig( )
{
	if ( g_Options.new_config_name.find( _( ".json" ) ) == -1 )
		g_Options.new_config_name += _( ".json" );

	g_ConfigMaster.save( g_Options.new_config_name.c_str( ) );
	g_ConfigMaster.config_files( );

	g_Options.new_config_name = "";
	g_Options.selected_config = g_ConfigMaster.files.size( ) - 1;
}

void Gui::Control::LoadConfig( )
{
	if ( !g_ConfigMaster.files.size( ) )
		return;

	g_ConfigMaster.load( g_ConfigMaster.files[ g_Options.selected_config ], false );
	m_Tabs.LoadFontFromConfig( );

	c_lua::Get( ).unload_all_scripts( );

	for ( auto& script : g_Options.m_InitScripts )
		c_lua::Get( ).load_script( c_lua::Get( ).get_script_id( script ) );

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

	g_Options.m_InitScripts.clear( );
	g_ConfigMaster.load( g_ConfigMaster.files[ g_Options.selected_config ], true );
	m_Tabs.UpdateSkins( );
}

void Gui::Control::SaveConfig( )
{
	if ( !g_ConfigMaster.files.size( ) )
		return;

	g_Options.m_InitScripts.clear( );

	for ( auto i = 0; i < c_lua::Get( ).scripts.size( ); ++i )
	{
		auto script = c_lua::Get( ).scripts.at( i );

		if ( c_lua::Get( ).loaded.at( i ) )
			g_Options.m_InitScripts.emplace_back( script );
	}

	g_ConfigMaster.save( g_ConfigMaster.files[ g_Options.selected_config ] );
	g_ConfigMaster.config_files( );
}

bool Gui::Control::IsPossible( )
{
	return Gui::m_Control.m_Opened
		[ Gui::m_Control.ControlType::COMBO ] == -1 &&
		Gui::m_Control.m_Opened
		[ Gui::m_Control.ControlType::MULTICOMBO ] == -1
		;
}

void Gui::ExternalRendering::Install( )
{
	for ( size_t i{ 0 }; i < RectDraw.size( ); i++ )
		g_Render->FilledRect( RectDraw[ i ].posX, RectDraw[ i ].posY, RectDraw[ i ].m_Width, RectDraw[ i ].m_Height, RectDraw[ i ].m_Color, 2 );

	for ( size_t i{ 0 }; i < GradientHDraw.size( ); i++ )
		g_Render->FilledRectGradient( GradientHDraw[ i ].posX, GradientHDraw[ i ].posY, GradientHDraw[ i ].m_Width, GradientHDraw[ i ].m_Height, GradientHDraw[ i ].m_Color, GradientHDraw[ i ].m_ColorNext, GradientHDraw[ i ].m_ColorNext, GradientHDraw[ i ].m_Color );

	for ( size_t i{ 0 }; i < GradientVDraw.size( ); i++ )
		g_Render->FilledRectGradient( GradientVDraw[ i ].posX, GradientVDraw[ i ].posY, GradientVDraw[ i ].m_Width, GradientVDraw[ i ].m_Height, GradientVDraw[ i ].m_Color, GradientVDraw[ i ].m_Color, GradientVDraw[ i ].m_ColorNext, GradientVDraw[ i ].m_ColorNext );

	for ( size_t i{ 0 }; i < OutlineDraw.size( ); i++ )
		g_Render->Rect( OutlineDraw[ i ].posX, OutlineDraw[ i ].posY, OutlineDraw[ i ].m_Width, OutlineDraw[ i ].m_Height, OutlineDraw[ i ].m_Color, 2 );

	for ( size_t i{ 0 }; i < TextDraw.size( ); i++ )
		g_Render->DrawString( TextDraw[ i ].posX, TextDraw[ i ].posY, TextDraw[ i ].m_Color, RenderImFonts::make_shadow, RenderImFonts::DefaultFont, TextDraw[ i ].m_Text.c_str( ) );

	RectDraw.clear( );
	OutlineDraw.clear( );
	GradientHDraw.clear( );
	GradientVDraw.clear( );
	TextDraw.clear( );
}

void Gui::Tabs::SetupRage( Window* m_Window, int m_Tab )
{
	std::vector<std::string> m_Hitgroup = { _( "none" ), _( "head" ), _( "body" ) };
	std::vector<std::string> m_Back = { _( "none" ), _( "default" ) };
	std::vector<std::string> m_Tar = { _( "fov" ), _( "low health" ), _( "distance" ), _( "cycle" ), _( "no priority" ) };
	std::vector<std::string> m_Pitch = { _( "none" ), _( "up" ), "down", "zero", "random" };
	std::vector<std::string> m_Yaw = { "none", "backward", "static" };
	std::vector<std::string> m_JitterMode = { "none", "range", "random in range" };
	std::vector<std::string> m_Based = { "default", "viewangles", "at target" };
	std::vector<std::string> m_FakeYaw = { "none", "static", "rotate" };
	std::vector<std::string> m_FakeJitter = { "none", "on lag changed" };
	std::vector<std::string> m_Explt = { "double shot", "hide shot" };
	std::vector<std::string> m_Speed = { _( "none" ), _( "full" ), _( "quick" ), _( "alternative" ) };
	std::vector<std::string> m_Wepns = { _( "awp" ), _( "scout" ), _( "heavy pistol" ), _( "auto" ), _( "rifle" ), _( "smg" ), _( "shotgun" ), _( "pistols" ) };

	static int m_SwitchAimTab; static int m_SwitchAntiAimTab;
	static int m_SwitchAcc; static int m_SelectWeapon;

	auto m_AimbotMain = new Groupbox( "main", 1, 180, 60, 290, 80, m_Tab );
	{
		m_Window->AddGroup( m_AimbotMain );

		m_AimbotMain->AddElement( new Checkbox( _( "enable" ), &g_Options.ragebot.Enable ) );
		m_AimbotMain->AddElement( new Combo( _( "group" ), &m_SelectWeapon, m_Wepns, 140 ) );
	}

	auto m_GlobalMain = new Groupbox( "global", 2, 180, 160, 290, 160, m_Tab );
	{
		m_Window->AddGroup( m_GlobalMain );

		m_GlobalMain->AddElement( new Checkbox( _( "auto fire" ), &g_Options.ragebot.AutoFire ) );
		m_GlobalMain->AddElement( new Checkbox( _( "silent aim" ), &g_Options.ragebot.SilentAim ) );
		m_GlobalMain->AddElement( new Checkbox( _( "experimental resolver" ), &g_Options.ragebot.Resolver ) );
		m_GlobalMain->AddElement( new Checkbox( _( "backtracking" ), &g_Options.ragebot.Backtracking ) );
		m_GlobalMain->AddElement( new Slider( _( "maximum fov" ), &g_Options.ragebot.FieldOfView, SliderType::Sld_Defaut, 0, 360 ) );
		m_GlobalMain->AddElement( new Combo( _( "target selection" ), &g_Options.ragebot.TargetSelection, m_Tar, 140 ) );
	}

	auto m_Scan = new Groupbox( "hitscan", 3, 180, 340, 290, 130, m_Tab, { 0 }, false );
	{
		m_Window->AddGroup( m_Scan );
		m_Scan->AddElement( new Combo( _( "hitbox priority" ), &g_Options.ragebot.PriorityHitbox[ m_SelectWeapon ], m_Hitgroup, 140 ) );

		auto m_MultiHibox = new Multi( _( "hitboxes to scan" ), 140 );
		m_MultiHibox->AddItem( _( "head" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 0 ] );
		m_MultiHibox->AddItem( _( "upper chest" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 1 ] );
		m_MultiHibox->AddItem( _( "chest" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 2 ] );
		m_MultiHibox->AddItem( _( "stomach" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 3 ] );
		m_MultiHibox->AddItem( _( "pelvis" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 4 ] );
		m_MultiHibox->AddItem( _( "arms" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 5 ] );
		m_MultiHibox->AddItem( _( "legs" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 6 ] );
		m_MultiHibox->AddItem( _( "feet" ), &g_Options.ragebot.Hitboxes[ m_SelectWeapon ][ 7 ] );
		m_Scan->AddElement( m_MultiHibox );

		m_Scan->AddElement( new Slider( _( "head scale" ), &g_Options.ragebot.HeadScale[ m_SelectWeapon ], SliderType::Sld_Defaut ) );
		m_Scan->AddElement( new Slider( _( "body scale" ), &g_Options.ragebot.BodyScale[ m_SelectWeapon ], SliderType::Sld_Defaut ) );
	}

	auto m_AntiAimbotMain = new Groupbox( _( "anti-aimbot" ), 4, 496, 245, 290, 225, m_Tab );
	{
		m_Window->AddGroup( m_AntiAimbotMain );

		auto m_Sub2 = new SubTab( { 286, 10 }, &m_SwitchAntiAimTab );
		m_Sub2->AddTab( _( "basic" ) );
		m_Sub2->AddTab( _( "settings" ) );
		m_AntiAimbotMain->AddElement( m_Sub2 );

		if ( m_SwitchAntiAimTab == 0 )
		{
			m_AntiAimbotMain->AddElement( new Combo( _( "pitch" ), &g_Options.antiaim.Pitch, m_Pitch, 140 ) );
			m_AntiAimbotMain->AddElement( new Combo( _( "base yaw" ), &g_Options.antiaim.BaseYaw, m_Based, 140 ) );
			m_AntiAimbotMain->AddElement( new Combo( _( "yaw" ), &g_Options.antiaim.Yaw, m_Yaw, 140 ) );
			m_AntiAimbotMain->AddElement( new Combo( _( "jitter" ), &g_Options.antiaim.JitterMode, m_JitterMode, 140 ) );

			if ( g_Options.antiaim.JitterMode > 0 )
				m_AntiAimbotMain->AddElement( new Slider( _( "value" ), &g_Options.antiaim.JitterValue, SliderType::Sld_Defaut, 0, 90 ) );

			m_AntiAimbotMain->AddElement( new Slider( _( "add yaw" ), &g_Options.antiaim.YawAdd, SliderType::Sld_Defaut, 0, 180 ) );
			m_AntiAimbotMain->AddElement( new Slider( _( "inverted yaw" ), &g_Options.antiaim.YawAddInvert, SliderType::Sld_Defaut, 0, 180 ) );
		}
		else if ( m_SwitchAntiAimTab == 1 )
		{
			m_AntiAimbotMain->AddElement( new Combo( _( "fake yaw" ), &g_Options.antiaim.FakeYaw, m_FakeYaw, 140 ) );
			m_AntiAimbotMain->AddElement( new Combo( _( "jitter" ), &g_Options.antiaim.JiterSideFake, m_FakeJitter, 140 ) );
			m_AntiAimbotMain->AddElement( new Slider( _( "add yaw" ), &g_Options.antiaim.FakeYawValue, SliderType::Sld_Defaut, 0, 60 ) );
			m_AntiAimbotMain->AddElement( new Slider( _( "inverted yaw" ), &g_Options.antiaim.FakeYawValueInvert, SliderType::Sld_Defaut, 0, 60 ) );
			m_AntiAimbotMain->AddElement( new Text( _( "invert key" ) ) );
			m_AntiAimbotMain->AddElement( new Bind( &g_Options.antiaim.KeyInvert ) );
		}
	}

	auto m_OtherMain = new Groupbox( _( "accuracy" ), 5, 496, 60, 290, 165, m_Tab );
	{
		m_Window->AddGroup( m_OtherMain );

		auto m_Sub3 = new SubTab( { 286, 20 }, &m_SwitchAcc );
		m_Sub3->AddTab( _( "basic" ) );
		m_Sub3->AddTab( _( "advanced" ) );
		m_Sub3->AddTab( _( "exploits" ) );
		m_OtherMain->AddElement( m_Sub3 );

		if ( m_SwitchAcc == 0 )
		{
			if ( m_SelectWeapon == 0 || m_SelectWeapon == 1 || m_SelectWeapon == 3 || m_SelectWeapon == 4 )
				m_OtherMain->AddElement( new Checkbox( _( "auto scope" ), &g_Options.ragebot.AutoScope[ m_SelectWeapon ] ) );

			m_OtherMain->AddElement( new Checkbox( _( "stop between shots" ), &g_Options.ragebot.AutoStop[ m_SelectWeapon ] ) );

			m_OtherMain->AddElement( new Slider( _( "min damage" ), &g_Options.ragebot.min_damage[ m_SelectWeapon ], SliderType::Sld_Defaut, 0, 100 ) );
			m_OtherMain->AddElement( new Slider( _( "hitchance" ), &g_Options.ragebot.hitchance[ m_SelectWeapon ], SliderType::Sld_Defaut, 0, 100 ) );
			m_OtherMain->AddElement( new Slider( _( "accuracy boost" ), &g_Options.ragebot.boost[ m_SelectWeapon ], SliderType::Sld_Defaut, 0, 100 ) );
		}
		else if ( m_SwitchAcc == 1 )
		{
			m_OtherMain->AddElement( new Checkbox( _( "on shot priority" ), &g_Options.ragebot.ShotPriority[ m_SelectWeapon ] ) );
			m_OtherMain->AddElement( new Checkbox( _( "force body aim" ), &g_Options.ragebot.ForceBaim ) );
			m_OtherMain->AddElement( new Bind( &g_Options.ragebot.BaimKey ) );
			m_OtherMain->AddElement( new Checkbox( _( "force optimal point" ), &g_Options.ragebot.PreferSafepoint ) );
			m_OtherMain->AddElement( new Bind( &g_Options.ragebot.SafeKey ) );

			auto m_SafeAdd = new Multi( _( "safe conditions" ), 140 );
			m_SafeAdd->AddItem( _( "high velocity" ), &g_Options.ragebot.Prefers[ m_SelectWeapon ][ 0 ] );
			m_SafeAdd->AddItem( _( "exploit" ), &g_Options.ragebot.Prefers[ m_SelectWeapon ][ 1 ] );
			m_SafeAdd->AddItem( _( "unsure" ), &g_Options.ragebot.Prefers[ m_SelectWeapon ][ 2 ] );
			m_OtherMain->AddElement( m_SafeAdd );

			auto m_BaimPre = new Multi( _( "body aim conditions" ), 140 );
			m_BaimPre->AddItem( _( "lethal" ), &g_Options.ragebot.LethalBaim[ m_SelectWeapon ] );
			m_BaimPre->AddItem( _( "air" ), &g_Options.ragebot.PrefersBo[ m_SelectWeapon ][ 0 ] );
			m_BaimPre->AddItem( _( "duck" ), &g_Options.ragebot.PrefersBo[ m_SelectWeapon ][ 1 ] );
			m_BaimPre->AddItem( _( "unresolved" ), &g_Options.ragebot.PrefersBo[ m_SelectWeapon ][ 2 ] );
			m_BaimPre->AddItem( _( "unsafe" ), &g_Options.ragebot.PreferBaim[ m_SelectWeapon ] );
			m_OtherMain->AddElement( m_BaimPre );
		}
		else if ( m_SwitchAcc == 2 )
		{
			m_OtherMain->AddElement( new Checkbox( _( "enable" ), &g_Options.exploit.ActMode ) );
			m_OtherMain->AddElement( new Bind( &g_Options.exploit.mExpltKey ) );
			m_OtherMain->AddElement( new Combo( _( "mode" ), &g_Options.exploit.mMode, m_Explt, 140 ) );

			if ( g_Options.exploit.mMode == 0 ) {
				m_OtherMain->AddElement( new Combo( _( "extra stop" ), &g_Options.exploit.m_DoubleTapSpeed, m_Speed, 140 ) );
				m_OtherMain->AddElement( new Slider( _( "lag tolerance" ), &g_Options.exploit.m_Tolerance, SliderType::Sld_Defaut, 0, 15 ) );
			}
		}
	}

	/*m_Scan->InitScroll( );*/

	delete m_AimbotMain; delete m_GlobalMain;
	delete m_Scan; delete m_AntiAimbotMain;
	delete m_OtherMain;
}

void Gui::Tabs::SetupLegit( Window* m_Window, int m_Tab )
{
	static int m_SelectWeapon;

	std::vector<std::string> m_Wepns = { _( "awp" ), _( "scout" ), _( "heavy pistol" ), _( "auto" ), _( "rifle" ), _( "smg" ), _( "shotgun" ), _( "pistols" ) };
	std::vector<std::string> m_Silent = { _( "none" ), _( "default" ), _( "aim" ) };
	std::vector<std::string> m_RCS = { _( "standalone" ), _( "aim" ) };

	auto m_AimbotMain = new Groupbox( "main", 271, 180, 60, 290, 80, m_Tab );
	{
		m_Window->AddGroup( m_AimbotMain );

		m_AimbotMain->AddElement( new Checkbox( _( "enable" ), &g_Options.active_legit ) );
		m_AimbotMain->AddElement( new Combo( _( "group" ), &m_SelectWeapon, m_Wepns, 140 ) );
	}

	auto m_GlobalMain = new Groupbox( _( "target" ), 272, 180, 160, 290, 140, m_Tab );
	{
		m_Window->AddGroup( m_GlobalMain );

		m_GlobalMain->AddElement( new Checkbox( _( "enable filter" ), &g_Options.m_legit[ m_SelectWeapon ].enabled ) );
		m_GlobalMain->AddElement( new Checkbox( _( "flash check" ), &g_Options.m_legit[ m_SelectWeapon ].flash_check ) );
		m_GlobalMain->AddElement( new Checkbox( _( "smoke check" ), &g_Options.m_legit[ m_SelectWeapon ].smoke_check ) );

		if ( m_SelectWeapon == 2 || m_SelectWeapon == 7 )
			m_GlobalMain->AddElement( new Checkbox( _( "auto pistol" ), &g_Options.m_legit[ m_SelectWeapon ].auto_pistol ) );

		auto m_Multi = new Multi( _( "hitboxes" ), 140 );
		m_Multi->AddItem( _( "head" ), &g_Options.m_legit[ m_SelectWeapon ].hitbox.head );
		m_Multi->AddItem( _( "chest" ), &g_Options.m_legit[ m_SelectWeapon ].hitbox.chest );
		m_Multi->AddItem( _( "hands" ), &g_Options.m_legit[ m_SelectWeapon ].hitbox.hands );
		m_Multi->AddItem( _( "legs" ), &g_Options.m_legit[ m_SelectWeapon ].hitbox.legs );
		m_GlobalMain->AddElement( m_Multi );
	}

	auto m_Settings = new Groupbox( _( "settings" ), 273, 180, 320, 290, 115, m_Tab );
	{
		m_Window->AddGroup( m_Settings );

		m_Settings->AddElement( new Slider( _( "fov" ), &g_Options.m_legit[ m_SelectWeapon ].fov, SliderType::Sld_Defaut, 0, 20.f ) );
		m_Settings->AddElement( new Slider( _( "smooth" ), &g_Options.m_legit[ m_SelectWeapon ].smooth, SliderType::Sld_Defaut, 0, 20.f ) );
		m_Settings->AddElement( new Slider( _( "shot delay" ), &g_Options.m_legit[ m_SelectWeapon ].shot_delay, SliderType::Sld_Defaut, 0, 500 ) );
		m_Settings->AddElement( new Slider( _( "kill delay" ), &g_Options.m_legit[ m_SelectWeapon ].kill_delay, SliderType::Sld_Defaut, 0, 500 ) );
	}

	auto m_SilentGroup = new Groupbox( _( "silent" ), 274, 496, 60, 290, 80, m_Tab );
	{
		m_Window->AddGroup( m_SilentGroup );

		m_SilentGroup->AddElement( new Slider( _( "fov" ), &g_Options.m_legit[ m_SelectWeapon ].silent_fov, SliderType::Sld_Defaut, 0, 20.f ) );
		m_SilentGroup->AddElement( new Combo( _( "mode" ), &g_Options.m_legit[ m_SelectWeapon ].silent_mode, m_Silent, 140 ) );
	}

	auto m_RecoilGroup = new Groupbox( _( "recoil control system" ), 275, 496, 160, 290, 120, m_Tab );
	{
		m_Window->AddGroup( m_RecoilGroup );

		m_RecoilGroup->AddElement( new Checkbox( _( "enable" ), &g_Options.m_legit[ m_SelectWeapon ].rcs ) );
		m_RecoilGroup->AddElement( new Slider( _( "recoil x" ), &g_Options.m_legit[ m_SelectWeapon ].rcs_x, SliderType::Sld_Defaut, 0, 100 ) );
		m_RecoilGroup->AddElement( new Slider( _( "recoil y" ), &g_Options.m_legit[ m_SelectWeapon ].rcs_y, SliderType::Sld_Defaut, 0, 100 ) );
		m_RecoilGroup->AddElement( new Combo( _( "mode" ), &g_Options.m_legit[ m_SelectWeapon ].rcs_mode, m_RCS, 140 ) );
	}

	auto m_AwallGroup = new Groupbox( _( "autowall" ), 276, 496, 300, 290, 90, m_Tab );
	{
		m_Window->AddGroup( m_AwallGroup );

		m_AwallGroup->AddElement( new Checkbox( _( "enable" ), &g_Options.m_legit[ m_SelectWeapon ].awall ) );
		m_AwallGroup->AddElement( new Bind( &g_Options.awall_key ) );
		m_AwallGroup->AddElement( new Checkbox( _( "visibility filter" ), &g_Options.m_legit[ m_SelectWeapon ].vis ) );
		m_AwallGroup->AddElement( new Slider( _( "min damage" ), &g_Options.m_legit[ m_SelectWeapon ].awall_damage, SliderType::Sld_Defaut, 0, 100 ) );
	}

	delete m_AimbotMain; delete m_GlobalMain;
	delete m_Settings; delete m_SilentGroup;
	delete m_RecoilGroup; delete m_AwallGroup;
}

void Gui::Tabs::RemoveFont( )
{
	if ( g_Options.CustomFonts.m_Font[ g_Options.CustomFonts.m_CurrentSlot ] > 0 )
		g_Options.CustomFonts.m_Font[ g_Options.CustomFonts.m_CurrentSlot ] = 0;
}

void Gui::Tabs::LoadFontFromConfig( )
{
	g_Options.CustomFonts.m_CurrentSlot = 0;
	g_Options.CustomFonts.m_FontName.clear( );
	g_Options.CustomFonts.m_FontSize.clear( );
	g_Options.CustomFonts.m_FontWeight.clear( );
	g_Options.CustomFonts.m_FlagAA = false;
	g_Options.CustomFonts.m_FlagOutline = false;
	g_Options.CustomFonts.m_FlagDropShadow = false;

	for ( int i = 0; i < 17; i++ )
	{
		g_Options.CustomFonts.m_Font[ i ] = 0;

		if ( g_Options.CustomFonts.m_VectorName[ i ].empty( ) )
			continue;

		EngineDraw::Get( ).CreateFontRea( i, g_Options.CustomFonts.m_VectorName[ i ],
			std::stoi( g_Options.CustomFonts.m_VectorSize[ i ] ), std::stoi( g_Options.CustomFonts.m_VectorWeight[ i ] ), g_Options.CustomFonts.m_VectorAA[ i ],
			g_Options.CustomFonts.m_VectorDropShadow[ i ], g_Options.CustomFonts.m_VectorOutline[ i ] );
	}
}

void Gui::Tabs::CreateFontFromParse( )
{
	if ( g_Options.CustomFonts.m_FontName.empty( ) ||
		g_Options.CustomFonts.m_FontSize.empty( ) ||
		g_Options.CustomFonts.m_FontWeight.empty( ) )
		return;

	g_Options.CustomFonts.m_VectorName[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FontName;
	g_Options.CustomFonts.m_VectorSize[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FontSize;
	g_Options.CustomFonts.m_VectorWeight[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FontWeight;
	g_Options.CustomFonts.m_VectorAA[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FlagAA;
	g_Options.CustomFonts.m_VectorOutline[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FlagOutline;
	g_Options.CustomFonts.m_VectorDropShadow[ g_Options.CustomFonts.m_CurrentSlot ] = g_Options.CustomFonts.m_FlagDropShadow;

	EngineDraw::Get( ).CreateFontMem( g_Options.CustomFonts.m_FontName,
		std::stoi( g_Options.CustomFonts.m_FontSize ), std::stoi( g_Options.CustomFonts.m_FontWeight ), g_Options.CustomFonts.m_FlagAA,
		g_Options.CustomFonts.m_FlagDropShadow, g_Options.CustomFonts.m_FlagOutline );

	g_Options.CustomFonts.m_FontName.clear( );
	g_Options.CustomFonts.m_FontSize.clear( );
	g_Options.CustomFonts.m_FontWeight.clear( );
	g_Options.CustomFonts.m_FlagAA = false;
	g_Options.CustomFonts.m_FlagOutline = false;
	g_Options.CustomFonts.m_FlagDropShadow = false;
}

void Gui::Tabs::SetupVisual( Window* m_Window, int m_Tab )
{
	static int m_Switch; static int m_Other; static int m_Advance;

	std::vector<std::string> m_None = { _( "none" ) };
	std::vector<std::string> m_Box = { _( "none" ), _( "non-outlined" ), _( "default" ) };
	std::vector<std::string> m_Background = { _( "none" ), _( "default" ), _( "default + line" ), _( "default + outline" ) };
	std::vector<std::string> m_Spread = { _( "default" ), _( "rainbow" ) };
	std::vector<std::string> m_Material = { _( "none" ), _( "default" ), _( "flat" ) };
	std::vector<std::string> m_Illuminate = { _( "none" ), _( "glow" ), _( "extra glow" ) };
	std::vector<std::string> m_Sky = { _( "none" ), _( "tibet" ), _( "baggage" ), _( "italy" ), _( "aztec" ), _( "vertigo" ), _( "daylight" ), _( "daylight 2" ), _( "clouds" ), _( "clouds 2" ), _( "gray" ), _( "clear" ), _( "canals" ), _( "cobblestone" ), _( "assault" ), _( "clouds dark" ), _( "night" ), _( "night 2" ), _( "night flat" ), _( "dusty" ), _( "rainy" ) };

	auto EspActivate = new Groupbox( "", 32, 180, 30, 0, 0, m_Tab, Groupbox::Flags::GROUP_NO_DRAW );
	{
		m_Window->AddGroup( EspActivate );
		EspActivate->AddElement( new Checkbox( _( "active" ), &g_Options.visuals.Active ) );
	}

	auto EspOptions = new Groupbox( _( "options" ), 30, 180, 90, 290, 385, m_Tab );
	{
		m_Window->AddGroup( EspOptions );
		auto m_TabS = new SubTab( { 286, 20 }, &m_Switch );
		m_TabS->AddTab( _( "enemy" ) );
		m_TabS->AddTab( _( "local" ) );
		m_TabS->AddTab( _( "advanced" ) );
		m_TabS->AddTab( _( "customization" ) );
		EspOptions->AddElement( m_TabS );

		if ( m_Switch == 0 )
		{
			EspOptions->AddElement( new Checkbox( _( "enable filter" ), &g_Options.visuals.Enemy ) );

			auto m_Vi = new Multi( _( "visibility filters" ), 140 );
			m_Vi->AddItem( _( "chams" ), &g_Options.visuals.vis_enemy.Visibility );
			EspOptions->AddElement( m_Vi );

			EspOptions->AddElement( new Combo( _( "box" ), &g_Options.visuals.vis_enemy.Box, m_Box, 140 ) );
			EspOptions->AddElement( new Combo( _( "chams" ), &g_Options.visuals.vis_enemy.Material, m_Material, 140 ) );
			EspOptions->AddElement( new Combo( _( "illuminate" ), &g_Options.visuals.vis_enemy.Illuminate, m_Illuminate, 140 ) );

			auto m_Multi = new Multi( _( "weapon" ), 140 );
			m_Multi->AddItem( _( "name" ), &g_Options.visuals.vis_enemy.Weapon[ 0 ] );
			m_Multi->AddItem( _( "icon" ), &g_Options.visuals.vis_enemy.Weapon[ 1 ] );
			EspOptions->AddElement( m_Multi );

			EspOptions->AddElement( new Combo( _( "background" ), &g_Options.visuals.vis_enemy.BackgroundType, m_Background, 140 ) );

			if ( g_Options.visuals.vis_enemy.BackgroundType > 0 )
			{
				auto m_MultiBack = new Multi( _( "background items" ), 140 );
				m_MultiBack->AddItem( _( "player name" ), &g_Options.visuals.vis_enemy.Background[ 0 ] );
				m_MultiBack->AddItem( _( "weapon name" ), &g_Options.visuals.vis_enemy.Background[ 1 ] );
				EspOptions->AddElement( m_MultiBack );
			}

			EspOptions->AddElement( new Slider( _( "glow" ), &g_Options.visuals.vis_enemy.Glow, SliderType::Sld_Defaut ) );

			EspOptions->AddElement( new Checkbox( _( "health" ), &g_Options.visuals.vis_enemy.HP ) );
			EspOptions->AddElement( new Checkbox( _( "health number" ), &g_Options.visuals.vis_enemy.HP_Percent ) );
			EspOptions->AddElement( new Checkbox( _( "name" ), &g_Options.visuals.vis_enemy.Name ) );
			EspOptions->AddElement( new Checkbox( _( "ammunition" ), &g_Options.visuals.vis_enemy.Ammo ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.vis_enemy.Ammo_Clr, &g_Options.visuals.vis_enemy.Ammo_hue ) );
		}
		else if ( m_Switch == 1 )
		{
			EspOptions->AddElement( new Checkbox( _( "enable filter" ), &g_Options.visuals.Local ) );
			EspOptions->AddElement( new Combo( _( "chams" ), &g_Options.visuals.vis_local.Material, m_Material, 140 ) );
			EspOptions->AddElement( new Combo( _( "illuminate" ), &g_Options.visuals.vis_local.Illuminate, m_Illuminate, 140 ) );
			EspOptions->AddElement( new Combo( _( "fake chams" ), &g_Options.visuals.vis_local.Desync_Material, m_Material, 140 ) );
			EspOptions->AddElement( new Combo( _( "fake illuminate" ), &g_Options.visuals.vis_local.Desync_Illuminate, m_Illuminate, 140 ) );
			EspOptions->AddElement( new Slider( _( "glow" ), &g_Options.visuals.vis_local.Glow, SliderType::Sld_Defaut ) );
		}
		else if ( m_Switch == 2 )
		{
			EspOptions->AddElement( new Checkbox( _( "dropped weapon name" ), &g_Options.visuals.world_weapons.name ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world_weapons.name_clr, &g_Options.visuals.world_weapons.name_clr_hue ) );

			EspOptions->AddElement( new Checkbox( _( "dropped weapon box" ), &g_Options.visuals.world_weapons.box ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world_weapons.box_clr, &g_Options.visuals.world_weapons.box_clr_hue ) );

			EspOptions->AddElement( new Checkbox( _( "dropped weapon ammo" ), &g_Options.visuals.world_weapons.ammo ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world_weapons.ammo_clr, &g_Options.visuals.world_weapons.ammo_clr_hue ) );

			EspOptions->AddElement( new Checkbox( _( "full bright" ), &g_Options.visuals.world.full_bright ) );

			EspOptions->AddElement( new Checkbox( _( "modulate walls" ), &g_Options.visuals.world.walls ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world.wall_clr, &g_Options.visuals.world.wall_clr_hue ) );

			EspOptions->AddElement( new Checkbox( _( "modulate props" ), &g_Options.visuals.world.props ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world.props_clr, &g_Options.visuals.world.props_clr_hue, true ) );

			EspOptions->AddElement( new Checkbox( _( "modulate sky" ), &g_Options.visuals.world.sky ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.world.sky_clr, &g_Options.visuals.world.sky_clr_hue ) );

			EspOptions->AddElement( new Combo( _( "sky texture" ), &g_Options.visuals.world.sky_mode, m_Sky, 140 ) );

			EspOptions->AddElement( new Checkbox( _( "out of fov arrows" ), &g_Options.visuals.vis_enemy.OOF ) );
			EspOptions->AddElement( new ColorPicker( &g_Options.visuals.vis_enemy.OOF_Clr, &g_Options.visuals.vis_enemy.OOF_hue, true ) );
			EspOptions->AddElement( new Slider( _( "size" ), &g_Options.visuals.vis_enemy.OOF_size, SliderType::Sld_Right, 0, 30 ) );
			EspOptions->AddElement( new Slider( _( "distance" ), &g_Options.visuals.vis_enemy.OOF_dist, SliderType::Sld_Right, 0, 100 ) );
		}
		else if ( m_Switch == 3 )
		{
			auto m_TabAdvancedCustom = new SubTab( { 286, 50 }, &m_Advance );
			m_TabAdvancedCustom->AddTab( _( "scaling 1" ) );
			m_TabAdvancedCustom->AddTab( _( "scaling 2" ) );
			EspOptions->AddElement( m_TabAdvancedCustom );

			if ( m_Advance == 0 )
			{
				EspOptions->AddElement( new Slider( _( "box first scale" ), &g_Options.CustomVisals.ScaleX, SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "box second scale" ), &g_Options.CustomVisals.ScaleY, SliderType::Sld_Defaut, -20, 20 ) );

				EspOptions->AddElement( new Slider( _( "box third scale" ), &g_Options.CustomVisals.ScaleZ, SliderType::Sld_Defaut, 0.00, 10.00 ) );
				EspOptions->AddElement( new Slider( _( "box fourth scale" ), &g_Options.CustomVisals.ScaleA, SliderType::Sld_Defaut, 1, 10 ) );

				EspOptions->AddElement( new Slider( _( "name pos" ), &g_Options.CustomVisals.NamePosY, SliderType::Sld_Defaut, -50, 50 ) );
				EspOptions->AddElement( new Slider( _( "background line pos" ), &g_Options.CustomVisals.OneLineFrame, SliderType::Sld_Defaut, -30, 30 ) );
				EspOptions->AddElement( new Slider( _( "pos weapon name" ), &g_Options.CustomVisals.SecondLineFrame, SliderType::Sld_Defaut, -30, 30 ) );

				EspOptions->AddElement( new Slider( _( "health bar size" ), &g_Options.CustomVisals.SizeHealthBar, SliderType::Sld_Defaut, 0, 10 ) );
				EspOptions->AddElement( new Slider( _( "health bar pos" ), &g_Options.CustomVisals.LeftSideHealthBar, SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "health text pos" ), &g_Options.CustomVisals.HealthTextPos, SliderType::Sld_Defaut, -20, 20 ) );

				EspOptions->AddElement( new Slider( _( "ammo bar pos" ), &g_Options.CustomVisals.AmmoPos, SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "ammo bar size" ), &g_Options.CustomVisals.AmmoSize, SliderType::Sld_Defaut, 0, 10 ) );
			}
			else if ( m_Advance == 1 )
			{
				EspOptions->AddElement( new Slider( _( "weapon name pos" ), &g_Options.CustomVisals.WeaponNamePos1, SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "pos with ammo bar" ), &g_Options.CustomVisals.WeaponNamePos2, SliderType::Sld_Defaut, -20, 20 ) );

				EspOptions->AddElement( new Slider( _( "weapon icon pos" ), &g_Options.CustomVisals.WeaponIcon[ 0 ], SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "pos with ammo + name" ), &g_Options.CustomVisals.WeaponIcon[ 1 ], SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "pos with name" ), &g_Options.CustomVisals.WeaponIcon[ 2 ], SliderType::Sld_Defaut, -20, 20 ) );
				EspOptions->AddElement( new Slider( _( "pos ( without )" ), &g_Options.CustomVisals.WeaponIcon[ 3 ], SliderType::Sld_Defaut, -20, 20 ) );
			}
		}
	}

	auto EspOther = new Groupbox( _( "other" ), 31, 496, 90, 290, 385, m_Tab );
	{
		m_Window->AddGroup( EspOther );
		auto m_TabOther = new SubTab( { 286, 20 }, &m_Other );
		m_TabOther->AddTab( _( "basic" ) );
		m_TabOther->AddTab( _( "advanced" ) );
		m_TabOther->AddTab( _( "colors" ) );
		EspOther->AddElement( m_TabOther );

		if ( m_Other == 0 )
		{
			EspOther->AddElement( new Slider( _( "viewmodel fov changer" ), &g_Options.visuals.view.view_v, SliderType::Sld_Defaut, 0, 180 ) );
			EspOther->AddElement( new Slider( _( "vield of view changer" ), &g_Options.visuals.view.fov_v, SliderType::Sld_Defaut, 0, 180 ) );

			EspOther->AddElement( new Slider( _( "viewmodel offset x" ), &g_Options.visuals.view.x_view, SliderType::Sld_Defaut, -30, 30 ) );
			EspOther->AddElement( new Slider( _( "viewmodel offset y" ), &g_Options.visuals.view.y_view, SliderType::Sld_Defaut, -30, 30 ) );
			EspOther->AddElement( new Slider( _( "viewmodel offset z" ), &g_Options.visuals.view.z_view, SliderType::Sld_Defaut, -30, 30 ) );

			auto m_MultiRemovals = new Multi( _( "removals" ), 140 );
			m_MultiRemovals->AddItem( _( "visuals recoil" ), &g_Options.visuals.removals.recoil );
			m_MultiRemovals->AddItem( _( "shadows" ), &g_Options.visuals.removals.shadows );
			m_MultiRemovals->AddItem( _( "scope" ), &g_Options.visuals.removals.scope );
			m_MultiRemovals->AddItem( _( "zoom" ), &g_Options.visuals.removals.zoom );
			m_MultiRemovals->AddItem( _( "smoke" ), &g_Options.visuals.removals.smoke );
			m_MultiRemovals->AddItem( _( "flash" ), &g_Options.visuals.removals.flash );
			m_MultiRemovals->AddItem( _( "post processing" ), &g_Options.visuals.removals.post_process );
			m_MultiRemovals->AddItem( _( "panorama blur" ), &g_Options.visuals.removals.panorama_blue );
			m_MultiRemovals->AddItem( _( "fog" ), &g_Options.visuals.removals.fog );
			EspOther->AddElement( m_MultiRemovals );

			EspOther->AddElement( new Checkbox( _( "modulate light" ), &g_Options.visuals.world.modulate_light.rAmbient ) );
			EspOther->AddElement( new Slider( _( "exposure" ), &g_Options.visuals.world.modulate_light.Exposure, SliderType::Sld_Right, 0, 2.0 ) );
			EspOther->AddElement( new Slider( _( "bloom" ), &g_Options.visuals.world.modulate_light.Bloom, SliderType::Sld_Right, 0, 10.0 ) );
			EspOther->AddElement( new Slider( _( "ambient" ), &g_Options.visuals.world.modulate_light.AmbientAmount, SliderType::Sld_Right, 0, 10.0 ) );

			EspOther->AddElement( new Checkbox( _( "modulate fog" ), &g_Options.visuals.world.fog.Enabled ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.world.fog.FogClr, &g_Options.visuals.world.fog.FogHue ) );
			EspOther->AddElement( new Slider( _( "density" ), &g_Options.visuals.world.fog.Density, SliderType::Sld_Right, 0, 100 ) );
			EspOther->AddElement( new Slider( _( "distance" ), &g_Options.visuals.world.fog.Dist, SliderType::Sld_Right, 0, 2500 ) );

			EspOther->AddElement( new Checkbox( _( "modulate molotov" ), &g_Options.visuals.world.ModulateFire ) );
			EspOther->AddElement( new Slider( _( "hue" ), &g_Options.visuals.world.FireHue, SliderType::Sld_Right, 0, 7 ) );
		}
		else if ( m_Other == 1 )
		{
			EspOther->AddElement( new Checkbox( _( "3d crosshair" ), &g_Options.visuals.view.DimCrosshair ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.view.DimColor, &g_Options.visuals.view.DimHue ) );
			EspOther->AddElement( new Slider( _( "size" ), &g_Options.visuals.view.DimSize, SliderType::Sld_Right, 1, 15 ) );

			EspOther->AddElement( new Checkbox( _( "grenade esp" ), &g_Options.visuals.view.NadeEsp ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.view.NadeColor, &g_Options.visuals.view.NadeEspHue ) );

			EspOther->AddElement( new Checkbox( _( "grenade prediction" ), &g_Options.visuals.view.grenade_prediction ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.view.nade_line, &g_Options.visuals.view.nade_hue ) );

			if ( g_Options.visuals.view.grenade_prediction )
			{
				EspOther->AddElement( new Checkbox( _( "corners" ), &g_Options.visuals.view.corners ) );
				EspOther->AddElement( new ColorPicker( &g_Options.visuals.view.corn_clr, &g_Options.visuals.view.corn_hue ) );
			}

			EspOther->AddElement( new Checkbox( _( "thirdperson" ), &g_Options.visuals.misc_.thirdperson ) );
			EspOther->AddElement( new Bind( &g_Options.visuals.misc_.key_tp ) );
			EspOther->AddElement( new Slider( _( "distance" ), &g_Options.visuals.misc_.tp_dist, SliderType::Sld_Right, 0, 180 ) );

			EspOther->AddElement( new Checkbox( _( "spread crosshair" ), &g_Options.visuals.misc_.SpreadCross ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.misc_.Spread_Clr, &g_Options.visuals.misc_.Spread_hue, true ) );
			EspOther->AddElement( new Combo( _( "style" ), &g_Options.visuals.misc_.m_CrossType, m_Spread, 140 ) );

			EspOther->AddElement( new Checkbox( _( "bind list" ), &g_Options.visuals.misc_.keybind_list ) );
			
			auto m_LogsCombo = new Multi( _( "logs output" ), 140 );
			m_LogsCombo->AddItem( _( "hit" ), &g_Options.visuals.misc_.LogsOut[ 0 ] );
			m_LogsCombo->AddItem( _( "hurt" ), &g_Options.visuals.misc_.LogsOut[ 1 ] );
			m_LogsCombo->AddItem( _( "buy" ), &g_Options.visuals.misc_.LogsOut[ 2 ] );
			m_LogsCombo->AddItem( _( "misses" ), &g_Options.visuals.misc_.LogsOut[ 3 ] );
			EspOther->AddElement( m_LogsCombo );
		}
		else if ( m_Other == 2 )
		{
			EspOther->AddElement( new Text( _( "enemy chams (visible)" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.enemy_chams, &g_Options.visuals.colors.enemy_chams_hue, true ) );
			EspOther->AddElement( new Text( _( "enemy chams (invisible)" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.enemy_chams_xqz, &g_Options.visuals.colors.enemy_chams_hue_xqz, true ) );
			EspOther->AddElement( new Text( _( "enemy illuminate" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.enemy_illuminate, &g_Options.visuals.colors.enemy_illuminate_hue, true ) );
			EspOther->AddElement( new Text( _( "enemy glow" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.enemy_glow, &g_Options.visuals.colors.enemy_glow_hue ) );
			EspOther->AddElement( new Text( _( "enemy box" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.vis_enemy.Box_Clr, &g_Options.visuals.vis_enemy.Box_hue ) );
			EspOther->AddElement( new Text( _( "background frame" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.vis_enemy.BackColor, &g_Options.visuals.vis_enemy.BackHue, true ) );
			EspOther->AddElement( new Text( _( "background line" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.vis_enemy.BackLineColor, &g_Options.visuals.vis_enemy.BackLineHue, true ) );
			EspOther->AddElement( new Text( _( "local chams (visible)" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.local_chams, &g_Options.visuals.colors.local_chams_hue, true ) );
			EspOther->AddElement( new Text( _( "local illuminate" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.local_illuminate, &g_Options.visuals.colors.local_illuminate_hue, true ) );
			EspOther->AddElement( new Text( _( "local glow" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.local_glow, &g_Options.visuals.colors.local_glow_hue ) );
			EspOther->AddElement( new Text( _( "fake chams" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.desync_chams, &g_Options.visuals.colors.desync_chams_hue, true ) );
			EspOther->AddElement( new Text( _( "fake illuminate" ) ) );
			EspOther->AddElement( new ColorPicker( &g_Options.visuals.colors.desync_illuminate, &g_Options.visuals.colors.desync_illuminate_hue, true ) );
		}
	}

	delete EspOptions; delete EspActivate;
	delete EspOther;
}

void Gui::Tabs::SetupMisc( Window* m_Window, int m_Tab )
{
	static int m_ToSw;

	std::vector<std::string> m_Null = { _( "none" ) };
	std::vector<std::string> m_Lags = { _( "factor" ), _( "adaptive" ) };
	std::vector<std::string> m_Strafe = { _( "none" ), _( "default" ), _( "rage" ) };
	std::vector<std::string> m_Slots = { _( "slot 1" ), _( "slot 2" ), _( "slot 3" ), _( "slot 4" ), _( "slot 5" ), _( "slot 6" ), _( "slot 7" ), _( "slot 8" ), _( "slot 9" ), _( "slot 10" ), _( "slot 11" ), _( "slot 12" ), _( "slot 13" ), _( "slot 14" ), _( "slot 15" ) };
	std::vector<std::string> m_VisualsSlots = { _( "default" ), _( "slot 1" ), _( "slot 2" ), _( "slot 3" ), _( "slot 4" ), _( "slot 5" ), _( "slot 6" ), _( "slot 7" ), _( "slot 8" ), _( "slot 9" ), _( "slot 10" ), _( "slot 11" ), _( "slot 12" ), _( "slot 13" ), _( "slot 14" ), _( "slot 15" ) };
	std::vector<std::string> m_MasterSwitch = { _( "rage" ), _( "legit" ) };
	std::vector<std::string> m_Soun = { _( "none" ), _( "arena switch" ), _( "clock" ) };
	std::vector<std::string> m_CfgVector = { _( "main" ), _( "skins" ) };

	auto MiscMain = new Groupbox( _( "other" ), 10, 180, 60, 290, 415, m_Tab );
	{
		m_Window->AddGroup( MiscMain );

		auto m_Subs = new SubTab( { 286, 20 }, &m_ToSw );
		m_Subs->AddTab( _( "main" ) );
		m_Subs->AddTab( _( "advanced" ) );
		m_Subs->AddTab( _( "custom fonts" ) );
		MiscMain->AddElement( m_Subs );

		if ( m_ToSw == 0 )
		{
			MiscMain->AddElement( new Checkbox( _( "unlimit duck" ), &g_Options.movement.unlimitDuck ) );
			MiscMain->AddElement( new Checkbox( _( "fake duck" ), &g_Options.movement.fakeduck ) );
			MiscMain->AddElement( new Bind( &g_Options.movement.m_FdKey ) );
			MiscMain->AddElement( new Checkbox( _( "auto peek" ), &g_Options.visuals.misc_.auto_peek ) );
			MiscMain->AddElement( new Bind( &g_Options.visuals.misc_.peek_key ) );
			MiscMain->AddElement( new Slider( _( "coloring" ), &g_Options.visuals.misc_.peek_hue, SliderType::Sld_Right, 0, 255 ) );
			MiscMain->AddElement( new Checkbox( _( "bunny hop" ), &g_Options.movement.bhop ) );
			MiscMain->AddElement( new Combo( _( "auto strafe" ), &g_Options.movement.autostrafe, m_Strafe, 140 ) );
			MiscMain->AddElement( new Checkbox( _( "slowwalk" ), &g_Options.movement.slowwalk ) );
			MiscMain->AddElement( new Bind( &g_Options.movement.m_SlowKey ) );
			MiscMain->AddElement( new Slider( _( "value" ), &g_Options.movement.slow_speed, SliderType::Sld_Right, 0, 100 ) );
			MiscMain->AddElement( new Checkbox( _( "force aspect ratio" ), &g_Options.visuals.misc_.force_aspect ) );
			MiscMain->AddElement( new Slider( _( "value" ), &g_Options.visuals.misc_.aspect_rat, SliderType::Sld_Right, 1, 100 ) );
		}
		else if ( m_ToSw == 1 )
		{
			MiscMain->AddElement( new Checkbox( _( "watermark" ), &g_Options.visuals.misc_.watermark ) );
			MiscMain->AddElement( new Checkbox( _( "something wrong" ), &g_Options.visuals.misc_.smth_wrong ) );
			MiscMain->AddElement( new Combo( _( "hitsound" ), &g_Options.visuals.misc_.hitsound, m_Soun, 140 ) );
			MiscMain->AddElement( new Combo( _( "master switch" ), &g_Options.MasterSwitch, m_MasterSwitch, 140 ) );
		}
		else if ( m_ToSw == 2 )
		{
			MiscMain->AddElement( new Combo( _( "font slot" ), &g_Options.CustomFonts.m_CurrentSlot, m_Slots, 140 ) );
			MiscMain->AddElement( new TextInput( _( "font name" ), &g_Options.CustomFonts.m_FontName ) );
			MiscMain->AddElement( new TextInput( _( "font size" ), &g_Options.CustomFonts.m_FontSize ) );
			MiscMain->AddElement( new TextInput( _( "font weight" ), &g_Options.CustomFonts.m_FontWeight ) );

			auto m_MultiS = new Multi( _( "font flags" ), 140 );
			m_MultiS->AddItem( _( "antialiasis" ), &g_Options.CustomFonts.m_FlagAA );
			m_MultiS->AddItem( _( "drop shadow" ), &g_Options.CustomFonts.m_FlagDropShadow );
			m_MultiS->AddItem( _( "outline" ), &g_Options.CustomFonts.m_FlagOutline );
			MiscMain->AddElement( m_MultiS );

			MiscMain->AddElement( new Button( _( "add" ), [ ]( ) { Gui::m_Tabs.CreateFontFromParse( ); }, { 5, 0 }, { 140, 15 } ) );
			MiscMain->AddElement( new Button( _( "remove" ), [ ]( ) { Gui::m_Tabs.RemoveFont( ); }, { 5, 25 }, { 140, 15 } ) );

			MiscMain->AddEmpty( 50, 50 );

			MiscMain->AddElement( new Combo( _( "player name font" ), &g_Options.CustomVisualsFont.NameFont, m_VisualsSlots, 140 ) );
			MiscMain->AddElement( new Combo( _( "player weapon font" ), &g_Options.CustomVisualsFont.WeaponFont, m_VisualsSlots, 140 ) );
			MiscMain->AddElement( new Combo( _( "player hp number font" ), &g_Options.CustomVisualsFont.HpNumberFont, m_VisualsSlots, 140 ) );
			MiscMain->AddElement( new Combo( _( "world weapon name font" ), &g_Options.CustomVisualsFont.WorldNameFont, m_VisualsSlots, 140 ) );
			MiscMain->AddElement( new Combo( _( "logs font" ), &g_Options.CustomVisualsFont.LogsFont, m_VisualsSlots, 140 ) );
		}
	}

	auto GroupLag = new Groupbox( _( "fake lag" ), 11, 496, 60, 290, 100, m_Tab );
	{
		m_Window->AddGroup( GroupLag );

		GroupLag->AddElement( new Checkbox( _( "enable" ), &g_Options.fakelag.Activate ) );
		GroupLag->AddElement( new Combo( _( "mode" ), &g_Options.fakelag.Mode, m_Lags, 140 ) );
		GroupLag->AddElement( new Slider( _( "limit" ), &g_Options.fakelag.Amount, SliderType::Sld_Defaut, 1, 14 ) );
	}

	auto GroupManager = new Groupbox( _( "config manager" ), 14, 496, 180, 290, 80, m_Tab );
	{
		m_Window->AddGroup( GroupManager );

		GroupManager->AddElement( new Combo( _( "display at list" ), &m_CurrentDisplay, m_CfgVector, 140 ) );
		GroupManager->AddElement( new Combo( _( "display at combo" ), &m_CurrentDisplayAtLeftSide, m_CfgVector, 140 ) );
	}

	auto GroupConfig = new Groupbox( _( "configs" ), 13, 496, 280, 290, 195, m_Tab );
	{
		m_Window->AddGroup( GroupConfig );

		if ( m_CurrentDisplay == 0 )
		{
			GroupConfig->AddElement( new Button( _( "add" ), [ ]( ) { Gui::m_Control.AddConfig( ); }, { 155, 3 }, { 114, 15 } ) );
			GroupConfig->AddElement( new TextInput( "", &g_Options.new_config_name ) );
			GroupConfig->AddElement( new ListBox( "", &g_Options.selected_config, g_ConfigMaster.files.size( ) <= 0 ? m_Null : g_ConfigMaster.files, 0, 0 ) );
			GroupConfig->AddEmpty( 3, 3 );
			GroupConfig->AddElement( new Button( _( "load" ), [ ]( ) { Gui::m_Control.LoadConfig( ); }, { 5, 3 }, { 150, 15 } ) );
			GroupConfig->AddElement( new Button( _( "save" ), [ ]( ) { Gui::m_Control.SaveConfig( ); }, { 165, 3 }, { 105, 15 } ) );
		}
		else
		{
			GroupConfig->AddElement( new Button( _( "add" ), [ ]( ) { Gui::m_Control.AddConfigSkin( ); }, { 155, 3 }, { 114, 15 } ) );
			GroupConfig->AddElement( new TextInput( "", &g_Options.new_config_skin_name ) );
			GroupConfig->AddElement( new ListBox( "", &g_Options.selected_config_at_skin, g_ConfigMaster.skin_files.size( ) <= 0 ? m_Null : g_ConfigMaster.skin_files, 0, 0 ) );
			GroupConfig->AddEmpty( 3, 3 );
			GroupConfig->AddElement( new Button( _( "load" ), [ ]( ) { Gui::m_Control.LoadConfigSkin( ); }, { 5, 3 }, { 150, 15 } ) );
			GroupConfig->AddElement( new Button( _( "save" ), [ ]( ) { Gui::m_Control.SaveConfigSkin( ); }, { 165, 3 }, { 105, 15 } ) );
		}
	}

	delete MiscMain; delete GroupLag;
	delete GroupConfig; delete GroupManager;
}

void Gui::Tabs::SetupConfig( Window* m_Window, int m_Tab )
{
	std::vector<std::string> m_Null = { _( "none" ) };

	auto MainConfig = new Groupbox( "", 90, 22, 370, 0, 0, m_Tab, Groupbox::Flags::GROUP_NO_DRAW );
	{
		m_Window->AddGroup( MainConfig );

		if ( m_CurrentDisplayAtLeftSide == 0 )
		{
			MainConfig->AddElement( new Combo( "", &g_Options.selected_config, g_ConfigMaster.files.size( ) <= 0 ? m_Null : g_ConfigMaster.files, 90 ) );
			MainConfig->AddElement( new Button( _( "load" ), [ ]( ) { Gui::m_Control.LoadConfig( ); }, { 5, -2 }, { 90, 18 } ) );
			MainConfig->AddElement( new Button( _( "save" ), [ ]( ) { Gui::m_Control.SaveConfig( ); }, { 5, 21 }, { 90, 18 } ) );
		}
		else
		{
			MainConfig->AddElement( new Combo( "", &g_Options.selected_config_at_skin, g_ConfigMaster.skin_files.size( ) <= 0 ? m_Null : g_ConfigMaster.skin_files, 90 ) );
			MainConfig->AddElement( new Button( _( "load" ), [ ]( ) { Gui::m_Control.LoadConfigSkin( ); }, { 5, -2 }, { 90, 18 } ) );
			MainConfig->AddElement( new Button( _( "save" ), [ ]( ) { Gui::m_Control.SaveConfigSkin( ); }, { 5, 21 }, { 90, 18 } ) );
		}
	}

	delete MainConfig;
}

void Gui::Tabs::SetColors( )
{
	m_Details.m_DefaultColor = g_Options.GUI_CLRS.Default_Clr;
}

void Gui::Tabs::ResetColors( )
{
	g_Options.GUI_CLRS.Default_Clr = Color( 200, 45, 90, 255 );

	g_Options.GUI_CLRS.Frame_1_Clr = Color( 37, 37, 37, 255 );
	g_Options.GUI_CLRS.Frame_2_Clr = Color( 45, 45, 45, 255 );
	g_Options.GUI_CLRS.Frame_3_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.Frame_4_Clr = Color( 37, 37, 37, 255 );
	g_Options.GUI_CLRS.Frame_5_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.Frame_6_Clr = Color( 37, 37, 37, 255 );
	g_Options.GUI_CLRS.NameWater_Clr = Color( 170, 170, 170, 255 );
	g_Options.GUI_CLRS.ElementsGradient_1_Clr = Color( 0, 0, 0, 55 );
	g_Options.GUI_CLRS.ElementsGradient_2_Clr = Color( 45, 45, 45, 55 );
	g_Options.GUI_CLRS.ElementsText_Clr = Color( 170, 170, 170, 255 );
	g_Options.GUI_CLRS.GroupBoxOuter_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.GroupBoxOuter_2_Clr = Color( 45, 45, 45, 255 );
	g_Options.GUI_CLRS.SelectorOuter_Clr = Color( 37, 37, 37, 255 );
	g_Options.GUI_CLRS.SelectorHovered_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.SelectorSelectedText_Clr = Color( 65, 65, 65, 255 );
	g_Options.GUI_CLRS.ColorPickerFrame_1_Clr = Color( 45, 45, 45, 255 );
	g_Options.GUI_CLRS.ColorPickerFrame_2_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.TabText_Clr = Color( 215, 215, 215, 255 );
	g_Options.GUI_CLRS.TabTextHovered_Clr = Color( 110, 110, 110, 255 );
	g_Options.GUI_CLRS.SubtabFilled_Clr = Color( 185, 185, 185, 255 );
	g_Options.GUI_CLRS.ListBoxLine_Clr = Color( 60, 60, 60, 255 );
	g_Options.GUI_CLRS.ListBoxLine_2_Clr = Color( 27, 27, 27, 255 );
	g_Options.GUI_CLRS.ColorPickerFrame_3_Clr = Color( 160, 160, 160, 255 );
	g_Options.GUI_CLRS.ColorPickerOutline_Clr = Color( 255, 255, 255, 255 );

	g_Options.GUI_CLRS.Default_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_1_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_2_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_3_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_4_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_5_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.Frame_6_Clr_Hue = 0.f;
	g_Options.GUI_CLRS.NameWater_Hue = 0.f;
	g_Options.GUI_CLRS.ElementsGradient_1_Hue = 0.f;
	g_Options.GUI_CLRS.ElementsGradient_2_Hue = 0.f;
	g_Options.GUI_CLRS.ElementsText_Hue = 0.f;
	g_Options.GUI_CLRS.GroupBoxOuter_Hue = 0.f;
	g_Options.GUI_CLRS.GroupBoxOuter_2_Hue = 0.f;
	g_Options.GUI_CLRS.SelectorOuter_Hue = 0.f;
	g_Options.GUI_CLRS.SelectorHovered_Hue = 0.f;
	g_Options.GUI_CLRS.SelectorSelectedText_Hue = 0.f;
	g_Options.GUI_CLRS.ColorPickerFrame_1_Hue = 0.f;
	g_Options.GUI_CLRS.ColorPickerFrame_2_Hue = 0.f;
	g_Options.GUI_CLRS.TabText_Hue = 0.f;
	g_Options.GUI_CLRS.TabTextHovered_Hue = 0.f;
	g_Options.GUI_CLRS.SubtabFilled_Hue = 0.f;
	g_Options.GUI_CLRS.ListBoxLine_Hue = 0.f;
	g_Options.GUI_CLRS.ListBoxLine_2_Hue = 0.f;
	g_Options.GUI_CLRS.ColorPickerFrame_3_Hue = 0.f;
	g_Options.GUI_CLRS.ColorPickerOutline_Hue = 0.f;
}

void Gui::Tabs::SetupGui( Window* m_Window, int m_Tab )
{
	m_Tabs.SetColors( );
	static int m_Swtch;

	auto GuiUpdate = new Groupbox( "", 370, 494, 35, 0, 0, m_Tab, Groupbox::Flags::GROUP_NO_DRAW );
	{
		m_Window->AddGroup( GuiUpdate );
		GuiUpdate->AddElement( new Button( _( "reset colors" ), [ ]( ) { m_Tabs.ResetColors( ); }, { 0, 0 }, { 100, 20 } ) );
	}

	auto GuiMain = new Groupbox( _( "colors" ), 311, 180, 60, 290, 415, m_Tab );
	{
		m_Window->AddGroup( GuiMain );

		GuiMain->AddElement( new Text( _( "global color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Default_Clr, &g_Options.GUI_CLRS.Default_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 1 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_1_Clr, &g_Options.GUI_CLRS.Frame_1_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 2 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_2_Clr, &g_Options.GUI_CLRS.Frame_2_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 3 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_3_Clr, &g_Options.GUI_CLRS.Frame_3_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 4 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_4_Clr, &g_Options.GUI_CLRS.Frame_4_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 5 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_5_Clr, &g_Options.GUI_CLRS.Frame_5_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "frame 6 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.Frame_6_Clr, &g_Options.GUI_CLRS.Frame_6_Clr_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "gui watermarks color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.NameWater_Clr, &g_Options.GUI_CLRS.NameWater_Hue, true ), true );

		GuiMain->AddElement( new Text( _( "gradient first color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ElementsGradient_1_Clr, &g_Options.GUI_CLRS.ElementsGradient_1_Hue, true ), true );

		GuiMain->AddElement( new Text( _( "gradient second color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ElementsGradient_2_Clr, &g_Options.GUI_CLRS.ElementsGradient_2_Hue, true ), true );

		GuiMain->AddElement( new Text( _( "elements text color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ElementsText_Clr, &g_Options.GUI_CLRS.ElementsText_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "elements frame 1 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.SelectorOuter_Clr, &g_Options.GUI_CLRS.SelectorOuter_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "elements frame 2 color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.SelectorHovered_Clr, &g_Options.GUI_CLRS.SelectorHovered_Hue, false ), true );

		GuiMain->AddElement( new Text( _( "elemens hovered color" ) ) );
		GuiMain->AddElement( new ColorPicker( &g_Options.GUI_CLRS.SelectorSelectedText_Clr, &g_Options.GUI_CLRS.SelectorSelectedText_Hue, true ), true );
	}

	auto GuiCol2 = new Groupbox( _( "colors" ), 315, 496, 90, 290, 385, m_Tab );
	{
		m_Window->AddGroup( GuiCol2 );

		GuiCol2->AddElement( new Text( _( "groupbox frame 1 color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.GroupBoxOuter_Clr, &g_Options.GUI_CLRS.GroupBoxOuter_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "groupbox frame 2 color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.GroupBoxOuter_2_Clr, &g_Options.GUI_CLRS.GroupBoxOuter_2_Hue, false ), true );

		GuiCol2->AddElement( new Text( "сolorpicker frame 1 color" ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ColorPickerFrame_1_Clr, &g_Options.GUI_CLRS.ColorPickerFrame_1_Hue, false ), true );

		GuiCol2->AddElement( new Text( "сolorpicker frame 2 color" ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ColorPickerFrame_2_Clr, &g_Options.GUI_CLRS.ColorPickerFrame_2_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "tab color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.TabTextHovered_Clr, &g_Options.GUI_CLRS.TabTextHovered_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "tab arrow color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ColorPickerFrame_3_Clr, &g_Options.GUI_CLRS.ColorPickerFrame_3_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "selected tab color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.TabText_Clr, &g_Options.GUI_CLRS.TabText_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "hovered tab color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.SubtabFilled_Clr, &g_Options.GUI_CLRS.SubtabFilled_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "listbox frame 1 color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ListBoxLine_Clr, &g_Options.GUI_CLRS.ListBoxLine_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "listbox frame 2 color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ListBoxLine_2_Clr, &g_Options.GUI_CLRS.ListBoxLine_2_Hue, false ), true );

		GuiCol2->AddElement( new Text( _( "combo arrow color" ) ) );
		GuiCol2->AddElement( new ColorPicker( &g_Options.GUI_CLRS.ColorPickerOutline_Clr, &g_Options.GUI_CLRS.ColorPickerOutline_Hue, false ), true );
	}

	delete GuiMain; delete GuiCol2;
	delete GuiUpdate;
}

struct hud_weapons_t {
	std::int32_t* get_weapon_count( ) {
		return reinterpret_cast< std::int32_t* >(std::uintptr_t( this ) + 0x80);
	}
};

template<class T>
static T* FindHudElement( const char* name )
{
	static auto pThis = *reinterpret_cast< DWORD** >(Utils::FindSig( _( "client.dll" ), _( "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08" ) ) + 1);

	static auto find_hud_element = reinterpret_cast< DWORD( __thiscall* )(void*, const char*) >(Utils::FindSig( _( "client.dll" ), _( "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28" ) ));
	return ( T* )find_hud_element( pThis, name );
}

void Gui::Tabs::UpdateSkins( )
{
	g_UpdateSkins = true;

	[[ DATA ]]
	{
		auto& m_FromCfg = g_Options.changers.skin.m_items_clr;
		auto m_ID = CTX::m_RedirectSkin;
		auto m_Next = k_skins[ m_ID ].id;

		int SetupFirstColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].First.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].First.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].First.r( ));

		int SetupSecondColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Second.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Second.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Second.r( ));

		int SetupThirdColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Third.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Third.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Third.r( ));

		int SetupFourthColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Fourth.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Fourth.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Fourth.r( ));

		for ( size_t m{ 0 }; m < memory.itemSchema( )->paintKits.lastElement; m++ )
		{
			auto pK = memory.itemSchema( )->paintKits.memory[ m ].value;

			if ( pK->nID == m_Next )
			{
				if ( m_FromCfg[ m_Next ].ShouldUse0 ) {
					pK->color1 = SetupFirstColor;
				}
				else
					pK->color1 = k_skin_clr[ m ].clr[ 0 ];

				if ( m_FromCfg[ m_Next ].ShouldUse1 ) {
					pK->color2 = SetupSecondColor;
				}
				else
					pK->color2 = k_skin_clr[ m ].clr[ 1 ];

				if ( m_FromCfg[ m_Next ].ShouldUse2 ) {
					pK->color3 = SetupThirdColor;
				}
				else
					pK->color3 = k_skin_clr[ m ].clr[ 2 ];

				if ( m_FromCfg[ m_Next ].ShouldUse3 ) {
					pK->color4 = SetupThirdColor;
				}
				else
					pK->color4 = k_skin_clr[ m ].clr[ 3 ];
			}
		}
	}

	typedef void(*ForceUpdate) (void);
	static ForceUpdate FullUpdate = ( ForceUpdate )Utils::FindSig( _( "engine.dll" ), _( "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85" ) );
	FullUpdate( );

	g_UpdateSkins = false;
}

void Gui::Tabs::SetupSkins( Window* m_Window, int m_Tab )
{
	static int definition_vector_index;
	static int def_skin_index;
	static int def_glove_index;

	auto& entries = g_Options.changers.skin.m_items;
	auto& selected_entry = entries[ k_weapon_names[ definition_vector_index ].definition_index ];
	auto& selected_entry_unchecked = entries[ k_weapon_names[ 0 ].definition_index ];

	selected_entry.definition_index = k_weapon_names[ definition_vector_index ].definition_index;
	selected_entry.definition_vector_index = definition_vector_index;

	std::vector<std::string> m_Weapons;
	std::vector<std::string> m_Knife;
	std::vector<std::string> m_Gloves;
	std::vector<std::string> m_Skins;
	std::vector<std::string> m_GloveSkins;

	bool m_Init = false;
	static std::string m_Filter[ Gui::m_Maximum ];

	if ( skins_parsed )
	{
		if ( !m_Init )
		{
			for ( size_t w = 0; w < k_weapon_names.size( ); w++ )
				m_Weapons.push_back( k_weapon_names[ w ].name );

			for ( size_t w = 0; w < k_knife_names.size( ); w++ ) {
				std::string name = k_knife_names.at( w ).name;
				std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
				m_Knife.push_back( name );
			}

			for ( size_t w = 0; w < k_glove_names.size( ); w++ ) {
				std::string name = k_glove_names.at( w ).name;
				std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
				m_Gloves.push_back( name );
			}

			for ( size_t w = 0; w < k_skins.size( ); w++ ) {
				for ( auto names : k_skins[ w ].weaponName ) {
					std::string name = k_skins[ w ].name;
					std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
					m_Skins.push_back( name.c_str( ) );
				}
			}

			for ( size_t w = 0; w < k_gloves.size( ); w++ ) {
				for ( auto names : k_gloves[ w ].weaponName ) {
					std::string name = k_gloves[ w ].name;
					std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
					m_GloveSkins.push_back( name.c_str( ) );
				}
			}

			m_Init = true;
		}
	}

	if ( !m_Init )
		return;

	bool is_knife = selected_entry.definition_index == 42 || selected_entry.definition_index == 59;
	bool is_glove = selected_entry.definition_index == 5028 || selected_entry.definition_index == 5029;

	if ( selected_entry.definition_index == 42 || selected_entry.definition_index == 59 )
		selected_entry.definition_override_index = k_knife_names.at( selected_entry.definition_override_vector_index ).definition_index;

	else if ( selected_entry.definition_index == 5028 || selected_entry.definition_index == 5029 )
		selected_entry.definition_override_index = k_glove_names.at( selected_entry.definition_override_vector_index ).definition_index;

	else
		selected_entry.definition_override_index = 0;

	static int m_ReasonToUpdate[ 10 ];

	if ( m_ReasonToUpdate[ 0 ] != selected_entry.definition_override_vector_index ||
		m_ReasonToUpdate[ 2 ] != def_skin_index ||
		m_ReasonToUpdate[ 1 ] != def_glove_index ||
		m_ReasonToUpdate[ 3 ] != g_Options.changers.enable_skins ||
		m_ReasonToUpdate[ 4 ] != selected_entry.definition_index ||
		m_ReasonToUpdate[ 5 ] != selected_entry.use )
	{
		m_ReasonToUpdate[ 1 ] = def_glove_index;
		m_ReasonToUpdate[ 2 ] = def_skin_index;
		m_ReasonToUpdate[ 0 ] = selected_entry.definition_override_vector_index;
		m_ReasonToUpdate[ 3 ] = g_Options.changers.enable_skins;
		m_ReasonToUpdate[ 4 ] = selected_entry.definition_index;
		m_ReasonToUpdate[ 5 ] = selected_entry.use;

		m_Tabs.UpdateSkins( );
	}

	auto WeaponsTab = new Groupbox( _( "weapons" ), 110, 180, 90, 290, 60, m_Tab );
	{
		m_Window->AddGroup( WeaponsTab );
		WeaponsTab->AddElement( new Combo( _( "list" ), &definition_vector_index, m_Weapons, 160 ) );
	}

	auto StateTab = new Groupbox( _( "state" ), 112, 180, 170, 290, 75, m_Tab );
	{
		m_Window->AddGroup( StateTab );
		StateTab->AddElement( new Checkbox( _( "default" ), &selected_entry.use ) );

		if ( is_knife || is_glove )
			StateTab->AddElement( new Combo( is_knife ? _( "knife" ) : _( "gloves" ), &selected_entry.definition_override_vector_index, is_knife ? m_Knife : m_Gloves, 160 ) );
	}

	auto ManageTab = new Groupbox( _( "management" ), 113, 180, 265, 290, 60 /* 275 */, m_Tab );
	{
		m_Window->AddGroup( ManageTab );
		ManageTab->AddElement( new TextInput( _( "filter" ), &m_Filter[ selected_entry.definition_index ] ) );
	}

	auto SkinsTab = new Groupbox( _( "paint" ), 111, 496, 60, 290, 415, m_Tab );
	{
		m_Window->AddGroup( SkinsTab );
		SkinsTab->AddEmpty( 5, 5 );

		if ( !is_glove )
		{
			if ( m_Skins.size( ) )
				SkinsTab->AddElement( new ListBoxAlternative( "", &selected_entry.paint_kit_vector_index, &def_skin_index, is_glove ? m_GloveSkins : m_Skins, m_Filter[ selected_entry.definition_index ], definition_vector_index + 1000, 1 ) );

			if ( selected_entry.use )
				selected_entry.paint_kit_index = -1;
			else
				selected_entry.paint_kit_index = k_skins[ selected_entry.paint_kit_vector_index ].id;
		}
		else
		{
			if ( m_GloveSkins.size( ) )
				SkinsTab->AddElement( new ListBoxAlternative( "", &selected_entry.paint_kit_vector_index, &def_glove_index, is_glove ? m_GloveSkins : m_Skins, m_Filter[ selected_entry.definition_index ], 8999, 1 ) );

			if ( selected_entry.use )
				selected_entry.paint_kit_index = -1;
			else
				selected_entry.paint_kit_index = k_gloves[ selected_entry.paint_kit_vector_index ].id;
		}
	}

	CTX::m_RedirectSkin = def_skin_index;

	if ( !is_glove )
	{
		auto& m_FromCfg = g_Options.changers.skin.m_items_clr;
		
		auto m_ID = def_skin_index;
		auto m_Next = k_skins[ m_ID ].id;

		auto AdvanceTab = new Groupbox( _( "skin modification" ), 115, 180, 345, 290, 130, m_Tab );
		{
			m_Window->AddGroup( AdvanceTab );

			AdvanceTab->AddElement( new Checkbox( _( "first color" ),
				&m_FromCfg[ m_Next ].ShouldUse0 ) );

			AdvanceTab->AddElement(
				new ColorPicker( &m_FromCfg[ m_Next ].First,
					&m_FromCfg[ m_Next ].FirstHue ) );

			AdvanceTab->AddElement( new Checkbox( _( "second color" ),
				&m_FromCfg[ m_Next ].ShouldUse1 ) );

			AdvanceTab->AddElement(
				new ColorPicker( &m_FromCfg[ m_Next ].Second,
					&m_FromCfg[ m_Next ].SecondHue ) );

			AdvanceTab->AddElement( new Checkbox( _( "third color" ),
				&m_FromCfg[ m_Next ].ShouldUse2 ) );

			AdvanceTab->AddElement(
				new ColorPicker( &m_FromCfg[ m_Next ].Third,
					&m_FromCfg[ m_Next ].ThirdHue ) );

			AdvanceTab->AddElement( new Checkbox( _( "fourth color" ),
				&m_FromCfg[ m_Next ].ShouldUse3 ) );

			AdvanceTab->AddElement(
				new ColorPicker( &m_FromCfg[ m_Next ].Fourth,
					&m_FromCfg[ m_Next ].FourthHue ) );
		}

		int SetupFirstColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].First.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].First.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].First.r( ));

		int SetupSecondColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Second.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Second.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Second.r( ));

		int SetupThirdColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Third.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Third.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Third.r( ));

		int SetupFourthColor = 0xFF000000 + (( int )(m_FromCfg[ m_Next ].Fourth.b( )) << 16) |
			(( int )(m_FromCfg[ m_Next ].Fourth.g( )) << 8)
			| ( int )(m_FromCfg[ m_Next ].Fourth.r( ));

		for ( size_t m{ 0 }; m < memory.itemSchema( )->paintKits.lastElement; m++ )
		{
			auto pK = memory.itemSchema( )->paintKits.memory[ m ].value;

			if ( pK->nID == m_Next )
			{
				if ( m_FromCfg[ m_Next ].ShouldUse0 ) {
					pK->color1 = SetupFirstColor;
				}
				else
					pK->color1 = k_skin_clr[ m ].clr[ 0 ];

				if ( m_FromCfg[ m_Next ].ShouldUse1 ) {
					pK->color2 = SetupSecondColor;
				}
				else
					pK->color2 = k_skin_clr[ m ].clr[ 1 ];

				if ( m_FromCfg[ m_Next ].ShouldUse2 ) {
					pK->color3 = SetupThirdColor;
				}
				else
					pK->color3 = k_skin_clr[ m ].clr[ 2 ];

				if ( m_FromCfg[ m_Next ].ShouldUse3 ) {
					pK->color4 = SetupThirdColor;
				}
				else
					pK->color4 = k_skin_clr[ m ].clr[ 3 ];
			}
		}

		delete AdvanceTab;
	}

	auto GrActivate = new Groupbox( "", 135, 180, 30, 0, 0, m_Tab, Groupbox::Flags::GROUP_NO_DRAW );
	{
		m_Window->AddGroup( GrActivate );
		GrActivate->AddElement( new Checkbox( _( "active" ), &g_Options.changers.enable_skins ) );
	}

	m_Weapons.clear( );
	m_Knife.clear( );
	m_Skins.clear( );
	m_Gloves.clear( );
	m_GloveSkins.clear( );

	delete WeaponsTab; delete SkinsTab;
	delete StateTab; delete ManageTab;
	delete GrActivate;
}

Vector2D m_StartPos = Vector2D( 0, 0 );
std::vector<std::string> m_GuiTabs = { _( "A" ), _( "C" ), _( "B" ), _( "E" ), _( "F" ), _( "H" ), _( "G" ) };

void Gui::Details::Install( )
{
	m_Input.PollInput( );

	if ( m_Input.KeyPressed( VK_INSERT ) ) {
		SetMenuState( !GetMenuState( ) );
		g_InputSystem->EnableInput( !GetMenuState( ) );
	}

	if ( !GetMenuState( ) )
		return;

	m_Control.SetIndex( 0 );

	m_Control.m_Opened[ m_Control.ControlType::COMBO ] = -1;
	m_Control.m_Opened[ m_Control.ControlType::MULTICOMBO ] = -1;

	g_Options.ragebot.Acitve = g_Options.ragebot.Enable;

	auto m_Window = new Window( _( "uwu [closed beta]" ), &m_StartPos, { 845, 530 }, m_GuiTabs, &m_Index );
	{
		for ( int i = 0; i < 7; i++ )
			m_Tabs.SetupConfig( m_Window, i );

		m_Tabs.SetupRage( m_Window, 0 );
		m_Tabs.SetupLegit( m_Window, 1 );
		m_Tabs.SetupVisual( m_Window, 2 );
		m_Tabs.SetupMisc( m_Window, 3 );

		if ( m_Index == 4 )
			m_Tabs.SetupSkins( m_Window, 4 );

		m_Tabs.SetupGui( m_Window, 5 );
		m_Tabs.SetupLua( m_Window, 6 );
	}

	delete m_Window;
	m_External.Install( );
}