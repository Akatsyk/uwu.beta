#pragma once
#include "../sdk/csgostructs.hpp"

#include "../sdk/sdk.hpp"
#include <vector>

class CMovement : public Singleton <CMovement>
{
	friend class Singleton<CMovement>;

public:
	void OnCreateMove(CUserCmd* m);
	void BHOP(CUserCmd* cmd);
	void Strafe(CUserCmd* cmd);
	void MouseFix(CUserCmd* cmd);
	void SlowWalk(CUserCmd* cmd);
	void FakeDuck(CUserCmd* cmd);
	bool LagPeek(CUserCmd* cmd);
	void AutoPeek(CUserCmd* cmd, float wish_yaw);

	Vector m_StartPos;
	bool m_FiredShot;

	bool m_InDuck;
	bool m_InSlow;
	bool m_InPeek;

	inline float FastSqrt(float x) {
		unsigned int i = *(unsigned int*)&x;
		i += 127 << 23;
		i >>= 1;
		return *(float*)&i;
	}
};