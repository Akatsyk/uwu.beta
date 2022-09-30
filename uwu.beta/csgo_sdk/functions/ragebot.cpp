#include "ragebot.h"
#include "autowall.h"

#include "../sdk/utils/math.hpp"

#include <algorithm>
#include "../config/config.h"

#include "tickbase.h"
#include "miss_counter.h"

#include "movement.h"
#include "ray_tracer.h"

namespace ForRage 
{
	static std::vector<std::tuple<float, float, float>> PrecomputedSeeds = { };

	typedef void(*RandomSeed_t)(UINT);
	RandomSeed_t m_RandomSeed = 0;
	void RandomSeed( uint32_t seed )
	{
		if ( m_RandomSeed == NULL )
			m_RandomSeed = ( RandomSeed_t )GetProcAddress( GetModuleHandle( _( "vstdlib.dll" ) ), _( "RandomSeed" ) );
		m_RandomSeed( seed );
	}

	typedef float(*RandomFloat_t)(float, float);
	RandomFloat_t m_RandomFloat;
	float RandomFloat( float flLow, float flHigh )
	{
		if ( m_RandomFloat == NULL )
			m_RandomFloat = ( RandomFloat_t )GetProcAddress( GetModuleHandle( _( "vstdlib.dll" ) ), _( "RandomFloat" ) );

		return m_RandomFloat( flLow, flHigh );
	}

	static const int TotalSeeds = 255;

	void BuildTable( )
	{
		if ( !PrecomputedSeeds.empty( ) )
			return;

		for ( auto i = 0; i < TotalSeeds; i++ ) {
			RandomSeed( i + 1 );

			const auto pi_seed = RandomFloat( 0.f, M_PI * 2 );

			PrecomputedSeeds.emplace_back( RandomFloat( 0.f, 1.f ),
				sin( pi_seed ), cos( pi_seed ) );
		}
	}

	bool ShouldTrackWiaExploit( )
	{
		bool IsTryShiftDt = g_Options.ragebot.Acitve && g_Options.exploit.ActMode
			&& GetKeyState( g_Options.exploit.mExpltKey ) && g_Options.exploit.mMode == 0;

		bool DTCan = IsTryShiftDt &&
			(!CMovement::Get( ).m_InDuck) && TickBaseSys::Get( ).IsCharged;

		return DTCan;
	}
};

bool ShouldStopSlide;

void CRage::OnCreateMove( CUserCmd* m_pcmd )
{
	g_EngineClient->GetViewAngles( &EngineAngles );
	auto m_Weapon = LocalPlayerData::m_Weapon;

	if ( !m_Weapon )
		return;

	Update( );

	auto m_Data = m_Weapon->GetCSWeaponData( );

	if ( !m_Data )
		return;

	if ( m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_C4 )
		return;

	if ( m_Weapon->IsGrenadeIDX( ) )
		return;

	if ( m_Weapon->IsKnifeIDX( ) )
		return;

	if ( m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_TASER )
		return;

	if ( !GetAsyncKeyState( VK_LBUTTON ) )
	{
		if ( !CockRevolver( m_pcmd, m_Weapon ) )
			return;
	}

	Aim( );
}

void CRage::Update( )
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

	m_Config.damage = g_Options.ragebot.min_damage[ element ];
	m_Config.hitchance = g_Options.ragebot.hitchance[ element ];
	m_Config.boost = g_Options.ragebot.boost[ element ];
	m_Config.head_scale = g_Options.ragebot.HeadScale[ element ];
	m_Config.body_scale = g_Options.ragebot.BodyScale[ element ];
	m_Config.prefer_bod = g_Options.ragebot.PreferBaim[ element ];
	m_Config.prefer_lethal = g_Options.ragebot.LethalBaim[ element ];
	m_Config.autoscope = g_Options.ragebot.AutoScope[ element ];
	m_Config.autostop = g_Options.ragebot.AutoStop[ element ];

	m_Config.hitbox_selected[ 0 ] = g_Options.ragebot.Hitboxes[ element ][ 0 ];
	m_Config.hitbox_selected[ 1 ] = g_Options.ragebot.Hitboxes[ element ][ 1 ];
	m_Config.hitbox_selected[ 2 ] = g_Options.ragebot.Hitboxes[ element ][ 2 ];
	m_Config.hitbox_selected[ 3 ] = g_Options.ragebot.Hitboxes[ element ][ 3 ];
	m_Config.hitbox_selected[ 4 ] = g_Options.ragebot.Hitboxes[ element ][ 4 ];
	m_Config.hitbox_selected[ 5 ] = g_Options.ragebot.Hitboxes[ element ][ 5 ];
	m_Config.hitbox_selected[ 6 ] = g_Options.ragebot.Hitboxes[ element ][ 6 ];

	m_Config.prior_hit = g_Options.ragebot.PriorityHitbox[ element ];
	m_Config.shoot_pr = g_Options.ragebot.ShotPriority[ element ];

	m_Config.body_priors[ 0 ] = g_Options.ragebot.PrefersBo[ element ][ 0 ];
	m_Config.body_priors[ 1 ] = g_Options.ragebot.PrefersBo[ element ][ 1 ];
	m_Config.body_priors[ 2 ] = g_Options.ragebot.PrefersBo[ element ][ 2 ];

	m_Config.default_priors[ 0 ] = g_Options.ragebot.Prefers[ element ][ 0 ];
	m_Config.default_priors[ 1 ] = g_Options.ragebot.Prefers[ element ][ 1 ];
	m_Config.default_priors[ 2 ] = g_Options.ragebot.Prefers[ element ][ 2 ];
}

void CRage::Aim( )
{
	bool isExploit = g_Options.exploit.ActMode && GetKeyState( g_Options.exploit.mExpltKey );
	auto m_Wpn = LocalPlayerData::m_Weapon;

	Vector Aimpoint = { 0, 0, 0 };
	Vector minus_origin = Vector( 0, 0, 0 );

	C_BasePlayer* Target = nullptr;
	targetID = 0;
	StoreAnimation* anims = nullptr;

	RageShot = false;
	int box = 0;
	int best_hitbox = 0;

	float bestFov = FLT_MAX;
	float bestDist = FLT_MAX;

	int bestHealth = INT_MAX;
	int bestIndex = INT_MAX;

	int tempDmg = 0;
	static bool shot = false;
	float simtime = 0;

	for ( int i = 1; i <= g_EngineClient->GetMaxClients( ); ++i )
	{
		C_BasePlayer* pPlayerEntity = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( i ));

		if ( !pPlayerEntity
			|| !pPlayerEntity->IsAlive( )
			|| pPlayerEntity->IsDormant( )
			|| pPlayerEntity->m_bGunGameImmunity( ) )
		{
			ShootNextTick = false;
			continue;
		}

		if ( pPlayerEntity == g_LocalPlayer )
			continue;

		if ( pPlayerEntity->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) )
			continue;

		if ( pPlayerEntity->m_fFlags( ) & FL_FROZEN )
			continue;

		Vector Hitbox = GetAimVector( pPlayerEntity, simtime, minus_origin, anims, box );

		if ( !anims || Hitbox == Vector( 0, 0, 0 ) )
			continue;

		/*if ( TIME_TO_TICKS( fabsf( anims->player->m_flSimulationTime( ) - simtime ) ) > 30 )
			continue;*/

		float fov = Math::Get_fov( Vector( EngineAngles.pitch, EngineAngles.yaw, EngineAngles.roll ), Math::CalcAng( LocalPlayerData::m_EyePosition, Hitbox ) );

		if ( fov > g_Options.ragebot.FieldOfView )
			continue;

		if ( Hitbox != Vector( 0, 0, 0 ) )
		{
			float m_DDistance = sqrt(
				pow( double( Hitbox.x - g_LocalPlayer->m_vecOrigin( ).x ), 2 ) +
				pow( double( Hitbox.y - g_LocalPlayer->m_vecOrigin( ).y ), 2 ) +
				pow( double( Hitbox.z - g_LocalPlayer->m_vecOrigin( ).z ), 2 ) );

			switch ( g_Options.ragebot.TargetSelection ) {
			case 0:

				if ( bestFov < fov )
					continue;

				bestFov = fov;
				Aimpoint = Hitbox;
				Target = pPlayerEntity;
				targetID = i;
				best_anims = anims;
				best_hitbox = box;

				break;
			case 1:

				if ( bestHealth < pPlayerEntity->m_iHealth( ) )
					continue;

				bestHealth = pPlayerEntity->m_iHealth( );
				Aimpoint = Hitbox;
				Target = pPlayerEntity;
				best_anims = anims;
				targetID = i;
				best_hitbox = box;

				break;
			case 2:

				if ( bestDist < m_DDistance )
					continue;

				bestDist = m_DDistance;
				Aimpoint = Hitbox;
				Target = pPlayerEntity;
				best_anims = anims;
				targetID = i;
				best_hitbox = box;

				break;

			case 3:

				if ( bestIndex < i )
					continue;

				bestIndex = i;
				Aimpoint = Hitbox;
				Target = pPlayerEntity;
				best_anims = anims;
				targetID = i;
				best_hitbox = box;

				break;
			case 4:

				Aimpoint = Hitbox;
				Target = pPlayerEntity;
				best_anims = anims;
				targetID = i;
				best_hitbox = box;

				break;
			}
		}
	}

	auto server_time = TICKS_TO_TIME( CSGO::m_TicksAllowed );
	bool canShoot = server_time > LocalPlayerData::m_Weapon->m_flNextPrimaryAttack( ) &&
		server_time > g_LocalPlayer->m_flNextAttack( ) && LocalPlayerData::m_Weapon->m_iClip1( ) > 0;

	if ( Target && best_anims && best_hitbox != -1 )
	{
		m_RageCurrentTargetIndex = targetID;
		ShouldStopSlide = false;

		Vector Angle = Math::CalcAng( LocalPlayerData::m_EyePosition, Aimpoint );

		if ( g_LocalPlayer->m_fFlags( ) & FL_ONGROUND && !CMovement::Get( ).m_InSlow )
		{
			bool should_stop = GetTicksToShoot( ) <= GetTicksToStop( )
				|| ((m_Config.autostop) && !canShoot);

			if ( should_stop )
			{
				if ( !ShouldStopSlide )
					AutoStop( );

				ShouldStopSlide = true;
			}
			else
				ShouldStopSlide = false;
		}

		if ( m_Config.autoscope && (m_Wpn->IsSniperIDX( ) || m_Wpn->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_AUG || m_Wpn->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_SG556
			) && m_Wpn->m_ZoomLVL( ) == 0 ) {
			g_pCmd->buttons |= IN_ATTACK2;
			return;
		}

		if ( canShoot && Hitchance( best_anims, Aimpoint, m_Config.hitchance / 100.f, best_hitbox ) )
		{
			if ( !CMovement::Get( ).m_InDuck )
				CTX::m_SendPacket = true;

			shot = RageShot = true;
			//RageShot = true;

			QAngle m_To = { Angle.x, Angle.y, Angle.z };

			if ( (g_pCmd->buttons & IN_ATTACK) && ShootNextTick )
				ShootNextTick = false;

			g_pCmd->viewangles = m_To;

			if ( !g_Options.ragebot.SilentAim )
				g_EngineClient->SetViewAngles( &m_To );

			if ( g_Options.ragebot.AutoFire )
				g_pCmd->buttons |= IN_ATTACK;

			static auto weapon_recoil_scale = g_CVar->FindVar( "weapon_recoil_scale" );

			if ( g_pCmd->buttons & IN_ATTACK )
				g_pCmd->viewangles -= g_LocalPlayer->m_aimPunchAngle( ) * weapon_recoil_scale->GetFloat( );

			if (/* g_Options.ragebot.Backtracking*/ /*&&*/ g_pCmd->buttons & IN_ATTACK /*&& !ForRage::ShouldTrackWiaExploit( )*/ )
				g_pCmd->tick_count = TIME_TO_TICKS( best_anims->SimTime + LagComp::Get( ).LerpTime( ) );

			MissCount::Get( ).SetData( Target, targetID, Angle );
		}
	}
}

// ----------------------------------------------------------- //

bool CRage::CockRevolver( CUserCmd* usercmd, C_BaseCombatWeapon* local_weapon )
{
	constexpr float REVOLVER_COCK_TIME = 0.2421875f;
	const int count_needed = floor( REVOLVER_COCK_TIME / g_GlobalVars->interval_per_tick );
	static int cocks_done = 0;

	if ( !local_weapon || local_weapon->m_Item( ).m_iItemDefinitionIndex( ) != WEAPON_REVOLVER ||
		g_LocalPlayer->m_flNextAttack( ) > g_GlobalVars->curtime ||
		local_weapon->IsReloading( ) )
	{
		if ( local_weapon && local_weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
			usercmd->buttons &= ~IN_ATTACK;

		cocks_done = 0;
		return true;
	}

	if ( cocks_done < count_needed )
	{
		usercmd->buttons |= IN_ATTACK;
		++cocks_done;
		return false;
	}
	else
	{
		usercmd->buttons &= ~IN_ATTACK;
		cocks_done = 0;
		return true;
	}

	usercmd->buttons |= IN_ATTACK;

	float curtime = TICKS_TO_TIME( g_LocalPlayer->m_nTickBase( ) );
	static float next_shoot_time = 0.f;

	bool ret = false;

	if ( next_shoot_time - curtime < -0.5 )
		next_shoot_time = curtime + 0.2f - g_GlobalVars->interval_per_tick;

	if ( next_shoot_time - curtime - g_GlobalVars->interval_per_tick <= 0.f ) 
	{
		next_shoot_time = curtime + 0.2f;
		ret = true;
	}

	return ret;
}

void CRage::AutoStop( )
{
	auto wpn_info = LocalPlayerData::m_Weapon->GetCSWeaponData( );

	if ( !wpn_info )
		return;

	auto get_standing_accuracy = [ & ]( ) -> const float
	{
		const auto max_speed = LocalPlayerData::m_Weapon->m_ZoomLVL( ) > 0 ? wpn_info->flMaxPlayerSpeedAlt : wpn_info->flMaxPlayerSpeed;
		return max_speed / 3.f;
	};

	auto velocity = g_LocalPlayer->m_vecVelocity( );
	float speed = velocity.Length2D( );
	float max_speed = (LocalPlayerData::m_Weapon->m_ZoomLVL( ) == 0 ? wpn_info->flMaxPlayerSpeed : wpn_info->flMaxPlayerSpeedAlt) * 0.1f;

	if ( speed > max_speed ) {
		ShouldStopSlide = false;
	}
	else {
		ShouldStopSlide = true;
		return;
	}

	if ( speed <= get_standing_accuracy( ) )
		return;

	Vector direction;
	Math::VectorToAng( velocity, direction );
	direction.y = m_ViewAngleStored.yaw - direction.y;
	Vector forward;

	Math::AngleToVec( direction, forward );
	Vector negated_direction = forward * -speed;
	g_pCmd->forwardmove = negated_direction.x;
	g_pCmd->sidemove = negated_direction.y;
}

bool CRage::CanHitHitbox( Vector start, Vector end, StoreAnimation* _animation, studiohdr_t* hdr, int box )
{
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel( _animation->Player->GetModel( ) );
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet( 0 );

	if ( !set )
		return false;

	mstudiobbox_t* studio_box = set->GetHitbox( box );
	if ( !studio_box )
		return false;

	Vector min, max;

	const auto is_capsule = studio_box->m_flRadius != -1.f;

	if ( is_capsule )
	{
		Math::VectorTransform( studio_box->bbmin, _animation->Bones[ studio_box->bone ], min );
		Math::VectorTransform( studio_box->bbmax, _animation->Bones[ studio_box->bone ], max );

		const auto dist = Segment( start, end, min, max );

		if ( dist < studio_box->m_flRadius )
			return true;
	}

	if ( !is_capsule )
	{
		Math::VectorTransform( Math::vector_rotate( studio_box->bbmin, studio_box->rotation ), _animation->Bones[ studio_box->bone ], min );
		Math::VectorTransform( Math::vector_rotate( studio_box->bbmax, studio_box->rotation ), _animation->Bones[ studio_box->bone ], max );

		Math::vector_i_transform( start, _animation->Bones[ studio_box->bone ], min );
		Math::vector_i_rotate( end, _animation->Bones[ studio_box->bone ], max );

		if ( Segment( min, max, studio_box->bbmin, studio_box->bbmax ) )
			return true;
	}

	return false;
}

void CRage::LinearExtrapolation( C_BasePlayer* m_entity )
{
	float simtime_delta = m_entity->m_flSimulationTime( ) - m_entity->m_flOldSimulationTime( );
	int choked_ticks = std::clamp( TIME_TO_TICKS( simtime_delta ), 1, 14 );
	Vector lastOrig;

	if ( lastOrig.Length( ) != m_entity->m_vecOrigin( ).Length( ) )
		lastOrig = m_entity->m_vecOrigin( );

	float delta_distance = (m_entity->m_vecOrigin( ) - lastOrig).LengthSqr( );
	if ( delta_distance > 4096.f )
	{
		Vector velocity_per_tick = m_entity->m_vecVelocity( ) * g_GlobalVars->interval_per_tick;
		auto new_origin = m_entity->m_vecOrigin( ) + (velocity_per_tick * choked_ticks);
		m_entity->SetAbsOrig( new_origin );
	}
}

int CRage::GetTicksToStop( )
{
	static auto predict_velocity = [ ]( Vector* velocity )
	{
		float speed = velocity->Length2D( );
		static auto sv_friction = g_CVar->FindVar( "sv_friction" );
		static auto sv_stopspeed = g_CVar->FindVar( "sv_stopspeed" );

		if ( speed >= 1.f )
		{
			float friction = sv_friction->GetFloat( );
			float stop_speed = std::max< float >( speed, sv_stopspeed->GetFloat( ) );
			float time = std::max< float >( g_GlobalVars->interval_per_tick, g_GlobalVars->frametime );
			*velocity *= std::max< float >( 0.f, speed - friction * stop_speed * time / speed );
		}
	};

	Vector vel = g_LocalPlayer->m_vecVelocity( );
	int ticks_to_stop = 0;
	while ( true )
	{
		if ( vel.Length2D( ) < 1.f )
			break;

		predict_velocity( &vel );
		ticks_to_stop++;
	}

	return ticks_to_stop;
}

bool CRage::IsAbleToShoot( float mtime )
{
	auto time = mtime;
	const auto info = LocalPlayerData::m_Weapon->GetCSWeaponData( );

	if ( !info )
		return false;

	if ( (g_LocalPlayer->m_flNextAttack( ) > time) || LocalPlayerData::m_Weapon->m_flNextPrimaryAttack( ) > time || LocalPlayerData::m_Weapon->m_flNextSecondaryAttack( ) > time )
	{
		if ( LocalPlayerData::m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) != 64 && LocalPlayerData::m_Weapon->IsPistolIDX( ) || LocalPlayerData::m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_DEAGLE )
			g_pCmd->buttons &= ~IN_ATTACK;

		return false;
	}

	return true;
}

int CRage::GetTicksToShoot( )
{
	if ( IsAbleToShoot( TICKS_TO_TIME( CSGO::m_TicksAllowed ) ) )
		return -1;

	auto flServerTime = ( CSGO::m_TicksAllowed )*g_GlobalVars->interval_per_tick;
	auto flNextPrimaryAttack = LocalPlayerData::m_Weapon->m_flNextPrimaryAttack( );

	return TIME_TO_TICKS( fabsf( flNextPrimaryAttack - flServerTime ) );
}

std::vector<Vector> CRage::GetPoints( C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[ 128 ] )
{
	std::vector<Vector> vPoints;

	if ( !pBaseEntity )
		return vPoints;

	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel( pBaseEntity->GetModel( ) );
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet( 0 );

	if ( !set )
		return vPoints;

	mstudiobbox_t* untransformedBox = set->GetHitbox( iHitbox );
	if ( !untransformedBox )
		return vPoints;

	Vector vecMin = { 0, 0, 0 };
	Math::VecTransWrap(
		untransformedBox->bbmin, BoneMatrix[ untransformedBox->bone ], vecMin );

	Vector vecMax = { 0, 0, 0 };
	Math::VecTransWrap(
		untransformedBox->bbmax, BoneMatrix[ untransformedBox->bone ], vecMax );

	float mod = untransformedBox->m_flRadius != -1.f ? untransformedBox->m_flRadius : 0.f;
	Vector max;
	Vector min;

	float PointScale = 0.75f;

	if ( pBaseEntity->m_vecVelocity( ).Length( ) > 300.f && iHitbox > 0 )
		PointScale = 0.f;
	else
	{
		if ( iHitbox <= ( int )1 )
			PointScale = m_Config.head_scale / 100.f;

		else if ( iHitbox <= ( int )7 )
			PointScale = m_Config.body_scale / 100.f;
	}

	Vector m_ModMin = Vector( untransformedBox->bbmin.x - mod, untransformedBox->bbmin.y - mod,
		untransformedBox->bbmin.z - mod );

	Vector m_ModMax = Vector( untransformedBox->bbmax.x - mod, untransformedBox->bbmax.y - mod,
		untransformedBox->bbmax.z - mod );

	Math::VecTransWrap(
		m_ModMax, BoneMatrix[ untransformedBox->bone ], max );

	Math::VecTransWrap(
		m_ModMin, BoneMatrix[ untransformedBox->bone ], min );

	auto center = (min + max) * 0.5f;
	if ( PointScale <= 0.05f ) {
		vPoints.push_back( center );
		return vPoints;
	}

	auto InLineClamp = [ ]( float val, float min, float max )
	{
		if ( val < min )
			return min;

		if ( val > max )
			return max;

		return val;
	};

	Vector curAngles = Math::CalcAng( center, LocalPlayerData::m_EyePosition );
	Vector forward;

	Math::AngleToVec( curAngles, forward );
	Vector right = forward.CrossProductVec( Vector( 0, 0, 1 ) );
	Vector left = Vector( -right.x, -right.y, right.z );

	if ( iHitbox == 0 )
	{
		for ( auto i = 0; i < 4; ++i )
			vPoints.push_back( center );

		vPoints[ 1 ].x += untransformedBox->m_flRadius * InLineClamp( 0.f, PointScale - 0.2f, 0.87f );
		vPoints[ 2 ].x -= untransformedBox->m_flRadius * InLineClamp( 0.f, PointScale - 0.2f, 0.87f );
		vPoints[ 3 ].z += untransformedBox->m_flRadius * PointScale - 0.05f;
	}

	else if ( iHitbox == 1 )
		vPoints.push_back( center );

	else if ( iHitbox == 7 || iHitbox == 8 || iHitbox == 9 || iHitbox == 10 || iHitbox == 11 || iHitbox == 12 ) {

		if ( iHitbox == 7 || iHitbox == 8 )
		{
			vPoints.push_back( center );
		}
		else if ( iHitbox == 9 || iHitbox == 10 )
		{
			vPoints.push_back( center );
		}
		else if ( iHitbox == 11 || iHitbox == 12 )
		{
			vPoints.push_back( center );
			vPoints[ 0 ].z += 5.f;
		}
	}
	else if ( iHitbox == 13 || iHitbox == 14 || iHitbox == 15 || iHitbox == 16 || iHitbox == 17 || iHitbox == 18 )
	{
		vPoints.push_back( center );
	}
	else
	{
		for ( auto i = 0; i < 3; ++i )
			vPoints.push_back( center );

		vPoints[ 1 ] += right * (untransformedBox->m_flRadius * PointScale);
		vPoints[ 2 ] += left * (untransformedBox->m_flRadius * PointScale);
	}

	return vPoints;
}

void CRage::BackupPlayer( StoreAnimation* anims )
{
	auto i = anims->Player->EntIndex( );
	backup_anims[ i ].Origin = anims->Player->m_vecOrigin( );
	backup_anims[ i ].AbsOrigin = anims->Player->GetAbsOrigin( );
	backup_anims[ i ].Mins = anims->Player->GetCollideable( )->OBBMins( );
	backup_anims[ i ].Maxs = anims->Player->GetCollideable( )->OBBMaxs( );
	backup_anims[ i ].BoneCache = anims->Player->m_CachedBoneData( ).Base( );
}

void CRage::SetAnims( StoreAnimation* anims )
{
	anims->Player->m_vecOrigin( ) = anims->Origin;
	anims->Player->SetAbsOrig( anims->AbsOrigin );
	anims->Player->GetCollideable( )->OBBMins( ) = anims->Mins;
	anims->Player->GetCollideable( )->OBBMaxs( ) = anims->Maxs;
	memcpy( anims->Player->m_CachedBoneData( ).Base( ), anims->Bones,
		sizeof( matrix3x4_t ) * anims->Player->m_CachedBoneData( ).Count( ) );
}

void CRage::RestorePlayer( StoreAnimation* anims )
{
	auto i = anims->Player->EntIndex( );
	anims->Player->m_vecOrigin( ) = backup_anims[ i ].Origin;
	anims->Player->SetAbsOrig( backup_anims[ i ].AbsOrigin );
	anims->Player->GetCollideable( )->OBBMins( ) = backup_anims[ i ].Mins;
	anims->Player->GetCollideable( )->OBBMaxs( ) = backup_anims[ i ].Maxs;
	memcpy( anims->Player->m_CachedBoneData( ).Base( ), backup_anims[ i ].BoneCache,
		sizeof( matrix3x4_t ) * anims->Player->m_CachedBoneData( ).Count( ) );
}

bool CRage::Hitchance( StoreAnimation* best_anims, Vector position, const float chance, int box ) {

	ForRage::BuildTable( );

	float HITCHANCE_MAX = 100.f;
	const auto weapon = LocalPlayerData::m_Weapon;

	if ( !weapon )
		return false;

	const auto info = weapon->GetCSWeaponData( );

	if ( !info )
		return false;

	if ( ( LocalPlayerData::m_EyePosition - position ).Length( ) > info->flRange ) {
		return true;
	}

	const auto studio_model = g_MdlInfo->GetStudiomodel( best_anims->Player->GetModel( ) );

	if ( !studio_model )
		return false;

	const auto id = weapon->m_Item( ).m_iItemDefinitionIndex( );
	const auto round_acc = [ ]( const float accuracy ) { return roundf( accuracy * 1000.f ) / 1000.f; };
	const auto crouched = g_LocalPlayer->m_fFlags( ) & FL_DUCKING;

	const auto weapon_inaccuracy = weapon->GetInaccuracy( );

	auto start = LocalPlayerData::m_EyePosition;
	const auto aim_angle = Math::CalcAng( start, position );
	Vector forward, right, up;
	Math::AngleVectorMulti( aim_angle, forward, right, up );

	auto current = 0;
	int awalls_hit = 0;

	Vector total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	for ( auto i = 0u; i < ForRage::TotalSeeds; i++ )
	{
		seed = &ForRage::PrecomputedSeeds[ i ];

		inaccuracy = std::get<0>( *seed ) * weapon_inaccuracy;
		spread_x = std::get<2>( *seed ) * inaccuracy;
		spread_y = std::get<1>( *seed ) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).Normalized( );

		Math::VectorToAng( total_spread, spread_angle );
		Math::AngleToVec( spread_angle, end );

		end = start + end.Normalized( ) * info->flRange;

		trace_t trace;
		Ray_t ray;

		ray.Init( start, end );
		g_EngineTrace->ClipRayToEntity( ray, MASK_SHOT | CONTENTS_GRATE, best_anims->Player, &trace );

		if ( auto ent = trace.hit_entity; ent )
		{
			if ( ent == best_anims->Player )
			{
				current++;

				if ( m_Config.boost > 1.f && Autowall::Get( ).CanHit(LocalPlayerData::m_EyePosition, end, g_LocalPlayer, best_anims->Player, box) > 1.f )
					awalls_hit++;
			}
		}
			
		if ( ((static_cast< float >(current) / static_cast< float >( ForRage::TotalSeeds )) >= (chance)) )
		{
			if ( ((static_cast< float >(awalls_hit) / static_cast< float >(ForRage::TotalSeeds)) >= (m_Config.boost / HITCHANCE_MAX)) || m_Config.boost <= 1.f )
				return true;
		}

		if ( static_cast< float >(current + ForRage::TotalSeeds - i) / static_cast< float >(ForRage::TotalSeeds) < chance )
			return false;
	}

	return static_cast< float >(current) / static_cast< float >(ForRage::TotalSeeds) >= chance;
}

Vector CRage::HeadScan( StoreAnimation* anims, int& hitbox, float& best_damage, float min_dmg ) {

	Vector best_point = Vector( 0, 0, 0 );

	CTX::m_Mutex.lock( );
	memcpy( BoneMatrix, anims->Bones, sizeof( matrix3x4_t[ 128 ] ) );
	CTX::m_Mutex.unlock( );

	SetAnims( anims );

	int health = anims->Player->m_iHealth( );
	if ( min_dmg > health )
		min_dmg = health + 1;

	bool IsTryShiftDt = g_Options.ragebot.Acitve && g_Options.exploit.ActMode
		&& GetKeyState( g_Options.exploit.mExpltKey );

	bool DTCan = IsTryShiftDt && (!CMovement::Get( ).m_InDuck);
	bool Priority = g_Options.ragebot.PreferSafepoint && GetKeyState( g_Options.ragebot.SafeKey ) || m_Config.default_priors[ 1 ] && DTCan || m_Config.default_priors[ 2 ];

	auto Points = GetPoints( anims->Player, 0, BoneMatrix );
	for ( auto HitBox : Points ) {

		bool is_safepoint = CanHitHitbox( LocalPlayerData::m_EyePosition, HitBox, anims, 0, 0 );

		if ( Priority && !is_safepoint )
			continue;

		auto info = Autowall::Get().CanHit(LocalPlayerData::m_EyePosition, HitBox, g_LocalPlayer, anims->Player, hitbox);
		//auto info = Autowall::Get( ).CanHit( HitBox );
		if ( info > min_dmg && info > best_damage )
		{
			hitbox = 0;
			best_point = HitBox;
			best_damage = info;
			CMovement::Get( ).m_InPeek = true;
		}
		else
			CMovement::Get( ).m_InPeek = false;
	}

	RestorePlayer( anims );
	return best_point;
}

Vector CRage::GetPointTo( C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[ 128 ] )
{
	std::vector<Vector> vPoints;

	if ( !pBaseEntity )
		return Vector( 0, 0, 0 );

	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel( pBaseEntity->GetModel( ) );
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet( 0 );

	if ( !set )
		return Vector( 0, 0, 0 );

	mstudiobbox_t* untransformedBox = set->GetHitbox( iHitbox );
	if ( !untransformedBox )
		return Vector( 0, 0, 0 );

	Vector vecMin = { 0, 0, 0 };
	Math::VecTransWrap( untransformedBox->bbmin, BoneMatrix[ untransformedBox->bone ], vecMin );

	Vector vecMax = { 0, 0, 0 };
	Math::VecTransWrap( untransformedBox->bbmax, BoneMatrix[ untransformedBox->bone ], vecMax );

	float mod = untransformedBox->m_flRadius != -1.f ? untransformedBox->m_flRadius : 0.f;
	Vector max;
	Vector min;

	Vector inCalcMax = { untransformedBox->bbmax.x + mod, untransformedBox->bbmax.y + mod, untransformedBox->bbmax.z + mod };
	Vector inCalcMin = { untransformedBox->bbmin.x + mod, untransformedBox->bbmin.y + mod, untransformedBox->bbmin.z + mod };

	Math::VectorTransform( inCalcMax, BoneMatrix[ untransformedBox->bone ], max );
	Math::VectorTransform( inCalcMin, BoneMatrix[ untransformedBox->bone ], min );

	return (min + max) * 0.5f;
}

Vector CRage::FullScan( StoreAnimation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg ) {

	CTX::m_Mutex.lock( );
	memcpy( BoneMatrix, anims->Bones, sizeof( matrix3x4_t[ 128 ] ) );
	CTX::m_Mutex.unlock( );

	simtime = anims->SimTime;
	best_damage = -1;

	Vector best_point = Vector( 0, 0, 0 );
	Vector best_baim_point = Vector( 0, 0, 0 );

	int best_baim_damage = 0;
	int baim_hitbox = -1;
	SetAnims( anims );

	int priority_hitbox = GetCurrentPriorityHitbox( anims->Player );
	auto hitboxes = GetHitboxesToScan( anims->Player );
	std::vector<int> baim_hitboxes;

	for ( auto HitboxID : hitboxes )
	{
		if ( HitboxID >= 2 && HitboxID <= 5 )
			baim_hitboxes.push_back( HitboxID );
	}

	bool IsTryShiftDt = g_Options.ragebot.Acitve && g_Options.exploit.ActMode
		&& GetKeyState( g_Options.exploit.mExpltKey );

	bool should_scan_baim = m_Config.prefer_bod;
	bool DTCan = IsTryShiftDt && (!CMovement::Get( ).m_InDuck);
	bool Priority = g_Options.ragebot.PreferSafepoint && GetKeyState( g_Options.ragebot.SafeKey ) || m_Config.default_priors[ 1 ] && DTCan || m_Config.default_priors[ 2 ];

	if ( should_scan_baim )
	{
		for ( auto HitboxID : baim_hitboxes ) 
		{
			auto Points = GetPoints( anims->Player, HitboxID, BoneMatrix );
			for ( int k = 0; k < Points.size( ); k++ )
			{
				bool is_safepoint = CanHitHitbox( LocalPlayerData::m_EyePosition, Points[ k ], anims, 0, HitboxID );

				if ( (Priority) && !is_safepoint )
					continue;

				auto info = Autowall::Get().CanHit(LocalPlayerData::m_EyePosition, Points[k], g_LocalPlayer, anims->Player, hitbox);
				//auto info = Autowall::Get( ).CanHit( Points[ k ] );

				if ( info > min_dmg && info > best_baim_damage )
				{
					baim_hitbox = HitboxID;
					best_baim_point = Points[ k ];
					best_baim_damage = info;

					if ( best_baim_damage > anims->Player->m_iHealth( ) + 4 && m_Config.prefer_lethal )
					{
						best_point = best_baim_point;
						hitbox = baim_hitbox;
						best_damage = best_baim_damage;
						CMovement::Get( ).m_InPeek = true;

						RestorePlayer( anims );

						return best_point;
					}
					else if ( best_baim_damage * 1.939f > anims->Player->m_iHealth( ) && m_Config.prefer_bod ) 
					{
						if ( (DTCan && TickBaseSys::Get( ).IsCharged) || TickBaseSys::Get( ).IsShotDT ) 
						{
							best_point = best_baim_point;
							hitbox = baim_hitbox;
							best_damage = best_baim_damage;
							CMovement::Get( ).m_InPeek = true;

							RestorePlayer( anims );

							return best_point;
						}
						else
							CMovement::Get( ).m_InPeek = false;
					}
					else
						CMovement::Get( ).m_InPeek = false;
				}

			}
		}
	}

	for ( auto HitboxID : hitboxes )
	{
		if ( m_Config.default_priors[ 0 ] && anims->Player->m_vecVelocity( ).Length2D( ) > 200 && anims->Player->m_fFlags( ) & FL_ONGROUND )
		{
			auto point = GetPointTo( anims->Player, HitboxID, BoneMatrix );
			auto info = Autowall::Get().CanHit(LocalPlayerData::m_EyePosition, point, g_LocalPlayer, anims->Player, hitbox);
			//auto info = Autowall::Get( ).CanHit( point );
			
			bool is_safepoint = CanHitHitbox( LocalPlayerData::m_EyePosition, point, anims, 0, HitboxID );

			if ( Priority && !is_safepoint )
				continue;

			if ( info > min_dmg && info > best_damage )
			{
				hitbox = HitboxID;
				best_point = point;
				best_damage = info;
				CMovement::Get( ).m_InPeek = true;
			}
			else
				CMovement::Get( ).m_InPeek = false;
		}
		else
		{
			auto Points = GetPoints( anims->Player, HitboxID, BoneMatrix );
			for ( int k = 0; k < Points.size( ); k++ )
			{
				bool is_safepoint = CanHitHitbox( LocalPlayerData::m_EyePosition, Points[ k ], anims, 0, HitboxID );

				if ( Priority && !is_safepoint )
					continue;

				auto info = Autowall::Get().CanHit(LocalPlayerData::m_EyePosition, Points[k], g_LocalPlayer, anims->Player, hitbox);
				//auto info = Autowall::Get( ).CanHit( Points[ k ] );

				if ( info > min_dmg && info > best_damage )
				{
					hitbox = HitboxID;
					best_point = Points[ k ];
					best_damage = info;
					CMovement::Get( ).m_InPeek = true;
				}
				else
					CMovement::Get( ).m_InPeek = false;
			}
		}
	}

	if ( hitbox == 0 && best_baim_damage > 0 && (best_baim_damage > anims->Player->m_iHealth( ) * 0.75 || best_damage < best_baim_damage * 1.3f) && baim_hitbox != -1 && m_Config.prefer_bod )
	{
		best_point = best_baim_point;
		hitbox = baim_hitbox;
		best_damage = best_baim_damage;
		CMovement::Get( ).m_InPeek = true;
	}
	else
		CMovement::Get( ).m_InPeek = false;

	RestorePlayer( anims );

	return best_point;
}

std::vector<int> CRage::GetHitboxesToScan( C_BasePlayer* e )
{
	std::vector<int> hitboxes;
	bool canAchieve = m_Config.hitbox_selected[ 1 ] || m_Config.hitbox_selected[ 2 ] || m_Config.hitbox_selected[ 3 ]
		|| m_Config.hitbox_selected[ 4 ] || m_Config.hitbox_selected[ 5 ];

	if ( canAchieve && GetCurrentPriorityHitbox( e ) == ( int )HITBOX_PELVIS )
	{
		hitboxes.push_back( ( int )HITBOX_CHEST );
		hitboxes.push_back( ( int )HITBOX_STOMACH );
		hitboxes.push_back( ( int )HITBOX_PELVIS );

		return hitboxes;
	}

	if ( m_Config.prior_hit == 1 )
	{
		hitboxes.push_back( ( int )HITBOX_HEAD );
	}
	else if ( m_Config.prior_hit == 2 )
	{
		hitboxes.push_back( ( int )HITBOX_CHEST );
		hitboxes.push_back( ( int )HITBOX_STOMACH );
		hitboxes.push_back( ( int )HITBOX_PELVIS );
	}

	if ( m_Config.hitbox_selected[ 0 ] )
		hitboxes.push_back( ( int )HITBOX_HEAD );

	if ( m_Config.hitbox_selected[ 1 ] )
		hitboxes.push_back( ( int )HITBOX_UPPER_CHEST );

	if ( m_Config.hitbox_selected[ 2 ] )
		hitboxes.push_back( ( int )HITBOX_CHEST );

	if ( m_Config.hitbox_selected[ 3 ] )
		hitboxes.push_back( ( int )HITBOX_STOMACH );

	if ( m_Config.hitbox_selected[ 4 ] )
		hitboxes.push_back( ( int )HITBOX_PELVIS );

	if ( m_Config.hitbox_selected[ 5 ] )
	{
		hitboxes.push_back( 17 );
		hitboxes.push_back( 15 );
	}

	if ( m_Config.hitbox_selected[ 6 ] )
	{
		hitboxes.push_back( 10 );
		hitboxes.push_back( 9 );
	}

	if ( m_Config.hitbox_selected[ 7 ] )
	{
		hitboxes.push_back( 11 );
		hitboxes.push_back( 12 );
	}

	return hitboxes;
}

Vector CRage::GetAimVector( C_BasePlayer* pTarget, float& simtime, Vector& origin, StoreAnimation*& best_anims, int& hitbox )
{
	CMovement::Get( ).m_InPeek = false;

	if ( GetHitboxesToScan( pTarget ).size( ) == 0 )
		return Vector( 0, 0, 0 );

	float m_damage = m_Config.damage;

	auto latest_animation = 
		Backtrack::Get( ).GetLatestAnimtion( pTarget );

	auto record = 
		latest_animation;

	if ( !record.has_value( ) || !record.value( )->Player )
		return Vector( 0, 0, 0 );

	BackupPlayer( record.value( ) );

	if ( !g_Options.ragebot.Backtracking /*|| ForRage::ShouldTrackWiaExploit( )*/ )
	{
		float damage = -1.f;
		best_anims = record.value( );
		return FullScan( record.value( ), hitbox, simtime, damage, m_damage );
	}

	bool isForcing = g_Options.ragebot.ForceBaim && GetKeyState( g_Options.ragebot.BaimKey );

	if ( m_Config.shoot_pr && !isForcing && LocalPlayerData::m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) != WEAPON_AWP )
	{
		record = Backtrack::Get( ).GetLatestFireAnimation( pTarget );
		if ( record.has_value( ) && record.value( )->Player ) 
		{
			float damage = -1.f;
			best_anims = record.value( );
			simtime = record.value( )->SimTime;
			Vector backshoot = HeadScan( record.value( ), hitbox, damage, m_damage );
			if ( backshoot != Vector( 0, 0, 0 ) )
				return backshoot;
		}
	}

	auto oldest_animation = Backtrack::Get( ).GetOldestAnimation( pTarget );
	float best_damage_0 = -1.f, best_damage_1 = -1.f, best_damage_2 = -1.f;

	record = latest_animation;
	Vector full_0;

	if ( record.has_value( ) )
	{
		float damage = -1.f;
		full_0 = FullScan( record.value( ), hitbox, simtime, damage, m_damage );
		if ( full_0 != Vector( 0, 0, 0 ) )
		{
			best_damage_0 = damage;
			if ( best_damage_0 > pTarget->m_iHealth( ) )
			{
				best_anims = record.value( );
				return full_0;
			}
		}
	}

	record = oldest_animation;
	Vector full_1;

	if ( record.has_value( ) && record.value()->Origin.DistTo( latest_animation.value()->Origin ) > 25.f )
	{
		float damage = -1.f;
		full_1 = FullScan( record.value( ), hitbox, simtime, damage, m_damage );
		if ( full_1 != Vector( 0, 0, 0 ) )
		{
			best_damage_1 = damage;
			if ( best_damage_1 > pTarget->m_iHealth( ) )
			{
				best_anims = record.value( );
				return full_1;
			}
		}
	}

	if ( !(best_damage_0 > 0.f || best_damage_1 > 0.f) )
	{
		auto valid_animations = Backtrack::Get( ).GetValidAnimationNonLimited( pTarget );
		int i = 0;

		if ( !isForcing ) 
		{
			float damage = -1.f;
			Vector best_point = Vector( 0, 0, 0 );
			for ( const auto& rec : valid_animations ) 
			{
				Vector point = HeadScan( rec, hitbox, m_damage, damage );
				if ( point != Vector( 0, 0, 0 ) ) {
					best_anims = rec;
					simtime = rec->SimTime;
					best_point = point;
				}
				if ( i > 1 )
					break;
			}
			if ( damage != -1.f )
				return best_point;
		}
	}

	if ( best_damage_0 >= best_damage_1 && best_damage_0 >= 1.f )
	{
		record = latest_animation;
		best_anims = record.value( );
		return full_0;
	}
	else if ( best_damage_1 >= 1.f )
	{
		record = oldest_animation;
		best_anims = record.value( );
		return full_1;
	}

	return Vector( 0, 0, 0 );
}

int CRage::GetCurrentPriorityHitbox( C_BasePlayer* e )
{
	bool isForcing = g_Options.ragebot.ForceBaim && GetKeyState( g_Options.ragebot.BaimKey );

	if ( isForcing )
		return 2;

	if ( m_Config.body_priors[ 0 ] && !(e->m_fFlags( ) & FL_ONGROUND) )
		return 2;

	if ( m_Config.body_priors[ 1 ] && e->m_flDuckAmount( ) > 0 )
		return 2;

	if ( m_Config.body_priors[ 2 ] && m_MissedShot[ e->EntIndex( ) ] > 2 )
		return 2;

	if ( m_Config.prefer_bod )
	{
		if ( !(e->m_fFlags( ) & FL_ONGROUND) || m_MissedShot[ e->EntIndex( ) ] > 3 )
			return 2;
	}

	return 0;
}

float CRage::Segment( const Vector s1, const Vector s2, const Vector k1, const Vector k2 )
{
	static auto constexpr epsilon = 0.00000001;

	auto u = s2 - s1;
	auto v = k2 - k1;
	const auto w = s1 - k1;

	const auto a = u.Dot( u );
	const auto b = u.Dot( v );
	const auto c = v.Dot( v );
	const auto d = u.Dot( w );
	const auto e = v.Dot( w );
	const auto D = a * c - b * b;
	float sn, sd = D;
	float tn, td = D;

	if ( D < epsilon ) {
		sn = 0.0;
		sd = 1.0;
		tn = e;
		td = c;
	}
	else {
		sn = b * e - c * d;
		tn = a * e - b * d;

		if ( sn < 0.0 ) {
			sn = 0.0;
			tn = e;
			td = c;
		}
		else if ( sn > sd ) {
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	if ( tn < 0.0 ) {
		tn = 0.0;

		if ( -d < 0.0 )
			sn = 0.0;
		else if ( -d > a )
			sn = sd;
		else {
			sn = -d;
			sd = a;
		}
	}
	else if ( tn > td ) {
		tn = td;

		if ( -d + b < 0.0 )
			sn = 0;
		else if ( -d + b > a )
			sn = sd;
		else {
			sn = -d + b;
			sd = a;
		}
	}

	const float sc = abs( sn ) < epsilon ? 0.0 : sn / sd;
	const float tc = abs( tn ) < epsilon ? 0.0 : tn / td;

	m128 n;
	auto dp = w + u * sc - v * tc;
	n.f[ 0 ] = dp.Dot( dp );
	const auto calc = sqrt_ps( n.v );
	return reinterpret_cast< const m128* >(&calc)->f[ 0 ];
}