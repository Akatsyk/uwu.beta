#pragma once

#include "sdk.hpp"

#include <array>

#include "utils/utils.hpp"

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
    }

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
    }

#define NETPROP(name, table, netvar) static RecvProp* name() \
{ \
	static auto prop_ptr = NetvarSys::Get().GetNetvarProp(table,netvar); \
	return prop_ptr; \
}

#define OFFSET(type, name, offset)\
type &name##() const\
{\
        return *(type*)(uintptr_t(this) + offset);\
}

#define CUSTOM_OFFSET( name, type, prop, offset ) \
type& name( ) \
{\
    return *( type* )( ( uintptr_t )( this ) + offset ); \
}\

struct datamap_t;
class AnimationLayer;
class CBasePlayerAnimState;
class CCSGOPlayerAnimState;
class C_BaseEntity;
class CAnimStateFull;

class C_BoneAccessor
{
public:
	matrix3x4_t* GetBoneArrayForWrite()
	{
		return m_aBoneArray;
	}

	void SetBoneArrayForWrite(matrix3x4_t* bone_array)
	{
		m_aBoneArray = bone_array;
	}

	int GetReadableBones()
	{
		return m_ReadableBones;
	}

	void SetReadableBones(int flags)
	{
		m_ReadableBones = flags;
	}

	int GetWritableBones()
	{
		return m_WritableBones;
	}

	void SetWritableBones(int flags)
	{
		m_WritableBones = flags;
	}

	alignas(16) matrix3x4_t* m_aBoneArray;
	int m_ReadableBones;
	int m_WritableBones;
};


enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

enum Collision_Group_t {
	COLLISION_GROUP_NONE,
	COLLISION_GROUP_DEBRIS,
	COLLISION_GROUP_DEBRIS_TRIGGER,
	COLLISION_GROUP_INTERACTIVE_DEB,
	COLLISION_GROUP_INTERACTIVE,
	COLLISION_GROUP_PLAYER,
	COLLISION_GROUP_BREAKABLE_GLASS,
	COLLISION_GROUP_VEHICLE,
	COLLISION_GROUP_PLAYER_MOVEMENT,
	COLLISION_GROUP_NPC,
	COLLISION_GROUP_IN_VEHICLE,
	COLLISION_GROUP_WEAPON,
	COLLISION_GROUP_VEHICLE_CLIP,
	COLLISION_GROUP_PROJECTILE,
	COLLISION_GROUP_DOOR_BLOCKER,
	COLLISION_GROUP_PASSABLE_DOOR,
	COLLISION_GROUP_DISSOLVING,
	COLLISION_GROUP_PUSHAWAY,
	COLLISION_GROUP_NPC_ACTOR,
	LAST_SHARED_COLLISION_GROUP
};

class C_BaseEntity;

// Created with ReClass.NET by KN4CK3R
class CHudTexture
{
public:
	char szShortName[ 64 ];    //0x0000
	char szTextureFile[ 64 ];  //0x0040
	bool bRenderUsingFont;   //0x0080
	bool bPrecached;         //0x0081
	int8_t cCharacterInFont; //0x0082
	uint8_t pad_0083[ 1 ];     //0x0083
	uint32_t hFont;          //0x0084
	int32_t iTextureId;      //0x0088
	float afTexCoords[ 4 ];    //0x008C
	uint8_t pad_009C[ 16 ];    //0x009C
};

class CCustomString // new CUtlString?
{
public:
	char* m_pszString;
	int m_iUnknown0, m_iUnknown1;
	int m_iLength;
};

class CPaintKitStruct
{
public:
	int id;

	CCustomString name;
	CCustomString description_string;
	CCustomString description_tag;
	CCustomString pattern;
	CCustomString logo_material;

	bool bBaseDiffuseOverride;
	int nRarity;

public:
	int style;

	int color0[ 4 ];
	float color1[ 4 ];
	float color2[ 4 ];
	float color3[ 4 ];

private:
	DWORD pad1[ 4 ];

public:
	float wear_default;
	float wear_remap_min;
	float wear_remap_max;

	BYTE seed;

	BYTE phongexponent;
	BYTE phongalbedoboost;
	BYTE phongintensity;

	float pattern_scale;
	float pattern_offset_y_start;
	float pattern_offset_y_end;
	float pattern_offset_x_start;
	float pattern_offset_x_end;
	float pattern_rotate_start;
	float pattern_rotate_end;

	float logo_scale;
	float logo_offset_x;
	float logo_offset_y;
	float logo_rotation;

	int ignore_weapon_size_scale;

	int view_model_exponent_override_size;

	int only_first_material;

private:
	DWORD pad2[ 2 ];
};

class C_EconItemView
{
private:
	using str_32 = char[ 32 ];
public:

	NETVAR( int32_t, m_bInitialized, "DT_BaseAttributableItem", "m_bInitialized" );
	NETVAR( int16_t, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
	NETVAR( int32_t, m_iEntityLevel, "DT_BaseAttributableItem", "m_iEntityLevel" );
	NETVAR( int32_t, m_iAccountID, "DT_BaseAttributableItem", "m_iAccountID" );
	NETVAR( int32_t, m_iItemIDLow, "DT_BaseAttributableItem", "m_iItemIDLow" );
	NETVAR( int32_t, m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh" );
	NETVAR( int32_t, m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality" );
	NETVAR( str_32, m_iCustomName, "DT_BaseAttributableItem", "m_szCustomName" );
	CPaintKitStruct* GetPaintKit();

	datamap_t* DescMap() {
		typedef datamap_t* (__thiscall* o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>( this, 15 )(this);
	}

	datamap_t* GetPredMap() {
		typedef datamap_t* (__thiscall* o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>( this, 17 )(this);
	}
};

class C_BaseEntity : public IClientEntity
{
public:
	datamap_t* GetDataDescMap() {
		typedef datamap_t* (__thiscall* o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>( this, 15 )(this);
	}

	datamap_t* GetPredDescMap() {
		typedef datamap_t* (__thiscall* o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>( this, 17 )(this);
	}
	static __forceinline C_BaseEntity* GetEntityByIndex( int index ) {
		return static_cast< C_BaseEntity* >(g_EntityList->GetClientEntity( index ));
	}
	static __forceinline C_BaseEntity* get_entity_from_handle( CBaseHandle h ) {
		return static_cast< C_BaseEntity* >(g_EntityList->GetClientEntityFromHandle( h ));
	}

	NETVAR( int32_t, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex" );
	NETVAR( int32_t, m_iTeamNum, "DT_BaseEntity", "m_iTeamNum" );
	NETVAR( Vector, m_vecOrigin, "DT_BaseEntity", "m_vecOrigin" );
	NETVAR( Vector, m_vecAngles, "DT_BaseEntity", "m_vecAngles" );
	NETVAR( bool, m_bShouldGlow, "DT_DynamicProp", "m_bShouldGlow" );
	NETVAR( CHandle<C_BasePlayer>, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity" );
	NETVAR( bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted" );
	NETVAR( float_t, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );


	const matrix3x4_t& m_rgflCoordinateFrame()
	{
		static auto _m_rgflCoordinateFrame = NetvarSys::Get().GetOffset( "DT_BaseEntity", "m_CollisionGroup" ) - 0x30;
		return *( matrix3x4_t* )(( uintptr_t )this + _m_rgflCoordinateFrame);
	}

	bool IsPlayer();
	bool IsLoot();
	bool IsWeapon();
	bool IsPlantedC4();
	bool IsDefuseKit();
	void SetAbsAngles( const Vector& angle );
	//bool isSpotted();
};

class C_PlantedC4
{
public:
	NETVAR( bool, m_bBombTicking, "DT_PlantedC4", "m_bBombTicking" );
	NETVAR( bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused" );
	NETVAR( float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );
	NETVAR( float, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength" );
	NETVAR( float, m_flDefuseLength, "DT_PlantedC4", "m_flDefuseLength" );
	NETVAR( float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown" );
	NETVAR( CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser" );
};

class C_BaseAttributableItem : public C_BaseEntity
{
public:
	NETVAR( uint64_t, m_OriginalOwnerXuid, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidLow, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidHigh, "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh" );
	NETVAR( int32_t, m_nFallbackStatTrak, "DT_BaseAttributableItem", "m_nFallbackStatTrak" );
	NETVAR( int32_t, m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit" );
	NETVAR( int32_t, m_nFallbackSeed, "DT_BaseAttributableItem", "m_nFallbackSeed" );
	NETVAR( float_t, m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear" );

	NETVAR( C_EconItemView, m_Item2, "DT_BaseAttributableItem", "m_Item" );

	C_EconItemView& m_Item()
	{
		// Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
		// A real fix for this requires a rewrite of the netvar manager
		return *( C_EconItemView* )this;
	}

	void SetGloveModelIndex( int modelIndex );

	void SetModelIndex( int modelIndex ) {
		return CallVFunction<void( __thiscall* )(void*, int)>( this, 75 )(this, modelIndex);
	}
};

class C_BaseWeaponWorldModel : public C_BaseEntity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseWeaponWorldModel", "m_nModelIndex" );
};

class C_BaseCombatWeapon : public C_BaseAttributableItem
{
public:
	NETVAR( float_t, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack" );
	NETVAR( float_t, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack" );
	NETVAR( int32_t, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1" );
	NETVAR( int32_t, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2" );
	NETVAR( int, weapon, "DT_BaseViewModel", "m_hWeapon" );
	NETVAR( float_t, m_flRecoilIndex, "DT_WeaponCSBase", "m_flRecoilIndex" );
	NETVAR( int32_t, m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex" );
	NETVAR( int32_t, m_iWorldModelIndex, "DT_BaseCombatWeapon", "m_iWorldModelIndex" );
	NETVAR( int, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
	NETVAR( int32_t, m_iWorldDroppedModelIndex, "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex" );
	NETVAR( bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled" );
	NETVAR( float_t, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime" );
	NETVAR( float_t, m_flPostponeFireReadyTime, "DT_BaseCombatWeapon", "m_flPostponeFireReadyTime" );
	NETVAR( CHandle<C_BaseWeaponWorldModel>, m_hWeaponWorldModel, "DT_BaseCombatWeapon", "m_hWeaponWorldModel" );
	NETVAR( float_t, m_fLastShotTime, "DT_WeaponCSBase", "m_fLastShotTime" );
	NETVAR( int, m_ZoomLVL, "DT_WeaponCSBaseGun", "m_zoomLevel" );

	CCSWeaponInfo* GetCSWeaponData();
	bool HasBullets();
	bool CanFire();
	bool IsGrenade();
	bool IsKnife();
	bool IsReloading();
	bool IsRifle();
	bool IsPistol();
	bool IsSniper();
	bool IsGun();
	float GetInaccuracy();
	float GetSpread();
	void UpdateAccuracyPenalty();
	CUtlVector<IRefCounted*>& m_CustomMaterials();
	bool* m_bCustomMaterialInitialized();
	std::string get_name();
	int SubWeaponType();

	bool IsKnifeIDX();
	bool IsRfileIDX();
	bool IsSmgIDX();
	bool IsShotgunIDX();
	bool IsPistolIDX();
	bool IsGrenadeIDX();
	bool IsSniperIDX();
};

class C_BasePlayer : public C_BaseEntity
{
public:
	static __forceinline C_BasePlayer* GetPlayerByUserId( int id )
	{
		return static_cast< C_BasePlayer* >(GetEntityByIndex( g_EngineClient->GetPlayerForUserID( id ) ));
	}
	static __forceinline C_BasePlayer* GetPlayerByIndex( int i )
	{
		return static_cast< C_BasePlayer* >(GetEntityByIndex( i ));
	}

	NETVAR( bool, m_bHasDefuser, "DT_CSPlayer", "m_bHasDefuser" );
	NETVAR( bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity" );
	NETVAR( int32_t, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired" );
	NETVAR( QAngle, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles[0]" );
	NETVAR( int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue" );
	NETVAR( bool, m_bHasHeavyArmor, "DT_CSPlayer", "m_bHasHeavyArmor" );
	NETVAR( bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet" );
	NETVAR( bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped" );;
	NETVAR( float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	NETVAR( float, m_flThirdpersonRecoil, "DT_CSPlayer", "m_flThirdpersonRecoil" );
	NETVAR( int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth" );
	NETVAR( int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState" );
	NETVAR( int32_t, m_fFlags, "DT_BasePlayer", "m_fFlags" );
	NETVAR( int32_t, m_nTickBase, "DT_BasePlayer", "m_nTickBase" );
	NETVAR( Vector, m_vecViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]" );
	NETVAR( QAngle, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle" );
	NETVAR( QAngle, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle" );
	NETVAR( Vector, m_aimPunchAngleVel, "DT_BasePlayer", "m_aimPunchAngleVel" );
	NETVAR( CHandle<C_BaseViewModel>, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]" );
	NETVAR( Vector, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]" );
	NETVAR( float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed" );
	NETVAR( CHandle<C_BasePlayer>, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget" );
	NETVAR( float, m_flFlashMaxAlpha, "DT_CSPlayer", "m_flFlashMaxAlpha" );
	NETVAR( int32_t, m_nHitboxSet, "DT_BaseAnimating", "m_nHitboxSet" );
	NETVAR( CHandle<C_BaseCombatWeapon>, m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon" );
	NETVAR( int32_t, m_iAccount, "DT_CSPlayer", "m_iAccount" );
	NETVAR( float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration" );
	NETVAR( float, m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime" );
	NETVAR( float, m_flCycle, "DT_BaseAnimating", "m_flCycle" );
	NETVAR( int, m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	NETVAR( float, m_flNextAttack, "DT_BaseCombatCharacter", "m_flNextAttack" );
	NETVAR( int, m_iObserverMode, "DT_BasePlayer", "m_iObserverMode" );
	NETVAR( bool, m_bClientSideAnimation, "DT_BaseAnimating", "m_bClientSideAnimation" );
	NETVAR( float, m_bVelMod, "DT_CSPlayer", "m_flVelocityModifier" )
		NETVAR( int, m_vphysicsCollisionState, "DT_BasePlayer", "m_vphysicsCollisionState" );
	//NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");
	OFFSET( float, m_flOldSimulationTime, NetvarSys::Get().GetOffset( "DT_BaseEntity", "m_flSimulationTime" ) + 0x4 );
	OFFSET( CUserCmd, GetLastCmd, 0x3288 )
	OFFSET( int, ShouldUseNewAnimState, 0x9B14 );

	NETVAR( bool, m_bUseCustomAutoExposureMin, "DT_EnvTonemapController", "m_bUseCustomAutoExposureMin" );
	NETVAR( bool, m_bUseCustomAutoExposureMax, "DT_EnvTonemapController" ,"m_bUseCustomAutoExposureMax" );
	NETVAR( bool, m_bUseCustomBloomScale, "DT_EnvTonemapController", "m_bUseCustomBloomScale" );
	NETVAR( float, m_flCustomAutoExposureMin, "DT_EnvTonemapController", "m_flCustomAutoExposureMin" );
	NETVAR( float, m_flCustomAutoExposureMax, "DT_EnvTonemapController", "m_flCustomAutoExposureMax" );
	NETVAR( float, m_flCustomBloomScale, "DT_EnvTonemapController", "m_flCustomBloomScale" );


		uint32_t& m_fEffects()
	{
		static auto m_fEffects = Utils::FindInDataMap( GetPredDescMap(), "m_fEffects" );
		return *( uint32_t* )(uintptr_t( this ) + m_fEffects);
	}

	uint32_t& m_iEFlags()
	{
		static auto m_iEFlags = Utils::FindInDataMap( GetPredDescMap(), "m_iEFlags" );
		return *( uint32_t* )(uintptr_t( this ) + m_iEFlags);
	}

	Vector& m_VecAbsVelocity()
	{
		static auto m_iEFlags = Utils::FindInDataMap( GetPredDescMap(), "m_vecAbsVelocity" );
		return *( Vector* )(uintptr_t( this ) + m_iEFlags);
	}

	void InvalidatePhysics( int change_flags )
	{
		static auto m_uInvalidatePhysics = Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04" );
		reinterpret_cast < void( __thiscall* )(void*, int) > (m_uInvalidatePhysics)(this, change_flags);
	}

	void SetAbsOrig( const Vector& origin )
	{
		if ( !this ) //-V704
			return;

		using Fn = void( __thiscall* )(void*, const Vector&);
		static auto fn = reinterpret_cast< Fn >(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8" ));

		return fn( this, origin );
	}

	void SetAbsVel( const Vector& velocity )
	{
		if ( !this ) //-V704
			return;

		using Fn = void( __thiscall* )(void*, const Vector&);
		static auto fn = reinterpret_cast< Fn >(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3" ));

		return fn( this, velocity );
	}

	NETVAR( QAngle, m_angAbsAngles, "DT_BaseEntity", "m_angAbsAngles" );
	NETVAR( Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin" );
	NETVAR( float, m_flDuckSpeed, "DT_BasePlayer", "m_flDuckSpeed" );
	NETVAR( float, m_flDuckAmount, "DT_BasePlayer", "m_flDuckAmount" );
	NETVAR( float, m_flModelScale, "DT_BaseAnimating", "m_flModelScale" );

	std::array<float, 24>& m_flPoseParameter() const {
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset( "DT_BaseAnimating", "m_flPoseParameter" );
		return *(std::array<float, 24>*)(( uintptr_t )this + _m_flPoseParameter);
	}

	CUtlVector <matrix3x4_t>& m_CachedBoneData() {
		return *(CUtlVector <matrix3x4_t>*)(uintptr_t( this ) + 0x2914);
	}

	//PNETVAR(CHandle<C_BaseCombatWeapon>, m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons");
	CBaseHandle* m_hMyWeapons()
	{
		return ( CBaseHandle* )(( uintptr_t )this + 0x2E08);
	}

	PNETVAR( CHandle<C_BaseAttributableItem>, m_hMyWearables, "DT_BaseCombatCharacter", "m_hMyWearables" );
	PNETVAR( char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName" );

	CUserCmd*& m_pCurrentCommand();

	/*gladiator v2*/
	void InvalidateBoneCache();

	C_BoneAccessor& GetBoneAccesor() 
	{
		static auto force_bone = NetvarSys::Get().GetOffset("DT_BaseAnimating", "m_nForceBone");
		static auto return_value = force_bone + 0x1C;
		return *(C_BoneAccessor*)(uintptr_t(this) + return_value);
	}

	void ForceBoneRebuild() 
	{
		GetBoneAccesor().m_WritableBones = GetBoneAccesor().m_ReadableBones = 0;
		*(int*)(uintptr_t(this) + 0x2928) = 0xFF7FFFFF;
		*(int*)(uintptr_t(this) + 0x2690) = 0;
	}

	int GetNumAnimOverlays();
	AnimationLayer* GetAnimOverlays();
	AnimationLayer* GetAnimOverlay( int i );
	int GetSequenceActivity( int sequence );
	CCSGOPlayerAnimState* GetPlayerAnimState();

	int& m_iOcclusionFlags() 
	{
		return *(int*)((uintptr_t)this + 0xA28);
	}

	int& m_iOcclusionFramecount() 
	{
		return *(int*)((uintptr_t)this + 0xA30);
	}

	uint32_t& get_readable_bones() 
	{
		return *(uint32_t*)((uintptr_t)this + 0x26AC);
	}

	uint32_t& get_writeable_bones() 
	{
		return *(uint32_t*)((uintptr_t)this + 0x26B0);
	}

	bool& m_JiggleBones() 
	{
		return *(bool*)((uintptr_t)this + 0x2930);
	}

	int& m_nFinalPredictedTick() {
		return *(int*)((DWORD)this + 0x3434);
	}

	static void UpdateAnimationState( CCSGOPlayerAnimState* state, QAngle angle );
	static void ResetAnimationState( CCSGOPlayerAnimState* state );
	void CreateAnimationState( CCSGOPlayerAnimState* state );

	float_t& m_surfaceFriction()
	{
		static unsigned int _m_surfaceFriction = Utils::FindInDataMap( GetPredDescMap(), "m_surfaceFriction" );
		return *( float_t* )(( uintptr_t )this + _m_surfaceFriction);
	}
	Vector& m_vecBaseVelocity()
	{
		static unsigned int _m_vecBaseVelocity = Utils::FindInDataMap( GetPredDescMap(), "m_vecBaseVelocity" );
		return *( Vector* )(( uintptr_t )this + _m_vecBaseVelocity);
	}

	float_t& m_flMaxspeed()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap( GetPredDescMap(), "m_flMaxspeed" );
		return *( float_t* )(( uintptr_t )this + _m_flMaxspeed);
	}

	bool& WaitForNoAttack( )
	{
		static auto m_bWaitForNoAttack = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_bWaitForNoAttack" );
		return *( bool* )(uintptr_t( this ) + m_bWaitForNoAttack);
	}

	bool& IsDefusing( )
	{
		static int m_bIsDefusing = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_bIsDefusing" );
		return *( bool* )(uintptr_t( this ) + m_bIsDefusing);
	}

	bool& IsJiggleBone( )
	{
		static int m_bIsJig = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_hLightingOrigin" ) - 0x18;
		return *( bool* )(uintptr_t( this ) + m_bIsJig);
	}

	int& GetPlayerState( )
	{
		static auto m_iPlayerState = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_iPlayerState" );
		return *( int* )(uintptr_t( this ) + m_iPlayerState);
	}

	CUSTOM_OFFSET(m_nClientEffects, int32_t, FNV32("ClientEffects"), 0x68);
	CUSTOM_OFFSET(m_nLastSkipFramecount, int32_t, FNV32("LastSkipFramecount"), 0xA68);
	CUSTOM_OFFSET(m_pInverseKinematics, LPVOID, FNV32("InverseKinematics"), 9840);
	CUSTOM_OFFSET(m_bMaintainSequenceTransition, bool, FNV32("MaintainSequenceTransition"), 0x9F0);

	void SetCollisionBounds( const Vector& mins, const Vector& maxs )
	{
		reinterpret_cast < void( __thiscall* )(ICollideable*, const Vector&, const Vector&) >
			(Utils::FindSig( "client.dll", "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 18 56 57 8B 7B" ))(GetCollideable( ), mins, maxs);
	}

	Vector        GetEyePos();
	player_info_t GetPlayerInfo();
	bool          IsAlive();
	bool		  IsFlashed();
	bool          HasC4();
	Vector        GetHitboxPos( int hitbox_id );
	mstudiobbox_t* GetHitbox( int hitbox_id );
	bool          GetHitboxPos( int hitbox, Vector& output );
	Vector        GetBonePos( int bone );
	bool          CanSeePlayer( C_BasePlayer* player, int hitbox );
	bool          CanSeePlayer( C_BasePlayer* player, const Vector& pos );
	void UpdateClientSideAnimation();

	CAnimStateFull* C_BasePlayer::GetPlayerAnimStateAlternative( )
	{
		return *( CAnimStateFull** )(( DWORD )this + 0x9960);
	}

	int m_nMoveType();
	QAngle* GetVAngles();
	float_t m_flSpawnTime();

	matrix3x4_t GetBoneMatrix( int BoneID )
	{
		matrix3x4_t matrix;

		auto offset = *reinterpret_cast< uintptr_t* >(uintptr_t( this ) + 0x26A8);
		if ( offset )
			matrix = *reinterpret_cast< matrix3x4_t* >(offset + 0x30 * BoneID);

		return matrix;
	}
};

class projectile_t : public C_BaseEntity
{
public:
	NETVAR( Vector, m_vInitialVelocity, "DT_BaseCSGrenadeProjectile", "m_vInitialVelocity" );
	NETVAR( int, m_flAnimTime, "DT_BaseCSGrenadeProjectile", "m_flAnimTime" );
	NETVAR( int, m_nExplodeEffectTickBegin, "DT_BaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin" );
	NETVAR( int, m_nBody, "DT_BaseCSGrenadeProjectile", "m_nBody" );
	NETVAR( int, m_nForceBone, "DT_BaseCSGrenadeProjectile", "m_nForceBone" );
	NETVAR( Vector, m_vecVelocity, "DT_BaseGrenade", "m_vecVelocity" );
	NETVAR( CHandle<C_BasePlayer>, m_hThrower, "DT_BaseGrenade", "m_hThrower" );
	NETVAR( Vector, m_vecOrigin, "DT_BaseCSGrenadeProjectile", "m_vecOrigin" );
	OFFSET( float, m_flSpawnTime, NetvarSys::Get( ).GetOffset( "DT_BaseCSGrenadeProjectile", "m_vecExplodeEffectOrigin" ) + 0xC );
};

class C_BaseViewModel : public C_BaseEntity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseViewModel", "m_nModelIndex" );
	NETVAR( int32_t, m_nViewModelIndex, "DT_BaseViewModel", "m_nViewModelIndex" );
	NETVAR( CHandle<C_BaseCombatWeapon>, m_hWeapon, "DT_BaseViewModel", "m_hWeapon" );
	NETVAR( CHandle<C_BasePlayer>, m_hOwner, "DT_BaseViewModel", "m_hOwner" );
	NETPROP( m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	void SendViewModelMatchingSequence( int sequence );
	NETVAR( int, m_nAnimationParity, "DT_BaseViewModel", "m_nAnimationParity" );
	NETVAR( int, m_nSequenceVar, "DT_BaseAnimating", "m_nSequence" );

	float& m_flCycleData()
	{
		static auto m_flCycle = Utils::FindInDataMap( GetPredDescMap(), "m_flCycle" );
		return *( float* )(uintptr_t( this ) + m_flCycle);
	}

	float& m_flAnimTimeData()
	{
		static auto m_flAnimTime = Utils::FindInDataMap( GetPredDescMap(), "m_flAnimTime" );
		return *( float* )(uintptr_t( this ) + m_flAnimTime);
	}
};

class AnimationLayer
{
public:
	char  pad_0000[ 20 ];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void* m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[ 4 ]; //0x0034
}; //Size: 0x0038

class CCSGOPlayerAnimState
{
public:
	void* pThis;
	char pad2[ 91 ];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[ 4 ]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[ 4 ];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[ 8 ];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[ 10 ];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[ 4 ]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[ 4 ]; //NaN
	float m_flUnknown3;
	char pad10[ 528 ];

	float& TimeSinceInAir()
	{
		return *( float* )(( uintptr_t )this + 0x110);
	}
}; //Size=0x344

struct animstate_pose_param_cache_t {
	std::uint8_t pad_0x0[ 0x4 ]; //0x0
	std::uint32_t m_idx; //0x4 
	char* m_name; //0x8

	void set_value( C_BasePlayer* e, float val );
};

struct procedural_foot_t
{
	Vector m_vecPosAnim;
	Vector m_vecPosAnimLast;
	Vector m_vecPosPlant;
	Vector m_vecPlantVel;
	float m_LockAmount;
	float m_LastPlantTime;
};

class CAnimStateFull {
public:
	int* m_layer_order_preset = nullptr;
	bool					m_first_run_since_init = false;

	bool					m_first_foot_plant_since_init = false;
	int						m_last_update_tick = 0;
	float					m_eye_position_smooth_lerp = 0.0f;

	float					m_strafe_change_weight_smooth_fall_off = 0.0f;

	float	m_stand_walk_duration_state_has_been_valid = 0.0f;
	float	m_stand_walk_duration_state_has_been_invalid = 0.0f;
	float	m_stand_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float	m_stand_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float	m_stand_walk_blend_value = 0.0f;

	float	m_stand_run_duration_state_has_been_valid = 0.0f;
	float	m_stand_run_duration_state_has_been_invalid = 0.0f;
	float	m_stand_run_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float	m_stand_run_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float	m_stand_run_blend_value = 0.0f;

	float	m_crouch_walk_duration_state_has_been_valid = 0.0f;
	float	m_crouch_walk_duration_state_has_been_invalid = 0.0f;
	float	m_crouch_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float	m_crouch_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float	m_crouch_walk_blend_value = 0.0f;

	//aimmatrix_transition_t	m_stand_walk_aim = {};
	//aimmatrix_transition_t	m_stand_run_aim = {};
	//aimmatrix_transition_t	m_crouch_walk_aim = {};

	int						m_cached_model_index = 0;

	float					m_step_height_left = 0.0f;
	float					m_step_height_right = 0.0f;

	C_BaseCombatWeapon* m_weapon_last_bone_setup = nullptr;

	C_BasePlayer* m_player = nullptr;//0x0060 
	C_BaseCombatWeapon* m_weapon = nullptr;//0x0064
	C_BaseCombatWeapon* m_weapon_last = nullptr;//0x0068

	float					m_last_update_time = 0.0f;//0x006C	
	int						m_last_update_frame = 0;//0x0070 
	float					m_last_update_increment = 0.0f;//0x0074 

	float					m_eye_yaw = 0.0f; //0x0078 
	float					m_eye_pitch = 0.0f; //0x007C 
	float					m_abs_yaw = 0.0f; //0x0080 
	float					m_abs_yaw_last = 0.0f; //0x0084 
	float					m_move_yaw = 0.0f; //0x0088 
	float					m_move_yaw_ideal = 0.0f; //0x008C 
	float					m_move_yaw_current_to_ideal = 0.0f; //0x0090 	
	float					m_time_to_align_lower_body;

	float					m_primary_cycle = 0.0f; //0x0098
	float					m_move_weight = 0.0f; //0x009C 

	float					m_move_weight_smoothed = 0.0f;
	float					m_anim_duck_amount = 0.0f; //0x00A4
	float					m_duck_additional = 0.0f; //0x00A8
	float					m_recrouch_weight = 0.0f;

	Vector					m_position_current = Vector(0.f, 0.f, 0.f); //0x00B0
	Vector					m_position_last = Vector(0.f, 0.f, 0.f); //0x00BC 

	Vector					m_velocity = Vector(0.f, 0.f, 0.f); //0x00C8
	Vector					m_velocity_normalized = Vector(0.f, 0.f, 0.f); // 
	Vector					m_velocity_normalized_non_zero = Vector(0.f, 0.f, 0.f); //0x00E0
	float					m_velocity_length_xy = 0.0f; //0x00EC
	float					m_velocity_length_z = 0.0f; //0x00F0

	float					m_speed_as_portion_of_run_top_speed = 0.0f; //0x00F4
	float					m_speed_as_portion_of_walk_top_speed = 0.0f; //0x00F8 
	float					m_speed_as_portion_of_crouch_top_speed = 0.0f; //0x00FC

	float					m_duration_moving = 0.0f; //0x0100
	float					m_duration_still = 0.0f; //0x0104

	bool					m_on_ground = false; //0x0108 

	bool					m_landing = false; //0x0109
	float					m_jump_to_fall = 0.0f;
	float					m_duration_in_air = 0.0f; //0x0110
	float					m_left_ground_height = 0.0f; //0x0114 
	float					m_land_anim_multiplier = 0.0f; //0x0118 

	float					m_walk_run_transition = 0.0f; //0x011C

	bool					m_landed_on_ground_this_frame = false;
	bool					m_left_the_ground_this_frame = false;
	float					m_in_air_smooth_value = 0.0f;

	bool					m_on_ladder = false; //0x0124
	float					m_ladder_weight = 0.0f; //0x0128
	float					m_ladder_speed = 0.0f;

	bool					m_walk_to_run_transition_state = false;

	bool					m_defuse_started = false;
	bool					m_plant_anim_started = false;
	bool					m_twitch_anim_started = false;
	bool					m_adjust_started = false;

	//CUtlVector<int>		m_activity_modifiers = {};
	char					m_activity_modifiers_server[20] = {};

	float					m_next_twitch_time = 0.0f;

	float					m_time_of_last_known_injury = 0.0f;

	float					m_last_velocity_test_time = 0.0f;
	Vector					m_velocity_last = Vector(0.f, 0.f, 0.f);
	Vector					m_target_acceleration = Vector(0.f, 0.f, 0.f);
	Vector					m_acceleration = Vector(0.f, 0.f, 0.f);
	float					m_acceleration_weight = 0.0f;

	float					m_aim_matrix_transition = 0.0f;
	float					m_aim_matrix_transition_delay = 0.0f;

	bool					m_flashed = false;

	float					m_strafe_change_weight = 0.0f;
	float					m_strafe_change_target_weight = 0.0f;
	float					m_strafe_change_cycle = 0.0f;
	int						m_strafe_sequence = 0;
	bool					m_strafe_changing = false;
	float					m_duration_strafing = 0.0f;

	float					m_foot_lerp = 0.0f;

	bool					m_feet_crossed = false;

	bool					m_player_is_accelerating = false;

	animstate_pose_param_cache_t m_pose_param_mappings[20] = {};

	float					m_duration_move_weight_is_too_high = 0.0f;
	float					m_static_approach_speed = 0.0f;

	int						m_previous_move_state = 0;
	float					m_stutter_step = 0.0f;

	float					m_action_weight_bias_remainder = 0.0f;

	Vector m_foot_left_pos_anim = Vector(0.f, 0.f, 0.f);
	Vector m_foot_left_pos_anim_last = Vector(0.f, 0.f, 0.f);
	Vector m_foot_left_pos_plant = Vector(0.f, 0.f, 0.f);
	Vector m_foot_left_plant_vel = Vector(0.f, 0.f, 0.f);
	float m_foot_left_lock_amount = 0.0f;
	float m_foot_left_last_plant_time = 0.0f;

	Vector m_foot_right_pos_anim = Vector(0.f, 0.f, 0.f);
	Vector m_foot_right_pos_anim_last = Vector(0.f, 0.f, 0.f);
	Vector m_foot_right_pos_plant = Vector(0.f, 0.f, 0.f);
	Vector m_foot_right_plant_vel = Vector(0.f, 0.f, 0.f);
	float m_foot_right_lock_amount = 0.0f;
	float m_foot_right_last_plant_time = 0.0f;

	float					m_camera_smooth_height = 0.0f;
	bool					m_smooth_height_valid = false;
	float					m_last_time_velocity_over_ten = 0.0f;

	float					m_aim_yaw_min = 0.0f;//0x0330
	float					m_aim_yaw_max = 0.0f;//0x0334
	float					m_aim_pitch_min = 0.0f;
	float					m_aim_pitch_max = 0.0f;

	int						m_animstate_model_version = 0;

	void reset();
	void update(Vector& ang);

	float& time_since_in_air() {
		return *(float*)((uintptr_t)this + 0x110);
	}

	float& yaw_desync_adjustment() {
		return *(float*)((uintptr_t)this + 0x334);
	}

	//bool& m_smooth_height_valid( ) {
	//	return *( bool* ) ( ( uintptr_t ) this + 0x328 );

};

class DT_CSPlayerResource
{
public:
	PNETVAR( int32_t, m_nActiveCoinRank, "DT_CSPlayerResource", "m_nActiveCoinRank" );
	PNETVAR( int32_t, m_nMusicID, "DT_CSPlayerResource", "m_nMusicID" );
	PNETVAR( int32_t, m_nPersonaDataPublicLevel, "DT_CSPlayerResource", "m_nPersonaDataPublicLevel" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsLeader, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsTeacher, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsFriendly, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly" );
	PNETVAR( int32_t, m_iCompetitiveRanking, "DT_CSPlayerResource", "m_iCompetitiveRanking" );
	PNETVAR( int32_t, m_iCompetitiveWins, "DT_CSPlayerResource", "m_iCompetitiveWins" );
	PNETVAR( int32_t, m_iPlayerVIP, "DT_CSPlayerResource", "m_iPlayerVIP" );
	PNETVAR( int32_t, m_iMVPs, "DT_CSPlayerResource", "m_iMVPs" );
	PNETVAR( int32_t, m_iScore, "DT_CSPlayerResource", "m_iScore" );
};

namespace LocalPlayerData
{
	extern C_BaseCombatWeapon* m_Weapon;
	extern Vector m_EyePosition;
}