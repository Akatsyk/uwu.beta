#pragma once

#include "../sdk/sdk.hpp"
#include "../hooks.hpp"

#include <deque>

class C_HookedEvents : public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent* event);
	void RegisterSelf();
	void RemoveSelf();
	int GetEventDebugID(void);
};

extern C_HookedEvents HookedEvents;