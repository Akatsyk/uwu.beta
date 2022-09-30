#include "events.h"
#include "miss_counter.h"

#include "logs.h"
#include "../config/config.h"
#include "visuals.h"

char* HitgroupToName( int hitgroup )
{
	switch ( hitgroup )
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_CHEST:
		return "chest";
	case HITGROUP_LEFTARM:
		return "arm";
	case HITGROUP_RIGHTARM:
		return "arm";
	case HITGROUP_LEFTLEG:
		return "leg";
	case HITGROUP_RIGHTLEG:
		return "leg";
	case HITGROUP_STOMACH:
		return "stomach";
	default:
		return "body";
	}
}

void C_HookedEvents::FireGameEvent( IGameEvent* pEvent )
{
	if ( !g_EngineClient->IsConnected( ) || !g_EngineClient->IsInGame( ) || !g_LocalPlayer )
		return;

	/*if (!g_LocalPlayer->IsAlive())
	{
		for (int i = 0; i < g_GlobalVars->maxClients; i++)
			m_MissedShot[i] = 0;
	}*/

	if ( !pEvent )
		return;

	static auto GetHGName = [ ]( int hitgroup ) -> std::string {
		switch ( hitgroup ) {
		case HITGROUP_HEAD:
			return "head";
		case HITGROUP_LEFTLEG:
			return "leg";
		case HITGROUP_RIGHTLEG:
			return "leg";
		case HITGROUP_STOMACH:
			return "stomach";
		case HITGROUP_LEFTARM:
			return "arm";
		case HITGROUP_RIGHTARM:
			return "arm";
		default:
			return "body";
		}
	};

	static auto hit_sound = [ &pEvent ]( ) -> void
	{
		auto attacker = pEvent->GetInt( "attacker" );
		auto user = pEvent->GetInt( "userid" );

		if ( g_EngineClient->GetPlayerForUserID( user ) != g_EngineClient->GetLocalPlayer( ) &&
			g_EngineClient->GetPlayerForUserID( attacker ) == g_EngineClient->GetLocalPlayer( ) ) {
			switch ( g_Options.visuals.misc_.hitsound ) {
			case 1:
				g_VGuiSurface->PlaySound_( "buttons\\arena_switch_press_02.wav" );
				break;
			case 2:
				g_VGuiSurface->PlaySound_( "physics\\metal\\paintcan_impact_hard3.wav" );
				break;
			}
		}
	};

	auto name = pEvent->GetName( );

	if ( g_LocalPlayer )
	{
		if ( strstr( pEvent->GetName( ), "player_hurt" ) ) {

			hit_sound( );

			auto
				userid = pEvent->GetInt( "userid" ),
				attacker = pEvent->GetInt( "attacker" );

			if ( !userid || !attacker ) {
				return;
			}

			auto
				userid_id = g_EngineClient->GetPlayerForUserID( userid ),
				attacker_id = g_EngineClient->GetPlayerForUserID( attacker );

			player_info_t userid_info, attacker_info;
			if ( !g_EngineClient->GetPlayerInfo( userid_id, &userid_info ) ) {
				return;
			}

			if ( !g_EngineClient->GetPlayerInfo( attacker_id, &attacker_info ) ) {
				return;
			}

			auto m_victim = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( userid_id ));

			std::stringstream ss;
			if ( attacker_id == g_EngineClient->GetLocalPlayer( ) )
			{
				if ( !g_Options.visuals.misc_.LogsOut[ 0 ] )
					return;

				auto damage = pEvent->GetInt( "dmg_health" );
				if ( !damage )
					return;

				auto hitbox = pEvent->GetInt( "hitgroup" );
				if ( !hitbox )
					return;

				auto hitgroup = HitgroupToName( hitbox );
				auto health = pEvent->GetInt( "health" );

				ss << "Hit " << userid_info.szName << " in the " << GetHGName( pEvent->GetInt( "hitgroup" ) ) << " for " << pEvent->GetInt( "dmg_health" ) << " damage";
				ss << " (" << std::clamp( m_victim->m_iHealth( ) - pEvent->GetInt( "dmg_health" ), 0, 100 ) << " health remaining)";

				Logs::Get( ).AddLog( ss.str( ), Color::White );
			}
			else if ( userid_id == g_EngineClient->GetLocalPlayer( ) ) {

				if ( !g_Options.visuals.misc_.LogsOut[ 1 ] )
					return;

				auto damage = pEvent->GetInt( "dmg_health" );
				if ( !damage )
					return;

				auto hitbox = pEvent->GetInt( "hitgroup" );
				if ( !hitbox )
					return;
				auto hitgroup = HitgroupToName( hitbox );
				auto health = pEvent->GetInt( "health" );

				ss << "Hurt" << " by " << attacker_info.szName << " in the " << GetHGName( pEvent->GetInt( "hitgroup" ) ) << " for " << pEvent->GetInt( "dmg_health" ) << " damage ";
				Logs::Get( ).AddLog( ss.str( ), Color::White );
			}
		}

		if ( g_Options.visuals.misc_.LogsOut[ 2 ] && strstr( pEvent->GetName( ), "item_purchase" ) ) {
			auto userid = pEvent->GetInt( "userid" );

			if ( !userid ) {
				return;
			}

			auto userid_id = g_EngineClient->GetPlayerForUserID( userid );

			player_info_t userid_info;
			if ( !g_EngineClient->GetPlayerInfo( userid_id, &userid_info ) ) {
				return;
			}

			auto m_player = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( userid_id ));

			if ( !m_player ) {
				return;
			}

			if ( m_player->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) ) {
				return;
			}

			std::stringstream ss;

			std::string ss_23;
			std::string ssweapon;

			ssweapon = pEvent->GetString( "weapon" );
			ss_23 = userid_info.szName;

			std::transform( ss_23.begin( ), ss_23.end( ), ss_23.begin( ), ::toupper );
			std::transform( ssweapon.begin( ), ssweapon.end( ), ssweapon.begin( ), ::toupper );

			ss << ss_23 << " Purchased a(n) " << ssweapon;
			Logs::Get( ).AddLog( ss.str( ), Color::White );
		}
	}

	if ( g_LocalPlayer )
	{
		if ( !strcmp( name, "round_prestart" ) || !strcmp( name, "round_start" ) ) {
			CSGO::m_Packets.clear( );
		}

		if ( !strcmp( name, "round_start" ) ) {
			for ( auto i = 1; i < g_GlobalVars->maxClients; i++ )
			{
				Visuals::Get( ).esp_alpha_fade[ i ] = 0.0f;
				Visuals::Get( ).health[ i ] = 100;
				DormantEsp::Get( ).m_cSoundPlayers[ i ].reset( );
			}
		}
	}

	if ( g_Options.ragebot.Acitve && g_Options.ragebot.Enable )
	{
		if ( !strcmp( name, "round_prestart" ) ) {
			for ( int i = 0; i < g_GlobalVars->maxClients; i++ )
				m_MissedShot[ i ] = 0;
		}

		if ( !strcmp( name, "round_end" ) ) {
			for ( int i = 0; i < g_GlobalVars->maxClients; i++ )
				m_MissedShot[ i ] = 0;
		}

		if ( !strcmp( name, "weapon_fire" ) && !GetAsyncKeyState( VK_LBUTTON ) ) {
			int iUser = pEvent->GetInt( "userid" );

			if ( g_EngineClient->GetPlayerForUserID( iUser ) != g_EngineClient->GetLocalPlayer( ) )
				return;

			MissCount::Get( ).dequeBulletImpacts.clear( );
		}

		if ( !strcmp( name, "player_hurt" ) )
		{
			auto
				userid = pEvent->GetInt( "userid" ),
				attacker = pEvent->GetInt( "attacker" );

			if ( !userid || !attacker ) {
				return;
			}

			auto
				userid_id = g_EngineClient->GetPlayerForUserID( userid ),
				attacker_id = g_EngineClient->GetPlayerForUserID( attacker );

			player_info_t userid_info, attacker_info;
			if ( !g_EngineClient->GetPlayerInfo( userid_id, &userid_info ) ) {
				return;
			}

			if ( !g_EngineClient->GetPlayerInfo( attacker_id, &attacker_info ) ) {
				return;
			}

			auto m_victim = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( userid_id ));

			if ( attacker_id == g_EngineClient->GetLocalPlayer( ) && m_RageCurrentTargetIndex ) {
				MissCount::Get( ).bPlayerHurt[ m_RageCurrentTargetIndex ] = true;
			}
		}

		if ( !strcmp( name, "bullet_impact" ) ) {
			int iUser = pEvent->GetInt( "userid" );

			if ( g_EngineClient->GetPlayerForUserID( iUser ) != g_EngineClient->GetLocalPlayer( ) )
				return;

			MissCount::Get( ).dequeBulletImpacts.push_back( Vector( pEvent->GetFloat( "x" ), pEvent->GetFloat( "y" ), pEvent->GetFloat( "z" ) ) );

			if ( m_RageCurrentTargetIndex )
				MissCount::Get( ).bBulletImpact[ m_RageCurrentTargetIndex ] = true;
		}

		MissCount::Get( ).Setup_Event( pEvent );
	}
}

int C_HookedEvents::GetEventDebugID( void ) {
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf( )
{
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	g_GameEvents->AddListener( this, "player_footstep", false );
	g_GameEvents->AddListener( this, "player_hurt", false );
	g_GameEvents->AddListener( this, "player_death", false );
	g_GameEvents->AddListener( this, "weapon_fire", false );
	g_GameEvents->AddListener( this, "item_purchase", false );
	g_GameEvents->AddListener( this, "bullet_impact", false );
	g_GameEvents->AddListener( this, "round_start", false );
	g_GameEvents->AddListener( this, "round_freeze_end", false );
	g_GameEvents->AddListener( this, "bomb_defused", false );
	g_GameEvents->AddListener( this, "bomb_begindefuse", false );
	g_GameEvents->AddListener( this, "bomb_beginplant", false );
}

void C_HookedEvents::RemoveSelf( )
{
	g_GameEvents->RemoveListener( this );
}