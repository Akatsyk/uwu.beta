#pragma once
#include "../sdk/csgostructs.hpp"

#include "../sdk/sdk.hpp"
#include <vector>

class CAntiAim : public Singleton <CAntiAim>
{

	friend class Singleton<CAntiAim>;

public:

	void OnCreateMove(CUserCmd* m);
	bool Condition(CUserCmd* m_pcmd, bool dynamic_check = true);
	float At_target(CUserCmd* m);

};