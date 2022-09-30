#pragma once
#include "../sdk/csgostructs.hpp"

#include "../sdk/sdk.hpp"
#include <vector>

#include <map>

class CVarSys : public Singleton <CVarSys>
{

	friend class Singleton<CVarSys>;

public:

	void CallUpdate( );
	bool isInit = false;

	std::map<std::string, ConVar*> m_StoredVars;
};