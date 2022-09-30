#pragma once

#include "groupbox.h"

struct MultiConstructor
{
	MultiConstructor( std::string m_Name, bool* m_Value )
	{
		this->m_Name = m_Name;
		this->m_Value = m_Value;
	}

	std::string m_Name;
	bool* m_Value;
};

class Multi : public Base
{
public:

	Multi( );
	Multi( std::string m_Name, float m_MultiComboSize = 160 );

	void Update( );
	void Draw( );

	void AddItem( std::string m_Name, bool* m_Value);

	Vector2D m_Pos;
	float m_MultiComboSize;

private:

	std::string m_Name;
	std::vector<MultiConstructor> m_Items;

	bool m_Focused;
};