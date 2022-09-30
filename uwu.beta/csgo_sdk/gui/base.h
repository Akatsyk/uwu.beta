#pragma once
#include "gui.h"

class Base
{
public:

	// Delete element | release memory
	virtual void Destroy( Base* m_Element );

};

extern Base m_Base;