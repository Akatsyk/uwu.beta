#pragma once
#include "../sdk/csgostructs.hpp"
#include "../sdk/sdk.hpp"

#include <vector>
#include <xmmintrin.h>

#include "lag_comp.h"

class CRage : public Singleton <CRage>
{

	friend class Singleton<CRage>;

public:

	typedef __declspec(align(16)) union {
		float f[ 4 ];
		__m128 v;
	} m128;

	inline __m128 sqrt_ps( const __m128 squared )
	{
		return _mm_sqrt_ps( squared );
	}

	void OnCreateMove( CUserCmd* m_pcmd );

	int Target_id;
	QAngle EngineAngles;

	bool Hitchance( StoreAnimation* best_anims, Vector position, const float chance, int box );
	void Update( );
	void Aim( );
	void GeneratePoints( C_BasePlayer* entity, int hitbox_id, std::vector< Vector >& points, StoreAnimation* anims );
	void AutoStop( );
	int GetTicksToShoot( );
	bool IsAbleToShoot( float mtime );
	int GetTicksToStop( );
	void LinearExtrapolation( C_BasePlayer* m_entity );
	bool CockRevolver( CUserCmd* usercmd, C_BaseCombatWeapon* local_weapon );
	bool SafePoint( Vector eyepos, Vector point, C_BasePlayer* record, int i, StoreAnimation* anims );
	float Segment( const Vector s1, const Vector s2, const Vector k1, const Vector k2 );
	Vector PrimaryScan( StoreAnimation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg );

	void BackupPlayer( StoreAnimation* anims );
	void SetAnims( StoreAnimation* anims );
	void RestorePlayer( StoreAnimation* anims );

	std::vector<Vector> GetPoints( C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[ 128 ] );
	bool CanHitHitbox( Vector start, Vector end, StoreAnimation* _animation, studiohdr_t* hdr, int box );

	StoreAnimation backup_anims[ 65 ];
	StoreAnimation* best_anims = nullptr;

	float bestEntDmg;
	int targetID = 0;
	bool ShootNextTick;
	bool RageShot;
	bool ShouldStopAt;

	matrix3x4_t BoneMatrix[ 128 ];

	Vector HeadScan( StoreAnimation* anims, int& hitbox, float& best_damage, float min_dmg );
	Vector FullScan( StoreAnimation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg );
	Vector GetAimVector( C_BasePlayer* pTarget, float& simtime, Vector& origin, StoreAnimation*& best_anims, int& hitbox );
	Vector GetPointTo( C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[ 128 ] );
	int GetCurrentPriorityHitbox( C_BasePlayer* e );
	std::vector<int> GetHitboxesToScan( C_BasePlayer* e );

	struct
	{
		int damage;
		float hitchance;
		float boost;
		float head_scale;
		float body_scale;
		bool autostop;
		bool autoscope;
		bool prefer_bod;
		bool prefer_lethal;
		bool hitbox_selected[ 8 ];
		bool shoot_pr;
		int prior_hit;

		bool body_priors[ 5 ];
		bool default_priors[ 5 ];

	} m_Config;

private:

};