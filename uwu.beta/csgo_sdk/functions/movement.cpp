#include "movement.h"
#include "../config/config.h"

#include "../sdk/utils/math.hpp"
#include "autowall.h"

#include "ragebot.h"

void CMovement::OnCreateMove( CUserCmd* m )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	if ( g_Options.movement.bhop )
		BHOP( m );
}

void CMovement::BHOP( CUserCmd* cmd )
{
	static auto last_jumped = false;
	static auto should_fake = false;

	if ( g_LocalPlayer->m_nMoveType( ) != MOVETYPE_LADDER && g_LocalPlayer->m_nMoveType( ) != MOVETYPE_NOCLIP )
	{
		if ( !last_jumped && should_fake )
		{
			should_fake = false;
			cmd->buttons |= IN_JUMP;
		}
		else if ( cmd->buttons & IN_JUMP )
		{
			if ( g_Options.movement.autostrafe > 0 )
				Strafe( cmd );

			if ( g_LocalPlayer->m_fFlags( ) & FL_ONGROUND )
			{
				last_jumped = true;
				should_fake = true;
			}
			else
			{
				cmd->buttons &= ~IN_JUMP;
				last_jumped = false;
			}
		}
		else
		{
			last_jumped = false;
			should_fake = false;
		}
	}
}

void CMovement::Strafe( CUserCmd* cmd )
{
	if ( g_Options.movement.autostrafe == 1 )
	{
		cmd->forwardmove = (10000.f / g_LocalPlayer->m_vecVelocity( ).Length2D( ) > 450.f) ? 450.f : 10000.f / g_LocalPlayer->m_vecVelocity( ).Length2D( );
		cmd->sidemove = (cmd->mousedx != 0) ? (cmd->mousedx < 0.0f) ? -450.f : 450.f : (cmd->command_number % 2) == 0 ? -450.f : 450.f;
	}
	else if ( g_Options.movement.autostrafe == 2 )
	{
		float yaw = m_ViewAngleStored.yaw;

		if ( cmd->buttons & IN_BACK ) {
			if ( cmd->buttons & IN_MOVERIGHT ) {
				yaw -= 125;
			}
			else if ( cmd->buttons & IN_MOVELEFT ) {
				yaw += 125;
			}
			else
			{
				yaw += 180;
			}
		}
		else if ( cmd->buttons & IN_MOVERIGHT ) {
			if ( cmd->buttons & IN_FORWARD ) {
				yaw -= 45;
			}
			else
			{
				yaw -= 90;
			}
		}
		else if ( cmd->buttons & IN_MOVELEFT ) {
			if ( cmd->buttons & IN_FORWARD ) {
				yaw += 45;
			}
			else
			{
				yaw += 90;
			}
		}
		else
		{
			cmd->buttons |= IN_FORWARD;
		}

		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;

		const auto delta = remainderf( yaw -
			RAD2DEG( atan2( g_LocalPlayer->m_vecVelocity( ).y, g_LocalPlayer->m_vecVelocity( ).x ) ), 360.f );

		cmd->sidemove = delta > 0.f ? -450.f : 450.f;
		m_ViewAngleStored.yaw = remainderf( yaw - delta, 360.f );
	}
}

void CMovement::MouseFix( CUserCmd* cmd )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	static ConVar* m_yaw = m_yaw = g_CVar->FindVar( "m_yaw" );
	static ConVar* m_pitch = m_pitch = g_CVar->FindVar( "m_pitch" );
	static ConVar* sensitivity = sensitivity = g_CVar->FindVar( "sensitivity" );

	static QAngle m_angOldViewangles = g_ClientState->viewangles;

	float delta_x = std::remainderf( cmd->viewangles.pitch - m_angOldViewangles.pitch, 360.0f );
	float delta_y = std::remainderf( cmd->viewangles.yaw - m_angOldViewangles.yaw, 360.0f );

	if ( delta_x != 0.0f ) {
		float mouse_y = -((delta_x / m_pitch->GetFloat( )) / sensitivity->GetFloat( ));
		short mousedy;
		if ( mouse_y <= 32767.0f ) {
			if ( mouse_y >= -32768.0f ) {
				if ( mouse_y >= 1.0f || mouse_y < 0.0f ) {
					if ( mouse_y <= -1.0f || mouse_y > 0.0f )
						mousedy = static_cast< short >(mouse_y);
					else
						mousedy = -1;
				}
				else {
					mousedy = 1;
				}
			}
			else {
				mousedy = 0x8000u;
			}
		}
		else {
			mousedy = 0x7FFF;
		}

		cmd->mousedy = mousedy;
	}

	if ( delta_y != 0.0f ) {
		float mouse_x = -((delta_y / m_yaw->GetFloat( )) / sensitivity->GetFloat( ));
		short mousedx;
		if ( mouse_x <= 32767.0f ) {
			if ( mouse_x >= -32768.0f ) {
				if ( mouse_x >= 1.0f || mouse_x < 0.0f ) {
					if ( mouse_x <= -1.0f || mouse_x > 0.0f )
						mousedx = static_cast< short >(mouse_x);
					else
						mousedx = -1;
				}
				else {
					mousedx = 1;
				}
			}
			else {
				mousedx = 0x8000u;
			}
		}
		else {
			mousedx = 0x7FFF;
		}

		cmd->mousedx = mousedx;
	}
}

void CMovement::SlowWalk( CUserCmd* cmd )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	m_InSlow = false;

	if ( !(g_LocalPlayer->m_fFlags( ) & FL_ONGROUND) )
		return;

	if ( !g_Options.movement.slowwalk )
		return;

	if ( !GetAsyncKeyState( g_Options.movement.m_SlowKey ) )
		return;

	auto weapon = LocalPlayerData::m_Weapon;
	if ( !weapon )
		return;

	auto data = weapon->GetCSWeaponData( );
	if ( !data )
		return;

	cmd->buttons &= ~IN_WALK;
	m_InSlow = true;

	auto speed = data->flMaxPlayerSpeed *
		0.32f * 0.5f
		* g_Options.movement.slow_speed / 100;

	const auto min_speed = FastSqrt( cmd->forwardmove * cmd->forwardmove + cmd->sidemove * cmd->sidemove + cmd->upmove * cmd->upmove );

	if ( min_speed <= 0.f )
		return;

	if ( min_speed <= speed )
		return;

	const auto final_speed = speed / min_speed;

	cmd->forwardmove *= final_speed;
	cmd->sidemove *= final_speed;
	cmd->upmove *= final_speed;
}

void CMovement::FakeDuck( CUserCmd* cmd )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	m_InDuck = false;

	if ( cmd->buttons & IN_JUMP || !(g_LocalPlayer->m_fFlags( ) & FL_ONGROUND) || !g_Options.movement.fakeduck || !GetAsyncKeyState( g_Options.movement.m_FdKey ) )
		return;

	m_InDuck = true;

	if (g_ClientState->m_nChokedCommands <= 6)
		cmd->buttons &= ~IN_DUCK;
	else
		cmd->buttons |= IN_DUCK;
}

bool CMovement::LagPeek( CUserCmd* cmd )
{
	return false;
}

bool IsZero( Vector m_Rec )
{
	return m_Rec.x == 0.0f && m_Rec.y == 0.0f
		&& m_Rec.z == 0.0f;
}

void CMovement::AutoPeek( CUserCmd* cmd, float wish_yaw )
{
	auto m_Weapon = LocalPlayerData::m_Weapon;

	if ( !m_Weapon )
		return;

	bool m_NonAim = m_Weapon->IsKnifeIDX( ) || m_Weapon->IsGrenadeIDX( )
		|| m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_HEALTHSHOT || m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_TASER;

	if ( !m_NonAim && g_Options.visuals.misc_.auto_peek && GetKeyState( g_Options.visuals.misc_.peek_key ) )
	{
		if ( IsZero( m_StartPos ) )
		{
			m_StartPos = g_LocalPlayer->GetAbsOrigin( );

			if ( !(g_LocalPlayer->m_fFlags( ) & FL_ONGROUND) )
			{
				Ray_t ray;
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				ray.Init( m_StartPos, m_StartPos - Vector( 0.0f, 0.0f, 1000.0f ) );
				g_EngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

				if ( trace.fraction < 1.0f )
					m_StartPos = trace.endpos + Vector( 0.0f, 0.0f, 2.0f );
			}
		}
		else
		{
			if ( cmd->buttons & IN_ATTACK )
				m_FiredShot = true;

			if ( m_FiredShot )
			{
				auto current_position = g_LocalPlayer->GetAbsOrigin( );
				auto difference = current_position - m_StartPos;

				if ( difference.Length2D( ) > 5.0f )
				{
					auto velocity = Vector( difference.x * cos( wish_yaw / 180.0f * M_PI ) + difference.y * sin( wish_yaw / 180.0f * M_PI ), difference.y * cos( wish_yaw / 180.0f * M_PI ) - difference.x * sin( wish_yaw / 180.0f * M_PI ), difference.z );

					cmd->forwardmove = -velocity.x * 20.0f;
					cmd->sidemove = velocity.y * 20.0f;
				}
				else
				{
					m_FiredShot = false;
					m_StartPos.Zero( );
				}
			}
		}
	}
	else
	{
		m_FiredShot = false;
		m_StartPos.Zero( );
	}
}