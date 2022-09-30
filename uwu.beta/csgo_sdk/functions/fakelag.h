#pragma once
#include "../sdk/csgostructs.hpp"

#include "../sdk/sdk.hpp"
#include <vector>

class CFakeLag : public Singleton <CFakeLag>
{

	friend class Singleton<CFakeLag>;

public:

	bool IsInit();
	void OnCreateMove(CUserCmd* m);

	bool isLag = false;
	bool isTry = false;

};