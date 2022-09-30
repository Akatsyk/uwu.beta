#include "autowall.h"
#include "Math.h"
#include "../sdk/interfaces/IEngineTrace.hpp"
#include "../sdk/utils/math.hpp"
#include "../sdk/utils/utils.hpp"

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

float Autowall::GetHitgroupDamageMultiplier( int iHitGroup ) {
	switch ( iHitGroup ) {
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}
	return 1.0f;
}

//void Autowall::ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr ) {
//	float smallestFraction = tr->fraction;
//	constexpr float maxRange = 60.0f;
//
//	Vector delta( vecAbsEnd - vecAbsStart );
//	const float delta_length = delta.Length( );
//	delta.NormalizeInPlace( );
//
//	Ray_t ray;
//	ray.Init( vecAbsStart, vecAbsEnd );
//
//	for ( int i = 1; i <= g_GlobalVars->maxClients; ++i ) {
//		auto ent = C_BasePlayer::GetPlayerByIndex( i );
//		if ( !ent || ent->IsDormant( ) || ent->m_lifeState( ) != LIFE_ALIVE )
//			continue;
//
//		if ( filter && !filter->ShouldHitEntity( ent, mask ) )
//			continue;
//
//		auto collideble = ent->GetCollideable( );
//		auto mins = collideble->OBBMins( );
//		auto maxs = collideble->OBBMaxs( );
//
//		auto obb_center = (maxs + mins) * 0.5f;
//		auto extend = (obb_center - vecAbsStart);
//		auto rangeAlong = delta.Dot( extend );
//
//		float range;
//		if ( rangeAlong >= 0.0f ) {
//			if ( rangeAlong <= delta_length )
//				range = Vector( obb_center - ((delta * rangeAlong) + vecAbsStart) ).Length( );
//			else
//				range = -(obb_center - vecAbsEnd).Length( );
//		}
//		else {
//			range = -extend.Length( );
//		}
//
//		if ( range >= 0.0f && range <= maxRange ) {
//			trace_t playerTrace;
//			g_EngineTrace->ClipRayToEntity( ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace );
//			if ( playerTrace.fraction < smallestFraction ) {
//				*tr = playerTrace;
//				smallestFraction = playerTrace.fraction;
//			}
//		}
//	}
//}

void IClientEntity::GetWorldSpaceCenter(Vector& wSpaceCenter)
{
	void* cRender = (void*)(this + 0x4);
	typedef void(__thiscall* fn)(void*, Vector&, Vector&);
	Vector va, vb;
	CallVFunction<fn>(cRender, 17)(cRender, va, vb); // GetRenderBounds : 17
	wSpaceCenter.z += (va.z + vb.z) * 0.5f;
}

void Autowall::ClipTraceToPlayers(C_BasePlayer* pPlayer, const Vector& start, const Vector& end, uint32_t mask, CTraceFilter* filter, trace_t* tr) {
	//Ray_t Ray;
	//Ray.Init(start, end);

	trace_t NewTrace;
	if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->IsDormant() || pPlayer->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return;

	if (filter && !filter->ShouldHitEntity(pPlayer, NULL))
		return;

	Vector WorldSpaceCenter; pPlayer->GetWorldSpaceCenter(WorldSpaceCenter);
	float_t flRange = Math::DistanceToRay(WorldSpaceCenter, start, end);
	if (flRange < 0.0f || flRange > 60.0f)
		return;

	Ray_t ray;
	ray.Init(start, end);
	g_EngineTrace->ClipRayToEntity(ray, mask, pPlayer, &NewTrace);
	if (NewTrace.fraction > tr->fraction)
		std::memcpy(tr, &NewTrace, sizeof(trace_t));
}

void Autowall::ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& current_damage ) 
{
	if (!enterTrace.hit_entity 
		|| !enterTrace.hit_entity->GetClientClass() 
		|| !g_LocalPlayer->m_hActiveWeapon().Get() 
		|| enterTrace.hit_entity->GetClientClass()->m_ClassID != ClassId_CCSPlayer)
		return;

	static auto mp_damage_scale_ct_head = g_CVar->FindVar( "mp_damage_scale_ct_head" );
	static auto mp_damage_scale_t_head = g_CVar->FindVar( "mp_damage_scale_t_head" );
	static auto mp_damage_scale_ct_body = g_CVar->FindVar( "mp_damage_scale_ct_body" );
	static auto mp_damage_scale_t_body = g_CVar->FindVar( "mp_damage_scale_t_body" );


	const int hitgroup = enterTrace.hitgroup;
	C_BasePlayer* enemy = (C_BasePlayer*)enterTrace.hit_entity;
	auto team = enemy->m_iTeamNum( );
	auto head_scale = team == 2 ? mp_damage_scale_ct_head->GetFloat( ) : mp_damage_scale_t_head->GetFloat( );
	auto body_scale = team == 2 ? mp_damage_scale_ct_body->GetFloat( ) : mp_damage_scale_t_body->GetFloat( );

	auto armor_heavy = enemy->m_bHasHeavyArmor( );
	auto armor_value = static_cast< float >(enemy->m_ArmorValue( ));

	if ( armor_heavy )
		head_scale *= 0.5f;

	// ref: CCSPlayer::TraceAttack
	switch ( hitgroup ) {
	case HITGROUP_HEAD:
		current_damage = (current_damage * 4.f) * head_scale;
		break;
	case HITGROUP_CHEST:
	case 8:
		current_damage *= body_scale;
		break;
	case HITGROUP_STOMACH:
		current_damage = (current_damage * 1.25f) * body_scale;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		current_damage *= body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage = (current_damage * 0.75f) * body_scale;
		break;
	default:
		break;
	}

	static auto IsArmored = [ ]( C_BasePlayer* player, int hitgroup ) {
		auto has_helmet = player->m_bHasHelmet( );
		auto armor_value = static_cast< float >(player->m_ArmorValue( ));

		if ( armor_value > 0.f ) {
			switch ( hitgroup ) {
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case 8:
				return true;
				break;
			case HITGROUP_HEAD:
				return has_helmet || player->m_bHasHeavyArmor( );
				break;
			default:
				return player->m_bHasHeavyArmor( );
				break;
			}
		}

		return false;
	};

	if ( IsArmored( enemy, hitgroup ) ) {
		auto armor_scale = 1.f;
		auto armor_ratio = (weaponData->flArmorRatio * 0.5f);
		auto armor_bonus_ratio = 0.5f;

		if ( armor_heavy ) {
			armor_ratio *= 0.2f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.25f;
		}

		float new_damage = current_damage * armor_ratio;
		float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
		if ( estiminated_damage > armor_value )
			new_damage = (current_damage - (armor_value / armor_bonus_ratio));

		current_damage = new_damage;
	}
}

bool Autowall::TraceToExit( Vector& out, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace ) {
	static auto* sv_clip_penetration_traces_to_players = g_CVar->FindVar("sv_clip_penetration_traces_to_players");

	Vector          new_end;
	float           dist = 0.0f;
	int				iterations = 23;
	int				first_contents = 0;
	int             contents;
	//Ray_t r{};

	std::array < std::uintptr_t, 4 > aFilter
		=
	{
		*(std::uintptr_t*)(GetFilterSimpleVtable()),
		NULL,
		NULL,
		NULL
	};

	while (1)
	{
		iterations--;

		if (iterations <= 0 || dist > 90.f)
			break;

		dist += 4.0f;
		out = start + (dir * dist);

		contents = g_EngineTrace->GetPointContents(out, 0x4600400B, nullptr);

		if (first_contents == -1)
			first_contents = contents;

		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
			continue;

		new_end = out - (dir * 4.f);

		TraceLineNew(out, start, 0x4600400B, (C_BasePlayer*)(aFilter.data()), NULL, exit_trace);


		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX) != 0)
		{

			//	Ray_t ray;
				//ray.Init(out, start);
			CTraceFilter filter;
			filter.pSkip = exit_trace->hit_entity;
			Ray_t ray;
			ray.Init(out, start);
			g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, exit_trace);
			//TraceLineNew(out, start, MASK_SHOT_HULL, filter, NULL, exit_trace);

			if (exit_trace->DidHit() && !exit_trace->startsolid)
			{
				out = exit_trace->endpos;
				return true;
			}
			continue;
		}

		if (!exit_trace->DidHit() || exit_trace->startsolid)
		{
			if (enter_trace->hit_entity != g_EntityList->GetClientEntity(0)) {
				if (exit_trace->hit_entity && BreakableEntity(exit_trace->hit_entity))
				{
					exit_trace->surface.surfaceProps = enter_trace->surface.surfaceProps;
					exit_trace->endpos = start + dir;
					return true;
				}
			}

			continue;
		}

		if ((exit_trace->surface.flags & 0x80u) != 0)
		{
			if (enter_trace->hit_entity && BreakableEntity(enter_trace->hit_entity)
				&& exit_trace->hit_entity && BreakableEntity(exit_trace->hit_entity))
			{
				out = exit_trace->endpos;
				return true;
			}

			if (!(enter_trace->surface.flags & 0x80u))
				continue;
		}

		if (exit_trace->plane.normal.Dot(dir) <= 1.f) // exit nodraw is only valid if our entrace is also nodraw
		{
			out -= dir * (exit_trace->fraction * 4.0f);
			return true;
		}
	}

	return false;
}
void Autowall::TraceLine( Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr )
{
	Ray_t ray;
	ray.Init( absStart, absEnd );
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_EngineTrace->TraceRay( ray, mask, &filter, ptr );
}
void Autowall::TraceLineNew(Vector& absStart, const Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, int collision_group, trace_t* ptr)
{
	std::uintptr_t filter[4] = {
	*reinterpret_cast<std::uintptr_t*>(GetFilterSimpleVtable()),
	reinterpret_cast<std::uintptr_t>(ignore),
	(uintptr_t)(collision_group),
	0
	};

	//auto ray = Ray_t();
	//ray.Init(absStart, absEnd);
	CTraceFilter trace_filterr;
	trace_filterr.pSkip = (CTraceFilter*)(filter);
	Ray_t ray;
	ray.Init(absStart, absEnd);

	return g_EngineTrace->TraceRay(ray, mask, &trace_filterr, ptr);
}
uint32_t Autowall::GetFilterSimpleVtable()
{
	uint32_t filtersig = *reinterpret_cast < uint32_t*>("55 8B EC 83 E4 F0 83 EC 7C 56 52");
	return *reinterpret_cast<uint32_t*>(filtersig + 0x3d);
}
bool Autowall::BreakableEntity( IClientEntity* entity )
{
	uint32_t breakable_sig = *reinterpret_cast <uint32_t*>("55 8B EC 51 56 8B F1 85 F6 74 68");
	auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(IClientEntity*)>(breakable_sig);

	if (!entity || !entity->GetCollideable() || !entity->GetClientClass())
		return false;

	auto client_class = entity->GetClientClass();

	if (entity->EntIndex() > 0) {
		if (client_class)
		{
			auto v3 = (int)client_class->m_pNetworkName;
			if (*(DWORD*)v3 == 0x65724243)
			{
				if (*(DWORD*)(v3 + 7) == 0x53656C62)
					return 1;
			}
			if (*(DWORD*)v3 == 0x73614243)
			{
				if (*(DWORD*)(v3 + 7) == 0x79746974)
					return 1;
			}

			return is_breakable_fn(entity);
		}

		return is_breakable_fn(entity);
	}
	return 0;
	/*ClientClass* pClass = ( ClientClass* )entity->GetClientClass( );

	if ( !pClass )
	{
		return false;
	}

	if ( pClass == nullptr )
	{
		return false;
	}

	return pClass->m_ClassID == ClassId_CBreakableProp || pClass->m_ClassID == ClassId_CBreakableSurface;*/

}
//bool Autowall::TraceToExit( CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction )
//{
//	Vector start, end;
//	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
//	int firstContents = 0;
//
//	std::array < std::uintptr_t, 4 > aFilter
//		=
//	{
//		*(std::uintptr_t*)(GetFilterSimpleVtable()),
//		NULL,
//		NULL,
//		NULL
//	};
//
//	while ( currentDistance <= maxDistance )
//	{
//		currentDistance += rayExtension;
//
//		start = startPosition + direction * currentDistance;
//
//		if ( !firstContents )
//			firstContents = g_EngineTrace->GetPointContents( start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr );
//
//		int pointContents = g_EngineTrace->GetPointContents( start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr );
//
//		if ( !(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents )
//		{
//			end = start - (direction * rayExtension);
//
//			TraceLineNew(start, startPosition, 0x4600400B, (C_BasePlayer*)(aFilter.data()), NULL, &exitTrace);
//			//TraceLine( start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace );
//
//			if ( exitTrace.startsolid && (exitTrace.surface.flags & SURF_HITBOX) != 0 )
//			{
//				CTraceFilter filter;
//				filter.pSkip = exitTrace.hit_entity;
//				Ray_t ray;
//				ray.Init(start, startPosition);
//				g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &exitTrace);
//				//TraceLine( start, startPosition, MASK_SHOT_HULL, exitTrace.hit_entity, &exitTrace );
//
//				if ( exitTrace.DidHit( ) && !exitTrace.startsolid )
//				{
//					start = exitTrace.endpos;
//					return true;
//				}
//				continue;
//			}
//
//			if ( exitTrace.DidHit( ) && !exitTrace.startsolid )
//			{
//
//				//if ( BreakableEntity( enterTrace.hit_entity ) && BreakableEntity( exitTrace.hit_entity ) )
//				//{
//				//	return true;
//				//}
//
//				//if ( enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot( direction ) <= 1.f) )
//				//{
//				//	float multAmount = exitTrace.fraction * 4.f;
//				//	start -= direction * multAmount;
//				//	return true;
//				//}
//
//				if (enterTrace.hit_entity != g_EntityList->GetClientEntity(0)) 
//				{
//					if (enterTrace.hit_entity && BreakableEntity(exitTrace.hit_entity))
//					{
//						exitTrace.surface.surfaceProps = enterTrace.surface.surfaceProps;
//						exitTrace.endpos = startPosition + direction;
//						return true;
//					}
//				}
//
//				continue;
//			}
//
//			if ((exitTrace.surface.flags & 0x80u) != 0)
//			{
//				if (enterTrace.hit_entity && BreakableEntity(enterTrace.hit_entity)
//					&& exitTrace.hit_entity && BreakableEntity(exitTrace.hit_entity))
//				{
//					out = exitTrace.endpos;
//					return true;
//				}
//
//				if (!(enterTrace.surface.flags & 0x80u))
//					continue;
//			}
//
//			if (exitTrace.plane.normal.Dot(direction) <= 1.f) // exit nodraw is only valid if our entrace is also nodraw
//			{
//				out -= direction * (exitTrace.fraction * 4.0f);
//				return true;
//			}
//
//			//if ( !exitTrace.DidHit( ) || exitTrace.startsolid )
//			//{
//			//	if ( enterTrace.DidHitNonWorldEntity( ) && BreakableEntity( enterTrace.hit_entity ) )
//			//	{
//			//		//auto t = enterTrace;
//			//		exitTrace = enterTrace;
//			//		exitTrace.endpos = start + direction;
//			//		return true;
//			//	}
//
//			//	continue;
//			//}
//		}
//	}
//	return false;
//}

bool Autowall::HandleBulletPenetration(C_BasePlayer* ignore, CCSWeaponInfo* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_bullet_penetration, bool pskip)
{
	static auto* ff_damage_reduction_bullets = g_CVar->FindVar("ff_damage_reduction_bullets");

	if (possibleHitsRemaining <= 0 || weaponData->flPenetration <= 0 || currentDamage < 1) {
		//possibleHitsRemaining = 0;
		return false;
	}

	//SafeLocalPlayer() false;
	trace_t exitTrace;
	auto pEnemy = (C_BasePlayer*)enterTrace.hit_entity;
	auto* const enterSurfaceData = g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);
	const int enter_material = enterSurfaceData->game.material;

	if (!enterSurfaceData || enterSurfaceData->game.flPenetrationModifier <= 0)
		return false;

	const auto enter_penetration_modifier = enterSurfaceData->game.flPenetrationModifier;
	//float enterDamageModifier = enterSurfaceData->game.damagemodifier;// , modifier, finalDamageModifier, combinedPenetrationModifier;
	const bool isSolidSurf = (enterTrace.contents & CONTENTS_GRATE);
	const bool isLightSurf = (enterTrace.surface.flags & SURF_NODRAW);

	//if (!TraceToExit(dummy, &data.enterTrace, data.enterTrace.endpos, data.direction, &pEnemy))
	//	return false;

	auto* const exitSurfaceData = g_PhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	const int exitMaterial = exitSurfaceData->game.material;
	const float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	//float exitDamageModifier = exitSurfaceData->game.damagemodifier;

	float combined_damage_modifier = 0.16f;
	float combined_penetration_modifier;

	//Are we using the newer penetration system?
	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_damage_modifier = 0.05f;
		combined_penetration_modifier = 3;
	}
	else if (isSolidSurf || isLightSurf) {
		combined_damage_modifier = 0.16f;
		combined_penetration_modifier = 1;
	}
	else if (enter_material == CHAR_TEX_FLESH && ff_damage_reduction_bullets->GetFloat() == 0 && pEnemy != nullptr && pEnemy->IsPlayer() && pEnemy->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
	{
		if (ff_damage_bullet_penetration == 0)
		{
			// don't allow penetrating players when FF is off
			combined_penetration_modifier = 0;
			return false;
		}

		combined_penetration_modifier = ff_damage_bullet_penetration;
	}
	else {
		combined_penetration_modifier = (enter_penetration_modifier + exitSurfPenetrationModifier) / 2;
	}

	if (enter_material == exitMaterial) {
		if (exitMaterial == CHAR_TEX_WOOD || exitMaterial == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3;
		else if (exitMaterial == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2;
	}

	auto penetration_mod = fmaxf(0.f, (3.f / penetrationPower) * 1.25f);

	float modifier = fmaxf(0, 1.0f / combined_penetration_modifier);

	auto thickness = (exitTrace.endpos - enterTrace.endpos).Length();

	const auto lost_damage = ((modifier * 3.f) * penetration_mod + (currentDamage * combined_damage_modifier)) + (((thickness * thickness) * modifier) / 24.f);

	currentDamage -= std::fmaxf(0.f, lost_damage);

	if (currentDamage < 1.f)
		return false;

	eyePosition = exitTrace.endpos;
	--possibleHitsRemaining;

	return true;

	/*surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = *(float*)((uint8_t*)exit_surface_data + 76);
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if ((data.enterTrace.contents & CONTENTS_GRATE) != 0 || enter_material == 89 || enter_material == 71) {
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

	if (enter_material == exit_material) {
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->flPenetration) * 1.25f);
	float thickness = (exitTrace.endpos - data.enterTrace.endpos).Length();

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;

	data.src = exitTrace.endpos;
	data.penetrate_count--;
	return true;*/
}

void TraceLine( Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr ) 
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd );
	CTraceFilter filter;
	filter.pSkip = ignore;
	g_EngineTrace->TraceRay( ray, mask, &filter, ptr );
}

float Autowall::CanHit(const Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* to_who, int hitbox, bool* was_viable)
{
	if (ignore_ent == nullptr || to_who == nullptr)
		return 0;

	g_InTrace = true;
	Vector direction;
	Vector tmp = point - vecEyePos;
	float currentDamage = 0;

	//Math::VectorAngles(tmp, angles);
	//Math::AngleVectors(angles, &direction);
	direction = tmp.Normalized();

	if (LocalPlayerData::m_Weapon != nullptr)
	{
		if (FireBullet(vecEyePos, LocalPlayerData::m_Weapon, direction, currentDamage, ignore_ent, to_who, hitbox, was_viable))
		{
			g_InTrace = false;
			return currentDamage;
		}
		else
		{
			g_InTrace = false;
			return -1.f;
		}
	}
	return -1.f;
}

void Autowall::GetBulletTypeParameters( float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type )
{
	if ( sv_penetration_type )
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if ( !strcmp( bulletType, ("BULLET_PLAYER_338MAG") ) )
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_762MM") ) )
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_556MM") ) || !strcmp( bulletType, ("BULLET_PLAYER_556MM_SMALL") ) || !strcmp( bulletType, ("BULLET_PLAYER_556MM_BOX") ) )
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_57MM") ) )
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_50AE") ) )
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_357SIG") ) || !strcmp( bulletType, ("BULLET_PLAYER_357SIG_SMALL") ) || !strcmp( bulletType, ("BULLET_PLAYER_357SIG_P250") ) || !strcmp( bulletType, ("BULLET_PLAYER_357SIG_MIN") ) )
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_9MM") ) )
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_45ACP") ) )
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if ( !strcmp( bulletType, ("BULLET_PLAYER_BUCKSHOT") ) )
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

void Autowall::FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent) {
	if (!ent)
		return;

	/*const*/ auto Collideable = ent->GetCollideable();

	if (!Collideable)
		return;

	const auto mins = Collideable->OBBMins();
	const auto maxs = Collideable->OBBMaxs();

	auto dir(end - start);
	auto len = dir.Normalize();

	const auto center = (mins + maxs) / 2;
	const auto pos(ent->m_vecOrigin() + center);

	auto to = pos - start;
	const float range_along = dir.Dot(to);

	float range;
	if (range_along < 0.f) {
		range = -(to).Length();
	}
	else if (range_along > len) {
		range = -(pos - end).Length();
	}
	else {
		auto ray(pos - (start + (dir * range_along)));
		range = ray.Length();
	}

	if (range <= 60.f) {

		Ray_t ray;
		ray.Init(start, end);

		trace_t trace;
		g_EngineTrace->ClipRayToEntity(ray, 0x4600400B, ent, &trace);

		if (oldtrace->fraction > trace.fraction)
			*oldtrace = trace;
	}
}

bool Autowall::FireBullet(Vector eyepos, C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who, int hitbox, bool* was_viable, std::vector<float>* power)
{
	if (!pWeapon || !ignore)
		return false;

	//SafeLocalPlayer() false;
	//bool sv_penetration_type;
	//	  Current bullet travel Power to penetrate Distance to penetrate Range               Player bullet reduction convars			  Amount to extend ray by
	float currentDistance = 0;

	static auto* damageBulletPenetration = g_CVar->FindVar("ff_damage_bullet_penetration");

	const float ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	CCSWeaponInfo* weaponData = pWeapon->GetCSWeaponData();
	trace_t enterTrace;

	CTraceFilter filter;
	filter.pSkip = ignore;

	if (!weaponData)
		return false;

	//Set our current damage to what our gun's initial damage reports it will do
	currentDamage = float(weaponData->iDamage);
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = weaponData->flRange;
	auto penetrationPower = weaponData->flPenetration;
	auto RangeModifier = weaponData->flRangeModifier;

	//This gets set in FX_Firebullets to 4 as a pass-through value.
	//CS:GO has a maximum of 4 surfaces a bullet can pass-through before it 100% stops.
	//Excerpt from Valve: https://steamcommunity.com/sharedfiles/filedetails/?id=275573090
	//"The total number of surfaces any bullet can penetrate in a single flight is capped at 4." -CS:GO Official

	if (power)
	{
		maxRange = power->at(0);
		penetrationDistance = power->at(1);
		penetrationPower = power->at(2);
		currentDamage = power->at(3);
		RangeModifier = power->at(4);
	}

	int penetrated = 0;
	int possibleHitsRemaining = 4;

	//If our damage is greater than (or equal to) 1, and we can shoot, let's shoot.
	while (/*ctx.last_penetrated_count >= 0 &&*/currentDamage > 0)
	{
		//Calculate max bullet range

		//Create endpoint of bullet
		Vector end = eyepos + direction * (maxRange - currentDistance);

		TraceLine(eyepos, end, 0x4600400B/*_HULL | CONTENTS_HITBOX*/, ignore, &enterTrace);

		/*if (enterTrace.startsolid)
		{
			enterTrace.endpos = enterTrace.startpos;
			enterTrace.fraction = 0.0f;
		}*/
		//else
		//if (!(enterTrace.contents & CONTENTS_HITBOX)) {
		if (to_who/* && target_hitbox == HITBOX_HEAD*/ || enterTrace.contents & CONTENTS_HITBOX && enterTrace.hit_entity) {
			//Pycache/aimware traceray fix for head while players are jumping
			FixTraceRay(eyepos + (direction * 40.f), eyepos, &enterTrace, (to_who != nullptr ? to_who : (C_BasePlayer*)enterTrace.hit_entity));
		}
		else
			ClipTraceToPlayers((to_who != nullptr ? to_who : (C_BasePlayer*)enterTrace.hit_entity), eyepos, eyepos + (direction * 40.f), 0x4600400B, &filter, &enterTrace);
		//}

		if (enterTrace.fraction == 1.0f)
			return false;

		//calculate the damage based on the distance the bullet traveled.
		currentDistance += enterTrace.fraction * (maxRange - currentDistance);

		//Let's make our damage drops off the further away the bullet is.
		currentDamage *= powf(RangeModifier, (currentDistance / 500));

		if (!(enterTrace.contents & CONTENTS_HITBOX))
			enterTrace.hitgroup = 1;

		//This looks gay as fuck if we put it into 1 long line of code.
		const bool canDoDamage = enterTrace.hitgroup > 0 && enterTrace.hitgroup <= 8 || enterTrace.hitgroup == HITGROUP_GEAR;
		const bool isPlayer = enterTrace.hit_entity != nullptr
			&& enterTrace.hit_entity->GetClientClass()
			&& enterTrace.hit_entity->GetClientClass()->m_ClassID == ClassId_CCSPlayer
			&& (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || ((C_BasePlayer*)enterTrace.hit_entity)->m_iTeamNum() != g_LocalPlayer->m_iTeamNum());

		if (to_who)
		{
			if (enterTrace.hit_entity && to_who == enterTrace.hit_entity && canDoDamage /*&& isPlayer*/) 
			{
				const int group = (pWeapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : enterTrace.hitgroup;
				ScaleDamage(enterTrace, weaponData, currentDamage);

				if (was_viable != nullptr)
					*was_viable = (penetrated == 0);

				return true;
			}
		}
		else
		{
			if (enterTrace.hit_entity && canDoDamage && isPlayer) {
				const int group = (pWeapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : enterTrace.hitgroup;

				ScaleDamage(enterTrace, weaponData, currentDamage);

				if (was_viable != nullptr)
					*was_viable = (penetrated == 0);


				return true;
			}
		}

		//Sanity checking / Can we actually shoot through?
		if (currentDistance > maxRange && penetrationPower
			|| g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps)->game.flPenetrationModifier < 0.1f) {
			return false;//ctx.last_penetrated_count = 0;
		}

		const auto prev = possibleHitsRemaining;

		//Calling HandleBulletPenetration here reduces our penetrationCounter, and if it returns true, we can't shoot through it.
		if (!HandleBulletPenetration(ignore, weaponData, enterTrace, eyepos, direction, possibleHitsRemaining, currentDamage, penetrationPower, ff_damage_bullet_penetration)) {
			break;
		}
		if (prev != possibleHitsRemaining)
			penetrated++;
	}

	return false;
}

//bool Autowall::HandleBulletPenetration( CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration )
//{
//	//Because there's been issues regarding this- putting this here.
//	if ( &currentDamage == nullptr )
//	{
//		handle_penetration = false;
//		return false;
//	}
//
//	C_BasePlayer* local = g_LocalPlayer;//(IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
//
//	FireBulletData data( LocalPlayerData::m_EyePosition );
//	data.filter = CTraceFilter( );
//	data.filter.pSkip = local;
//	CGameTrace exitTrace;
//	C_BasePlayer* pEnemy = ( C_BasePlayer* )enterTrace.hit_entity;
//	surfacedata_t* enterSurfaceData = g_PhysSurface->GetSurfaceData( enterTrace.surface.surfaceProps );
//	int enterMaterial = enterSurfaceData->game.material;
//
//	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
//	float enterDamageModifier = enterSurfaceData->game.flDamageModifier;
//	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
//	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
//	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);
//
//	if ( possibleHitsRemaining <= 0
//		|| (enterTrace.surface.name == ( const char* )0x2227c261 && exitTrace.surface.name == ( const char* )0x2227c868)
//		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
//		|| weaponData->flPenetration <= 0.f
//		|| !TraceToExit( enterTrace, exitTrace, enterTrace.endpos, direction )
//		&& !(g_EngineTrace->GetPointContents( enterTrace.endpos, MASK_SHOT_HULL, nullptr ) & MASK_SHOT_HULL) )
//	{
//		handle_penetration = false;
//		return false;
//	}
//
//	surfacedata_t* exitSurfaceData = g_PhysSurface->GetSurfaceData( exitTrace.surface.surfaceProps );
//	int exitMaterial = exitSurfaceData->game.material;
//	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
//	float exitDamageModifier = exitSurfaceData->game.flDamageModifier;
//
//	if ( sv_penetration_type )
//	{
//		if ( enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS )
//		{
//			combinedPenetrationModifier = 3.f;
//			finalDamageModifier = 0.05f;
//		}
//		else if ( isSolidSurf || isLightSurf )
//		{
//			combinedPenetrationModifier = 1.f;
//			finalDamageModifier = 0.16f;
//		}
//		else if ( enterMaterial == CHAR_TEX_FLESH && (pEnemy->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) && ff_damage_reduction_bullets == 0.f) )
//		{
//			if ( ff_damage_bullet_penetration == 0.f )
//			{
//				handle_penetration = false;
//				return false;
//			}
//			combinedPenetrationModifier = ff_damage_bullet_penetration;
//			finalDamageModifier = 0.16f;
//		}
//		else
//		{
//			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
//			finalDamageModifier = 0.16f;
//		}
//
//		if ( enterMaterial == exitMaterial )
//		{
//			if ( exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD )
//			{
//				combinedPenetrationModifier = 3.f;
//			}
//			else if ( exitMaterial == CHAR_TEX_PLASTIC )
//			{
//				combinedPenetrationModifier = 2.f;
//			}
//		}
//
//		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr( );
//		modifier = fmaxf( 1.f / combinedPenetrationModifier, 0.f );
//
//		lostDamage = fmaxf(
//			((modifier * thickness) / 24.f)
//			+ ((currentDamage * finalDamageModifier)
//				+ (fmaxf( 3.75f / penetrationPower, 0.f ) * 3.f * modifier)), 0.f );
//
//		if ( lostDamage > currentDamage )
//		{
//			handle_penetration = false;
//			return false;
//		}
//
//		if ( lostDamage > 0.f )
//		{
//			currentDamage -= lostDamage;
//		}
//
//		if ( currentDamage < 1.f )
//		{
//			handle_penetration = false;
//			return false;
//		}
//
//		eyePosition = exitTrace.endpos;
//		--possibleHitsRemaining;
//
//		handle_penetration = true;
//		return true;
//	}
//	else
//	{
//		combinedPenetrationModifier = 1.f;
//
//		if ( isSolidSurf || isLightSurf )
//		{
//			finalDamageModifier = 0.99f;
//		}
//		else
//		{
//			finalDamageModifier = fminf( enterDamageModifier, exitDamageModifier );
//			combinedPenetrationModifier = fminf( enterSurfPenetrationModifier, exitSurfPenetrationModifier );
//		}
//
//		if ( enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD) )
//		{
//			combinedPenetrationModifier += combinedPenetrationModifier;
//		}
//
//		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr( );
//
//		if ( sqrt( thickness ) <= combinedPenetrationModifier * penetrationPower )
//		{
//			currentDamage *= finalDamageModifier;
//			eyePosition = exitTrace.endpos;
//			--possibleHitsRemaining;
//			handle_penetration = true;
//			return true;
//		}
//		handle_penetration = false;
//		return false;
//	}
//}