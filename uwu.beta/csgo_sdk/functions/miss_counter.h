#pragma once

#pragma once

#include "../sdk/sdk.hpp"
#include "../hooks.hpp"

#include <deque>

struct event_data_t
{
	Vector pos = Vector(0, 0, 0);
	bool got_pos = false;
	bool got_hurt = false;
	float time = 0.f;
	int hurt_player = -1;
	int userid = -1;
	bool hurt = false;
	int damage = 0;
	bool died = false;
	int hitbox = HITBOX_MAX;
};

class MissCount : public Singleton <MissCount> {

	friend class Singleton<MissCount>;

public:

	matrix3x4_t matrixed[128];

	std::deque< event_data_t > last_events;

	int last_rbot_entity = -1;
	float last_rbot_shot_time = 0.f;

	void Setup_Event(IGameEvent* event);
	bool Valid(C_BasePlayer* x);
	void SetData(C_BasePlayer* player, int index, Vector angle);

	void Build_();

	Vector last_rbot_shot_eyepos = Vector(0, 0, 0);
	Vector last_rbot_shot_angle = Vector(0, 0, 0);

	void CreateMove();
	std::deque<Vector> dequeBulletImpacts;
	bool bPlayerHurt[64];
	bool bBulletImpact[64];
	bool fire[64];
};