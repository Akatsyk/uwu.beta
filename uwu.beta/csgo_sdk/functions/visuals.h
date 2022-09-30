#pragma once

#include "../sdk/sdk.hpp"
#include "../hooks.hpp"

#include <algorithm>
#include <math.h>

#include "../sdk/utils/math.hpp"

class Visuals : public Singleton <Visuals>
{

	friend class Singleton<Visuals>;

public:

	struct
	{
		int left, top, right, bottom;
	}bbox;

	struct EntBoxInfoT
	{
		int x, y, w, h, alpha, alpha_anim;
		C_BasePlayer* ent;
	};

	struct bottom_info_t
	{
		bool isString;
		std::string str;
		double maxValue;
		double percentage;
		Color color;
	};

	EntBoxInfoT infos[4096];

	float esp_alpha_fade[65];
	int health[65];

	void Run();
	void BoundBox(C_BasePlayer* player);
	void Box(C_BasePlayer* e);
	void Health(C_BasePlayer* e);
	void Ammo(C_BasePlayer* e);
	void Name(C_BasePlayer* e);
	void Weapon(C_BasePlayer* e);
	void Icon(C_BasePlayer* e);
	void OOF(C_BasePlayer* e);
	const char* GetWeaponIC(C_BaseCombatWeapon* e);
	void SkyBox();
	void RunWorld();
	bool BoundBoxWeapon(C_BasePlayer* m_entity, EntBoxInfoT& box, bool dynamic);
	void DrawNameWeapon(EntBoxInfoT inf);
	void DrawBoxWPN(EntBoxInfoT inf);
	void DrawAmmoWpn(EntBoxInfoT inf);
	bool W2S(const Vector& origin, Vector2D& screen);
	void ThirdPerson();
	void Spread(IDirect3DDevice9* m_device);
	void DrawWatermark();
	void DrawSpectatorList();
	void DrawKeyBinds();
	void DrawAutoPeekIndicator();
	void FogChanger( );
	void GrenadeEsp( C_BaseEntity* entity );
	void DimensionCrosshair( );

private:

	std::vector<Vector> path;
	std::vector<std::pair<Vector, QAngle>> others;

	int type = 0;
	int act = 0;

	void Setup( Vector& vecSrc, Vector& vecThrow, Vector viewangles );
	void Simulate( CViewSetup* setup );

	int Step( Vector& vecSrc, Vector& vecThrow, int tick, float interval );
	bool CheckDetonate( const Vector& vecThrow, const trace_t& tr, int tick, float interval );

	void TraceHull( Vector& src, Vector& end, trace_t& tr );
	void AddGravityMove( Vector& move, Vector& vel, float frametime, bool onground );
	void PushEntity( Vector& src, const Vector& move, trace_t& tr );
	void ResolveFlyCollisionCustom( trace_t& tr, Vector& vecVelocity, float interval );
	int PhysicsClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce );

public:

	void Tick( int buttons );
	void View( CViewSetup* setup );
	void Paint( );

};

class DormantEsp : public Singleton <DormantEsp>
{
	friend class Singleton<DormantEsp>;

public:
	void start();

	bool adjust_sound(C_BasePlayer* player);
	void setup_adjust(C_BasePlayer* player, SndInfo_t& sound);
	bool valid_sound(SndInfo_t& sound);

	struct SoundPlayer
	{
		void reset(bool store_data = false, const Vector& origin = Vector(0,0,0), int flags = 0)
		{
			if (store_data)
			{
				m_iReceiveTime = g_GlobalVars->realtime;
				m_vecOrigin = origin;
				m_nFlags = flags;
			}
			else
			{
				m_iReceiveTime = 0.0f;
				m_vecOrigin.Zero();
				m_nFlags = 0;
			}
		}

		void Override(SndInfo_t& sound)
		{
			m_iReceiveTime = g_GlobalVars->realtime;
			m_vecOrigin = *sound.m_pOrigin;
		}

		float m_iReceiveTime = 0.0f;
		Vector m_vecOrigin = Vector(0, 0, 0);
		int m_nFlags = 0;
	} m_cSoundPlayers[65];

	CUtlVector <SndInfo_t> m_utlvecSoundBuffer;
	CUtlVector <SndInfo_t> m_utlCurSoundList;
};
