#pragma once
#include "Groupbox.h"

class Checkbox : public Base {
public:

	// Constructor.
	Checkbox( std::string m_Name, bool* m_Value );

	void Draw( );
	void Update( );

	Vector2D m_Pos;

private:

	std::string m_Name;
	bool* m_Value;
};