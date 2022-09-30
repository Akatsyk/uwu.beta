#pragma once
#include "groupbox.h"

class Combo : public Base
{
public:

	Combo( std::string m_Name, int* m_Value, std::vector<std::string> m_Items, float m_ComboSize );

	void Update( );
	void Draw( );

	Vector2D m_Pos;
	bool m_Focused;
	float m_ComboSize;

private:

	std::string m_Name;
	int* m_Value;
	
	std::vector<std::string> m_Items;
};