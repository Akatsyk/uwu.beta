#pragma once
#include "groupbox.h"
#include <corecrt_math.h>

class ColorPicker : public Base {
public:

	// Constructor.
	ColorPicker( Color* m_Select, float* m_Hue, bool m_Alpha = false );

	void Draw( );
	void Update( );

	Vector2D m_Pos;
	Vector2D m_CursorPos;

private:

	bool m_Focused;
	bool m_AlphaBar;

	float* m_Hue;
	float m_Alpha;

	Color* m_Select;
};