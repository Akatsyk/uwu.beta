#pragma once
#include "../sdk/csgostructs.hpp"
#include "../sdk/sdk.hpp"

#include <vector>

class TickBaseSys : public Singleton< TickBaseSys > {

	friend class Singleton<TickBaseSys>;

public:

	void CallExploit(CUserCmd* m);
	void WriteUserCmd(bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd);
	void TryDefensive(CUserCmd* m);

	bool IsShotDT;
	bool IsCharged;

	int ValueToShift;
	bool DidShot = false;
	bool Key = false;
	int ShiftRelloc;

	int TicksToSkip;
	int ShiftedCommand;
	int32_t OriginalTickBase;

	int ShiftTime;
	int m_ReinitShift;

	struct
	{
		bool m_bIsPeeking = false;
		bool m_bIsPrevPeek = false;
	} m_Peek;
};