#pragma once

#include "groupbox.h"

class Text
{
public:

	Text();
	Text(std::string m_Name);

	void Draw();
	Vector2D m_Pos;

private:

	std::string m_Name;

};