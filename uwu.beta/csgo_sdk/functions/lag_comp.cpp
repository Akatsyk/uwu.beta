#include "lag_comp.h"
#include "../config/config.h"

#include <algorithm>
#include "fakelag.h"

#include "tickbase.h"
#include "../sdk/utils/math.hpp"
#include "autowall.h"

namespace InterpolatePlace
{
	Vector Vec( const Vector from, const Vector to, const float percent ) {
		return to * percent + from * (1.f - percent);
	}

	QAngle Angle( const QAngle from, const QAngle to, const float percent ) {
		return to * percent + from * (1.f - percent);
	}

	float Float( const float from, const float to, const float percent ) {
		return to * percent + from * (1.f - percent);
	}

	bool IsVectorZero( Vector m_Rec ) {
		return m_Rec.x == 0.0f && m_Rec.y == 0.0f
			&& m_Rec.z == 0.0f;
	}

	[[ set ]];
}

StoreAnimation::StoreAnimation( C_BasePlayer* player )
{
	const auto m_Weapon = player->m_hActiveWeapon( ).Get( );

	Player = player;
	Index = player->EntIndex( );
	Dormant = player->IsDormant( );
	Velocity = player->m_vecVelocity( );
	Origin = player->m_vecOrigin( );
	AbsOrigin = player->GetAbsOrigin( );
	Mins = player->GetCollideable( )->OBBMins( );
	Maxs = player->GetCollideable( )->OBBMaxs( );
	memcpy( Layers, player->GetAnimOverlays( ), sizeof( AnimationLayer ) * 13 );
	Poses = player->m_flPoseParameter( );
	AnimState = player->GetPlayerAnimState( );
	SimTime = player->m_flSimulationTime( );
	InterpTime = 0.f;
	LastShotTime = m_Weapon ? m_Weapon->m_fLastShotTime( ) : 0.f;
	Duck = player->m_flDuckAmount( );
	LBY = player->m_flLowerBodyYawTarget( );
	EyeAngle = player->m_angEyeAngles( );
	AbsAng = player->GetAbsAngles( );
	Flags = player->m_fFlags( );
	Eflags = player->m_iEFlags( );
	Effects = player->m_fEffects( );
	Lag = TIME_TO_TICKS( player->m_flSimulationTime( ) - player->m_flOldSimulationTime( ) );
	Valid = Lag >= 0 && Lag <= 15;
	Lag = std::clamp( Lag, 0, 15 );
}

StoreAnimation::StoreAnimation( C_BasePlayer* player, QAngle last_reliable_angle ) : StoreAnimation( player )
{
	LastReliableAngle = last_reliable_angle;
}

void StoreAnimation::Restore( C_BasePlayer* player ) const
{
	player->m_vecVelocity( ) = Velocity;
	player->m_fFlags( ) = Flags;
	player->m_iEFlags( ) = Eflags;
	player->m_flDuckAmount( ) = Duck;
	memcpy( player->GetAnimOverlays( ), Layers, sizeof( AnimationLayer ) * 13 );
	player->m_flLowerBodyYawTarget( ) = LBY;
	player->m_vecOrigin( ) = Origin;
	player->SetAbsOrig( Origin );
}

void StoreAnimation::Apply( C_BasePlayer* player ) const
{
	player->m_flPoseParameter( ) = Poses;
	player->m_angEyeAngles( ) = EyeAngle;
	player->m_vecVelocity( ) = Velocity;
	player->SetAbsVel( Velocity );
	player->m_flLowerBodyYawTarget( ) = LBY;
	player->m_flDuckAmount( ) = Duck;
	player->m_fFlags( ) = Flags;
	player->m_vecOrigin( ) = Origin;
	player->SetAbsOrig( Origin );
}

void ProcessVelocity( C_BasePlayer* e, StoreAnimation* m_MainRecord, StoreAnimation* m_PrevRecord, AnimationLayer animlayers[ 13 ] )
{
	if ( !m_PrevRecord )
		return;

	auto Velocity = e->m_vecVelocity( );
	auto WasInAir = e->m_fFlags( ) & FL_ONGROUND && m_PrevRecord->Flags & FL_ONGROUND;

	auto TimeDiff = std::max( g_GlobalVars->interval_per_tick, e->m_flSimulationTime( ) - m_PrevRecord->SimTime );
	auto Origin = e->m_vecOrigin( ) - m_PrevRecord->Origin;

	auto AnimSpeed = 0.0f;

	if ( !InterpolatePlace::IsVectorZero( Origin ) && TIME_TO_TICKS( TimeDiff ) > 0 )
	{
		e->m_vecVelocity( ) = Origin * (1.0f / TimeDiff);

		if ( e->m_fFlags( ) & FL_ONGROUND && animlayers[ 11 ].m_flWeight > 0.0f && animlayers[ 11 ].m_flWeight < 1.0f && animlayers[ 11 ].m_flCycle > m_PrevRecord->Layers[ 11 ].m_flCycle )
		{
			auto weapon = e->m_hActiveWeapon( ).Get( );

			if ( weapon )
			{
				auto max_speed = 260.0f;
				auto weapon_info = e->m_hActiveWeapon( ).Get( )->GetCSWeaponData( );

				if ( weapon_info )
					max_speed = e->m_bIsScoped( ) ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

				auto modifier = 0.35f * (1.0f - animlayers[ 11 ].m_flWeight);

				if ( modifier > 0.0f && modifier < 1.0f )
					AnimSpeed = max_speed * (modifier + 0.55f);
			}
		}

		if ( AnimSpeed > 0.0f )
		{
			AnimSpeed /= e->m_vecVelocity( ).Length2D( );

			e->m_vecVelocity( ).x *= AnimSpeed;
			e->m_vecVelocity( ).y *= AnimSpeed;
		}

		if ( m_MainRecord && TimeDiff > g_GlobalVars->interval_per_tick )
		{
			auto previous_velocity = (m_PrevRecord->Origin - m_MainRecord->Origin) * (1.0f / TimeDiff );

			if ( !InterpolatePlace::IsVectorZero( previous_velocity ) && !WasInAir )
			{
				auto current_direction = Math::NormalizeYaw( RAD2DEG( atan2( e->m_vecVelocity( ).y, e->m_vecVelocity( ).x ) ) );
				auto previous_direction = Math::NormalizeYaw( RAD2DEG( atan2( previous_velocity.y, previous_velocity.x ) ) );

				auto average_direction = current_direction - previous_direction;
				average_direction = DEG2RAD( Math::NormalizeYaw( current_direction + average_direction * 0.5f ) );

				auto direction_cos = cos( average_direction );
				auto dirrection_sin = sin( average_direction );

				auto velocity_speed = e->m_vecVelocity( ).Length2D( );

				e->m_vecVelocity( ).x = direction_cos * velocity_speed;
				e->m_vecVelocity( ).y = dirrection_sin * velocity_speed;
			}
		}

		if ( !(e->m_fFlags( ) & FL_ONGROUND) )
		{
			static auto sv_gravity = g_CVar->FindVar( "sv_gravity" );

			auto fixed_timing = Math::clamp( TimeDiff, g_GlobalVars->interval_per_tick, 1.0f );
			e->m_vecVelocity( ).z -= sv_gravity->GetFloat( ) * fixed_timing * 0.5f;
		}
		else
			e->m_vecVelocity( ).z = 0.0f;
	}
}

void Backtrack::AnimationInfo::UpdateAnims( StoreAnimation* m_MainRecord, StoreAnimation* m_PrevRecord )
{
	AnimationLayer animlayers[ 13 ];
	float pose_parametrs[ 24 ];

	auto e = m_MainRecord->Player;
	auto animstate = e->GetPlayerAnimState( );

	if ( !animstate )
		return;

	player_info_t player_info;

	if ( !g_EngineClient->GetPlayerInfo( e->EntIndex( ), &player_info ) )
		return;

	memcpy( pose_parametrs, &e->m_flPoseParameter( ), 24 * sizeof( float ) );
	memcpy( animlayers, e->GetAnimOverlays( ), 13 * sizeof( AnimationLayer ) );
	memcpy( m_MainRecord->Layers, animlayers, 13 * sizeof( AnimationLayer ) );

	auto backup_lower_body_yaw_target = e->m_flLowerBodyYawTarget( );
	auto backup_duck_amount = e->m_flDuckAmount( );
	auto backup_flags = e->m_fFlags( );
	auto backup_eflags = e->m_iEFlags( );

	auto backup_curtime = g_GlobalVars->curtime;
	auto backup_frametime = g_GlobalVars->frametime;
	auto backup_realtime = g_GlobalVars->realtime;
	auto backup_framecount = g_GlobalVars->framecount;
	auto backup_tickcount = g_GlobalVars->tickcount;
	auto backup_interpolation_amount = g_GlobalVars->interpolation_amount;

	float m_StoredLerp = FLT_MAX;

	g_GlobalVars->curtime = e->m_flSimulationTime( );
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

	if ( m_PrevRecord )
		m_MainRecord->Didshot = m_MainRecord->LastShotTime > m_PrevRecord->SimTime && m_MainRecord->LastShotTime <= m_MainRecord->SimTime;

	ProcessVelocity( e, m_MainRecord, m_PrevRecord, animlayers );

	e->m_iEFlags( ) &= ~0x1000;

	if ( e->m_fFlags( ) & FL_ONGROUND && e->m_vecVelocity( ).Length( ) > 0.0f && animlayers[ 6 ].m_flWeight <= 0.0f )
		e->m_vecVelocity( ).Zero( );

	e->SetAbsVel( e->m_vecVelocity( ) );

	auto updated_animations = false;
	auto ticks_chocked = 1;

	if ( m_PrevRecord )
	{
		memcpy( e->GetAnimOverlays( ), m_PrevRecord->Layers, 13 * sizeof( AnimationLayer ) );
		memcpy( &e->m_flPoseParameter( ), pose_parametrs, 24 * sizeof( float ) );

		auto simulation_ticks = TIME_TO_TICKS( e->m_flSimulationTime( ) - m_PrevRecord->SimTime );

		if ( simulation_ticks > 0 && simulation_ticks < 31 )
			ticks_chocked = simulation_ticks;

		if ( ticks_chocked > 1 )
		{
			auto land_time = 0.0f;
			auto land_in_cycle = false;
			auto is_landed = false;
			auto on_ground = false;

			if ( animlayers[ 4 ].m_flCycle < 0.5f && (!(e->m_fFlags( ) & FL_ONGROUND) || !(m_PrevRecord->Flags & FL_ONGROUND)) )
			{
				land_time = e->m_flSimulationTime( ) - animlayers[ 4 ].m_flPlaybackRate * animlayers[ 4 ].m_flCycle;
				land_in_cycle = land_time >= m_PrevRecord->SimTime;
			}

			auto duck_amount_per_tick = (e->m_flDuckAmount( ) - m_PrevRecord->Duck ) / ticks_chocked;

			for ( auto i = 0; i < ticks_chocked; ++i )
			{
				auto simulated_time = m_PrevRecord->SimTime + TICKS_TO_TIME( i );
				float lerpTime = 1.f - (m_MainRecord->SimTime - simulated_time) / (m_MainRecord->SimTime - m_PrevRecord->SimTime );

				m_StoredLerp = lerpTime;

				if ( duck_amount_per_tick )
					e->m_flDuckAmount( ) = m_PrevRecord->Duck + duck_amount_per_tick * ( float )i;

				on_ground = e->m_fFlags( ) & FL_ONGROUND;

				if ( land_in_cycle && !is_landed )
				{
					if ( land_time <= simulated_time )
					{
						is_landed = true;
						on_ground = true;
					}
					else
						on_ground = m_PrevRecord->Flags & FL_ONGROUND;
				}

				if ( on_ground )
					e->m_fFlags( ) |= FL_ONGROUND;
				else
					e->m_fFlags( ) &= ~FL_ONGROUND;

				auto simulated_ticks = TIME_TO_TICKS( simulated_time );

				g_GlobalVars->realtime = simulated_time;
				g_GlobalVars->curtime = simulated_time;
				g_GlobalVars->framecount = simulated_ticks;
				g_GlobalVars->tickcount = simulated_ticks;
				g_GlobalVars->interpolation_amount = 0.0f;

				g_CallLocalUpdate = true;
				e->UpdateClientSideAnimation( );
				g_CallLocalUpdate = false;

				g_GlobalVars->realtime = backup_realtime;
				g_GlobalVars->curtime = backup_curtime;
				g_GlobalVars->framecount = backup_framecount;
				g_GlobalVars->tickcount = backup_tickcount;
				g_GlobalVars->interpolation_amount = backup_interpolation_amount;

				updated_animations = true;
			}
		}
	}

	if ( !updated_animations )
	{
		g_CallLocalUpdate = true;
		e->UpdateClientSideAnimation( );
		g_CallLocalUpdate = false;
	}

	auto SetupMatrix = [ & ]( C_BasePlayer* e, AnimationLayer* layers, matrix3x4_t* matrix, bool hitbox_mask ) -> void
	{
		e->InvalidatePhysics( 8 );

		AnimationLayer backup_layers[ 13 ];

		memcpy( backup_layers, e->GetAnimOverlays( ), 13 * sizeof( AnimationLayer ) );
		memcpy( e->GetAnimOverlays( ), layers, 13 * sizeof( AnimationLayer ) );
		m_MainRecord->BuildBones( e, matrix, hitbox_mask);
		memcpy( e->GetAnimOverlays( ), backup_layers, 13 * sizeof( AnimationLayer ) );
	};

	if ( !player_info.fakeplayer && ticks_chocked >= 1 )
	{
		if ( g_Options.ragebot.Resolver )
		{
			if ( m_StoredLerp != FLT_MAX && m_PrevRecord && !m_MainRecord->Didshot )
			{
				auto m_Eye = InterpolatePlace::Float( m_PrevRecord->AnimState->m_flGoalFeetYaw,
					m_MainRecord->AnimState->m_flGoalFeetYaw, m_StoredLerp );

				animstate->m_flGoalFeetYaw = Math::NormalizeYaw( m_Eye );
			}
		}
	}

	g_CallLocalUpdate = true;
	e->UpdateClientSideAnimation( );
	g_CallLocalUpdate = false;

	SetupMatrix( e, animlayers, m_MainRecord->Bones, true);
	memcpy( e->m_CachedBoneData( ).Base( ), m_MainRecord->Bones, e->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

	g_GlobalVars->curtime = backup_curtime;
	g_GlobalVars->frametime = backup_frametime;

	e->m_flLowerBodyYawTarget( ) = backup_lower_body_yaw_target;
	e->m_flDuckAmount( ) = backup_duck_amount;
	e->m_fFlags( ) = backup_flags;
	e->m_iEFlags( ) = backup_eflags;

	memcpy( e->GetAnimOverlays( ), animlayers, 13 * sizeof( AnimationLayer ) );

	if ( e->m_flSimulationTime( ) < e->m_flOldSimulationTime( ) )
		m_MainRecord->Valid = false;
}

bool StoreAnimation::IsValidExtended( )
{
	if ( !g_ClientState->m_NetChannel || !Valid )
		return false;

	auto nci = g_EngineClient->GetNetChannelInfo( );

	if ( !nci )
		return false;

	auto correct = nci->GetLatency( FLOW_OUTGOING ) + nci->GetLatency( FLOW_INCOMING ) + LagComp::Get( ).LerpTime( );

	static auto sv_maxunlag = g_CVar->FindVar( "sv_maxunlag" );
	correct = std::clamp( correct, 0.0f, sv_maxunlag->GetFloat( ) );

	auto delta_time = fabs( correct - (g_GlobalVars->curtime - SimTime ) );
	float ping = sv_maxunlag->GetFloat( );

	return delta_time <= sv_maxunlag->GetFloat( ) && delta_time >= ping - .2f;
}

bool StoreAnimation::IsValid( float flSimulationTime, bool bValid, float flRange ) {

	if ( !g_ClientState->m_NetChannel || !bValid )
		return false;

	auto nci = g_EngineClient->GetNetChannelInfo( );

	if ( !nci )
		return false;

	static auto sv_maxunlag = g_CVar->FindVar( "sv_maxunlag" );

	const auto flCorrect = std::clamp( nci->GetLatency( FLOW_INCOMING )
		+ nci->GetLatency( FLOW_OUTGOING )
		+ LagComp::Get( ).LerpTime( ), 0.f, sv_maxunlag->GetFloat( ) );

	return fabsf( flCorrect - (g_GlobalVars->curtime - flSimulationTime) ) <= flRange;
}

Backtrack::AnimationInfo* Backtrack::GetAnimationInfo( C_BasePlayer* player )
{
	auto info = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	if ( info == AnimInfo.end( ) )
		return nullptr;

	return &info->second;
}

std::optional<StoreAnimation*> Backtrack::GetLatestAnimtion( C_BasePlayer* player )
{
	const auto info = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	if ( info == AnimInfo.end( ) || info->second.Frames.empty( ) )
		return std::nullopt;

	StoreAnimation* first_invalid = nullptr;

	for ( auto it = info->second.Frames.begin( ); it != info->second.Frames.end( ); it = next( it ) ) {

		if ( !first_invalid )
			first_invalid = &*it;

		if ( it->IsValid( it->SimTime, it->Valid ) ) {
			return &*it;
		}
	}

	if ( first_invalid )
		return first_invalid;
	else
		return std::nullopt;
}

std::optional<StoreAnimation*> Backtrack::GetOldestAnimation( C_BasePlayer* player )
{
	const auto pInfo = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	if ( pInfo == AnimInfo.end( ) || pInfo->second.Frames.empty( ) )
		return std::nullopt;

	for ( auto it = pInfo->second.Frames.rbegin( ); it != pInfo->second.Frames.rend( ); it = next( it ) ) {
		if ( it->IsValid( it->SimTime, it->Valid ) ) {
			return &*it;
		}
	}

	return std::nullopt;
}

std::vector<StoreAnimation*> Backtrack::GetValidAnimation( C_BasePlayer* player, const float range )
{
	std::vector<StoreAnimation*> result;

	const auto info = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	if ( info == AnimInfo.end( ) || info->second.Frames.empty( ) )
		return result;

	result.reserve( static_cast< int >(std::ceil( range * .2f / g_GlobalVars->interval_per_tick )) );

	for ( auto it = info->second.Frames.begin( ); it != info->second.Frames.end( ); it = next( it ) )
		if ( it->IsValid( it->SimTime, it->Valid, range * 0.2f ) )
			result.push_back( &*it );

	return result;
}

std::optional<StoreAnimation*> Backtrack::GetLatestFireAnimation( C_BasePlayer* player )
{
	const auto info = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	if ( info == AnimInfo.end( ) || info->second.Frames.empty( ) )
		return std::nullopt;

	for ( auto it = info->second.Frames.begin( ); it != info->second.Frames.end( ); it = next( it ) )
		if ( it->IsValid( it->SimTime, it->Valid ) && ((*it).Didshot || (fabs((*it).EyeAngle.pitch) < 25)))
			return &*it;

	return std::nullopt;
}

std::vector<StoreAnimation*> Backtrack::GetValidAnimationNonLimited( C_BasePlayer* player )
{
	const auto info = AnimInfo.find( player->GetRefEHandle( ).ToInt( ) );

	std::vector<StoreAnimation*> ret = { };

	if ( info == AnimInfo.end( ) || info->second.Frames.empty( ) )
		return ret;

	Vector last_origin = Vector( 0, 0, 0 );

	for ( auto it = info->second.Frames.begin( ); it != info->second.Frames.end( ); it = next( it ) ) 
	{
		if ( it->IsValid( it->SimTime, it->Valid ) ) 
		{
			float diff = 0.f;

			if ( it != info->second.Frames.begin( ) && last_origin != Vector( 0, 0, 0 ) ) 
			{
				diff = it->Origin.DistTo( last_origin );
			}

			if ( diff > 25.f || it->EyeAngle.pitch <= 25.f )
				ret.emplace_back( &*it );

			last_origin = it->Origin;
		}
	}

	return ret;
}

void StoreAnimation::BuildBones( C_BasePlayer* ent, matrix3x4_t* matrix, bool hitbox_mask)
{
	if (!ent || !ent->IsAlive() || !ent->IsPlayer())
		return;

	std::array < AnimationLayer, 13 > aAnimationLayers{ };

	float curtime = g_GlobalVars->curtime;
	float realtime = g_GlobalVars->realtime;
	float absframetime = g_GlobalVars->absoluteframetime;
	float frametime = g_GlobalVars->frametime;
	float framecount = g_GlobalVars->framecount;
	float tickcount = g_GlobalVars->tickcount;
	float interpolationamount = g_GlobalVars->interpolation_amount;
	bool clientsideanimation = ent->m_bClientSideAnimation();

	float time = (g_LocalPlayer != ent ? ent->m_flSimulationTime() : TICKS_TO_TIME(g_ClientState->m_ClockDriftMgr.m_nServerTick));
	int ticks = TIME_TO_TICKS(time);

	g_GlobalVars->curtime = time;
	g_GlobalVars->realtime = time;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->absoluteframetime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->framecount = ticks;
	g_GlobalVars->tickcount = ticks;
	g_GlobalVars->interpolation_amount = 0.f;

	LPVOID pInverseKinematics = ent->m_pInverseKinematics();
	int nClientEffects = ent->m_nClientEffects();
	int nLastSkipFramecount = ent->m_nLastSkipFramecount();
	int nOcclusionMask = ent->m_iOcclusionFlags();
	int nOcclusionFrame = ent->m_iOcclusionFramecount();

	bool bJiggleBones = ent->m_JiggleBones();
	bool bMaintainSequenceTransition = ent->m_bMaintainSequenceTransition();
	Vector vecAbsOrigin = ent->GetAbsOrigin();
	int iEffects = ent->m_fEffects();
	int iMask = BONE_USED_BY_ANYTHING;
	if (hitbox_mask)
		iMask = BONE_USED_BY_HITBOX;

	std::memcpy(aAnimationLayers.data(), ent->GetAnimOverlays(), sizeof(AnimationLayer) * 13);

	ent->InvalidateBoneCache();
	ent->get_readable_bones() = NULL;
	ent->get_writeable_bones() = NULL;

	if (ent->GetPlayerAnimStateAlternative())
		ent->GetPlayerAnimStateAlternative()->m_weapon_last = ent->GetPlayerAnimStateAlternative()->m_weapon;

	ent->m_iOcclusionFramecount() = 0;
	ent->m_iOcclusionFlags() = 0;
	ent->m_nLastSkipFramecount() = 0;

	if (ent != g_LocalPlayer)
		ent->SetAbsOrig(ent->m_vecOrigin());

	ent->m_fEffects() |= 0x008;
	ent->m_nClientEffects() |= 0x002;
	ent->m_pInverseKinematics() = nullptr;
	ent->m_JiggleBones() = false;
	ent->m_bMaintainSequenceTransition() = false;

	ent->GetAnimOverlays()[12].m_flWeight = 0.0f;
	if (hitbox_mask)
		ent->GetAnimOverlays()[3].m_pOwner = NULL;
	else if (ent == g_LocalPlayer)
	{
		if (ent->GetSequenceActivity(ent->GetAnimOverlays()[3].m_nSequence) == 979)
		{
			ent->GetAnimOverlays()[3].m_flCycle = 0.0f;
			ent->GetAnimOverlays()[3].m_flWeight = 0.0f;
		}
	}

	m_CallPlayerSetupBone = true;
	ent->m_bClientSideAnimation() = false;
	ent->SetupBones(matrix, matrix ? MAXSTUDIOBONES : -1, iMask, time);
	ent->m_bClientSideAnimation() = clientsideanimation;
	m_CallPlayerSetupBone = false;

	ent->m_bMaintainSequenceTransition() = bMaintainSequenceTransition;
	ent->m_pInverseKinematics() = pInverseKinematics;
	ent->m_nClientEffects() = nClientEffects;
	ent->m_JiggleBones() = bJiggleBones;
	ent->m_fEffects() = iEffects;
	ent->m_nLastSkipFramecount() = nLastSkipFramecount;
	ent->m_iOcclusionFramecount() = nOcclusionFrame;
	ent->m_iOcclusionFlags() = nOcclusionMask;

	if (ent != g_LocalPlayer)
		ent->SetAbsOrig(vecAbsOrigin);

	std::memcpy(ent->GetAnimOverlays(), aAnimationLayers.data(), sizeof(AnimationLayer) * 13);

	g_GlobalVars->curtime = curtime;
	g_GlobalVars->realtime = realtime;
	g_GlobalVars->absoluteframetime = absframetime;
	g_GlobalVars->frametime = frametime;
	g_GlobalVars->framecount = framecount;
	g_GlobalVars->tickcount = tickcount;
	g_GlobalVars->interpolation_amount = interpolationamount;

	//auto backup_value = *( uint8_t* )(( uintptr_t )ent + 0x274);
	//*( uint8_t* )(( uintptr_t )ent + 0x274) = 0;

	//auto backup_effects = ent->m_fEffects( );
	//ent->m_fEffects( ) |= 8;

	//auto animstate = ent->GetPlayerAnimState( );
	//auto previous_weapon = animstate ? animstate->pLastActiveWeapon : nullptr;

	//if ( previous_weapon )
	//	animstate->pLastActiveWeapon = animstate->pActiveWeapon;

	//auto backup_abs_origin = ent->GetAbsOrigin( );
	//ent->SetAbsOrig( ent->m_vecOrigin( ) );

	//m_CallPlayerSetupBone = true;

	//ent->InvalidateBoneCache( );
	//ent->SetupBones( Bones, Bones ? MAXSTUDIOBONES : -1, 0x7FF00, ent->m_flSimulationTime( ) );

	//m_CallPlayerSetupBone = false;

	//ent->SetAbsOrig( backup_abs_origin );

	//if ( previous_weapon )
	//	animstate->pLastActiveWeapon = previous_weapon;

	//ent->m_fEffects( ) = backup_effects;
	//*( uint8_t* )(( uintptr_t )ent + 0x274) = backup_value;
}

void Backtrack::Run( )
{
	for ( auto it = AnimInfo.begin( ); it != AnimInfo.end( );) {
		auto player = reinterpret_cast< C_BasePlayer* >(g_EntityList->GetClientEntityFromHandle( it->first ));

		if ( !player || player != it->second.Player || !player->IsAlive( ) ) {
			it = AnimInfo.erase( it );
		}
		else
			it = next( it );
	}

	for ( int i = 1; i <= g_EngineClient->GetMaxClients( ); ++i )
	{
		C_BasePlayer* pPlayerEntity = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( i ));

		if ( !pPlayerEntity
			|| !pPlayerEntity->IsAlive( ) || pPlayerEntity->IsDormant( )
			|| !pPlayerEntity->IsPlayer( ) ) {
			continue;
		}

		if ( pPlayerEntity->EntIndex( ) == g_LocalPlayer->EntIndex( ) ) {
			continue;
		}

		if ( pPlayerEntity->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) ) {
			continue;
		}

		if ( AnimInfo.find( pPlayerEntity->GetRefEHandle( ).ToInt( ) ) == AnimInfo.end( ) )
			AnimInfo.insert_or_assign( pPlayerEntity->GetRefEHandle( ).ToInt( ), AnimationInfo( pPlayerEntity, { } ) );
	}

	for ( auto& info : AnimInfo )
	{
		auto& Anim = info.second;
		const auto Enemy = Anim.Player;

		for ( auto i = Anim.Frames.rbegin( ); i != Anim.Frames.rend( );)
		{
			if ( g_GlobalVars->curtime - i->SimTime > 1.2f )
				i = decltype(i) { info.second.Frames.erase( next( i ).base( ) ) };

			else
				i = next( i );
		}

		if ( Enemy->m_flSimulationTime( ) == Enemy->m_flOldSimulationTime( ) )
			continue;

		if ( Anim.LastSpawnTime != Enemy->m_flSpawnTime( ) )
		{
			const auto state = Enemy->GetPlayerAnimState( );

			if ( state )
				Enemy->ResetAnimationState( state );

			Anim.LastSpawnTime = Enemy->m_flSpawnTime( );
		}

		const auto weapon = Enemy->m_hActiveWeapon( ).Get( );

		StoreAnimation* m_Prev = nullptr;

		if ( !Anim.Frames.empty( ) && !Anim.Frames.front( ).Dormant )
			m_Prev = &Anim.Frames.front( );

		const auto shot = weapon && m_Prev && weapon->m_fLastShotTime( ) > m_Prev->SimTime
			&& weapon->m_fLastShotTime( ) <= Enemy->m_flSimulationTime( );

		if ( !shot )
			info.second.LastAnge = Enemy->m_angEyeAngles( );

		auto& record = Anim.Frames.emplace_front( Enemy, info.second.LastAnge );
		Anim.UpdateAnims( &record, m_Prev );
	}
}

float LagComp::LerpTime( )
{
	static auto cl_interp = g_CVar->FindVar( "cl_interp" );
	static auto cl_interp_ratio = g_CVar->FindVar( "cl_interp_ratio" );
	static auto sv_client_min_interp_ratio = g_CVar->FindVar( "sv_client_min_interp_ratio" );
	static auto sv_client_max_interp_ratio = g_CVar->FindVar( "sv_client_max_interp_ratio" );
	static auto cl_updaterate = g_CVar->FindVar( "cl_updaterate" );
	static auto sv_minupdaterate = g_CVar->FindVar( "sv_minupdaterate" );
	static auto sv_maxupdaterate = g_CVar->FindVar( "sv_maxupdaterate" );

	auto updaterate = std::clamp( cl_updaterate->GetFloat( ), sv_minupdaterate->GetFloat( ), sv_maxupdaterate->GetFloat( ) );
	auto lerp_ratio = std::clamp( cl_interp_ratio->GetFloat( ), sv_client_min_interp_ratio->GetFloat( ), sv_client_max_interp_ratio->GetFloat( ) );

	return std::clamp( lerp_ratio / updaterate, cl_interp->GetFloat( ), 1.0f );
}

bool LagComp::ValidTick( int tick )
{
	auto nci = g_EngineClient->GetNetChannelInfo( );

	if ( !nci )
		return false;

	auto PredictedCmdArrivalTick = g_pCmd->tick_count + 1 + TIME_TO_TICKS( nci->GetAvgLatency( FLOW_INCOMING ) + nci->GetAvgLatency( FLOW_OUTGOING ) );
	auto Correct = std::clamp( LerpTime( ) + nci->GetLatency( FLOW_OUTGOING ), 0.f, 1.f ) - TICKS_TO_TIME( PredictedCmdArrivalTick + TIME_TO_TICKS( LerpTime( ) ) - (tick + TIME_TO_TICKS( LerpTime( ) )) );

	return (abs( Correct ) <= 0.2f);
}

bool IsRageA( )
{
	if ( g_Options.MasterSwitch == 0 && g_Options.ragebot.Acitve && g_Options.ragebot.Enable )
		return true;

	return false;
}

void LagComp::AnimationFix( ClientFrameStage_t stage )
{
	for ( int i = 1; i <= g_EngineClient->GetMaxClients( ); ++i )
	{
		C_BasePlayer* pPlayerEntity = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( i ));

		if ( !pPlayerEntity
			|| !pPlayerEntity->IsAlive( )
			|| pPlayerEntity->IsDormant( ) ) {
			continue;
		}

		if ( pPlayerEntity == g_LocalPlayer ) {
			continue;
		}

		if ( pPlayerEntity->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) ) {
			continue;
		}

		if ( !IsRageA( ) )
		{
			if ( stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START )
				pPlayerEntity->SetAbsOrig( pPlayerEntity->m_vecOrigin( ) );

			if ( stage == ClientFrameStage_t::FRAME_RENDER_START )
				InitAnim( pPlayerEntity );
		}
	}
}

void LagComp::InitAnim( C_BasePlayer* pEnt )
{
	float curtime = g_GlobalVars->curtime;
	float frametime = g_GlobalVars->frametime;
	float realtime = g_GlobalVars->realtime;
	float abstime = g_GlobalVars->absoluteframetime;
	float framecount = g_GlobalVars->framecount;
	float interp = g_GlobalVars->interpolation_amount;

	float m_flSimulationTime = pEnt->m_flSimulationTime( );
	int m_iNextSimulationTick = m_flSimulationTime / g_GlobalVars->interval_per_tick + 1;

	g_GlobalVars->curtime = m_flSimulationTime;
	g_GlobalVars->realtime = m_flSimulationTime;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->absoluteframetime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->framecount = m_iNextSimulationTick;
	g_GlobalVars->interpolation_amount = 0;

	pEnt->m_iEFlags( ) &= ~0x1000;
	pEnt->SetAbsVel( pEnt->m_vecVelocity( ) );

	pEnt->m_bClientSideAnimation( ) = true;
	pEnt->UpdateClientSideAnimation( );
	pEnt->m_bClientSideAnimation( ) = false;

	pEnt->InvalidatePhysics( 8 );

	g_GlobalVars->curtime = curtime;
	g_GlobalVars->realtime = realtime;
	g_GlobalVars->frametime = frametime;
	g_GlobalVars->absoluteframetime = abstime;
	g_GlobalVars->framecount = framecount;
	g_GlobalVars->interpolation_amount = interp;
}

/*bool*/ void SetupBonesFixed( C_BasePlayer* ent, matrix3x4_t* matrix, int mask )
{
	std::array < AnimationLayer, 13 > aAnimationLayers{ };

	float curtime = g_GlobalVars->curtime;
	float realtime = g_GlobalVars->realtime;
	float absframetime = g_GlobalVars->absoluteframetime;
	float frametime = g_GlobalVars->frametime;
	float framecount = g_GlobalVars->framecount;
	float tickcount = g_GlobalVars->tickcount;
	float interpolationamount = g_GlobalVars->interpolation_amount;
	bool clientsideanimation = ent->m_bClientSideAnimation();

	float time = TICKS_TO_TIME(g_ClientState->m_ClockDriftMgr.m_nServerTick);
	int ticks = TIME_TO_TICKS(time);

	g_GlobalVars->curtime = time;
	g_GlobalVars->realtime = time;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->absoluteframetime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->framecount = ticks;
	g_GlobalVars->tickcount = ticks;
	g_GlobalVars->interpolation_amount = 0.f;

	LPVOID pInverseKinematics = ent->m_pInverseKinematics();
	int nClientEffects = ent->m_nClientEffects();
	int nLastSkipFramecount = ent->m_nLastSkipFramecount();
	int nOcclusionMask = ent->m_iOcclusionFlags();
	int nOcclusionFrame = ent->m_iOcclusionFramecount();

	bool bJiggleBones = ent->m_JiggleBones();
	bool bMaintainSequenceTransition = ent->m_bMaintainSequenceTransition();
	Vector vecAbsOrigin = ent->GetAbsOrigin();
	int iEffects = ent->m_fEffects();
	int iMask = BONE_USED_BY_ANYTHING;

	std::memcpy(aAnimationLayers.data(), ent->GetAnimOverlays(), sizeof(AnimationLayer) * 13);

	ent->InvalidateBoneCache();
	ent->get_readable_bones() = NULL;
	ent->get_writeable_bones() = NULL;

	if (ent->GetPlayerAnimStateAlternative())
		ent->GetPlayerAnimStateAlternative()->m_weapon_last = ent->GetPlayerAnimStateAlternative()->m_weapon;

	ent->m_iOcclusionFramecount() = 0;
	ent->m_iOcclusionFlags() = 0;
	ent->m_nLastSkipFramecount() = 0;

	ent->m_fEffects() |= 0x008;
	ent->m_nClientEffects() |= 0x002;
	ent->m_pInverseKinematics() = nullptr;
	ent->m_JiggleBones() = false;
	ent->m_bMaintainSequenceTransition() = false;

	ent->GetAnimOverlays()[12].m_flWeight = 0.0f;
	
	if (ent->GetSequenceActivity(ent->GetAnimOverlays()[3].m_nSequence) == 979)
	{
		ent->GetAnimOverlays()[3].m_flCycle = 0.0f;
		ent->GetAnimOverlays()[3].m_flWeight = 0.0f;
	}

	g_ForceBone = true;
	//ent->m_bClientSideAnimation() = false;
	ent->SetupBones(matrix, matrix ? MAXSTUDIOBONES : -1, iMask, time);
	//ent->m_bClientSideAnimation() = clientsideanimation;
	g_ForceBone = false;

	ent->m_bMaintainSequenceTransition() = bMaintainSequenceTransition;
	ent->m_pInverseKinematics() = pInverseKinematics;
	ent->m_nClientEffects() = nClientEffects;
	ent->m_JiggleBones() = bJiggleBones;
	ent->m_fEffects() = iEffects;
	ent->m_nLastSkipFramecount() = nLastSkipFramecount;
	ent->m_iOcclusionFramecount() = nOcclusionFrame;
	ent->m_iOcclusionFlags() = nOcclusionMask;

	std::memcpy(ent->GetAnimOverlays(), aAnimationLayers.data(), sizeof(AnimationLayer) * 13);

	g_GlobalVars->curtime = curtime;
	g_GlobalVars->realtime = realtime;
	g_GlobalVars->absoluteframetime = absframetime;
	g_GlobalVars->frametime = frametime;
	g_GlobalVars->framecount = framecount;
	g_GlobalVars->tickcount = tickcount;
	g_GlobalVars->interpolation_amount = interpolationamount;

	//auto setuped = false;

	//auto backup_value = *( uint8_t* )(( uintptr_t )ent + 0x274);
	//*( uint8_t* )(( uintptr_t )ent + 0x274) = 0;

	//auto backup_effects = ent->m_fEffects( );
	//ent->m_fEffects( ) |= 8;

	//auto animstate = ent->GetPlayerAnimState( );
	//auto previous_weapon = animstate ? animstate->pLastActiveWeapon : nullptr;

	//if ( previous_weapon )
	//	animstate->pLastActiveWeapon = animstate->pActiveWeapon;

	//g_ForceBone = true;

	//ent->InvalidateBoneCache( );
	//ent->SetupBones( matrix, matrix ? MAXSTUDIOBONES : -1, mask, ent->m_flSimulationTime( ) );

	//g_ForceBone = false;

	//if ( previous_weapon )
	//	animstate->pLastActiveWeapon = previous_weapon;

	//ent->m_fEffects( ) = backup_effects;
	//*( uint8_t* )(( uintptr_t )ent + 0x274) = backup_value;

	//return setuped;
}

void LagComp::FixLocalAnims( )
{
	if ( !g_pCmd )
		return;

	auto animstate = g_LocalPlayer->GetPlayerAnimState( );

	if ( !animstate )
		return;

	real_server_update = false;
	fake_server_update = false;

	if ( g_LocalPlayer->m_flSimulationTime( ) != real_simulation_time || g_LocalPlayer->m_flSimulationTime( ) != fake_simulation_time )
	{
		real_server_update = fake_server_update = true;
		real_simulation_time = fake_simulation_time = g_LocalPlayer->m_flSimulationTime( );
	}

	g_LocalPlayer->GetAnimOverlays( )[ 3 ].m_flWeight = 0.0f;
	g_LocalPlayer->GetAnimOverlays( )[ 3 ].m_flCycle = 0.0f;
	g_LocalPlayer->GetAnimOverlays( )[ 12 ].m_flWeight = 0.0f;

	if ( g_Options.visuals.misc_.smth_wrong )
		g_LocalPlayer->m_flModelScale( ) = 0.4285717010498047;

	else
		g_LocalPlayer->m_flModelScale( ) = 1;

	auto alloc = !m_nState;
	auto change = !alloc && handle != &g_LocalPlayer->GetRefEHandle( );
	auto reset = !alloc && !change && g_LocalPlayer->m_flSpawnTime( ) != spawntime;

	if ( change )
		g_Memory->free( m_nState );

	if ( reset )
	{
		g_LocalPlayer->ResetAnimationState( m_nState );
		spawntime = g_LocalPlayer->m_flSpawnTime( );
	}

	if ( alloc || change )
	{
		m_nState = ( CCSGOPlayerAnimState* )g_Memory->alloc( sizeof( CCSGOPlayerAnimState ) );

		if ( m_nState )
			g_LocalPlayer->CreateAnimationState( m_nState );

		handle = ( CBaseHandle* )&g_LocalPlayer->GetRefEHandle( );
		spawntime = g_LocalPlayer->m_flSpawnTime( );
	}

	if ( !alloc && !change && !reset && fake_server_update )
	{
		float pose_parameter[ 24 ];
		memcpy( pose_parameter, &g_LocalPlayer->m_flPoseParameter( ), 24 * sizeof( float ) );

		AnimationLayer layers[ 13 ];
		memcpy( layers, g_LocalPlayer->GetAnimOverlays( ), 13 * sizeof( AnimationLayer ) );

		auto backup_frametime = g_GlobalVars->frametime;
		auto backup_curtime = g_GlobalVars->curtime;

		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
		g_GlobalVars->curtime = g_LocalPlayer->m_flSimulationTime( );

		m_nState->pBaseEntity = g_LocalPlayer;
		g_LocalPlayer->UpdateAnimationState( m_nState, m_RealAngle );

		m_nState->m_bInHitGroundAnimation = false;
		m_nState->m_fLandingDuckAdditiveSomething = 0.0f;
		m_nState->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

		//StoreAnimation::BuildBones(g_LocalPlayer, fake_matrix, false);
		SetupBonesFixed( g_LocalPlayer, fake_matrix, BONE_USED_BY_ANYTHING );

		for ( auto& i : fake_matrix )
		{
			i[ 0 ][ 3 ] -= g_LocalPlayer->GetRenderOrigin( ).x;
			i[ 1 ][ 3 ] -= g_LocalPlayer->GetRenderOrigin( ).y;
			i[ 2 ][ 3 ] -= g_LocalPlayer->GetRenderOrigin( ).z;
		}

		g_GlobalVars->frametime = backup_frametime;
		g_GlobalVars->curtime = backup_curtime;

		memcpy( &g_LocalPlayer->m_flPoseParameter( ), pose_parameter, 24 * sizeof( float ) );
		memcpy( g_LocalPlayer->GetAnimOverlays( ), layers, 13 * sizeof( AnimationLayer ) );
	}

	if ( tickcount != g_GlobalVars->tickcount )
	{
		tickcount = g_GlobalVars->tickcount;
		memcpy( layers, g_LocalPlayer->GetAnimOverlays( ), 13 * sizeof( AnimationLayer ) );

		if ( m_nState )
			animstate->m_fDuckAmount = m_nState->m_fDuckAmount;

		animstate->m_iLastClientSideAnimationUpdateFramecount = 0;
		g_LocalPlayer->UpdateAnimationState( animstate, m_RealAngle );

		if ( real_server_update )
		{
			abs_angles = animstate->m_flGoalFeetYaw;
			memcpy( pose_parameter, &g_LocalPlayer->m_flPoseParameter( ), 24 * sizeof( float ) );
		}
	}
	else
		animstate->m_iLastClientSideAnimationUpdateFramecount = g_GlobalVars->framecount;

	animstate->m_flGoalFeetYaw = CFakeLag::Get( ).isLag ? m_FakeAngle.yaw : abs_angles;
	g_LocalPlayer->SetAbsAngles( Vector( 0.0f, abs_angles, 0.0f ) );

	memcpy( g_LocalPlayer->GetAnimOverlays( ), layers, 13 * sizeof( AnimationLayer ) );
	memcpy( &g_LocalPlayer->m_flPoseParameter( ), pose_parameter, 24 * sizeof( float ) );
}

