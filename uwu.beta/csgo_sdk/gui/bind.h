#pragma once
#include "groupbox.h"

class Bind : public Base {
public:

	Bind( int* m_Value );

	void Update( );
	void Draw( );

	Vector2D m_Pos;

private:

	int* m_Value;
	bool m_Focused;
};