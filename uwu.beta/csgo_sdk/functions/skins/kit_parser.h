#pragma once
#include <vector>
#include <map>
#include <xstring>
struct paint_kit
{
	int id;
	int rarity;
	std::string name;
	std::string name_short;
	std::vector<std::string> weaponName;

	auto operator < (const paint_kit& other) const -> bool
	{
		return name < other.name;
	}
};

struct skin_clr_info {
	int clr[ 5 ];
};

extern bool skins_parsed;

extern std::vector<paint_kit> k_skins;
extern std::vector<paint_kit> k_gloves;
extern std::vector<skin_clr_info> k_skin_clr;

extern auto initialize_kits() -> void;