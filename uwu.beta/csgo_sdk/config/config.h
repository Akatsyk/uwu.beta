#pragma once
#include <iomanip>

#include <ShlObj_core.h>
#include "../sdk/sdk.hpp"
#include "json/json.hpp"
#include "../lua/Clua.h"

using json = nlohmann::json;

class ConfigManager
{

	class item {
	public:
		std::string name;
		void* pointer;
		std::string type;

		item(std::string name, void* pointer, std::string type)
		{
			this->name = name;
			this->pointer = pointer;
			this->type = type;
		}
	};

public:

	std::vector<item*> items;
	std::vector<item*> skin_items;

public:

	void setup_config();
	void setup_skin_clr();
	
	////////////////////////////////////////////////////////////////////////////////

	void add_item(void* pointer, const char* name, std::string type)
	{
		items.push_back(new item(std::string(name), pointer, type));
	}

	void add_item_for_skins( void* pointer, const char* name, std::string type )
	{
		skin_items.push_back( new item( std::string( name ), pointer, type ) );
	}

	////////////////////////////////////////////////////////////////////////////////

	void setup_item(int* pointer, int value, std::string name)
	{
		add_item(pointer, name.c_str(), "int");
		*pointer = value;
	}

	void setup_item( std::vector< std::string >* pointer, const std::string& name )
	{
		add_item( pointer, name.c_str( ), "vector<string>" );
	}

	void setup_item(bool* pointer, bool value, std::string name)
	{
		add_item(pointer, name.c_str(), "bool");
		*pointer = value;
	}

	void setup_item(float* pointer, float value, std::string name)
	{
		add_item(pointer, name.c_str(), "float");
		*pointer = value;
	}

	void setup_item(Color* pointer, Color value, std::string name)
	{
		add_item(pointer, name.c_str(), "col_t");
		*pointer = value;
	}

	void setup_item(std::string* pointer, std::string value, std::string name)
	{
		add_item(pointer, name.c_str(), "string");
		*pointer = value;
	}

	////////////////////////////////////////////////////////////////////////////////


	void setup_item_skins( int* pointer, int value, std::string name )
	{
		add_item_for_skins( pointer, name.c_str( ), "int" );
		*pointer = value;
	}

	void setup_item_skins( bool* pointer, bool value, std::string name )
	{
		add_item_for_skins( pointer, name.c_str( ), "bool" );
		*pointer = value;
	}

	void setup_item_skins( float* pointer, float value, std::string name )
	{
		add_item_for_skins( pointer, name.c_str( ), "float" );
		*pointer = value;
	}

	void setup_item_skins( Color* pointer, Color value, std::string name )
	{
		add_item_for_skins( pointer, name.c_str( ), "col_t" );
		*pointer = value;
	}

	void setup_item_skins( std::string* pointer, std::string value, std::string name )
	{
		add_item_for_skins( pointer, name.c_str( ), "string" );
		*pointer = value;
	}

	////////////////////////////////////////////////////////////////////////////////

	void save_skins_items( std::string config )
	{
		std::string folder, file;

		auto get_dir = [ &folder, &file, &config ]( ) ->void {
			static TCHAR path[ MAX_PATH ];
			if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
				folder = std::string( path ) + "\\uwucsgo\\skins\\";
				file = std::string( path ) + "\\uwucsgo\\skins\\" + config;
			}

			CreateDirectory( folder.c_str( ), NULL );
		};
		get_dir( );

		std::ofstream ofs;

		ofs.open( file + "", std::ios::out | std::ios::trunc );

		json allJson;

		for ( auto it : skin_items ) {
			json j;

			j[ "name" ] = it->name;
			j[ "type" ] = it->type;

			if ( !it->type.compare( "int" ) ) {
				j[ "value" ] = ( int )*( int* )it->pointer;
			}
			else if ( !it->type.compare( "float" ) ) {
				j[ "value" ] = ( float )*( float* )it->pointer;
			}
			else if ( !it->type.compare( "bool" ) ) {
				j[ "value" ] = ( bool )*( bool* )it->pointer;
			}
			else if ( !it->type.compare( "string" ) ) {
				j[ "value" ] = (std::string) * ( std::string* )it->pointer;
			}
			else if ( !it->type.compare( "col_t" ) ) {
				Color c = *( Color* )(it->pointer);

				std::vector<int> a = { c.r( ), c.g( ), c.b( ), c.a( ) };

				json ja;

				for ( auto& i : a ) {
					ja.push_back( i );
				}

				j[ "value" ] = ja.dump( );
			}
			else if ( !it->type.compare( ("vector<string>") ) )
			{
				auto& ptr = *(std::vector<std::string>*)(it->pointer);
				json ja;

				for ( auto& i : ptr )
					ja.push_back( i );

				j[ ("value") ] = ja.dump( );
			}

			allJson.push_back( j );
		}

		std::string data = allJson.dump( );

		ofs << std::setw( 4 ) << data << std::endl;

		ofs.close( );
	}

	////////////////////////////////////////////////////////////////////////////////

	void save(std::string config)
	{
		std::string folder, file;

		auto get_dir = [&folder, &file, &config]() ->void {
			static TCHAR path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
				folder = std::string(path) + "\\uwucsgo\\";
				file = std::string(path) + "\\uwucsgo\\" + config;
			}

			CreateDirectory(folder.c_str(), NULL);
		};
		get_dir();

		std::ofstream ofs;

		ofs.open(file + "", std::ios::out | std::ios::trunc);

		json allJson;

		for (auto it : items) {
			json j;

			j["name"] = it->name;
			j["type"] = it->type;

			if (!it->type.compare("int")) {
				j["value"] = (int)*(int*)it->pointer;
			}
			else if (!it->type.compare("float")) {
				j["value"] = (float)*(float*)it->pointer;
			}
			else if (!it->type.compare("bool")) {
				j["value"] = (bool)*(bool*)it->pointer;
			}
			else if (!it->type.compare("string")) {
				j["value"] = (std::string)*(std::string*)it->pointer;
			}
			else if (!it->type.compare("col_t")) {
				Color c = *(Color*)(it->pointer);

				std::vector<int> a = { c.r(), c.g(), c.b(), c.a() };

				json ja;

				for (auto& i : a) {
					ja.push_back(i);
				}

				j["value"] = ja.dump();
			}
			else if (!it->type.compare(("vector<string>")))
			{
				auto& ptr = *(std::vector<std::string>*)(it->pointer);
				json ja;

				for (auto& i : ptr)
					ja.push_back(i);

				j[("value")] = ja.dump();
			}

			allJson.push_back(j);
		}

		auto get_type = [](menu_item_type type)
		{
			switch (type) //-V719
			{
			case CHECK_BOX:
				return "bool";
			case COMBO_BOX:
				return "int";
			case SLIDER_FLOAT:
				return "float";
			case COLOR_PICKER:
				return "col_t";
			}
		};

		for (auto i = 0; i < c_lua::Get().scripts.size(); ++i)
		{
			auto& script = c_lua::Get().scripts.at(i);

			for (auto& item : c_lua::Get().items.at(i))
			{
				if (item.second.type == NEXT_LINE)
					continue;

				json j;
				auto type = (std::string)get_type(item.second.type);

				j[("name")] = item.first;
				j[("type")] = type;

				if (!type.compare(("bool")))
					j[("value")] = item.second.check_box_value;
				else if (!type.compare(("int")))
					j[("value")] = item.second.combo_box_value;
				else if (!type.compare(("float")))
				{
					if (item.second.type == SLIDER_FLOAT)
						j[("value")] = item.second.slider_float_value;

					else if (item.second.type == COLOR_PICKER)
						j[("value")] = item.second.color_picker_hue;
				}
				else if (!type.compare(("col_t")))
				{
					std::vector <int> color =
					{
						item.second.color_picker_value.r(),
						item.second.color_picker_value.g(),
						item.second.color_picker_value.b(),
						item.second.color_picker_value.a()
					};

					json j_color;

					for (auto& i : color)
						j_color.push_back(i);

					j[("value")] = j_color.dump();
				}

				allJson.push_back(j);
			}
		}

		std::string data = allJson.dump();

		ofs << std::setw(4) << data << std::endl;

		ofs.close();
	}

	////////////////////////////////////////////////////////////////////////////////

	void load(std::string config, bool m_load)
	{
		static auto find_item = [](std::vector< item* > items, std::string name) -> item* {
			for (int i = 0; i < (int)items.size(); i++) {
				if (items[i]->name.compare(name) == 0)
					return (items[i]);
			}

			return nullptr;
		};

		static auto right_of_delim = [](std::string const& str, std::string const& delim) -> std::string {
			return str.substr(str.find(delim) + delim.size());
		};

		if (m_load)
		{
			std::string folder, file;

			auto get_dir = [&folder, &file, &config]() ->void {
				static TCHAR path[MAX_PATH];
				if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
					folder = std::string(path) + "\\uwucsgo\\";
					file = std::string(path) + "\\uwucsgo\\" + config;
				}

				CreateDirectory(folder.c_str(), NULL);
			};

			get_dir();

			std::ifstream ifs;
			std::string data;

			std::string path = file + "";

			ifs.open(path);

			json allJson;

			ifs >> allJson;

			for (json::iterator it = allJson.begin(); it != allJson.end(); ++it) {

				json j = *it;

				std::string name = j["name"];
				std::string type = j["type"];

				item* m_item = find_item(items, name);

				auto script_item = std::count_if(name.begin(), name.end(),
					[](char& c)
					{
						return c == '.';
					}
				) >= 2;

				if (script_item)
				{
					std::string script_name;
					auto first_point = false;

					for (auto& c : name)
					{
						if (c == '.')
						{
							if (first_point)
								break;
							else
								first_point = true;
						}

						script_name.push_back(c);
					}

					auto script_id = c_lua::Get().get_script_id(script_name);

					if (script_id == -1)
						continue;

					for (auto& current_item : c_lua::Get().items.at(script_id))
					{
						if (current_item.first == name)
						{
							if (!type.compare(("bool")))
							{
								current_item.second.type = CHECK_BOX;
								current_item.second.check_box_value = j[("value")].get<bool>();
							}
							else if (!type.compare(("int")))
							{
								if (current_item.second.type == COMBO_BOX)
									current_item.second.combo_box_value = j[("value")].get<int>();
							}
							else if (!type.compare(("float")))
							{
								if (current_item.second.type == SLIDER_FLOAT)
									current_item.second.slider_float_value = j[("value")].get<float>();

								else if (current_item.second.type == COLOR_PICKER)
									current_item.second.color_picker_hue = j[("value")].get<float>();
							}
							else if (!type.compare(("col_t")))
							{
								std::vector<int> a;
								json ja = json::parse(j[("value")].get<std::string>().c_str());

								for (json::iterator it = ja.begin(); it != ja.end(); ++it)
									a.push_back(*it);

								current_item.second.color_picker_value = Color(a[0], a[1], a[2], a[3]);
							}
						}
					}
				}
			}

			ifs.close();
		}
		else
		{
			std::string folder, file;

			auto get_dir = [&folder, &file, &config]() ->void {
				static TCHAR path[MAX_PATH];
				if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
					folder = std::string(path) + "\\uwucsgo\\";
					file = std::string(path) + "\\uwucsgo\\" + config;
				}

				CreateDirectory(folder.c_str(), NULL);
			};

			get_dir();

			std::ifstream ifs;
			std::string data;

			std::string path = file + "";

			ifs.open(path);

			json allJson;

			ifs >> allJson;

			for (json::iterator it = allJson.begin(); it != allJson.end(); ++it) {

				json j = *it;

				std::string name = j["name"];
				std::string type = j["type"];

				item* m_item = find_item(items, name);

				auto script_item = std::count_if(name.begin(), name.end(),
					[](char& c)
					{
						return c == '.';
					}
				) >= 2;

				if (m_item) {
					if (!type.compare("int")) {
						*(int*)m_item->pointer = j["value"].get<int>();
					}
					else if (!type.compare("float")) {
						*(float*)m_item->pointer = j["value"].get<float>();
					}
					else if (!type.compare("bool")) {
						*(bool*)m_item->pointer = j["value"].get<bool>();
					}
					else if (!type.compare("string")) {
						*(std::string*)m_item->pointer = j["value"].get<std::string>();
					}
					else if (!type.compare("col_t")) {
						std::vector<int> a;

						json ja = json::parse(j["value"].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							a.push_back(*it);

						*(Color*)m_item->pointer = Color(a[0], a[1], a[2], a[3]);
					}
					else if (!type.compare(("vector<string>")))
					{
						auto ptr = static_cast<std::vector <std::string>*> (m_item->pointer);
						ptr->clear();

						json ja = json::parse(j[("value")].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							ptr->push_back(*it);
					}
				}
			}

			ifs.close();
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	void load_skins( std::string config )
	{
		static auto find_item = [ ]( std::vector< item* > items, std::string name ) -> item* {
			for ( int i = 0; i < ( int )items.size( ); i++ ) {
				if ( items[ i ]->name.compare( name ) == 0 )
					return (items[ i ]);
			}

			return nullptr;
		};

		static auto right_of_delim = [ ]( std::string const& str, std::string const& delim ) -> std::string {
			return str.substr( str.find( delim ) + delim.size( ) );
		};


		std::string folder, file;

		auto get_dir = [ &folder, &file, &config ]( ) ->void {
			static TCHAR path[ MAX_PATH ];
			if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
				folder = std::string( path ) + "\\uwucsgo\\skins\\";
				file = std::string( path ) + "\\uwucsgo\\skins\\" + config;
			}

			CreateDirectory( folder.c_str( ), NULL );
		};

		get_dir( );

		std::ifstream ifs;
		std::string data;

		std::string path = file + "";

		ifs.open( path );

		json allJson;

		ifs >> allJson;

		for ( json::iterator it = allJson.begin( ); it != allJson.end( ); ++it ) {

			json j = *it;

			std::string name = j[ "name" ];
			std::string type = j[ "type" ];

			item* m_item = find_item( skin_items, name );

			auto script_item = std::count_if( name.begin( ), name.end( ),
				[ ]( char& c )
			{
				return c == '.';
			}
			) >= 2;

			if ( m_item ) {
				if ( !type.compare( "int" ) ) {
					*( int* )m_item->pointer = j[ "value" ].get<int>( );
				}
				else if ( !type.compare( "float" ) ) {
					*( float* )m_item->pointer = j[ "value" ].get<float>( );
				}
				else if ( !type.compare( "bool" ) ) {
					*( bool* )m_item->pointer = j[ "value" ].get<bool>( );
				}
				else if ( !type.compare( "string" ) ) {
					*( std::string* )m_item->pointer = j[ "value" ].get<std::string>( );
				}
				else if ( !type.compare( "col_t" ) ) {
					std::vector<int> a;

					json ja = json::parse( j[ "value" ].get<std::string>( ).c_str( ) );

					for ( json::iterator it = ja.begin( ); it != ja.end( ); ++it )
						a.push_back( *it );

					*( Color* )m_item->pointer = Color( a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
				}
				else if ( !type.compare( ("vector<string>") ) )
				{
					auto ptr = static_cast< std::vector <std::string>* > (m_item->pointer);
					ptr->clear( );

					json ja = json::parse( j[ ("value") ].get<std::string>( ).c_str( ) );

					for ( json::iterator it = ja.begin( ); it != ja.end( ); ++it )
						ptr->push_back( *it );
				}
			}
		}

		ifs.close( );

	}

	std::vector<std::string> files;
	std::vector<std::string> skin_files;

	void skin_config_files( )
	{
		std::string folder;

		auto get_dir = [ &folder ]( ) -> void {
			static TCHAR path[ MAX_PATH ];
			if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
				folder = std::string( path ) + "\\uwucsgo\\skins\\";
			}

			CreateDirectory( folder.c_str( ), NULL );
		};

		get_dir( );

		skin_files.clear( );

		std::string path = folder + "/*.json";// "/*.*";

		WIN32_FIND_DATA fd;

		HANDLE hFind = ::FindFirstFile( path.c_str( ), &fd );

		if ( hFind != INVALID_HANDLE_VALUE ) {
			do {
				if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
					skin_files.push_back( fd.cFileName );
				}
			} while ( ::FindNextFile( hFind, &fd ) );

			::FindClose( hFind );
		}
	}

	void config_files()
	{
		std::string folder;

		auto get_dir = [&folder]() -> void {
			static TCHAR path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
				folder = std::string(path) + "\\uwucsgo\\";
			}

			CreateDirectory(folder.c_str(), NULL);
		};

		get_dir();

		files.clear();

		std::string path = folder + "/*.json";// "/*.*";

		WIN32_FIND_DATA fd;

		HANDLE hFind = ::FindFirstFile(path.c_str(), &fd);

		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					files.push_back(fd.cFileName);
				}
			} while (::FindNextFile(hFind, &fd));

			::FindClose(hFind);
		}
	}
};

enum VISUALS_POS
{
	AMMO_MINUS_NAME,
	AMMO_PLUS_NAME,
	MINUS_AMMO_NAME,
	NOTHING
};

struct item_setting
{
	char name[32];
	int stickers_place;
	int definition_vector_index;
	int definition_index;
	bool enabled_stickers;
	int paint_kit_vector_index;
	int paint_kit_index;
	int definition_override_vector_index;
	int definition_override_index;
	int seed;
	bool stat_trak;
	float wear;
	char custom_name[32];
	bool use;
};

struct statrack_setting
{
	int definition_index = 1;

	struct
	{
		int counter;
	}statrack_new;
};

struct CustomClrsForSkins
{
	bool ShouldUse0;
	bool ShouldUse1;
	bool ShouldUse2;
	bool ShouldUse3;
	Color First;
	float FirstHue;
	Color Second;
	float SecondHue;
	Color Third;
	float ThirdHue;
	Color Fourth;
	float FourthHue;
};

class ConfigList
{
public:

	struct
	{
		bool Active;
		bool Enemy;
		bool Local;

		struct
		{
			int Box;
			Color Box_Clr;
			float Box_hue;
			bool HP;
			bool HP_Percent;
			bool Ammo;
			bool Name;
			Color Ammo_Clr;
			float Ammo_hue;
			bool Weapon[2];
			int Material;
			int Illuminate;
			bool Visibility;
			bool OOF;
			Color OOF_Clr;
			float OOF_hue;
			float OOF_dist;
			float OOF_size;
			float Glow;

			bool Background[ 3 ];
			int BackgroundType;
			Color BackColor;
			float BackHue;
			Color BackLineColor;
			float BackLineHue;


		} vis_enemy;

		struct
		{
			int Material;
			int Illuminate;

			int Desync_Material;
			int Desync_Illuminate;

			float Glow;
		} vis_local;

		struct
		{
			Color enemy_chams; float enemy_chams_hue;
			Color enemy_chams_xqz; float enemy_chams_hue_xqz;
			Color enemy_illuminate; float enemy_illuminate_hue;

			Color local_chams; float local_chams_hue;
			Color local_illuminate; float local_illuminate_hue;
			Color desync_chams; float desync_chams_hue;
			Color desync_illuminate; float desync_illuminate_hue;

			Color enemy_glow; float enemy_glow_hue;
			Color local_glow; float local_glow_hue;

		} colors;

		struct
		{
			float fov_v;
			float view_v;

			float x_view;
			float y_view;
			float z_view;

			bool grenade_prediction;
			bool corners;

			Color nade_line;
			float nade_hue;

			Color corn_clr;
			float corn_hue;

			bool NadeEsp;
			Color NadeColor;
			float NadeEspHue;

			bool DimCrosshair;
			Color DimColor;
			float DimHue;
			float DimSize;

		} view;

		struct
		{
			bool recoil;
			bool shadows;
			bool scope;
			bool zoom;
			bool smoke;
			bool fog;
			bool post_process;
			bool flash;
			bool panorama_blue;
		} removals;

		struct
		{
			bool walls;
			bool props;
			bool sky;

			Color wall_clr; float wall_clr_hue;
			Color props_clr; float props_clr_hue;
			Color sky_clr; float sky_clr_hue;

			int sky_mode;
			bool full_bright;

			struct
			{
				bool rAmbient;
				float AmbientAmount;
				float Exposure;
				float Bloom;

			} modulate_light;

			struct
			{
				bool Enabled;
				Color FogClr;
				float FogHue;
				float Dist;
				float Density;

			} fog;

			bool ModulateFire;
			Color FireClr;
			float FireHue;

			struct
			{
				bool Enabled;
				float CSM_X;
				float CSM_Y;
			} shadow;

		} world;

		struct
		{
			bool name;
			bool ammo;
			bool box;

			Color name_clr; float name_clr_hue;
			Color ammo_clr; float ammo_clr_hue;
			Color box_clr; float box_clr_hue;

		} world_weapons;

		struct
		{

			bool thirdperson;
			float tp_dist;
			bool force_tp;
			int key_tp;
			bool grenade_tp;

			bool LogsOut[7];

			bool SpreadCross;
			int m_CrossType;
			Color Spread_Clr; float Spread_hue;

			bool spectator_list;
			bool keybind_list;
			bool watermark;

			int hitsound;
			bool force_aspect;
			float aspect_rat;
			bool smth_wrong;

			bool auto_peek;

			float peek_hue;
			float peek_second;

			int peek_key;
			bool fast_stop;

		} misc_;

	} visuals;

	struct
	{

		bool Acitve;
		bool Enable;

		float hitchance[8];
		float min_damage[8];
		float boost[8];
		
		bool AutoFire;
		bool SilentAim;

		float FieldOfView;
		int TargetSelection;
		
		bool Backtracking;
		bool Resolver;

		bool Hitboxes[8][8];
		int PriorityHitbox[8];

		float HeadScale[8];
		float BodyScale[8];

		bool AutoScope[8];
		bool AutoStop[8];
		
		bool ForceBaim;
		int BaimKey;

		bool ShotPriority[8];
		bool PreferBaim[8];
		bool LethalBaim[8];

		bool PreferSafepoint;
		int SafeKey;

		bool Prefers[8][5];
		bool PrefersBo[8][5];

	} ragebot;

	struct
	{
		bool Acitve;

		int Yaw;
		int Pitch;
		int BaseYaw;

		float YawAdd;
		float YawAddInvert;
		int KeyInvert;

		int FakeYaw;
		int JitterMode;
		float JitterValue;

		float FakeYawValue;
		float FakeYawValueInvert;

		int JiterSideFake;

	} antiaim;

	struct
	{
		bool ActMode;
		int mMode;
		int mExpltKey;
		int m_DoubleTapSpeed;
		bool ExpltCheck;
		float m_Tolerance;

	} exploit;

	struct
	{
		bool Activate;
		float Amount;
		int Mode;

	} fakelag;

	struct
	{
		bool bhop;
		int autostrafe;
		bool slowwalk;
		bool fakeduck;
		float slow_speed;
		int m_SlowKey;
		int m_FdKey;
		bool unlimitDuck;
	} movement;

	struct
	{

		bool Enable;
		Color Default_Clr; float Default_Clr_Hue;
		Color Frame_1_Clr; float Frame_1_Clr_Hue;
		Color Frame_2_Clr; float Frame_2_Clr_Hue;
		Color Frame_3_Clr; float Frame_3_Clr_Hue;
		Color Frame_4_Clr; float Frame_4_Clr_Hue;
		Color Frame_5_Clr; float Frame_5_Clr_Hue;
		Color Frame_6_Clr; float Frame_6_Clr_Hue;
		Color NameWater_Clr; float NameWater_Hue;
		Color TabText_Clr; float TabText_Hue;
		Color TabTextHovered_Clr; float TabTextHovered_Hue;

		Color ElementsText_Clr; float ElementsText_Hue;
		Color ElementsGradient_1_Clr; float ElementsGradient_1_Hue;
		Color ElementsGradient_2_Clr; float ElementsGradient_2_Hue;
		Color ElementsHovered_Clr; float ElementsHovered_Hue;
		Color ElementsOuterBox_Clr; float ElementsOuterBox_Hue;

		Color SliderGradient_1_Clr; float SliderGradient_1_Hue;
		Color SliderGradient_2_Clr; float SliderGradient_2_Hue;
		Color SliderVal_Clr; float SliderVal_Hue;

		Color GroupBoxOuter_Clr; float GroupBoxOuter_Hue;
		Color GroupBoxOuter_2_Clr; float GroupBoxOuter_2_Hue;
		Color GroupBoxText_Clr; float GroupBoxText_Hue;

		Color Selector_Clr; float Selector_Hue;
		Color SelectorOuter_Clr; float SelectorOuter_Hue;

		Color SelectorText_Clr; float SelectorText_Hue;
		Color SelectorSelectedText_Clr; float SelectorSelectedText_Hue;

		Color SelectorHovered_Clr; float SelectorHovered_Hue;
		Color SelectorTextSelected_Clr; float SelectorTextSelected_Hue;

		Color SubtabLine_Clr; float SubtabLine_Hue;
		Color SubtabLine_2_Clr; float SubtabLine_2_Hue;

		Color SubtabText_Clr; float SubtabText_Hue;
		Color SubtabTextUncheck_Clr; float SubtabTextUncheck_Hue;
		Color SubtabFilled_Clr; float SubtabFilled_Hue;

		Color ListBoxLine_Clr; float ListBoxLine_Hue;
		Color ListBoxLine_2_Clr; float ListBoxLine_2_Hue;

		Color ListText_Clr; float ListText_Hue;
		Color ListTextUnchecked_Clr; float ListTextUnchecked_Hue;

		Color ColorPickerFrame_1_Clr; float ColorPickerFrame_1_Hue;
		Color ColorPickerFrame_2_Clr; float ColorPickerFrame_2_Hue;
		Color ColorPickerFrame_3_Clr; float ColorPickerFrame_3_Hue;
		Color ColorPickerOutline_Clr; float ColorPickerOutline_Hue;

	} GUI_CLRS;

	struct
	{
		int m_CurrentSlot;
		
		std::string m_FontName;
		std::string m_FontSize;
		std::string m_FontWeight;

		bool m_FlagAA;
		bool m_FlagOutline;
		bool m_FlagDropShadow;

		DWORD m_Font[ 19 ];

		std::string m_VectorName[ 19 ];
		std::string m_VectorSize[ 19 ];
		std::string m_VectorWeight[ 19 ];

		bool m_VectorAA[ 19 ];
		bool m_VectorOutline[ 19 ];
		bool m_VectorDropShadow[ 19 ];

	} CustomFonts;

	struct
	{

		int NameFont;
		int WeaponFont;
		int HpNumberFont;
		int LogsFont;
		int WorldNameFont;

	} CustomVisualsFont;

	struct
	{
		float ScaleX;
		float ScaleY;

		float ScaleZ;
		float ScaleA;

		float SizeHealthBar;
		float LeftSideHealthBar;

		float OneLineFrame;
		float SecondLineFrame;

		float NamePosX;
		float NamePosY;

		float AmmoPos;

		float WeaponNamePos1;
		float WeaponNamePos2;

		float WeaponIcon[ 5 ];
		float HealthTextPos;

		float AmmoSize;

	} CustomVisals;

	bool active_legit;
	int awall_key;

	struct
	{
		bool enabled;
		bool flash_check;

		bool smoke_check;
		bool auto_pistol;

		struct
		{
			bool head;
			bool chest;
			bool hands;
			bool legs;
		} hitbox;

		float fov;
		float smooth;

		float shot_delay;
		float kill_delay;

		float silent_fov;
		int silent_mode;

		bool rcs;
		float rcs_x;
		float rcs_y;
		int rcs_mode;

		bool awall;
		int awall_mode;
		float awall_damage;
		bool vis;

	} m_legit[ 9 ];

	struct
	{
		bool enable_skins;
		

		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, CustomClrsForSkins> m_items_clr = { };
			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;

	struct
	{
		/*bool should_use[ 5 ][ 10011 ];
		Color colors_[ 5 ][ 10011 ];
		float hues[ 5 ][ 10011 ];*/

	} custom_color_for_skins;

	int MasterSwitch;

	int selected_config = 0;
	std::string new_config_name;

	int selected_config_at_skin = 0;
	std::string new_config_skin_name;

	std::vector<std::string> m_InitScripts;
};

extern ConfigManager g_ConfigMaster;
extern ConfigList g_Options;