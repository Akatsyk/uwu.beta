#pragma once

#include "groupbox.h"

struct Tab_T
{
	Tab_T(std::string m_Name) { this->m_Name = m_Name; }
	std::string m_Name;
};

class SubTab
{
public:

	SubTab();
	SubTab(Vector2D size, int* m_Value);

	void Draw();
	void Update();

	Vector2D m_Pos;
	Vector2D m_Size;

	std::vector<Tab_T> m_Items;

	void AddTab(std::string m_Name);

private:

	int* m_Value;
};