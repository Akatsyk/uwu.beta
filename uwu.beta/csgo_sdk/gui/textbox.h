#pragma once
#include "groupbox.h"

class TextInput : public Base {
public:

	TextInput( std::string m_Name, std::string* m_Label );

	void Update( );
	void Draw( );

	Vector2D m_Pos;

private:

	std::string m_Name;
	std::string* m_Label;

	bool m_Focused;

};