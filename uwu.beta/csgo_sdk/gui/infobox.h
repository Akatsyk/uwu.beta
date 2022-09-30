#pragma once
#include "Groupbox.h"

class Infobox : public Base {
public:

	// Constructor.
	Infobox( int m_Value, std::vector<std::string> m_Data );

	void Draw( );
	void Update( );

	Vector2D m_Pos;
	Vector2D m_CustomPos;

private:

	int m_Value;
};