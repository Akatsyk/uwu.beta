#include "Clua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include "../render/engine/engine_draw.h"
#include "../functions/logs.h"
#include "../gui/gui.h"
#include "../config/config.h"

void lua_panic(sol::optional <std::string> message)
{
	if (!message)
		return;

	auto log = ("Error: ") + message.value_or("unknown");
	Logs::Get().AddLog(log, Color::White);
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return c_lua::Get().get_script_id(get_current_script(s));
}

std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}

menu_item find_item(std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}

namespace ns_gui
{
	void add_check_box(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::Get().get_script_id(script);

		auto& items = c_lua::Get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(false)));
	}

	void add_combo_box(sol::this_state s, std::string name, std::vector <std::string> labels)
	{
		if (labels.empty())
			return;

		auto script = get_current_script(s);
		auto script_id = c_lua::Get().get_script_id(script);

		auto& items = c_lua::Get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(labels, 0)));
	}

	void add_slider_float(sol::this_state s, const std::string& name, float min, float max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::Get().get_script_id(script);

		auto& items = c_lua::Get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_color_picker(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::Get().get_script_id(script);

		auto& items = c_lua::Get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(Color::White, 0)));
	}

	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, menu_item> stored_values;
	std::unordered_map <std::string, void*> config_items;

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : g_ConfigMaster.items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						Logs::Get().AddLog(("Error: invalid config item type, must be ") + type, Color::White);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				Logs::Get().AddLog(("Error: cannot find config variable \"") + name + '\"', Color::White);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Gui::m_Details.GetMenuState() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto& it = find_item(c_lua::Get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("bool")))
				return *(bool*)config_items[name];

			Logs::Get().AddLog(("Error: cannot find menu variable \"") + name + '\"', Color::White);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Gui::m_Details.GetMenuState() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].combo_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name]; //-V206
			else
				return 0;
		}

		auto& it = find_item(c_lua::Get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("int")))
				return *(int*)config_items[name]; //-V206

			Logs::Get().AddLog(("Error: cannot find menu variable \"") + name + '\"', Color::White);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.combo_box_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Gui::m_Details.GetMenuState() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto& it = find_item(c_lua::Get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("float")))
				return *(float*)config_items[name];

			Logs::Get().AddLog(("Error: cannot find menu variable \"") + name + '\"', Color::White);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Gui::m_Details.GetMenuState() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].color_picker_value;
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color::White;
		}

		auto& it = find_item(c_lua::Get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("Color")))
				return *(Color*)config_items[name];

			Logs::Get().AddLog(("Error: cannot find menu variable \"") + name + '\"', Color::White);
			return Color::White;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.color_picker_value;
	}
}

namespace ns_render
{
	void draw_rect_filled(float x, float y, float w, float h, Color c) {
		EngineDraw::Get().FilledRect((int)x, (int)y, (int)w, (int)h, c);
	}
}

namespace ns_client
{
	void add_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{
		if (eventname != ("on_paint"))
		{
			Logs::Get().AddLog(("Error: invalid hook \"") + eventname + '\"', Color::White);
			return;
		}

		if (c_lua::Get().loaded.at(get_current_script_id(s)))
			c_lua::Get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}
}

sol::state lua;
void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package);

	lua[("collectgarbage")] = sol::nil;
	lua[("dofilsse")] = sol::nil;
	lua[("load")] = sol::nil;
	lua[("loadfile")] = sol::nil;
	lua[("pcall")] = sol::nil;
	lua[("print")] = sol::nil;
	lua[("xpcall")] = sol::nil;
	lua[("getmetatable")] = sol::nil;
	lua[("setmetatable")] = sol::nil;
	lua[("__nil_callback")] = [](){};
	
	lua.new_usertype <Color>(("col_t"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)("r"), &Color::r,
		(std::string)("g"), &Color::g,
		(std::string)("b"), &Color::b,
		(std::string)("a"), &Color::a
		);

	auto render = lua.create_table();
	render[("rect_filled")] = ns_render::draw_rect_filled;
	lua[("render")] = render;

	auto client = lua.create_table();
	client[("add")] = ns_client::add_callback;
	lua[("hooks")] = client;

	auto menu = lua.create_table();
	menu[("checkbox")] = ns_gui::add_check_box;
	menu[("combobox")] = ns_gui::add_combo_box;
	menu[("slider")] = ns_gui::add_slider_float;
	menu[("color_picker")] = ns_gui::add_color_picker;
	menu[("get_bool")] = ns_gui::get_bool;
	menu[("get_int")] = ns_gui::get_int;
	menu[("get_float")] = ns_gui::get_float;
	menu[("get_color")] = ns_gui::get_color;
	lua[("gui")] = menu;

	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name) //-V106
			return i;

	return -1;
}

int c_lua::get_script_id_by_path(const std::string& path)
{
	for (auto i = 0; i < pathes.size(); i++)
		if (pathes.at(i).string() == path) //-V106
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	pathes.clear();
	scripts.clear();

	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
	{
		folder = std::string(path) + ("\\uwucsgo\\scripts\\");
		CreateDirectory(folder.c_str(), NULL);

		auto i = 0;

		for (auto& entry : std::filesystem::directory_iterator(folder))
		{
			if (entry.path().extension() == (".lua") || entry.path().extension() == (".luac"))
			{
				auto path = entry.path();
				auto filename = path.filename().string();

				auto didPut = false;

				for (auto i = 0; i < oldScripts.size(); i++)
				{
					if (filename == oldScripts.at(i)) //-V106
					{
						loaded.emplace_back(oldLoaded.at(i)); //-V106
						didPut = true;
					}
				}

				if (!didPut)
					loaded.emplace_back(false);

				pathes.emplace_back(path);
				scripts.emplace_back(filename);
				items.emplace_back(std::vector <std::pair <std::string, menu_item>>());

				++i;
			}
		}
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id)) //-V106
		return;

	auto path = get_script_path(id);

	if (path == (""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path,
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = ("Error: ") + (std::string)error.what();
				Logs::Get().AddLog(log, Color::White);
				error_load = true;

			}

			return result;
		}
	);

	if (error_load | loaded.at(id) == false)
	{
		loaded.at(id) = false;
		return;
	}
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id)) //-V106
		return;

	items.at(id).clear();
	hooks.unregisterHooks(id);
	loaded.at(id) = false; //-V106
}

void c_lua::reload_all_scripts()
{
	for (auto current : scripts)
	{
		if (!loaded.at(get_script_id(current))) //-V106
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return ("");

	return pathes.at(id).string(); //-V106
}