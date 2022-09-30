#pragma once

#include <functional>
#include "groupbox.h"

class Button : public Base {
public:

	Button( std::string m_Name, std::function< void( ) > Function, Vector2D m_CustomPos, Vector2D m_CustomSize );

	void Draw( );
	void Update( );

	Vector2D m_Pos;

	Vector2D m_CustomPos;
	Vector2D m_CustomSize;

private:

	std::string m_Name;
	std::function< void( ) > Function = { };

	bool m_Focused;
};