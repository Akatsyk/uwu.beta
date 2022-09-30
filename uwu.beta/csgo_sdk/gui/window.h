#pragma once
#include <vector>
#include <string>

#include "base.h"

class Groupbox;
class Window : public Base {
public:

	// Constructor.
	Window(std::string m_Name, Vector2D* m_Pos, Vector2D m_Size, std::vector<std::string> m_WindowTabs = { }, int* m_Selected = 0);

	// Add groubox to window.
	void AddGroup(Groupbox* Groupbox);

	// Window position.
	Vector2D* m_WindowPosition;

	// Vector.
	std::vector<std::string> m_WindowTabs;

	// Current selected tab.
	int* m_Selected;

private:

	// Draw window.
	void Draw();

	// Init window.
	void Run();

	void InitTabs();
	void InitPosition();
	void ControlCenter();

	// Some additions.
	std::string m_Name;
	Vector2D m_Size;

	// Tab swithced.
	bool m_SetSwitch;

};