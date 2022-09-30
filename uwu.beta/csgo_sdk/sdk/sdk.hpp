#pragma once

#define NOMINMAX
#include <Windows.h>

#include "Misc/Enums.hpp"
#include "Misc/vfunc.hpp"

#include "Math/VMatrix.hpp"
#include "Math/QAngle.hpp"
#include "Math/Vector.hpp"
#include "Misc/Studio.hpp"

#include "Interfaces/IAppSystem.hpp"
#include "Interfaces/IBaseClientDll.hpp"
#include "Interfaces/IClientEntity.hpp"
#include "Interfaces/IClientEntityList.hpp"
#include "Interfaces/IClientMode.hpp"
#include "Interfaces/IConVar.hpp"
#include "Interfaces/ICvar.hpp"
#include "Interfaces/IEngineTrace.hpp"
#include "Interfaces/IVEngineClient.hpp"
#include "Interfaces/IVDebugOverlay.hpp"
#include "Interfaces/ISurface.hpp"
#include "Interfaces/CInput.hpp"
#include "Interfaces/IVModelInfoClient.hpp"
#include "Interfaces/IVModelRender.hpp"
#include "Interfaces/IRenderView.hpp"
#include "Interfaces/IGameEventmanager.hpp"
#include "Interfaces/IMaterialSystem.hpp"
#include "Interfaces/IMoveHelper.hpp"
#include "Interfaces/IMDLCache.hpp"
#include "Interfaces/IPrediction.hpp"
#include "Interfaces/IPanel.hpp"
#include "Interfaces/IEngineSound.hpp"
#include "Interfaces/IViewRender.hpp"
#include "Interfaces/CClientState.hpp"
#include "Interfaces/IPhysics.hpp"
#include "Interfaces/IInputSystem.hpp"
#include "interfaces/IRefCounted.hpp"

#include "Misc/Convar.hpp"
#include "Misc/CUserCmd.hpp"
#include "Misc/glow_outline_effect.hpp"
#include "Misc/datamap.hpp"
#include "utils/xor.h"

#include "netvars.hpp"
#include <deque>
#include <mutex>
#include <shared_mutex>

struct IDirect3DDevice9;

namespace Interfaces
{
    void Initialize( );
    void Dump( );
}

inline IVEngineClient* g_EngineClient = nullptr;
inline IBaseClientDLL* g_CHLClient = nullptr;
inline IClientEntityList* g_EntityList = nullptr;
inline CGlobalVarsBase* g_GlobalVars = nullptr;
inline IEngineTrace* g_EngineTrace = nullptr;
inline ICvar* g_CVar = nullptr;
inline IPanel* g_VGuiPanel = nullptr;
inline IClientMode* g_ClientMode = nullptr;
inline IVDebugOverlay* g_DebugOverlay = nullptr;
inline ISurface* g_VGuiSurface = nullptr;
inline IEngineVGUI* g_EngineVGui = nullptr;
inline CInput* g_Input = nullptr;
inline IVModelInfoClient* g_MdlInfo = nullptr;
inline IVModelRender* g_MdlRender = nullptr;
inline IVRenderView* g_RenderView = nullptr;
inline IMaterialSystem* g_MatSystem = nullptr;
inline IGameEventManager2* g_GameEvents = nullptr;
inline IMoveHelper* g_MoveHelper = nullptr;
inline IMDLCache* g_MdlCache = nullptr;
inline IPrediction* g_Prediction = nullptr;
inline CGameMovement* g_GameMovement = nullptr;
inline IEngineSound* g_EngineSound = nullptr;
inline CGlowObjectManager* g_GlowObjManager = nullptr;
inline IViewRender* g_ViewRender = nullptr;
inline IDirect3DDevice9* g_D3DDevice9 = nullptr;
inline CClientState* g_ClientState = nullptr;
inline IPhysicsSurfaceProps* g_PhysSurface = nullptr;
inline IInputSystem* g_InputSystem = nullptr;
inline IWeaponSystem* g_WeaponSystem = nullptr;
inline IMemAlloc* g_Memory = nullptr;

template<typename... Args>
void ConMsg( const char* pMsg, Args... args )
{
    static auto import = (void(*)(const char*, ...))GetProcAddress( GetModuleHandleW( L"tier0.dll" ), "?ConMsg@@YAXPBDZZ" );
    return import( pMsg, args... );
}
template<typename... Args>
void ConColorMsg( const Color& clr, const char* pMsg, Args... args )
{
    static auto import = (void(*)(const Color&, const char*, ...))GetProcAddress( GetModuleHandleW( L"tier0.dll" ), "?ConColorMsg@@YAXABVColor@@PBDZZ" );
    return import( clr, pMsg, args... );
}

#include "Misc/EHandle.hpp"

class C_LocalPlayer
{
    friend bool operator==( const C_LocalPlayer& lhs, void* rhs );
public:
    C_LocalPlayer( ) : m_local( nullptr ) {}

    operator bool( ) const { return *m_local != nullptr; }
    operator C_BasePlayer* () const { return *m_local; }

    C_BasePlayer* operator->( ) { return *m_local; }

private:
    C_BasePlayer** m_local;
};

inline C_LocalPlayer g_LocalPlayer;
inline bool g_UpdateSkins;
inline bool g_InTrace = false;
inline CUserCmd* g_pCmd;

inline QAngle m_RealAngle;
inline QAngle m_FakeAngle;

inline bool g_CallLocalUpdate;
inline bool g_ForceBone;

inline int m_GoalShift;
inline int m_BackupedTick;
inline int m_BackupedPlayerTick;

inline int m_RateStored;
inline ClientFrameStage_t m_StoredStage;

inline QAngle m_ViewAngleStored;
inline int m_RageCurrentTargetIndex;

inline int m_MissedShot[ 65 ];
inline int m_MissedShotSpread[ 65 ];

inline bool m_OverrideVelocity;
inline float m_VelocityMod;

inline bool m_CallPlayerSetupBone;
inline std::string m_LastFontName;

struct CommandData
{
    int previous_command_number = 0;
    bool is_used = false;
    int cmd_number = 0;
    bool is_outgoing = false;
};

struct CorrectionData
{
    int tickcount = 0;
    int choked_commands = 0;
    int command_number = 0;
};

namespace CSGO
{
    inline std::deque<CorrectionData> m_Data;
    inline std::deque<CommandData> m_Packets;
    inline std::vector<int> m_ChokedNumber;
    inline int m_BackupedLocalTickbase;
    inline int m_TicksAllowed;
    inline bool m_CallShift;
}

namespace CTX
{
    inline bool m_SendPacket;
    inline std::mutex m_Mutex;
    inline int m_RedirectSkin;
}