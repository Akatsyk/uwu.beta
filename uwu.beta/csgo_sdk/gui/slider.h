#pragma once
#include "groupbox.h"

enum SliderType
{
	Sld_Defaut,
	Sld_Checkbox,
	Sld_Right
};

class Slider : public Base {
public:

	// Constructor.
	Slider( std::string m_Name, float* m_Value, int m_Type = 0, int m_Min = 0, int m_Max = 100, std::string m_Suffix = "" );

	void Draw( );
	void Update( );

	Vector2D m_Pos;
	int m_Type;

private:

	std::string m_Name;
	std::string m_Suffix;

	float* m_Value;

	int m_Min;
	int m_Max;

};