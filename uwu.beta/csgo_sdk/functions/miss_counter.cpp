#include "miss_counter.h"
#include "../sdk/utils/math.hpp"

#include "../config/config.h"
#include "logs.h"

// REWORK

void MissCount::Setup_Event( IGameEvent* event )
{
	if ( !strcmp( event->GetName( ), ("bullet_impact") ) )
	{
		const auto pos = Vector( event->GetFloat( ("x") ), event->GetFloat( ("y") ), event->GetFloat( ("z") ) );
		const auto userid = g_EngineClient->GetPlayerForUserID( event->GetInt( ("userid") ) );

		if ( userid == g_EngineClient->GetLocalPlayer( ) && g_LocalPlayer )
		{
			static auto last_shot_from = g_LocalPlayer->GetEyePos( );
			static auto last_curtime = g_GlobalVars->curtime;

			if ( last_curtime != g_GlobalVars->curtime )
			{
				last_shot_from = g_LocalPlayer->GetEyePos( );
				last_curtime = g_GlobalVars->curtime;
			}
		}
		else
		{
			C_BasePlayer* pla = dynamic_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( userid ));

			if ( pla && pla->IsAlive( ) && !pla->IsDormant( ) )
			{
				if ( pla->m_iTeamNum( ) != g_LocalPlayer->m_iTeamNum( ) )
				{
					static auto last_shot_from = pla->GetEyePos( );
					static auto last_curtime = g_GlobalVars->curtime;

					if ( last_curtime != g_GlobalVars->curtime )
					{
						last_shot_from = pla->GetEyePos( );
						last_curtime = g_GlobalVars->curtime;
					}
				}
				else
				{
					static auto last_shot_from = pla->GetEyePos( );
					static auto last_curtime = g_GlobalVars->curtime;

					if ( last_curtime != g_GlobalVars->curtime )
					{
						last_shot_from = pla->GetEyePos( );
						last_curtime = g_GlobalVars->curtime;
					}
				}
			}
		}

		const auto size = last_events.size( );

		if ( size == 0 )
		{
			event_data_t d;
			d.hurt_player = g_GlobalVars->curtime - last_rbot_shot_time < 2.f ? last_rbot_entity : -1;
			d.pos = pos;
			d.userid = userid;
			d.hurt = false;
			d.time = g_GlobalVars->curtime;
			d.got_pos = true;
			last_events.push_back( d );
		}
		else
		{
			if ( last_events[ size - 1 ].time == g_GlobalVars->curtime && last_events[ size - 1 ].hurt_player == last_rbot_entity )
				last_events[ size - 1 ].pos = pos;
			else
			{
				event_data_t d;
				d.hurt_player = g_GlobalVars->curtime - last_rbot_shot_time < 2.f ? last_rbot_entity : -1;
				d.pos = pos;
				d.userid = userid;
				d.hurt = false;
				d.time = g_GlobalVars->curtime;
				last_events.push_back( d );
			}
		}
	}

	static auto last_hurt_curtime = 0.f;
	static auto last_hurt_attacker = -1;
	static auto last_hurt_userid = -1;
	static auto last_hurt_damage = -1;
	static auto last_hurt_health = -1;
	static auto last_hurt_hitgroup = 0;

	if ( !strcmp( event->GetName( ), ("player_hurt") ) )
	{
		const auto attacker = g_EngineClient->GetPlayerForUserID( event->GetInt( ("attacker") ) );
		const auto hurt = g_EngineClient->GetPlayerForUserID( event->GetInt( ("userid") ) );
		const auto health = event->GetInt( ("health") );
		const auto dmg_health = event->GetInt( ("dmg_health") );
		const auto hitgroup = event->GetInt( ("hitgroup") );

		if ( last_hurt_curtime == g_GlobalVars->curtime && last_hurt_attacker == attacker
			&& last_hurt_userid == hurt && last_hurt_damage == dmg_health
			&& last_hurt_health == health && last_hurt_hitgroup == hitgroup )
			return;

		last_hurt_curtime = g_GlobalVars->curtime;
		last_hurt_attacker = attacker;
		last_hurt_userid = hurt;
		last_hurt_damage = dmg_health;
		last_hurt_health = health;
		last_hurt_hitgroup = hitgroup;

		const auto weapon = event->GetString( ("weapon") );

		if ( weapon == ("inferno") || weapon == ("hegrenade") || weapon == ("decoy") )
			return;

		for ( auto& last_event : last_events )
		{
			if ( last_event.userid == attacker && (last_event.hurt_player == hurt || last_event.hurt_player == -1) )
			{
				last_event.hurt = true;
				last_event.damage = dmg_health;
				last_event.died = health <= 0;
				last_event.got_hurt = true;
				last_event.hitbox = hitgroup;
			}
		}
	}

}

void MissCount::SetData( C_BasePlayer* player, int index, Vector angle )
{
	if ( !player )
		return;

	last_rbot_entity = index;
	last_rbot_shot_time = g_GlobalVars->curtime;
	last_rbot_shot_eyepos = LocalPlayerData::m_EyePosition;
	last_rbot_shot_angle = angle;
}

void MissCount::Build_( )
{
	if ( !g_LocalPlayer )
	{
		last_events = std::deque< event_data_t >( );
		return;
	}

	for ( auto& last_event : last_events )
	{
		if ( !last_event.got_pos )
			continue;

		auto player = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( last_event.userid ));

		if ( !player )
			continue;

		if ( last_event.hurt_player == -1 )
			last_event.hurt_player = last_rbot_entity;

		auto player_hurt = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( last_event.hurt_player ));

		if ( player == g_LocalPlayer )
		{
			if ( player_hurt && player_hurt->IsAlive( ) && last_event.hurt_player != -1 )
			{
				auto trace_hit_player = false;

				trace_t trace;
				Ray_t ray;
				Vector view;
				const auto angles = Math::CalcAng( last_rbot_shot_eyepos, last_event.pos );
				Math::AngVec( angles, view );
				view.NormalizeInPlace( );

				view = last_rbot_shot_eyepos + view * 4096.f;

				ray.Init( last_rbot_shot_eyepos, view );
				CTraceFilterPlayersOnlySkipOne filter( g_LocalPlayer );
				filter.ShouldHitEntity( player_hurt, 0 );

				g_EngineTrace->TraceRay( ray, MASK_ALL | CONTENTS_GRATE, &filter, &trace );

				if ( trace.hit_entity )
					trace_hit_player = true;

				if ( !last_event.hurt && g_GlobalVars->curtime - last_rbot_shot_time < 2.f && !GetAsyncKeyState( VK_LBUTTON ) )
				{
					const auto fov = Math::Get_fov( last_rbot_shot_angle, Math::CalcAng( last_rbot_shot_eyepos, last_event.pos ) );

					if ( !trace_hit_player && fov >= 0.01f && fov < 30.f ) {

						m_MissedShotSpread[ player_hurt->EntIndex( ) ]++;

						if ( g_Options.visuals.misc_.LogsOut[ 3 ] )
							Logs::Get( ).AddLog( "Missed shot due to spread", Color::White );
					}
					else {
						m_MissedShot[ player_hurt->EntIndex( ) ]++;

						std::string m_Proto = "";
						m_Proto = "Missed shot due to resolver";

						
						if ( g_Options.visuals.misc_.LogsOut[ 3 ] )
							Logs::Get( ).AddLog( m_Proto, Color::White );
					}
				}
			}
		}
	}

	last_events = std::deque< event_data_t >( );
}