#pragma once
#include "groupbox.h"

class ListBoxAlternative : public Base {
public:

	// Constructors.
	ListBoxAlternative(std::string m_Name, int* m_Select, int* m_SelectPlus, std::vector<std::string>& m_Items, int m_Num, int m_Addition);
	ListBoxAlternative(std::string m_Name, int* m_Select, int* m_SelectPlus, std::vector<std::string>& m_Items, std::string& m_Filter, int m_Num, int m_Addition);

	void Draw();
	void Update();

	Vector2D m_Pos;

private:

	void GetItems();
	void Scroll(int m_Limit);
	bool IsHold();

	int* m_Select;
	int* m_SelectPlus;

	std::string ToLower(std::string String);

	int m_Addition;
	int m_Num;

	std::vector<std::string> m_LowerItems;
	std::vector<std::string>* m_Items;

	std::string* m_Filter;
	std::string m_PreviousFilter = "123";

	std::string m_Name;
	std::vector< std::pair<std::string, int >> m_Temp;
};