#include "antiaim.h"
#include "../sdk/utils/math.hpp"

#include <algorithm>
#include "../config/config.h"

bool CAntiAim::Condition( CUserCmd* m_pcmd, bool dynamic_check )
{
	if ( g_LocalPlayer->m_bGunGameImmunity( ) || g_LocalPlayer->m_fFlags( ) & FL_FROZEN )
		return true;

	if ( g_LocalPlayer->m_nMoveType( ) == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType( ) == MOVETYPE_LADDER )
		return true;

	auto weapon = LocalPlayerData::m_Weapon;

	if ( m_pcmd->buttons & IN_ATTACK && weapon->m_Item( ).m_iItemDefinitionIndex( ) != WEAPON_REVOLVER && !weapon->IsKnifeIDX( ) && !weapon->IsGrenadeIDX( ) )
		return true;

	auto revolver_shoot = weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_REVOLVER && (m_pcmd->buttons & IN_ATTACK || m_pcmd->buttons & IN_ATTACK2);

	if ( revolver_shoot )
		return true;

	if ( (m_pcmd->buttons & IN_ATTACK || m_pcmd->buttons & IN_ATTACK2) && weapon->IsKnifeIDX( ) )
		return true;

	if ( dynamic_check && m_pcmd->buttons & IN_USE )
		return true;

	if ( dynamic_check && weapon->IsGrenadeIDX( ) && weapon->m_fThrowTime( ) )
		return true;

	return false;
}

float CAntiAim::At_target( CUserCmd* m )
{
	int cur_tar = 0;
	float last_dist = 999999999999.0f;

	for ( int i = 0; i < g_GlobalVars->maxClients; i++ ) {
		auto entity = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( i ));

		if ( !entity || entity->EntIndex( ) == g_LocalPlayer->EntIndex( ) )
			continue;

		if ( !entity->IsPlayer( ) )
			continue;

		auto m_player = ( C_BasePlayer* )entity;
		if ( !m_player->IsDormant( ) && m_player->IsAlive( ) && m_player->m_iTeamNum( ) != g_LocalPlayer->m_iTeamNum( ) ) {
			float cur_dist = (entity->m_vecOrigin( ) - g_LocalPlayer->m_vecOrigin( )).Length( );

			if ( !cur_tar || cur_dist < last_dist ) {
				cur_tar = i;
				last_dist = cur_dist;
			}
		}
	}

	if ( cur_tar ) {
		auto entity = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( cur_tar ));
		if ( !entity ) {
			return m->viewangles.yaw;
		}

		Vector target_angle = Math::CalcAng( g_LocalPlayer->m_vecOrigin( ), entity->m_vecOrigin( ) );
		return target_angle.y;
	}

	return m->viewangles.yaw;
}

static bool m_Flip;
void CAntiAim::OnCreateMove( CUserCmd* m )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	auto m_Weapon = LocalPlayerData::m_Weapon;

	if ( !m_Weapon )
		return;

	if ( Condition( m ) )
		return;

	if ( CTX::m_SendPacket )
		m_Flip = !m_Flip;

	switch ( g_Options.antiaim.Pitch )
	{
	case 1:
		m->viewangles.pitch = -89.9f;
		break;
	case 2:
		m->viewangles.pitch = 89.9f;
		break;
	case 3:
		m->viewangles.pitch = 0.f;
		break;
	case 4:
		m->viewangles.pitch = Math::RandomFloat( -89.f, 89.f );
		break;
	}

	if ( g_Options.antiaim.BaseYaw == 1 ) {
		m->viewangles.yaw = g_LocalPlayer->GetVAngles( )->yaw;
	}
	else if ( g_Options.antiaim.BaseYaw == 2 )
		m->viewangles.yaw = At_target( m );

	switch ( g_Options.antiaim.Yaw )
	{
	case 1:
		m->viewangles.yaw += 180.f;
		break;
	case 2:
		m->viewangles.yaw = -90.f;
		break;
	}

	if ( g_Options.antiaim.JitterMode == 1 ) {
		m_Flip ? m->viewangles.yaw -= g_Options.antiaim.JitterValue : m->viewangles.yaw += g_Options.antiaim.JitterValue;
	}
	else if ( g_Options.antiaim.JitterMode == 2 ) {
		m->viewangles.yaw += Math::RandomFloat( -g_Options.antiaim.JitterValue, g_Options.antiaim.JitterValue );
	}

	bool m_State = GetKeyState( g_Options.antiaim.KeyInvert );
	m->viewangles.yaw += m_State ? g_Options.antiaim.YawAddInvert : g_Options.antiaim.YawAdd;

	static float m_Side;
	m_Side = g_Options.antiaim.JiterSideFake == 1 ? (m_State ? m_Flip ? -1 : 1 : m_Flip ? 1 : -1) : m_State ? 1 : -1;

	float m_Amount =
		m_State ? g_Options.antiaim.FakeYawValueInvert : g_Options.antiaim.FakeYawValue;

	if ( g_Options.antiaim.FakeYaw == 1 )
	{
		if ( fabsf( m->forwardmove ) < 5.0f ) {
			if ( m->buttons & IN_DUCK )
				m->forwardmove = g_GlobalVars->tickcount & 1 ? 3.25f : -3.25f;
			else
				m->forwardmove = g_GlobalVars->tickcount & 1 ? 1.1f : -1.1f;
		}

		if ( !CTX::m_SendPacket )
			m->viewangles.yaw -= m_Amount * m_Side;
	}
	else if ( g_Options.antiaim.FakeYaw == 2 )
	{
		if ( !CTX::m_SendPacket )
			m->viewangles.yaw -= m_Amount * m_Side;
	}
}