#include "base.h"

Base m_Base;

void Base::Destroy( Base* m_Element )
{
	delete m_Element;
}