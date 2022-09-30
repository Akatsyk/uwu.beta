#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/sdk.hpp"

#include <vector>
#include <xmmintrin.h>

#include "lag_comp.h"

class CLegitbot : public Singleton <CLegitbot>
{
public:

	CLegitbot( );

public:

	void Run( CUserCmd* cmd );
	bool IsEnabled( CUserCmd* cmd );
	float GetFovToPlayer( QAngle viewAngle, QAngle aimAngle );

	float GetFov( );
	void Update( );

private:

	void RCS( QAngle& angle, C_BasePlayer* target );
	bool IsLineGoesThroughSmoke( Vector vStartPos, Vector vEndPos );
	void Smooth( QAngle currentAngle, QAngle aimAngle, QAngle& angle );
	bool IsSilent( );

	C_BasePlayer* GetClosestPlayer( CUserCmd* cmd, int& bestBone, float& fov, QAngle& angles );
	C_BasePlayer* target = nullptr;

	int lastShotTick;
	int shotsFired;

	bool shot_delay = false;
	int shot_delay_time = 0;

	bool kill_delay = false;
	int kill_delay_time = 0;

	QAngle current_punch = { 0, 0, 0 };
	QAngle last_punch = { 0, 0, 0 };

	struct
	{

		bool enabled;
		bool flash;

		bool smoke;
		bool auto_pistol;

		struct
		{
			bool head;
			bool chest;
			bool hands;
			bool legs;
		} hitboxes;

		float fov;
		float smooth;

		float shot_delay;
		float kill_delay;

		float silent_fov;
		int silent_mode;

		bool rcs;
		float rcs_x;
		float rcs_y;
		int rcs_mode;

		bool awall;
		float awall_damage;

		int start;
		bool vis;

	} m_config;
};