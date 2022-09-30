#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/sdk.hpp"

#include <deque>
#include <optional>

struct StoreAnimation
{
	StoreAnimation( ) = default;
	explicit StoreAnimation( C_BasePlayer* player );
	explicit StoreAnimation( C_BasePlayer* player, QAngle last_reliable_angle );
	void Restore( C_BasePlayer* player ) const;
	void Apply( C_BasePlayer* player ) const;
	void BuildBones( C_BasePlayer* player, matrix3x4_t* matrix, bool hitbox_mask);
	bool IsValidIn( float range, float max_unlag );
	bool IsValidExtended( );
	bool IsValid( float flSimulationTime, bool bValid, const float flRange = 0.2f );

	C_BasePlayer* Player{ };
	int32_t Index{ };

	bool Valid{ }, HasState{ };
	alignas(16) matrix3x4_t Bones[ 128 ];

	bool Dormant{ };
	Vector Velocity;
	Vector Origin;
	matrix3x4_t* BoneCache;
	Vector AbsOrigin;
	Vector Mins;
	Vector Maxs;
	AnimationLayer Layers[ 13 ];
	std::array<float, 24> Poses;
	CCSGOPlayerAnimState* AnimState;
	float AnimTime{ };
	float SimTime{ };
	float InterpTime{ };
	float Duck{ };
	float LBY{ };
	float LastShotTime{ };
	QAngle LastReliableAngle{ };
	QAngle EyeAngle;
	Vector AbsAng;
	int Flags{ };
	int Eflags{ };
	int Effects{ };
	float Cycle{ };
	float YawRate{ };
	int Lag{ };
	bool Didshot;
};

class Backtrack : public Singleton <Backtrack>
{
	friend class Singleton<Backtrack>;
private:

	struct AnimationInfo {
		AnimationInfo( C_BasePlayer* player, std::deque<StoreAnimation> animations )
			: Player( player ), Frames( std::move( animations ) ), LastSpawnTime( 0 )
		{
		}

		void UpdatePlayer( C_BasePlayer* pEnt, StoreAnimation* rec, StoreAnimation* from, int i );
		void UpdateAnims( StoreAnimation* to, StoreAnimation* from );

		C_BasePlayer* Player{ };
		std::deque<StoreAnimation> Frames;

		float LastSpawnTime;
		QAngle LastAnge;
	};

	std::unordered_map<unsigned long, AnimationInfo> AnimInfo;

public:

	void Run( );

	AnimationInfo* GetAnimationInfo( C_BasePlayer* player );
	std::optional<StoreAnimation*> GetLatestAnimtion( C_BasePlayer* player );
	std::optional<StoreAnimation*> GetOldestAnimation( C_BasePlayer* player );
	std::optional<std::pair<StoreAnimation*, StoreAnimation*>> GetIntermediateAnimations( C_BasePlayer* player, float range = 1.f );
	std::vector<StoreAnimation*> GetValidAnimation( C_BasePlayer* player, float range = 1.f );
	std::optional<StoreAnimation*> GetLatestFireAnimation( C_BasePlayer* player );
	std::vector<StoreAnimation*> GetValidAnimationNonLimited( C_BasePlayer* player );
};

class LagComp : public Singleton <LagComp>
{

	friend class Singleton<LagComp>;

public:

	float LerpTime( );
	bool ValidTick( int tick );
	void Store( C_BasePlayer* pEnt );
	void Clear( int i );

	float ShotTime[ 65 ];
	int ShotTick[ 65 ];

	void AnimationFix( ClientFrameStage_t stage );
	void InitAnim( C_BasePlayer* pEnt );

	void FixLocalAnims( );
	void CreateFakeChams( );
	void UpdateRecords( C_BasePlayer* pEnt, bool m_bPrevious, StoreAnimation* anim, StoreAnimation* fr );

	float pose_parameter[ 24 ];
	AnimationLayer layers[ 15 ];

	bool real_server_update = false;
	bool fake_server_update = false;

	float real_simulation_time = 0.0f;
	float fake_simulation_time = 0.0f;

	float spawntime = 0.0f;
	float tickcount = 0.0f;

	float abs_angles = 0.0f;

	CCSGOPlayerAnimState* m_nState;
	matrix3x4_t fake_matrix[ 128 ];
	CBaseHandle* handle = nullptr;
};