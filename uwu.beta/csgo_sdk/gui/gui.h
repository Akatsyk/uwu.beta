#pragma once

#include "../render/directx render/font.h"
#include "../render/directx render/directx.h"
#include "../render/imgui render/im_main.h"

class Window;
namespace Gui {

	// Max of elements.
	inline constexpr int m_Maximum = 10000;

	namespace External
	{
		struct EDrawRect
		{
			EDrawRect( float posX, float posY, float m_Width, float h, Color m_Color )
			{
				this->posX = posX;
				this->posY = posY;
				this->m_Width = m_Width;
				this->m_Height = h;
				this->m_Color = m_Color;
			}

			float posX;
			float posY;
			float m_Width;
			float m_Height;

			Color m_Color;
		};

		struct EDrawText
		{
			EDrawText( float posX, float posY, std::string m_Text, Color m_Color )
			{
				this->posX = posX;
				this->posY = posY;
				this->m_Text = m_Text;
				this->m_Color = m_Color;
			}

			float posX;
			float posY;

			std::string m_Text;
			Color m_Color;
		};

		struct EDrawGradient
		{
			EDrawGradient( float posX, float posY, float m_Width, float h, Color m_Color, Color m_ColorNext )
			{
				this->posX = posX;
				this->posY = posY;
				this->m_Width = m_Width;
				this->m_Height = h;
				this->m_Color = m_Color;
				this->m_ColorNext = m_ColorNext;
			}

			float posX;
			float posY;
			float m_Width;
			float m_Height;

			Color m_Color;
			Color m_ColorNext;
		};
	}

	class Details {
	private:

		// Menu state.
		bool m_Show{ };

		// Our tabs.
		int m_Index{ };

	public:

		// Deconstructor.
		~Details( void ) = default;

		// Set show state.
		void SetMenuState( const bool m_State );

		// Get show state.
		bool GetMenuState( );

		// Setup all elements.
		void Install( );

		// Gui color
		Color m_DefaultColor = Color( 200, 45, 90, 255 );
	};

	class Input {
	private:

		// Mouse position.
		Vector2D m_MousePos{ };

		// Mouse wheel activity.
		float m_MouseWheel{ };

	public:

		// Init input ( mouse, ... ).
		void PollInput( );

		// Return mouse position.
		Vector2D GetMousePos( );

		// Set mouse position.
		void SetMousePos( const Vector2D m_Pos );

		// Return pressed key.
		bool KeyPressed( const uintptr_t m_Key );

		// Return whether the mouse is hovering over a specific object.
		bool MousePointer( const Vector2D m_Pos, const Vector2D m_Size );

		// Set mouse wheel.
		void SetMouseWheel( const float m_Mouse );

		// Get mouse wheel.
		float GetMouseWheel( );

		// Some additions.
		struct
		{

			bool m_KeyState[ 256 ]{ };
			bool m_PrevKeyState[ 256 ]{ };

		} Helpers;

	};

	class Control {
	private:

		// Elements index.
		int m_Index{ };

	public:

		// Controls type.
		enum ControlType
		{
			// Type: combo.
			COMBO,
			// Type: multi.
			MULTICOMBO,
			// Type: colorpicker.
			COLORSELECTOR,
			// Type: textinput.
			TEXTBOX,
			// Enum size.
			SIZE
		};

		// Return last element index.
		int GetIndex( );

		// Set last element index.
		void SetIndex( const uintptr_t m_Last );

		// Time since opened.
		int m_Opened[ ControlType::SIZE ];

		// Element opened.
		bool m_OpenedState[ ControlType::SIZE ][ m_Maximum ];

		// Can scroll or do animations.
		bool IsPossible( );

		// Is color picker opened.
		bool ColorPickerOpened;

// MAIN: 

		// Config add to list
		void AddConfig();

		// Load config from list
		void LoadConfig();

		// Save config from list
		void SaveConfig();

// SKIN:

		// Config add to list
		void AddConfigSkin( );

		// Load config from list
		void LoadConfigSkin( );

		// Save config from list
		void SaveConfigSkin( );

		// Lua table
		struct
		{
			int m_SelectedScript;
			std::vector <std::string> m_Scripts;
		} m_LuaExecute;
	};

	class Tabs
	{
	public:

		// Install tab: legit.
		void SetupRage( Window* m_Window, int m_Tab );

		// Install tab: legit.
		void SetupLegit( Window* m_Window, int m_Tab );

		// Install tab: visual.
		void SetupVisual( Window* m_Window, int m_Tab );

		// Install tab: skins.
		void SetupSkins( Window* m_Window, int m_Tab );

		// Install tab: misc.
		void SetupMisc(Window* m_Window, int m_Tab);

		// Install tab: config.
		void SetupConfig(Window* m_Window, int m_Tab);

		// Install tab: gui.
		void SetupGui(Window* m_Window, int m_Tab);

		// Install tab: lua.
		void SetupLua(Window* m_Window, int m_Tab);

		// Force full update.
		void UpdateSkins( );

		// Load from config r2u fonts;
		void LoadFontFromConfig();

		// Create custom font.
		void CreateFontFromParse();

		// Remove created font.
		void RemoveFont();

		// Setup custom colors for gui elements.
		void SetColors();

		// Reset colors for gui elements.
		void ResetColors();
	};

	class ExternalRendering {
	public:

		void Install( );

		std::vector< External::EDrawText > TextDraw;
		std::vector< External::EDrawRect > RectDraw;
		std::vector< External::EDrawRect > OutlineDraw;
		std::vector< External::EDrawGradient > GradientVDraw;
		std::vector< External::EDrawGradient > GradientHDraw;
	};

	extern Gui::Input m_Input;
	extern Gui::Details m_Details;
	extern Gui::Control m_Control;
	extern Gui::ExternalRendering m_External;
	extern Gui::Tabs m_Tabs;
}