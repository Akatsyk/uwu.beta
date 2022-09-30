#include "legitbot.h"
#include "../sdk/utils/math.hpp"

#include <algorithm>
#include "../config/config.h"
#include "autowall.h"

CLegitbot::CLegitbot( )
{
	current_punch = last_punch = { 0, 0, 0 };
	target = nullptr;

	lastShotTick = shotsFired = 0;

	shot_delay_time = kill_delay_time = 0;
	shot_delay = kill_delay = false;
}

bool CanSeePlayer( C_BasePlayer* player, const Vector& pos )
{
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;

	ray.Init( LocalPlayerData::m_EyePosition, pos );
	g_EngineTrace->TraceRay( ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr );

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

void CLegitbot::Update( )
{
	auto weapon = LocalPlayerData::m_Weapon;

	if ( !weapon )
		return;

	auto id = weapon->m_Item( ).m_iItemDefinitionIndex( );
	static int element;

	if ( id == WEAPON_AWP )
		element = 0;

	else if ( id == WEAPON_SSG08 )
		element = 1;

	else if ( id == WEAPON_DEAGLE || id == WEAPON_REVOLVER )
		element = 2;

	else if ( id == WEAPON_G3SG1 || id == WEAPON_SCAR20 )
		element = 3;

	else if ( weapon->IsRfileIDX( ) )
		element = 4;

	else if ( weapon->IsSmgIDX( ) )
		element = 5;

	else if ( weapon->IsShotgunIDX( ) )
		element = 6;

	else if ( weapon->IsPistolIDX( ) )
		element = 7;

	m_config.enabled = g_Options.m_legit[ element ].enabled;
	m_config.flash = g_Options.m_legit[ element ].flash_check;
	m_config.smoke = g_Options.m_legit[ element ].smoke_check;
	m_config.auto_pistol = g_Options.m_legit[ element ].auto_pistol;

	m_config.hitboxes.head = g_Options.m_legit[ element ].hitbox.head;
	m_config.hitboxes.chest = g_Options.m_legit[ element ].hitbox.chest;
	m_config.hitboxes.hands = g_Options.m_legit[ element ].hitbox.hands;
	m_config.hitboxes.legs = g_Options.m_legit[ element ].hitbox.legs;

	m_config.fov = g_Options.m_legit[ element ].fov;
	m_config.smooth = g_Options.m_legit[ element ].smooth;

	m_config.silent_fov = g_Options.m_legit[ element ].silent_fov;
	m_config.silent_mode = g_Options.m_legit[ element ].silent_mode;

	m_config.shot_delay = g_Options.m_legit[ element ].shot_delay;
	m_config.kill_delay = g_Options.m_legit[ element ].kill_delay;

	m_config.rcs = g_Options.m_legit[ element ].rcs;
	m_config.rcs_mode = g_Options.m_legit[ element ].rcs_mode;

	m_config.rcs_x = g_Options.m_legit[ element ].rcs_x;
	m_config.rcs_y = g_Options.m_legit[ element ].rcs_y;

	m_config.awall = g_Options.m_legit[ element ].awall;
	m_config.awall_damage = g_Options.m_legit[ element ].awall_damage;
	m_config.vis = g_Options.m_legit[ element ].vis;
}

float CLegitbot::GetFovToPlayer( QAngle viewAngle, QAngle aimAngle )
{
	QAngle delta = aimAngle - viewAngle;
	return sqrtf( powf( delta.pitch, 2.0f ) + powf( delta.yaw, 2.0f ) );
}

bool CLegitbot::IsLineGoesThroughSmoke( Vector startPos, Vector endPos )
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector, Vector))Utils::FindSig( "client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0" );
	return LineGoesThroughSmokeFn( startPos, endPos );
}

bool CLegitbot::IsEnabled( CUserCmd* cmd )
{
	auto weapon = LocalPlayerData::m_Weapon;
	if ( !weapon || weapon->IsGrenadeIDX( ) || weapon->IsKnifeIDX( ) ||
		weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER || weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_HEALTHSHOT )
		return false;

	Update( );

	if ( !m_config.enabled )
		return false;

	if ( !(weapon->m_iClip1( ) > 0) && !weapon->IsReloading( ) )
		return false;

	return (cmd->buttons & IN_ATTACK) || (m_config.awall && GetAsyncKeyState( g_Options.awall_key ));
}

void CLegitbot::Smooth( QAngle currentAngle, QAngle aimAngle, QAngle& angle )
{
	auto smooth_value = std::max( 1.0f, m_config.smooth );

	Vector current, aim;

	Math::AngleVectors( currentAngle, current );
	Math::AngleVectors( aimAngle, aim );

	const Vector delta = aim - current;
	const Vector smoothed = current + delta / smooth_value;

	Math::VectorAngles( smoothed, angle );
}

void CLegitbot::RCS( QAngle& angle, C_BasePlayer* target )
{
	if ( !m_config.rcs )
		return;

	if ( shotsFired < m_config.start - 1 )
		return;

	if ( !m_config.rcs_x && !m_config.rcs_y )
		return;

	static auto recoil_scale = g_CVar->FindVar( "weapon_recoil_scale" );
	auto scale = recoil_scale->GetFloat( );

	const auto x = float( m_config.rcs_x ) / 100.f * scale;
	const auto y = float( m_config.rcs_y ) / 100.f * scale;

	QAngle punch = { 0, 0, 0 };

	if ( target )
		punch = { current_punch.pitch * x, current_punch.yaw * y, 0 };

	else if ( m_config.rcs_mode == 0 )
		punch = { (current_punch.pitch - last_punch.pitch) * x, (current_punch.yaw - last_punch.yaw) * y, 0 };

	if ( (punch.pitch != 0.f || punch.yaw != 0.f) && current_punch.roll == 0.f ) {
		angle -= punch;
	}
}

bool CLegitbot::IsSilent( )
{
	if ( m_config.silent_mode == 2 )
		return !(shotsFired > 0 || !m_config.silent_mode || !m_config.silent_fov);

	if ( m_config.silent_mode == 1 )
		return !(!m_config.silent_mode || !m_config.silent_fov);

	if ( m_config.silent_mode == 0 )
		return !(shotsFired > 0 || !m_config.silent_mode || !m_config.silent_fov);
}

float CLegitbot::GetFov( )
{
	if ( IsSilent( ) )
		return m_config.silent_fov;

	return m_config.fov;
}

C_BasePlayer* CLegitbot::GetClosestPlayer( CUserCmd* cmd, int& bestBone, float& bestFov, QAngle& bestAngles )
{
	if ( target && !kill_delay && m_config.kill_delay > 0 && !target->IsAlive( ) )
	{
		target = nullptr;
		kill_delay = true;
		kill_delay_time = int( GetTickCount( ) ) + m_config.kill_delay;
	}
	if ( kill_delay )
	{
		if ( kill_delay_time <= int( GetTickCount( ) ) )
			kill_delay = false;
		else
			return nullptr;
	}

	target = nullptr;

	std::vector<int> hitboxes;

	if ( m_config.hitboxes.head )
		hitboxes.emplace_back( HITBOX_HEAD );

	if ( m_config.hitboxes.chest )
	{
		hitboxes.emplace_back( HITBOX_UPPER_CHEST );
		hitboxes.emplace_back( HITBOX_CHEST );
		hitboxes.emplace_back( HITBOX_LOWER_CHEST );
	}

	if ( m_config.hitboxes.hands )
	{
		hitboxes.emplace_back( 18 );
		hitboxes.emplace_back( 14 );
		hitboxes.emplace_back( 17 );

		hitboxes.emplace_back( 16 );
		hitboxes.emplace_back( 13 );
		hitboxes.emplace_back( 15 );
	}

	if ( m_config.hitboxes.legs )
	{
		hitboxes.emplace_back( 12 );
		hitboxes.emplace_back( 10 );
		hitboxes.emplace_back( 8 );

		hitboxes.emplace_back( 11 );
		hitboxes.emplace_back( 9 );
		hitboxes.emplace_back( 7 );
	}

	auto eyePos = LocalPlayerData::m_EyePosition;

	for ( int i = 1; i <= g_EngineClient->GetMaxClients( ); ++i )
	{
		C_BasePlayer* player = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( i ));

		if ( !player
			|| !player->IsAlive( )
			|| player->IsDormant( )
			|| player->m_bGunGameImmunity( ) 
			|| !player->IsPlayer( ) )
		{
			continue;
		}

		if ( player == g_LocalPlayer )
			continue;

		if ( player->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) )
			continue;

		if ( player->m_fFlags( ) & FL_FROZEN )
			continue;

		for ( const auto hitbox : hitboxes )
		{
			Vector hitboxPos = player->GetHitboxPos( hitbox );
			QAngle ang;
			Math::VectorAngles( hitboxPos - eyePos, ang );
			const float fov = GetFovToPlayer( cmd->viewangles + last_punch * 2.f, ang );

			if ( fov > GetFov( ) )
				continue;

			if ( m_config.awall )
			{
				auto info = Autowall::Get( ).CanHit(LocalPlayerData::m_EyePosition, hitboxPos, g_LocalPlayer, player, hitbox);
				//auto info = Autowall::Get( ).CanHit( hitboxPos );

				if ( m_config.vis ) {
					if ( CanSeePlayer( player, hitboxPos ) )
					{
						if ( info < m_config.awall_damage )
							continue;
					}
				}
				else {
					if ( info < m_config.awall_damage )
						continue;
				}
			}

			if ( m_config.smoke && IsLineGoesThroughSmoke( eyePos, hitboxPos ) )
				continue;

			if ( bestFov > fov )
			{
				bestBone = hitbox;
				bestAngles = ang;
				bestFov = fov;
				target = player;
			}
		}
	}

	return target;
}

void CLegitbot::Run( CUserCmd* cmd )
{
	if ( int( GetTickCount( ) ) > lastShotTick + 50 )
		shotsFired = 0;

	current_punch = g_LocalPlayer->m_aimPunchAngle( );

	if ( !IsEnabled( cmd ) )
	{
		last_punch = { 0, 0, 0 };
		shot_delay = false;
		kill_delay = false;
		target = nullptr;
		return;
	}

	auto weapon = LocalPlayerData::m_Weapon;
	if ( !weapon )
		return;

	auto weapon_data = weapon->GetCSWeaponData( );
	if ( !weapon_data )
		return;

	if ( m_config.flash && g_LocalPlayer->IsFlashed( ) > 200.0 )
		return;

	auto angles = cmd->viewangles;
	const auto current = angles;

	float fov = FLT_MAX;
	int bestBone = -1;

	if ( GetClosestPlayer( cmd, bestBone, fov, angles ) )
	{
		if ( !m_config.silent_mode && !shot_delay && m_config.shot_delay > 0 && !shotsFired )
		{
			shot_delay = true;
			shot_delay_time = int( GetTickCount( ) ) + m_config.shot_delay;
		}

		if ( shot_delay && shot_delay_time <= int( GetTickCount( ) ) )
			shot_delay = false;

		if ( shot_delay )
			cmd->buttons &= ~IN_ATTACK;

		if ( m_config.awall && GetAsyncKeyState( g_Options.awall_key ) )
			cmd->buttons |= IN_ATTACK;
	}

	if ( (cmd->buttons & IN_ATTACK) )
		RCS( angles, target );

	last_punch = current_punch;

	if ( !IsSilent( ) )
		Smooth( current, angles, angles );

	cmd->viewangles = angles;

	if ( !IsSilent( ) )
		g_EngineClient->SetViewAngles( &angles );

	if ( weapon->IsPistolIDX( ) || weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_DEAGLE && m_config.auto_pistol )
	{
		const float server_time = g_LocalPlayer->m_nTickBase( ) * g_GlobalVars->interval_per_tick;
		const float next_shot = weapon->m_flNextPrimaryAttack( ) - server_time;

		if ( next_shot > 0 )
			cmd->buttons &= ~IN_ATTACK;
	}

	if ( cmd->buttons & IN_ATTACK )
	{
		lastShotTick = GetTickCount( );
		shotsFired++;
	}
}