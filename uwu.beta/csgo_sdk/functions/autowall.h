#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/math.hpp"

#define CHAR_TEXT_WOOD 87
#define CHAT_TEXT_GLASS 2
#define CHAT_TEXT_METAL 60

#define	HITGROUP_GENERIC	0
#define	HITGROUP_HEAD		1
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7
#define HITGROUP_GEAR		10			// alerts NPC, but doesn't do damage or bleed (1/100th damage)
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3

struct FireBulletData
{
	Vector src;
	trace_t enter_trace;
	Vector direction;
	CTraceFilter filter;
	float trace_length;
	float trace_length_remaining;
	float current_damage;
	int penetrate_count;
	FireBulletData(const Vector& eye_pos) : src(eye_pos) { }
	FireBulletData() { }
};

class Autowall : public Singleton<Autowall>
{

	friend class Singleton<Autowall>;

public:

	//void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr);
	void ClipTraceToPlayers(C_BasePlayer* pPlayer, const Vector& start, const Vector& end, uint32_t mask, CTraceFilter* filter, trace_t* tr);
	float GetHitgroupDamageMultiplier(int iHitGroup);
	bool HandleBulletPenetration(C_BasePlayer* ignore, CCSWeaponInfo* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_bullet_penetration, bool pskip = false);
	//bool HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration);
	bool TraceToExit(Vector& vecEnd, trace_t* pEnterTrace, Vector vecStart, Vector vecDir, trace_t* pExitTrace);
	void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, unsigned int nMask, C_BasePlayer* pCSIgnore, trace_t* pTrace);
	void TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr);
	void TraceLineNew(Vector& absStart, const Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, int collision_group, trace_t* ptr);
	uint32_t GetFilterSimpleVtable();
	void ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage);
	float CanHit(const Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* start_ent, int hitbox, bool* was_viable = nullptr);
	bool FireBullet(Vector eyepos, C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who = nullptr, int hitbox = -1, bool* was_viable = nullptr, std::vector<float>* = nullptr);
	void GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type);
	void FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent);
	//bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction);
	bool BreakableEntity(IClientEntity* entity);

	bool handle_penetration = false;
};