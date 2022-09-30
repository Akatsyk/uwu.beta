#include "hooks.hpp"
#include <intrin.h>  
#include "detours.h"

#include "sdk/utils/input.hpp"
#include "sdk/utils/utils.hpp"

#include "gui/gui.h"
#include "render/engine/engine_draw.h"
#include "config/config.h"

#include "functions/visuals.h"
#include "functions/chams.h"
#include "functions/glow.h"

#include "functions/prediction.h"
#include "functions/skins/skins.h"

#include "functions/ragebot.h"
#include "sdk/utils/math.hpp"
#include "functions/lag_comp.h"

#include "functions/antiaim.h"
#include "functions/fakelag.h"
#include "functions/tickbase.h"
#include "functions/miss_counter.h"

#include "functions/events.h"
#include "functions/movement.h"
#include "functions/legitbot.h"

#include "functions/logs.h"
#include "lua/Clua.h"

#include "functions/p_cvar.h"
#include "render/imgui render/im_main.h"
#include "render/imgui render/im_font.h"
#include "sdk/mem_init.h"

#include "sdk/misc/UtlVector.hpp"
#include "sdk/utils/fnv.h"

C_HookedEvents HookedEvents;
#pragma intrinsic(_ReturnAddress)

#define MAX_COORD_FLOAT (16384.0f)
#define MIN_COORD_FLOAT (-MAX_COORD_FLOAT)

#ifndef PATTERN_ANALYSIS

struct ModuleInfo
{
	void* base;
	std::size_t size;
};

[[nodiscard]] static auto generateBadCharTable( std::string_view pattern ) noexcept
{
	assert( !pattern.empty( ) );

	std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

	auto lastWildcard = pattern.rfind( '?' );
	if ( lastWildcard == std::string_view::npos )
		lastWildcard = 0;

	const auto defaultShift = ( std::max )(std::size_t( 1 ), pattern.length( ) - 1 - lastWildcard);
	table.fill( defaultShift );

	for ( auto i = lastWildcard; i < pattern.length( ) - 1; ++i )
		table[ static_cast< std::uint8_t >(pattern[ i ]) ] = pattern.length( ) - 1 - i;

	return table;
}

LPVOID zGetInterface( HMODULE hModule, const char* InterfaceName )
{
	typedef void* (*CreateInterfaceFn)(const char*, int*);
	return reinterpret_cast< void* >(reinterpret_cast< CreateInterfaceFn >(GetProcAddress( hModule, ("CreateInterface") ))(InterfaceName, NULL));
}

static ModuleInfo getModuleInformation( const char* name ) noexcept
{
	if ( HMODULE handle = GetModuleHandleA( name ) )
	{
		if ( MODULEINFO moduleInfo; GetModuleInformation( GetCurrentProcess( ), handle, &moduleInfo, sizeof( moduleInfo ) ) )
			return ModuleInfo{ moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage };
	}
	return { };
}

template <bool ReportNotFound = true>
static std::uintptr_t findPattern31( ModuleInfo moduleInfo, std::string_view pattern ) noexcept
{
	static auto id = 0;
	++id;

	if ( moduleInfo.base && moduleInfo.size )
	{
		const auto lastIdx = pattern.length( ) - 1;
		const auto badCharTable = generateBadCharTable( pattern );

		auto start = static_cast< const char* >(moduleInfo.base);
		const auto end = start + moduleInfo.size - pattern.length( );

		while ( start <= end ) {
			int i = lastIdx;
			while ( i >= 0 && (pattern[ i ] == '?' || start[ i ] == pattern[ i ]) )
				--i;

			if ( i < 0 )
				return reinterpret_cast< std::uintptr_t >(start);

			start += badCharTable[ static_cast< std::uint8_t >(start[ lastIdx ]) ];
		}
	}

	assert( false );
#ifdef _WIN32
	if constexpr ( ReportNotFound )
		MessageBoxA( nullptr, ("Failed to find pattern #" + std::to_string( id ) + '!').c_str( ), "Osiris", MB_OK | MB_ICONWARNING );
#endif
	return 0;
}

template <bool ReportNotFound = true>
static std::uintptr_t findPattern( const char* moduleName, std::string_view pattern ) noexcept
{
	return findPattern31<ReportNotFound>( getModuleInformation( moduleName ), pattern );
}

#endif

class CCStudio
{
public:
	studiohdr_t* m_studio_hdr;
	virtualmodel_t* m_virtual_model;
	CUtlVector< const studiohdr_t* > m_studio_hdr_cache;
	int					m_frame_unlock_counter;
	int* m_p_frame_unlock_counter_ptr;
	char pad_0x0024[ 0x8 ];
	CUtlVector< int >	m_bone_flags;
	CUtlVector< int >	m_bone_parent;
	inline int			numbones( void ) const { return m_studio_hdr->numbones; };
	inline mstudiobone_t* bone( int i ) const { return m_studio_hdr->GetBone( i ); };
};

struct RenderInfo_T {
	IClientRenderable* renderable;
	void* alpha_property;
	int enum_count;
	int render_frame;
	unsigned short first_shadow;
	unsigned short leaf_list;
	short area;
	std::uint16_t flags;
	std::uint16_t flags2;
	Vector bloated_abs_mins;
	Vector bloated_abs_maxs;
	Vector abs_mins;
	Vector abs_maxs;
	int pad;
};

class CCStudioHDR
{
public:
	studiohdr_t* m_studio_hdr;
	virtualmodel_t* m_virtual_model;
	CUtlVector< const studiohdr_t* > m_studio_hdr_cache;
	int					m_frame_unlock_counter;
	int* m_p_frame_unlock_counter_ptr;
	char pad_0x0024[ 0x8 ];
	CUtlVector< int >	m_bone_flags;
	CUtlVector< int >	m_bone_parent;
	inline int			numbones( void ) const { return m_studio_hdr->numbones; };
	inline mstudiobone_t* bone( int i ) const { return m_studio_hdr->GetBone( i ); };
};

class CParticleCollection;
class C_INIT_RandomColor {
	BYTE pad_0[ 92 ];
public:
	Vector m_flNormColorMin;
	Vector m_flNormColorMax;
};

Hooks::GetClrFN Hooks::GetClrOriginal;
Hooks::ModifyEyePosFn Hooks::OriginalModEyePos;
Hooks::CalculateViewFn Hooks::OriginalCalcView;

void AntiCheatSimpleFix()
{
	const char* modules[ ] = { "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long sub_21445 = 0x69690004C201B0;
	for ( auto test : modules )
		WriteProcessMemory( GetCurrentProcess(), ( LPVOID )Utils::PatternScan( GetModuleHandleA( test ), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08" ), &sub_21445, 7, 0 );
}

Hooks::UpdateAnimation_t Hooks::originAnimation;

using SetBoundsFn = void( __thiscall* )(ICollideable*, Vector*, Vector*);
SetBoundsFn OriginalSetBounds;

using StandardBlendingRulesFn = void( __thiscall* )(C_BasePlayer*, CStudioHdr*, Vector*, Quaternion*, float, int);
DWORD OriginalBlend;

using PhysicsSimulateFn = void( __fastcall* )(C_BasePlayer*);
DWORD OriginalPhycics;

using GetForeignFallbackFontNameFn = const char* (__thiscall*)(void*);
DWORD OriginalFallBackFont;

using NewParticeFN = void( __thiscall* )(C_INIT_RandomColor*, CParticleCollection*, int, int, int, void*);
NewParticeFN OriginalPartice;

float m_StoredForwardMove = 0.f;
float m_StoredSideMove = 0.f;

bool m_CallPacketStart = false;
bool m_CallNeed = false;

bool m_BreakLC = false;

bool IsRage()
{
	if ( g_Options.MasterSwitch == 0 && g_Options.ragebot.Acitve && g_Options.ragebot.Enable )
		return true;

	return false;
}

std::uintptr_t newFunctionClientDLL;
std::uintptr_t newFunctionEngineDLL;
std::uintptr_t newFunctionStudioRenderDLL;
std::uintptr_t newFunctionMaterialSystemDLL;

void SimplestFix( )
{
	newFunctionClientDLL = findPattern( "client", "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08" );
	newFunctionEngineDLL = findPattern( "engine", "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08" );
	newFunctionStudioRenderDLL = findPattern( "studiorender", "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08" );
	newFunctionMaterialSystemDLL = findPattern( "materialsystem", "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08" );
}

DWORD newFunctionClientDLL_hook;
DWORD newFunctionEngineDLL_hook;
DWORD newFunctionStudioRenderDLL_hook;
DWORD newFunctionMaterialSystemDLL_hook;

static char __fastcall newFunctionClientBypass( void* thisPointer, void* edx, const char* moduleName ) noexcept
{
	return 1;
}

static char __fastcall newFunctionEngineBypass( void* thisPointer, void* edx, const char* moduleName ) noexcept
{
	return 1;
}

static char __fastcall newFunctionStudioRenderBypass( void* thisPointer, void* edx, const char* moduleName ) noexcept
{
	return 1;
}

static char __fastcall newFunctionMaterialSystemBypass( void* thisPointer, void* edx, const char* moduleName ) noexcept
{
	return 1;
}

namespace Hooks
{
	IMaterial* __fastcall hkFindMaterial( void* _this, void* ebp, const char* pMaterialName, const char* pTextureGroupName, bool complain, const char* pComplainPrefix )
	{
		static auto ofunc = Hooks::mat_sys_hook.get_original<decltype(&hkFindMaterial) >( 84 );

		if ( strstr( pMaterialName, "engine" ) )
			return ofunc( _this, ebp, pMaterialName, pTextureGroupName, complain, pComplainPrefix );

		IMaterial* mat = ofunc( g_MatSystem, ebp, pMaterialName, pTextureGroupName, complain, pComplainPrefix );
		static IMaterial* crystalMat = ofunc( g_MatSystem, ebp, "debug/debugambientcube", nullptr, true, NULL );
		
		if ( mat->IsErrorMaterial( ) || !mat || pTextureGroupName == nullptr || pMaterialName == nullptr ) {
			return mat;
		}

		else {

			if ( !strcmp( pTextureGroupName, "World textures" )
				|| !strcmp( pTextureGroupName, "StaticProp textures" ) ) {

				if ( crystalMat ) {

					return crystalMat;
				}
				else {
					return mat;
				}
			}
			else {
				return mat;
			}
		}

		return nullptr;
	}

	void __fastcall ResetLatched( void* ecx, void* edx )
	{
		return;
	}

	void __fastcall SetCollisionBounds( ICollideable* ecx, Vector* min, Vector* max )
	{
		/*
		
		ex:

		Vector m_LimitMin = Vector( 0, 0, 0 );
		Vector m_LimitMax = Vector( 15, 15, 15 );

		auto m_Player = ecx - 0x4;

		if ( m_Player == g_LocalPlayer )
		{
			return OriginalSetBounds( ecx, m_LimitMin, m_LimitMax );
		}
		else
			return OriginalSetBounds( ecx, min, max );

		*/

		return OriginalSetBounds( ecx, min, max );
	}

	void __fastcall CheckFileCRC( void* ecx, void* edx )
	{

	}

	bool __fastcall LooseFileAllow( void* ecx, void* edx )
	{
		return true;
	}

	void __fastcall InitNewParticlesScalar( C_INIT_RandomColor* thisPtr, void* edx, CParticleCollection* pParticles, int start_p, int nParticleCount, int nAttributeWriteMask, void* pContext ) {
		Vector o_min = thisPtr->m_flNormColorMin;
		Vector o_max = thisPtr->m_flNormColorMax;

		const char* mat_name = *( char** )(*( uintptr_t* )(( uintptr_t )pParticles + 0x48) + 0x40);
		assert( mat_name );

		std::vector<Color> m_ReadyColors =
		{
			Color( 0, 0, 0 ),
			Color( 255, 0, 0 ),
			Color( 0, 255, 0 ),
			Color( 0, 0, 255 ),
			Color( 255, 255, 0 ),
			Color( 255, 0, 255 ),
			Color( 0, 0, 255 ),
			Color( 255, 255, 255 )
		};

		if ( g_Options.visuals.Active && g_Options.visuals.world.ModulateFire )
		{
			switch ( FNV::Runtime( mat_name ) )
			{
			case FNV::Hash( "particle\\fire_burning_character\\fire_env_fire.vmt" ):
			case FNV::Hash( "particle\\fire_burning_character\\fire_env_fire_depthblend.vmt" ):
			case FNV::Hash( "particle\\fire_burning_character\\fire_burning_character_depthblend.vmt" ):
			case FNV::Hash( "particle\\fire_burning_character\\fire_burning_character.vmt" ):
			case FNV::Hash( "particle\\fire_burning_character\\fire_burning_character_nodepth.vmt" ):
			case FNV::Hash( "particle\\particle_flares\\particle_flare_001.vmt" ):
			case FNV::Hash( "particle\\particle_flares\\particle_flare_004.vmt" ):
			case FNV::Hash( "particle\\particle_flares\\particle_flare_004b_mod_ob.vmt" ):
			case FNV::Hash( "particle\\particle_flares\\particle_flare_004b_mod_z.vmt" ):
			case FNV::Hash( "particle\\fire_explosion_1\\fire_explosion_1_bright.vmt" ):
			case FNV::Hash( "particle\\fire_explosion_1\\fire_explosion_1b.vmt" ):
			case FNV::Hash( "particle\\fire_particle_4\\fire_particle_4.vmt" ):
			case FNV::Hash( "particle\\fire_explosion_1\\fire_explosion_1_oriented.vmt" ):
				thisPtr->m_flNormColorMin = thisPtr->m_flNormColorMax 
					= Vector( m_ReadyColors.at( ( int )g_Options.visuals.world.FireHue ).r( ) / 255, 
						m_ReadyColors.at( ( int )g_Options.visuals.world.FireHue ).g( ) / 255,
						m_ReadyColors.at( ( int )g_Options.visuals.world.FireHue ).b( ) / 255 );
				break;
			}
		}

		OriginalPartice( thisPtr, pParticles, start_p, nParticleCount, nAttributeWriteMask, pContext );

		thisPtr->m_flNormColorMin = o_min;
		thisPtr->m_flNormColorMax = o_max;
	}

	using ProtoPlayer = QAngle * (__thiscall*)(void*);
	void* pBuildTransformationsEntity;
	QAngle vecBuildTransformationsAngles;

	QAngle* __fastcall hkGetEyeAngles( void* ecx, void* edx )
	{
		static int* WantedReturnAddress1 = ( int* )Utils::FindSig( "client.dll", "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5" );
		static int* WantedReturnAddress2 = ( int* )Utils::FindSig( "client.dll", "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?" );
		static int* WantedReturnAddress3 = ( int* )Utils::FindSig( "client.dll", "F3 0F 10 55 ? 51 8B 8E ? ? ? ?" );

		static auto oGetEyeAngles = Hooks::player_hook.get_original <ProtoPlayer>( 170 );

		if ( _ReturnAddress( ) == ( void* )WantedReturnAddress1 || _ReturnAddress( ) == ( void* )WantedReturnAddress2 || _ReturnAddress( ) == ( void* )WantedReturnAddress3 )
			return oGetEyeAngles( ecx );

		if ( !ecx || pBuildTransformationsEntity != ecx )
			return oGetEyeAngles( ecx );

		pBuildTransformationsEntity = nullptr;
		return &vecBuildTransformationsAngles;
	}

	using BuildTransformations = Vector * (__thiscall*)(void*, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, byte* computed);
	void __fastcall hkBuildTransformations( void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, byte* computed )
	{
		static auto oBuildTransformations = Hooks::player_hook.get_original <BuildTransformations>( 190 );

		static const auto m_Jiggle = g_CVar->FindVar( "r_jiggle_bones" );
		m_Jiggle->SetValue( 0 );

		if ( ecx && (( C_BasePlayer* )ecx)->EntIndex( ) == g_EngineClient->GetLocalPlayer( ) )
		{
			pBuildTransformationsEntity = ecx;
			vecBuildTransformationsAngles = (( C_BasePlayer* )ecx)->GetRenderAngles( );
		}

		oBuildTransformations( ecx, hdr, pos, q, cameraTransform, bonemask, computed );
		pBuildTransformationsEntity = nullptr;
	}

	void __fastcall hkModifyEyePos( void* ecx, void* edx, Vector& input_eye_pos )
	{
		if ( !ecx )
			return OriginalModEyePos( ecx, edx, input_eye_pos );

		const auto anim_state = static_cast< CAnimStateFull* >(ecx);

		if ( !anim_state )
			return OriginalModEyePos( ecx, edx, input_eye_pos );

		if ( !anim_state->m_landing && anim_state->m_anim_duck_amount == 0.0f )
		{
			anim_state->m_smooth_height_valid = false;
			anim_state->m_camera_smooth_height = 0x7F7FFFFF;
			return;
		}

		using bone_lookup_fn = int( __thiscall* )(void*, const char*);
		static auto lookup_bone = reinterpret_cast< bone_lookup_fn >(Utils::FindSig( "client.dll", "55 8B EC 53 56 8B F1 57 83 BE ? ? ? ? ? 75 14 8B 46 04 8D 4E 04 FF 50 20 85 C0 74 07 8B CE E8 ? ? ? ? 8B 8E ? ? ? ? 85 C9 0F 84" ));

		auto head_pos = anim_state->m_player->m_CachedBoneData( )[ lookup_bone( anim_state->m_player, "head_0" ) ].at( 3 );
		head_pos.z += 1.7f;

		if ( input_eye_pos.z > head_pos.z )
		{
			const auto lol = abs( input_eye_pos.z - head_pos.z );
			const auto v22 = (lol - 4.0) * 0.16666667;
			float v23;

			if ( v22 >= 0.0 )
				v23 = fminf( v22, 1.0 );
			else
				v23 = 0.0;

			input_eye_pos.z = (head_pos.z - input_eye_pos.z) * (v23 * v23 * 3.0 - v23 * v23 * 2.0 * v23) +
				input_eye_pos.z;
		}
	}

	void __fastcall hkCalcView( void* ecx, void* edx, Vector& eye_origin, QAngle& eye_angles, float& z_near, float& z_far, float& fov )
	{
		const auto player = static_cast< C_BasePlayer* >(ecx);

		if ( !player || !g_LocalPlayer || player != g_LocalPlayer )
			return OriginalCalcView( ecx, edx, eye_origin, eye_angles, z_near, z_far, fov );

		const auto backup_use_new_anim_state = player->ShouldUseNewAnimState( );

		player->ShouldUseNewAnimState( ) = false;
		{
			OriginalCalcView( ecx, edx, eye_origin, eye_angles, z_near, z_far, fov );
		}
		player->ShouldUseNewAnimState( ) = backup_use_new_anim_state;
	}

	using ClipRayCollideable_t = void( __thiscall* )(void*, const Ray_t&, uint32_t, ICollideable*, CGameTrace*);
	void __fastcall ClipRay( void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace )
	{
		static auto original_fn = Hooks::traced_hook.get_original <ClipRayCollideable_t>( 4 );

		auto backup_maxs = pCollide->OBBMaxs().z;
		pCollide->OBBMaxs().z += 5.0f;

		original_fn( ecx, ray, fMask, pCollide, pTrace );
		pCollide->OBBMaxs().z = backup_maxs;
	}

	using TraceRay_t = void( __thiscall* )(void*, const Ray_t&, unsigned int, ITraceFilter*, trace_t*);
	void __fastcall HookTace( void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace )
	{
		static auto original_fn = Hooks::traced_hook.get_original <TraceRay_t>( 5 );

		if ( !IsRage() || !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return original_fn( ecx, ray, fMask, pTraceFilter, pTrace );

		if ( !g_LocalPlayer->IsAlive() )
			return original_fn( ecx, ray, fMask, pTraceFilter, pTrace );

		if ( !g_InTrace )
			return original_fn( ecx, ray, fMask, pTraceFilter, pTrace );

		original_fn( ecx, ray, fMask, pTraceFilter, pTrace );
		pTrace->surface.flags |= SURF_SKY;
	}

	float __fastcall AspectRatio( void* pEcx, void* pEdx, int32_t iWidth, int32_t iHeight )
	{
		static auto ofunc = Hooks::engine_hook.get_original<decltype(&AspectRatio)>( 101 );

		if ( !g_Options.visuals.misc_.force_aspect )
			return ofunc( pEcx, pEdx, iWidth, iHeight );

		else
			return (( float )g_Options.visuals.misc_.aspect_rat / 10);

		ofunc( pEcx, pEdx, iWidth, iHeight );
	}

	_declspec(noinline)const char* FontFallBackDetour( void* ecx, uint32_t i )
	{
		if ( m_LastFontName.empty() )
			return (( GetForeignFallbackFontNameFn )OriginalFallBackFont)(ecx);

		return m_LastFontName.c_str();
	}

	const char* __fastcall FontFallBack( void* ecx, uint32_t i )
	{
		return FontFallBackDetour( ecx, i );
	}

	using PacketStart_t = void( __thiscall* )(void*, int, int);

	void __fastcall PacketStart( void* ecx, void* edx, int incoming, int outgoing )
	{
		static auto original_fn = Hooks::clientstate_hook.get_original<PacketStart_t>( 5 );

		if ( !IsRage() || !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return original_fn( ecx, incoming, outgoing );

		if ( !g_LocalPlayer->IsAlive() )
			return original_fn( ecx, incoming, outgoing );

		if ( CSGO::m_Packets.empty() )
			return original_fn( ecx, incoming, outgoing );

		for ( auto it = CSGO::m_Packets.rbegin(); it != CSGO::m_Packets.rend(); ++it )
		{
			if ( !it->is_outgoing )
				continue;

			if ( it->cmd_number == outgoing || outgoing > it->cmd_number && (!it->is_used || it->previous_command_number == outgoing) )
			{
				it->previous_command_number = outgoing;
				it->is_used = true;
				original_fn( ecx, incoming, outgoing );
				break;
			}
		}

		auto result = false;

		for ( auto it = CSGO::m_Packets.begin(); it != CSGO::m_Packets.end();)
		{
			if ( outgoing == it->cmd_number || outgoing == it->previous_command_number )
				result = true;

			if ( outgoing > it->cmd_number && outgoing > it->previous_command_number )
				it = CSGO::m_Packets.erase( it );
			else
				++it;
		}

		if ( !result )
			original_fn( ecx, incoming, outgoing );
	}

	using PacketEnd_t = void( __thiscall* )(void*);

	void __fastcall PacketEnd( void* ecx, void* edx )
	{
		static auto original_fn = Hooks::clientstate_hook.get_original <PacketEnd_t>( 6 );
		static float m_OldVelMod = FLT_MAX;

		if ( !IsRage() || !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return original_fn( ecx );

		if ( !g_LocalPlayer->IsAlive() )
		{
			CSGO::m_Data.clear();
			return original_fn( ecx );
		}

		if ( *( int* )(( uintptr_t )ecx + 0x164) == *( int* )(( uintptr_t )ecx + 0x16C) )
		{
			auto ack_cmd = *( int* )(( uintptr_t )ecx + 0x4D2C);
			auto correct = std::find_if( CSGO::m_Data.begin(), CSGO::m_Data.end(),
				[ &ack_cmd ]( const CorrectionData& other_data )
				{
					return other_data.command_number == ack_cmd;
				}
			);

			auto netchannel = g_EngineClient->GetNetChannelInfo();

			if ( netchannel && correct != CSGO::m_Data.end() )
			{
				if ( m_OldVelMod > g_LocalPlayer->m_bVelMod() + 0.1f )
				{
					auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

					if ( !weapon || weapon->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->IsGrenadeIDX() )
					{
						for ( auto& number : CSGO::m_ChokedNumber )
						{
							auto cmd = &g_Input->m_pCommands[ number % MULTIPLAYER_BACKUP ];
							auto verified = &g_Input->m_pVerifiedCommands[ number % MULTIPLAYER_BACKUP ];

							if ( cmd->buttons & (IN_ATTACK | IN_ATTACK2) )
							{
								cmd->buttons &= ~IN_ATTACK;

								verified->m_cmd = *cmd;
								verified->m_crc = cmd->GetChecksum();
							}
						}
					}
				}

				m_OldVelMod = g_LocalPlayer->m_bVelMod();
			}
		}

		return original_fn( ecx );
	}

	_declspec(noinline)void PhysicSimDetour( C_BasePlayer* player )
	{
		if ( !IsRage() || !player || !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return (( PhysicsSimulateFn )OriginalPhycics)(player);

		auto simulation_tick = *( int* )(( uintptr_t )player + 0x2AC);

		if ( player != g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_GlobalVars->tickcount == simulation_tick )
		{
			(( PhysicsSimulateFn )OriginalPhycics)(player);
			return;
		}

		Engine_Prediction::Get().restore_netvars();
		(( PhysicsSimulateFn )OriginalPhycics)(player);
		Engine_Prediction::Get().store_netvars();
	}

	void __fastcall PhysicSim( C_BasePlayer* player )
	{
		return PhysicSimDetour( player );
	}

	_declspec(noinline)void StandartBlendRule( C_BasePlayer* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask )
	{
		if ( !player || !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return (( StandardBlendingRulesFn )OriginalBlend)(player, hdr, pos, q, curtime, boneMask);

		if ( !g_LocalPlayer->IsAlive() )
			return (( StandardBlendingRulesFn )OriginalBlend)(player, hdr, pos, q, curtime, boneMask);

		auto backup_effects = player->m_fEffects();

		if ( player == g_LocalPlayer )
			player->m_fEffects() |= 8;

		(( StandardBlendingRulesFn )OriginalBlend)(player, hdr, pos, q, curtime, boneMask);

		if ( player == g_LocalPlayer )
			player->m_fEffects() = backup_effects;
	}

	void __fastcall StandartBlendRuleMain( C_BasePlayer* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask )
	{
		return StandartBlendRule( player, i, hdr, pos, q, curtime, boneMask );
	}

	bool __fastcall DrawFog( void* ecx, void* edx )
	{
		return !g_Options.visuals.removals.fog;
	}

	int __fastcall ListLeavesInBox( std::uintptr_t ecx, std::uintptr_t edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max )
	{
		static auto original_fn = bsp_hook.get_original< decltype(&ListLeavesInBox) >( 6 );
		static auto insert_into_tree_call_list_leaves_in_box = Utils::FindSig( "client.dll", "89 44 24 14 EB 08 C7 44 24 ? ? ? ? ? 8B 45" );

		if ( !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return original_fn( ecx, edx, mins, maxs, list, list_max );

		if ( reinterpret_cast< std::uintptr_t >(_ReturnAddress()) != insert_into_tree_call_list_leaves_in_box )
			return original_fn( ecx, edx, mins, maxs, list, list_max );

		auto info = *reinterpret_cast< RenderInfo_T** >(reinterpret_cast< std::uintptr_t >(_AddressOfReturnAddress()) + 0x14);

		if ( !info || !info->renderable )
			return original_fn( ecx, edx, mins, maxs, list, list_max );

		auto base_entity = info->renderable->GetIClientUnknown()->GetBaseEntity();

		if ( !base_entity || !base_entity->IsPlayer() )
			return original_fn( ecx, edx, mins, maxs, list, list_max );

		info->flags &= ~0x100;
		info->flags2 |= 0xC0;

		static Vector map_min = Vector( MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT );
		static Vector map_max = Vector( MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT );

		return original_fn( ecx, edx, map_min, map_max, list, list_max );
	}

	void __fastcall GetColorModulation( void* ecx, void* edx, float* r, float* g, float* b )
	{
		GetClrOriginal( ecx, r, g, b );

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return;

		if ( !g_LocalPlayer )
			return;

		const auto material = reinterpret_cast< IMaterial* >(ecx);
		auto group = material->GetTextureGroupName();

		bool is_prop = strstr( group, "StaticProp" );
		bool is_wall = strstr( group, "World textures" );
		bool is_sky = strstr( group, "SkyBox" );

		if ( g_Options.visuals.Active )
		{
			if ( is_prop && g_Options.visuals.world.props )
			{
				*r *= g_Options.visuals.world.props_clr.r() / 255.f;
				*g *= g_Options.visuals.world.props_clr.g() / 255.f;
				*b *= g_Options.visuals.world.props_clr.b() / 255.f;

				*( float* )(( DWORD )material->GetShaderParams()[ 5 ] + 0xC) = g_Options.visuals.world.props_clr.a() / 255.f;
			}
			else if ( is_wall && g_Options.visuals.world.walls )
			{
				*r *= g_Options.visuals.world.wall_clr.r() / 255.f;
				*g *= g_Options.visuals.world.wall_clr.g() / 255.f;
				*b *= g_Options.visuals.world.wall_clr.b() / 255.f;
			}
			else if ( is_sky && g_Options.visuals.world.sky )
			{
				*r *= g_Options.visuals.world.sky_clr.r() / 255.f;
				*g *= g_Options.visuals.world.sky_clr.g() / 255.f;
				*b *= g_Options.visuals.world.sky_clr.b() / 255.f;
			}
		}
	}

	bool __stdcall IsUsingProps()
	{
		return g_Options.visuals.Active && g_Options.visuals.world.props;
	}

	bool __fastcall SetupBonesFN( IClientRenderable* ecx, void* edx, matrix3x4_t* bones, int max_bones, int mask, float time )
	{
		typedef bool( __thiscall* T )(IClientRenderable*, matrix3x4_t*, int, int, float);

		static const auto original = m_renderable_hook.get_original<T>( 13 );
		auto pEnt = reinterpret_cast< C_BasePlayer* >(uintptr_t( ecx ) - 0x4);

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return original( ecx, bones, max_bones, mask, time );

		if ( g_UpdateSkins || !g_LocalPlayer || !pEnt || !bones )
			return original( ecx, bones, max_bones, mask, time );

		bool result = original( ecx, bones, max_bones, mask, time );

		if ( pEnt->IsPlayer() && pEnt->IsAlive() )
		{
			if ( pEnt->EntIndex() == g_EngineClient->GetLocalPlayer() )
			{
				auto animstate = pEnt->GetPlayerAnimState();
				auto previous_weapon = animstate ? animstate->pLastActiveWeapon : nullptr;

				if ( animstate && previous_weapon )
					animstate->pLastActiveWeapon = animstate->pActiveWeapon;

				auto& last_animation_framecount = *reinterpret_cast< int* > (uintptr_t( pEnt ) + 0xA68);
				last_animation_framecount = 0;

				if ( g_ForceBone )
					result = original( ecx, bones, max_bones, mask, time );

				else if ( !pEnt->m_CachedBoneData().Count() )
					result = original( ecx, bones, max_bones, mask, time );

				else if ( bones && max_bones != -1 )
				{
					memcpy( bones, pEnt->m_CachedBoneData().Base(), pEnt->m_CachedBoneData().Count() * sizeof( matrix3x4_t ) );
				}

				if ( g_ForceBone ) {
					static const auto offset = 0x268C;
					*reinterpret_cast< int* > (reinterpret_cast< uintptr_t > (pEnt) + offset) = 0;
				}
			}
			else if ( IsRage() && pEnt->m_iTeamNum() != g_LocalPlayer->m_iTeamNum() )
			{
				if ( m_CallPlayerSetupBone )
					result = original( ecx, bones, max_bones, mask, time );

				else if ( !pEnt->m_CachedBoneData().Count() )
					result = original( ecx, bones, max_bones, mask, time );

				else if ( bones && max_bones != -1 )
				{
					memcpy( bones, pEnt->m_CachedBoneData().Base(), pEnt->m_CachedBoneData().Count() * sizeof( matrix3x4_t ) );
				}
			}
		}

		return result;
	}

	recv_prop_hook* sequence_hook;
	void RecvProxy( const CRecvProxyData* pData, void* entity, void* output )
	{
		static auto ofunc = sequence_hook->get_original_function();

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return ofunc( pData, entity, output );

		if ( g_LocalPlayer && g_LocalPlayer->IsAlive() ) {
			const auto proxy_data = const_cast< CRecvProxyData* >(pData);
			const auto view_model = static_cast< C_BaseViewModel* >(entity);

			if ( view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid() ) {
				const auto owner = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntityFromHandle( view_model->m_hOwner() ));
				if ( owner == g_EntityList->GetClientEntity( g_EngineClient->GetLocalPlayer() ) ) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if ( view_model_weapon_handle.IsValid() ) {
						const auto view_model_weapon = static_cast< C_BaseAttributableItem* >(g_EntityList->GetClientEntityFromHandle( view_model_weapon_handle ));
						if ( view_model_weapon ) {
							if ( k_weapon_info.count( view_model_weapon->m_Item().m_iItemDefinitionIndex() ) ) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at( view_model_weapon->m_Item().m_iItemDefinitionIndex() ).model;
								proxy_data->m_Value.m_Int = skins::GetNewAnimation( override_model, proxy_data->m_Value.m_Int );
							}
						}
					}
				}
			}

		}

		ofunc( pData, entity, output );
	}

	bool __fastcall SkipFrame()
	{
		return false;
	}

	void __fastcall DoExtraBone( C_BasePlayer* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context )
	{
		return;
	}

	using ProcessInterpolatedListFn = int(*)(void);
	DWORD original_processinterpolatedlist;
	int ProcessInterp()
	{
		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return (( ProcessInterpolatedListFn )original_processinterpolatedlist)();

		if ( !g_LocalPlayer )
			return (( ProcessInterpolatedListFn )original_processinterpolatedlist)();

		static auto Allow = *( bool** )(Utils::FindSig( ("client.dll"), ("A2 ? ? ? ? 8B 45 E8") ) + 0x1);

		if ( Allow )
			*Allow = false;

		return (( ProcessInterpolatedListFn )original_processinterpolatedlist)();
	}

	typedef bool( __thiscall* WriteUsercmdDeltaToBufferFn ) (void*, int, void*, int, int, bool);

	bool ShiftCmd( int* new_commands, int* backup_commands, void* ecx, int slot, bf_write* buf, int unk, bool real_cmd ) {
		static auto original_fn = Hooks::hlclient_hook.get_original< WriteUsercmdDeltaToBufferFn >( 24 );

		auto new_from = -1;
		auto shift_amount = TickBaseSys::Get().ValueToShift;
		TickBaseSys::Get().ValueToShift = 0;

		auto commands = *new_commands;
		auto shift_commands = std::clamp( commands + shift_amount, 1, 62 );
		*new_commands = shift_commands;
		*backup_commands = 0;

		auto next_cmd_nr = g_ClientState->m_nChokedCommands + g_ClientState->m_nLastOutgoingCommand + 1;
		auto new_to = next_cmd_nr - commands + 1;
		if ( new_to <= next_cmd_nr ) {
			while ( original_fn( ecx, slot, buf, new_from, new_to, true ) ) {
				new_from = new_to++;

				if ( new_to > next_cmd_nr )
					goto next_cmd;
			}
			return false;
		}
	next_cmd:
		*( int* )(( uintptr_t )g_Prediction + 0x1C) = 0;
		*( int* )(( uintptr_t )g_Prediction + 0xC) = -1;

		auto fake_cmd = g_Input->GetUserCmd( slot, new_from );
		if ( !fake_cmd )
			return true;

		CUserCmd to_cmd;
		CUserCmd from_cmd;

		from_cmd = *fake_cmd;
		to_cmd = from_cmd;

		++to_cmd.command_number;

		if ( real_cmd ) {

			int iterator = 0;

			do
			{
				g_Prediction->Update(
					g_ClientState->m_nDeltaTick, g_ClientState->m_nDeltaTick > 0,
					g_ClientState->m_nLastCommandAck,
					g_ClientState->m_nLastOutgoingCommand + g_ClientState->m_nChokedCommands );

				to_cmd.buttons &= ~0xFFBEFFF9;

				auto new_cmd = g_Input->GetUserCmd( to_cmd.command_number );
				auto verified_cmd = g_Input->GetVerifiedCmd( to_cmd.command_number );

				if ( new_cmd && verified_cmd )
				{
					std::memcpy( new_cmd, &to_cmd, sizeof( CUserCmd ) );
					std::memcpy( &verified_cmd->m_cmd, &to_cmd, sizeof( CUserCmd ) );
					verified_cmd->m_crc = new_cmd->GetChecksum();
				}

				TickBaseSys::Get().WriteUserCmd( buf, &to_cmd, &from_cmd );

				++iterator;
				if ( iterator >= shift_amount ) {
					auto& out = CSGO::m_Packets.emplace_back();

					out.is_outgoing = true;
					out.is_used = false;
					out.cmd_number = g_ClientState->m_nLastOutgoingCommand + g_ClientState->m_nChokedCommands + 1;
					out.previous_command_number = 0;
				}
				else
				{
					auto net_chan = g_ClientState->m_NetChannel;
					if ( net_chan ) {
						++net_chan->m_nOutSequenceNr;
						++net_chan->m_nChokedPackets;
					}
					++g_ClientState->m_nChokedCommands;
				}

				if ( new_cmd )
				{
					if ( g_Options.exploit.m_DoubleTapSpeed == 1 ) {
						new_cmd->forwardmove = 0.f;
						new_cmd->sidemove = 0.f;
					}
					else if ( g_Options.exploit.m_DoubleTapSpeed == 2 ) {
						static auto cl_forwardspeed = g_CVar->FindVar( ("cl_forwardspeed") );
						static auto cl_sidespeed = g_CVar->FindVar( ("cl_sidespeed") );

						if ( m_StoredForwardMove >= 5.0f )
							new_cmd->forwardmove = cl_forwardspeed->GetFloat();

						else if ( m_StoredForwardMove <= -5.0f )
							new_cmd->forwardmove = -cl_forwardspeed->GetFloat();

						if ( m_StoredSideMove >= 5.0f )
							new_cmd->sidemove = cl_sidespeed->GetFloat();

						else if ( m_StoredSideMove <= -5.0f )
							new_cmd->sidemove = -cl_sidespeed->GetFloat();
					}
					else if ( g_Options.exploit.m_DoubleTapSpeed == 3 ) {
						auto v64 = (g_LocalPlayer->m_flDuckAmount() * 0.34f) + 1.0f;
						auto v47 = 1.0f / (v64 - g_LocalPlayer->m_flDuckAmount());

						new_cmd->forwardmove = new_cmd->forwardmove * v47;
						new_cmd->sidemove = v47 * new_cmd->sidemove;

						if ( new_cmd->sidemove > 450.f )
							new_cmd->sidemove = 450.f;
						else if ( new_cmd->sidemove < -450.f )
							new_cmd->sidemove = -450.f;

						if ( new_cmd->forwardmove > 450.f )
							new_cmd->forwardmove = 450.f;
						else if ( new_cmd->forwardmove < -450.f )
							new_cmd->forwardmove = -450.f;
					}

					Math::NormalizeAngles( new_cmd->viewangles );
					Math::MovementFix( new_cmd, m_ViewAngleStored, new_cmd->viewangles );
				}

				from_cmd = to_cmd;
				++to_cmd.command_number;

			} while ( iterator < shift_amount );
		}
		else {
			to_cmd.tick_count = INT_MAX;
			do {
				TickBaseSys::Get().WriteUserCmd( buf, &to_cmd, &from_cmd );

				++to_cmd.command_number;
				shift_amount--;
			} while ( shift_amount > 0 );
		}
	}

	bool __fastcall WriteUsercmdDeltaToBuffer( void* ecx, void*, int slot, bf_write* buf, int from, int to, bool isnewcommand )
	{
		static auto original_fn = Hooks::hlclient_hook.get_original< WriteUsercmdDeltaToBufferFn >( 24 );

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( !g_LocalPlayer )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( !g_LocalPlayer->IsAlive() )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( !IsRage() || !g_Options.ragebot.Acitve || !g_Options.exploit.ActMode )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( !TickBaseSys::Get().ValueToShift )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( TickBaseSys::Get( ).TicksToSkip )
			return original_fn( ecx, slot, buf, from, to, isnewcommand );

		if ( from != -1 )
			return true;

		uintptr_t frame_ptr = 0;
		__asm mov frame_ptr, ebp;

		if ( g_Options.exploit.mMode == 0 )
		{
			auto backup_commands = reinterpret_cast < int* > (frame_ptr + 0xFD8);
			auto new_commands = reinterpret_cast < int* > (frame_ptr + 0xFDC);

			return ShiftCmd( new_commands, backup_commands, ecx, slot, buf, -1, true );
		}
		else if ( g_Options.exploit.mMode == 1 )
		{
			auto final_from = -1;

			auto backup_commands = reinterpret_cast < int* > (frame_ptr + 0xFD8);
			auto new_commands = reinterpret_cast < int* > (frame_ptr + 0xFDC);

			auto newcmds = *new_commands;
			auto shift = TickBaseSys::Get().ValueToShift;
			TickBaseSys::Get().ValueToShift = 0;

			*backup_commands = 0;

			auto choked_modifier = newcmds + shift;

			if ( choked_modifier > 62 )
				choked_modifier = 62;

			*new_commands = choked_modifier;

			auto next_cmdnr = g_ClientState->m_nChokedCommands + g_ClientState->m_nLastOutgoingCommand + 1;
			auto final_to = next_cmdnr - newcmds + 1;

			if ( final_to <= next_cmdnr )
			{
				while ( original_fn( ecx, slot, buf, final_from, final_to, true ) )
				{
					final_from = final_to++;

					if ( final_to > next_cmdnr )
						goto label_cmd;
				}

				return false;
			}

		label_cmd:

			auto user_cmd = g_Input->GetUserCmd( final_from );

			if ( !user_cmd )
				return true;

			CUserCmd to_cmd;
			CUserCmd from_cmd;

			from_cmd = *user_cmd;
			to_cmd = from_cmd;

			to_cmd.command_number++;
			to_cmd.tick_count += 200;

			if ( newcmds > choked_modifier )
				return true;

			for ( auto i = choked_modifier - newcmds + 1; i > 0; --i )
			{
				TickBaseSys::Get().WriteUserCmd( buf, &to_cmd, &from_cmd );

				from_cmd = to_cmd;
				to_cmd.command_number++;
				to_cmd.tick_count++;
			}

			return true;
		}
	}

	using m_fn = void( __thiscall* )(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);
	void __fastcall RunCommand( void* ecx, void*, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* helper )
	{
		static auto original = Hooks::predict_hook.get_original< m_fn >( 19 );

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return original( ecx, player, ucmd, helper );

		if ( !g_LocalPlayer )
			return original( ecx, player, ucmd, helper );

		if ( !g_LocalPlayer->IsAlive() )
			return original( ecx, player, ucmd, helper );

		if ( !IsRage() || player->EntIndex() != g_LocalPlayer->EntIndex() )
			return original( ecx, player, ucmd, helper );

		if ( ucmd->tick_count >= (m_BackupedTick + int( 1 / g_GlobalVars->interval_per_tick ) + 8) ) {
			ucmd->hasbeenpredicted = true;
			return;
		}

		int backup_tickbase = player->m_nTickBase();
		float backup_curtime = g_GlobalVars->curtime;
		float m_flVelModBackup = player->m_bVelMod();

		if ( ucmd->command_number == TickBaseSys::Get().ShiftedCommand ) {
			player->m_nTickBase() = (TickBaseSys::Get().OriginalTickBase - TickBaseSys::Get().ValueToShift + 1);
			++player->m_nTickBase();

			g_GlobalVars->curtime = TICKS_TO_TIME( player->m_nTickBase() );
		}

		if ( m_OverrideVelocity && ucmd->command_number == g_ClientState->m_nLastCommandAck + 1 )
			player->m_bVelMod() = m_VelocityMod;

		original( ecx, player, ucmd, helper );

		if ( !m_OverrideVelocity )
			player->m_bVelMod( ) = m_flVelModBackup;

		if ( ucmd->command_number == TickBaseSys::Get().ShiftedCommand ) {
			player->m_nTickBase() = backup_tickbase;
			g_GlobalVars->curtime = backup_curtime;
		}

		player->m_vphysicsCollisionState() = false;
	}

	decltype(&SendMove) original_cl_move;
	void __cdecl SendMove( float_t m1, bool m2 )
	{
		bool IsTryShift = g_Options.ragebot.Acitve && g_Options.exploit.ActMode
			&& GetKeyState( g_Options.exploit.mExpltKey );

		static bool mPrevShift = false;
		static bool mShiftAfterDuck = false;

		if ( TickBaseSys::Get().TicksToSkip > 0 && !TickBaseSys::Get().IsCharged && !CMovement::Get().m_InDuck ) {

			g_pCmd->tick_count = INT_MAX;
			TickBaseSys::Get().TicksToSkip--;

			if ( TickBaseSys::Get().TicksToSkip == 0 ) {
				TickBaseSys::Get().IsCharged = true;
				CTX::m_SendPacket = true;
			}
			else
				CTX::m_SendPacket = false;

			return;
		}

		if ( IsTryShift )
		{
			if ( !mPrevShift )
			{
				TickBaseSys::Get().IsCharged = false;

				if ( !TickBaseSys::Get().IsCharged ) {
					TickBaseSys::Get().TicksToSkip = m_GoalShift;
				}

				mPrevShift = true;
			}
		}
		else
		{
			mPrevShift = false;
		}

		if ( CMovement::Get().m_InDuck )
		{
			if ( IsTryShift )
				mShiftAfterDuck = false;

			else
				mShiftAfterDuck = true;
		}
		else
		{
			if ( IsTryShift && !mShiftAfterDuck ) {

				TickBaseSys::Get().IsCharged = false;

				if ( !TickBaseSys::Get().IsCharged ) {
					TickBaseSys::Get().TicksToSkip = m_GoalShift;
				}

				mShiftAfterDuck = true;
			}
		}

		original_cl_move( m1, m2 );
	}

	using in_prediction_t = bool( __thiscall* ) (void*);

	const auto MaintainSequenceTransitions = ( void* )Utils::FindSig( "client.dll", "84 C0 74 17 8B 87" );
	static auto ptrSetupBonesTiming = ( void* )Utils::FindSig( "client.dll", "84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05" );
	static void* calcplayerview_return = ( void* )Utils::FindSig( "client.dll", "84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C" );

	bool __fastcall InPrediction( void* p )
	{
		const auto ofunc = Hooks::predict_hook.get_original< in_prediction_t >( 14 );

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return ofunc( p );

		if ( !g_LocalPlayer )
			return ofunc( p );

		if ( !g_LocalPlayer->IsAlive() )
			return ofunc( p );

		if ( !IsRage() )
			return ofunc( p );

		if ( MaintainSequenceTransitions && m_CallPlayerSetupBone && _ReturnAddress( ) == MaintainSequenceTransitions )
			return true;

		if ( ptrSetupBonesTiming && _ReturnAddress( ) == ptrSetupBonesTiming )
			return false;

		if ( _ReturnAddress( ) == calcplayerview_return )
			return true;
		
		return ofunc( p );
	}

	auto ptr_accumulate_layers = reinterpret_cast< void* > (Utils::FindSig( "client.dll", "84 C0 75 0D F6 87" ));
	auto setupvelocity_call = reinterpret_cast< void* > (Utils::FindSig( ("client.dll"), ("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0") ));

	using is_hltv_t = bool( __fastcall* ) ();

	bool __fastcall IsHLTV()
	{
		const auto org_f = Hooks::engine_hook.get_original< is_hltv_t >( 93 );

		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return org_f();

		if ( !g_LocalPlayer )
			return org_f();

		if ( !g_LocalPlayer->IsAlive() )
			return org_f();

		if ( !IsRage( ) )
			return org_f( );

		if ( m_CallPlayerSetupBone )
			return true;

		if ( g_CallLocalUpdate )
			return true;

		if ( reinterpret_cast< uintptr_t > (_ReturnAddress()) == reinterpret_cast< uintptr_t > (ptr_accumulate_layers) )
			return true;

		if ( reinterpret_cast< uintptr_t > (_ReturnAddress()) == reinterpret_cast< uintptr_t > (setupvelocity_call) )
			return true;

		return org_f();
	}

	static uintptr_t* GetRenderTabel() {
		static const auto vtable = reinterpret_cast< uintptr_t* >(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C" ) + 0x4E);
		return vtable;
	}

	void Initialize()
	{
		if ( g_CVar )
			CVarSys::Get().CallUpdate();

		if ( !CVarSys::Get().isInit )
			return;

		SimplestFix( );

		newFunctionClientDLL_hook = 
			( DWORD )DetourFunction( ( PBYTE )newFunctionClientDLL, ( PBYTE )newFunctionClientBypass );

		newFunctionEngineDLL_hook = 
			( DWORD )DetourFunction( ( PBYTE )newFunctionEngineDLL, ( PBYTE )newFunctionEngineBypass );

		newFunctionStudioRenderDLL_hook = 
			( DWORD )DetourFunction( ( PBYTE )newFunctionStudioRenderDLL, ( PBYTE )newFunctionStudioRenderBypass );

		newFunctionMaterialSystemDLL_hook =
			( DWORD )DetourFunction( ( PBYTE )newFunctionMaterialSystemDLL, ( PBYTE )newFunctionMaterialSystemBypass );

		static const auto c_cs_player_table = 
			Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C" ) + 0x47;

		hlclient_hook.setup( g_CHLClient );
		direct3d_hook.setup( g_D3DDevice9 );
		vguipanel_hook.setup( g_VGuiPanel );
		vguisurf_hook.setup( g_VGuiSurface );
		sound_hook.setup( g_EngineSound );
		mdlrender_hook.setup( g_MdlRender );
		clientmode_hook.setup( g_ClientMode );
		enginegui_hook.setup( g_EngineVGui );
		engine_hook.setup( g_EngineClient );
		m_renderable_hook.setup( GetRenderTabel() );
		bsp_hook.setup( g_EngineClient->GetBSPTreeQuery() );
		predict_hook.setup( g_Prediction );
		traced_hook.setup( g_EngineTrace );
		mat_sys_hook.setup( g_MatSystem );
		player_hook.setup( reinterpret_cast< DWORD** >(c_cs_player_table) );
		clientstate_hook.setup( ( CClientState* )(uint32_t( g_ClientState ) + 0x8) );
		static ConVar* sv_cheats_con = g_CVar->FindVar( "sv_cheats" );
		sv_cheats.setup( sv_cheats_con );

		player_hook.hook_index( 190, hkBuildTransformations );
		player_hook.hook_index( 170, hkGetEyeAngles );
		direct3d_hook.hook_index( index::Reset, hkReset );
		direct3d_hook.hook_index( index::Present, hkPresent );
		hlclient_hook.hook_index( index::FrameStageNotify, hkFrameStageNotify );
		hlclient_hook.hook_index( index::CreateMove, hkCreateMove_Proxy );
		hlclient_hook.hook_index( 24, WriteUsercmdDeltaToBuffer );
		clientstate_hook.hook_index( 5, PacketStart );
		clientstate_hook.hook_index( 6, PacketEnd );
		vguipanel_hook.hook_index( index::PaintTraverse, hkPaintTraverse );

		auto g_pFileSystem = **reinterpret_cast< void*** >(
			Utils::FindSig( "engine.dll", "8B 0D ? ? ? ? 8D 95 ? ? ? ? 6A 00 C6" ) + 0x2);

		if ( g_pFileSystem )
		{
			file_hook.setup( g_pFileSystem );
			file_hook.hook_index( 128, LooseFileAllow );
		}

		vguisurf_hook.hook_index( index::LockCursor, hkLockCursor );
		mdlrender_hook.hook_index( index::DrawModelExecute, hkDrawModelExecute );
		clientmode_hook.hook_index( index::DoPostScreenSpaceEffects, hkDoPostScreenEffects );
		clientmode_hook.hook_index( index::OverrideView, hkOverrideView );
		clientmode_hook.hook_index( 17, DrawFog );
		sv_cheats.hook_index( index::SvCheatsGetBool, hkSvCheatsGetBool );
		enginegui_hook.hook_index( index::PaintV, hkPaint );
		bsp_hook.hook_index( 6, ListLeavesInBox );
		m_renderable_hook.hook_index( 13, SetupBonesFN );
		predict_hook.hook_index( 19, RunCommand );
		predict_hook.hook_index( 14, InPrediction );
		engine_hook.hook_index( 93, IsHLTV );
		engine_hook.hook_index( 101, AspectRatio );
		traced_hook.hook_index( 4, ClipRay );
		traced_hook.hook_index( 5, HookTace );

		/*mat_sys_hook.hook_index( 84,
			hkFindMaterial );*/

		static const auto Getclr = Utils::FindSig( "materialsystem.dll", "55 8B EC 83 EC ? 56 8B F1 8A 46" );
		GetClrOriginal = ( GetClrFN )DetourFunction( ( PBYTE )Getclr, ( PBYTE )GetColorModulation );

		static auto ResetLatchAddr = ( DWORD )(Utils::FindSig( "client.dll", "57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 35" ));
		DetourFunction( ( PBYTE )ResetLatchAddr, ( PBYTE )ResetLatched );

		static const auto ISProp = Utils::FindSig( "engine.dll", "8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 ? A1 ? ? ? ? 35 ? ? ? ? EB ? 8B 01 FF 50 ? 83 F8 ? 0F 85 ? ? ? ? 8B 0D" );
		DetourFunction( ( PBYTE )ISProp, ( PBYTE )IsUsingProps );

		static auto ShouldSkipAnimFrame = ( DWORD )(Utils::FindSig( "client.dll", "57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 02" ));
		DetourFunction( ( PBYTE )ShouldSkipAnimFrame, ( PBYTE )SkipFrame );

		static auto DoExtra = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 81 ? ? ? ? ? 53 56 8B F1 57 89 74 24 1C" ));
		DetourFunction( ( PBYTE )DoExtra, ( PBYTE )DoExtraBone );

		static auto ModEye = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60" ));
		OriginalModEyePos = ( ModifyEyePosFn )DetourFunction( ( PBYTE )ModEye, ( PBYTE )hkModifyEyePos );

		static auto CalcV = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 EC 14 53 56 57 FF 75 18" ));
		OriginalCalcView = ( CalculateViewFn )DetourFunction( ( PBYTE )CalcV, ( PBYTE )hkCalcView );

		static auto processinterpolatedlist = ( DWORD )(Utils::FindSig( "client.dll", "53 0F B7 1D ? ? ? ? 56" ));
		original_processinterpolatedlist = ( DWORD )DetourFunction( ( byte* )processinterpolatedlist, ( byte* )ProcessInterp );

		static auto standardblendingrules = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85" ));
		OriginalBlend = ( DWORD )DetourFunction( ( PBYTE )standardblendingrules, ( PBYTE )StandartBlendRuleMain );

		static const auto SendMoveAdd = Utils::FindSig( "engine.dll", "55 8B EC 81 EC 64 01 00 00 53 56 8A F9" );
		original_cl_move = ( decltype(&SendMove) )DetourFunction( ( PBYTE )SendMoveAdd, ( PBYTE )SendMove );

		static auto PhysAdd = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 EC 7C 8B ? ? ? ? ? 53 56" ));
		OriginalPhycics = ( DWORD )DetourFunction( ( PBYTE )PhysAdd, ( PBYTE )PhysicSim );

		static auto GetForeIgn = ( DWORD )(Utils::FindSig( "vguimatsurface.dll", "80 3D ? ? ? ? ? 74 06 B8" ));
		OriginalFallBackFont = ( DWORD )DetourFunction( ( PBYTE )GetForeIgn, ( PBYTE )FontFallBack );

		static auto GetPartice = ( DWORD )(Utils::FindSig( "client.dll", "55 8B EC 83 EC 18 56 8B F1 C7 45" ));
		OriginalPartice = ( NewParticeFN )DetourFunction( ( PBYTE )GetPartice, ( PBYTE )InitNewParticlesScalar );

		/*static auto AddressSetBounds = ( DWORD )(Utils::FindSig( "client.dll", "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 18 56 57 8B 7B" ));
		OriginalSetBounds = ( SetBoundsFn )DetourFunction( ( PBYTE )AddressSetBounds, ( PBYTE )SetCollisionBounds );*/

		DWORD* ex_pointer = ( DWORD* )*( DWORD* )(c_cs_player_table);
		originAnimation = ( UpdateAnimation_t )DetourFunction( ( PBYTE )ex_pointer[ 224 ], ( PBYTE )UpdateAnimationHook );

		/*static auto checkfilecrcswithserver = ( DWORD )(Utils::FindSig( "engine.dll", "55 8B EC 81 EC ? ? ? ? 53 8B D9 89 5D F8" ));
		DetourFunction( ( PBYTE )checkfilecrcswithserver, ( PBYTE )CheckFileCRC );*/

		sequence_hook = new recv_prop_hook( C_BaseViewModel::m_nSequence(), RecvProxy );

		HookedEvents.RegisterSelf();
		EngineDraw::Get().InitFonts();

		g_VGuiSurface->PlaySound_( "ui\\lobby_notification_matchready.wav" );
	}

	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();
		enginegui_hook.unhook_all();
		bsp_hook.unhook_all();
		// Glow::Get().Shutdown();
	}

	void __cdecl hkCL_Move( float_t flFrametime, bool bIsFinalTick )
	{

	}

	void __fastcall UpdateAnimationHook( C_BasePlayer* player, uint32_t )
	{
		if ( !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected() )
			return originAnimation( player );

		if ( !IsRage() || g_UpdateSkins || !g_LocalPlayer || !player || !player->IsAlive() || player->IsDormant() )
			return originAnimation( player );

		if ( player->EntIndex() == g_LocalPlayer->EntIndex() )
			return originAnimation( player );

		if ( player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() )
			return originAnimation( player );

		if ( g_CallLocalUpdate )
			return originAnimation( player );
	}

	void __stdcall hkPaint( int mode )
	{
		typedef void( __thiscall* m_Prototype )(IEngineVGUI*, int);
		static auto oPaint = enginegui_hook.get_original<m_Prototype>( index::PaintV );
		oPaint( g_EngineVGui, mode );

		typedef void( __thiscall* start_drawing )(void*);
		typedef void( __thiscall* finish_drawing )(void*);

		static start_drawing start_draw = ( start_drawing )Utils::FindSig( "vguimatsurface.dll", "55 8B EC 83 E4 C0 83 EC 38" );
		static finish_drawing end_draw = ( finish_drawing )Utils::FindSig( "vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05" );

		if ( mode & 1 )
		{
			start_draw( g_VGuiSurface );
			{
				if ( g_Options.visuals.Active ) {
					Visuals::Get().Run();

					if ( g_Options.visuals.misc_.keybind_list )
						Visuals::Get().DrawKeyBinds();
				}

				if ( g_Options.visuals.misc_.watermark )
					Visuals::Get().DrawWatermark();

				Logs::Get().DrawLogs();

				for ( auto current : c_lua::Get().hooks.getHooks( ("on_paint") ) )
					current.func();
			}
			end_draw( g_VGuiSurface );
		}
	}

	long __stdcall hkPresent( IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region )
	{
		static auto oPresent = direct3d_hook.get_original<decltype(&hkPresent)>( index::Present );

		IDirect3DStateBlock9* state = NULL;
		IDirect3DVertexDeclaration9* vertDec;
		IDirect3DVertexShader9* vertShader;

		device->CreateStateBlock( D3DSBT_PIXELSTATE, &state );
		device->GetVertexDeclaration( &vertDec );
		device->GetVertexShader( &vertShader );

		static bool m_CallOnce{ false };
		if ( !m_CallOnce ) {

			ImGui::CreateContext();

			ImGui_ImplWin32_Init( InputSys::Get().GetMainWindow() );
			ImGui_ImplDX9_Init( device );

			ImGuiIO& io = ImGui::GetIO();

			ImFontConfig m_MainConfig;
			m_MainConfig.RasterizerFlags = 0x0;

			ImFontConfig m_SecondConfig;
			m_SecondConfig.RasterizerFlags = ImGuiFreeType::MonoHinting | ImGuiFreeType::Monochrome;

			RenderImFonts::DefaultFont = io.Fonts->AddFontFromFileTTF( "C:/windows/fonts/tahoma.ttf", 12, &m_SecondConfig, io.Fonts->GetGlyphRangesCyrillic() );
			ImGuiFreeType::BuildFontAtlas( io.Fonts, m_SecondConfig.RasterizerFlags );

			RenderImFonts::IconFont = io.Fonts->AddFontFromMemoryTTF( Astrix, sizeof( Astrix ), 40, &m_MainConfig, io.Fonts->GetGlyphRangesDefault() );
			ImGuiFreeType::BuildFontAtlas( io.Fonts, m_MainConfig.RasterizerFlags );

			m_CallOnce = true;
		}

		g_Render->SetupPresent( device );

		static void* dwReturnAddress = _ReturnAddress();
		if ( dwReturnAddress == _ReturnAddress() )
		{
			Visuals::Get().Spread( device );

			g_Render->PreRender( device );
			g_Render->PostRender( device );
			{
				Gui::m_Details.Install();
			}
			g_Render->EndPresent( device );
		}

		state->Apply();
		state->Release();

		device->SetVertexDeclaration( vertDec );
		device->SetVertexShader( vertShader );

		return oPresent( device, src_rect, dest_rect, dest_wnd_override, dirty_region );
	}

	long __stdcall hkEndScene( IDirect3DDevice9* pDevice )
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>( index::EndScene );
		return oEndScene( pDevice );
	}

	long __stdcall hkReset( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters )
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>( index::Reset );

		g_Render->InvalidateObjects();

		auto hr = oReset( device, pPresentationParameters );
		if ( hr >= 0 ) {
			g_Render->CreateObjects( device );
		}

		return hr;
	}

	void Patch( PVOID address, const int type, const int bytes )
	{
		DWORD d, ds;
		VirtualProtect( address, bytes, PAGE_EXECUTE_READWRITE, &d );
		memset( address, type, bytes );
		VirtualProtect( address, bytes, d, &ds );
	}

	void __stdcall hkCreateMove( int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket )
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>( index::CreateMove );
		oCreateMove( g_CHLClient, 0, sequence_number, input_sample_frametime, active );

		m_OverrideVelocity = false;

		if ( !g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() )
			return;

		auto cmd = g_Input->GetUserCmd( sequence_number );
		auto verified = g_Input->GetVerifiedCmd( sequence_number );
		
		if ( !cmd || !cmd->command_number )
			return;

		LocalPlayerData::m_Weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );
		LocalPlayerData::m_EyePosition = g_LocalPlayer->GetEyePos( );

		if ( !LocalPlayerData::m_Weapon )
			return;

		CTX::m_SendPacket = true;
		m_ViewAngleStored = cmd->viewangles;

		g_pCmd = cmd;
		m_GoalShift = g_Options.exploit.mMode == 0 ? 12 : 5;

		static bool m_CallPatch = false;
		if ( !m_CallPatch ) {
			m_CallPatch = true;
			static int* fakelagboost = reinterpret_cast< int* > ( Utils::FindSig( "engine.dll", "0F 4F F0 89 5D FC" ) - 0x6 );
			Patch( static_cast< PVOID > ( fakelagboost ), 17, 1 );
		}

		if ( IsRage() )
		{
			MissCount::Get().Build_();

			if ( TickBaseSys::Get().ValueToShift )
				CSGO::m_TicksAllowed = g_LocalPlayer->m_nTickBase() - TickBaseSys::Get().ValueToShift;

			else
				CSGO::m_TicksAllowed = g_LocalPlayer->m_nTickBase();
		}
		else
			CSGO::m_TicksAllowed = g_LocalPlayer->m_nTickBase();

		m_BackupedTick = cmd->tick_count;
		m_StoredForwardMove = cmd->forwardmove;
		m_StoredSideMove = cmd->sidemove;

		CFakeLag::Get().OnCreateMove( cmd );

		Vector m_UnpredictedVelocity{ };
		Vector m_UnpredictedAbsVelocity{ };

		if ( !g_UpdateSkins )
		{
			if ( g_pCmd )
			{
				m_OverrideVelocity = true;

				if ( IsRage() && g_LocalPlayer->IsAlive() ) {
					m_UnpredictedVelocity = g_LocalPlayer->m_vecVelocity();
					m_UnpredictedAbsVelocity = g_LocalPlayer->m_VecAbsVelocity();
				}

				CMovement::Get().MouseFix( cmd );
				CMovement::Get().OnCreateMove( cmd );

				if ( g_Options.movement.unlimitDuck )
					cmd->buttons |= IN_BULLRUSH;

				if ( g_LocalPlayer->IsAlive( ) && g_Options.visuals.Active &&
					g_Options.visuals.view.grenade_prediction )
					Visuals::Get( ).Tick( cmd->buttons );

				Engine_Prediction::Get().Begin( cmd );
				{
					if ( IsRage() && g_LocalPlayer->IsAlive() ) {

						if ( LocalPlayerData::m_Weapon )
						{
							auto velocity_predicted = g_LocalPlayer->m_vecVelocity();
							auto abs_velocity_predicted = g_LocalPlayer->m_VecAbsVelocity();

							g_LocalPlayer->m_vecVelocity() = m_UnpredictedVelocity;
							g_LocalPlayer->m_VecAbsVelocity() = m_UnpredictedAbsVelocity;

							LocalPlayerData::m_Weapon->UpdateAccuracyPenalty();

							g_LocalPlayer->m_vecVelocity() = velocity_predicted;
							g_LocalPlayer->m_VecAbsVelocity() = abs_velocity_predicted;
						}
					}

					CMovement::Get().SlowWalk( cmd );
					CMovement::Get().FakeDuck( cmd );

					if ( IsRage( ) ) {
						CAntiAim::Get().OnCreateMove( cmd );
						CRage::Get().OnCreateMove( cmd );
						TickBaseSys::Get().CallExploit( cmd );
					}

					if ( g_Options.MasterSwitch == 1 && g_Options.active_legit )
						CLegitbot::Get( ).Run( cmd );

					CMovement::Get().AutoPeek( cmd, m_ViewAngleStored.yaw );
				}
				Engine_Prediction::Get().End();

				m_OverrideVelocity = false;
			}

			if ( g_LocalPlayer->IsAlive() && IsRage() )
			{
				auto& correct = CSGO::m_Data.emplace_front();

				correct.command_number = cmd->command_number;
				correct.choked_commands = g_ClientState->m_nChokedCommands + 1;
				correct.tickcount = g_GlobalVars->tickcount;

				if ( CTX::m_SendPacket )
					CSGO::m_ChokedNumber.clear();
				else
					CSGO::m_ChokedNumber.emplace_back( correct.command_number );

				while ( CSGO::m_Data.size() > ( int )(2.0f / g_GlobalVars->interval_per_tick) )
					CSGO::m_Data.pop_back();

				auto& out = CSGO::m_Packets.emplace_back();

				out.is_outgoing = CTX::m_SendPacket;
				out.is_used = false;
				out.cmd_number = cmd->command_number;
				out.previous_command_number = 0;

				while ( CSGO::m_Packets.size() > ( int )(1.0f / g_GlobalVars->interval_per_tick) )
					CSGO::m_Packets.pop_front();

				if ( !CTX::m_SendPacket )
				{
					auto net_channel = g_ClientState->m_NetChannel;

					if ( net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4) )
					{
						auto backup_choke = net_channel->m_nChokedPackets;
						net_channel->m_nChokedPackets = 0;

						net_channel->SendDatagram();
						--net_channel->m_nOutSequenceNr;

						net_channel->m_nChokedPackets = backup_choke;
					}
				}
			}
		}

		if ( CTX::m_SendPacket )
			m_RealAngle = cmd->viewangles;
		else
			m_FakeAngle = cmd->viewangles;

		Math::NormalizeAngles( cmd->viewangles );
		Math::ClampAngles( cmd->viewangles );
		Math::MovementFix( cmd, m_ViewAngleStored, cmd->viewangles );

		if ( g_ClientState->m_nChokedCommands >= 14 )
			CTX::m_SendPacket = true;

		bSendPacket = CTX::m_SendPacket;

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}

	__declspec(naked) void __fastcall hkCreateMove_Proxy( void* _this, int, int sequence_number, float input_sample_frametime, bool active )
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[ active ]
			push dword ptr[ input_sample_frametime ]
			push dword ptr[ sequence_number ]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}

	void __fastcall hkPaintTraverse( void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce )
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>( index::PaintTraverse );

		if ( !strcmp( "HudZoom", g_VGuiPanel->GetName( panel ) ) && (g_Options.visuals.Active && g_Options.visuals.removals.scope) )
			return;

		oPaintTraverse( g_VGuiPanel, edx, panel, forceRepaint, allowForce );

		if ( !panelId ) {
			const auto panelName = g_VGuiPanel->GetName( panel );
			if ( !strcmp( panelName, "FocusOverlayPanel" ) ) {
				panelId = panel;
			}
		}

		if ( panelId == panel ) {
			static auto blur = g_CVar->FindVar( "@panorama_disable_blur" );
			blur->SetValue( g_Options.visuals.Active && g_Options.visuals.removals.panorama_blue );

			if ( g_LocalPlayer && g_EngineClient->IsInGame( ) && g_EngineClient->IsConnected( ) ) {
				Visuals::Get( ).SkyBox( );
			}
		}
	}

	/*void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}*/

	int __fastcall hkDoPostScreenEffects( void* _this, int edx, int a1 )
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>( index::DoPostScreenSpaceEffects );

		if ( g_EngineClient->IsConnected() && g_EngineClient->IsInGame()
			&& g_LocalPlayer && g_Options.visuals.Active )
			Glow::Get().Run();

		return oDoPostScreenEffects( g_ClientMode, edx, a1 );
	}

	void __fastcall hkFrameStageNotify( void* _this, int edx, ClientFrameStage_t stage )
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>( index::FrameStageNotify );

		QAngle aim_punch;
		QAngle view_punch;

		if ( g_EngineClient->IsConnected() && g_EngineClient->IsInGame() )
		{
			static auto rModelLight = g_CVar->FindVar( "r_modelAmbientMin" );
			*( float* )(( DWORD )&rModelLight->m_fnChangeCallbacks + 0xC) = NULL;

			static bool mBackup = false;
			static float mOld;

			if ( !mBackup )
			{
				mOld = rModelLight->GetFloat( );
				mBackup = true;
			}

			if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START )
				skins::on_frame_stage_notify( false );

			else if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END )
				skins::on_frame_stage_notify( true );

			if ( g_LocalPlayer )
			{
				bool isInUse = g_Options.visuals.removals.shadows && g_Options.visuals.Active;
				bool isInUseMat = g_Options.visuals.world.full_bright && g_Options.visuals.Active;

				if ( stage == FRAME_RENDER_END )
				{
					CVarSys::Get().m_StoredVars[ "r_shadows" ]->SetValue( !isInUse );
					CVarSys::Get().m_StoredVars[ "cl_csm_shadows" ]->SetValue( !isInUse );
					CVarSys::Get().m_StoredVars[ "cl_foot_contact_shadows" ]->SetValue( !isInUse );
					CVarSys::Get().m_StoredVars[ "cl_csm_viewmodel_shadows" ]->SetValue( !isInUse );
					CVarSys::Get().m_StoredVars[ "cl_csm_rope_shadows" ]->SetValue( !isInUse );
					CVarSys::Get().m_StoredVars[ "mat_fullbright" ]->SetValue( isInUseMat );
				}

				if ( g_Options.visuals.Active && g_Options.visuals.removals.flash ) {
					g_LocalPlayer->m_flFlashMaxAlpha() = 0.f;
				}
				else
					g_LocalPlayer->m_flFlashMaxAlpha() = 255.f;

				if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && g_LocalPlayer->IsAlive() )
				{
					auto viewmodel = g_LocalPlayer->m_hViewModel().Get();

					if ( viewmodel && Engine_Prediction::Get().viewmodel_data.weapon == viewmodel->m_hWeapon().Get() && Engine_Prediction::Get().viewmodel_data.sequence == viewmodel->m_nSequenceVar() && Engine_Prediction::Get().viewmodel_data.animation_parity == viewmodel->m_nAnimationParity() )
					{
						viewmodel->m_flCycleData() = Engine_Prediction::Get().viewmodel_data.cycle;
						viewmodel->m_flAnimTimeData() = Engine_Prediction::Get().viewmodel_data.animation_time;
					}
				}

				if ( g_LocalPlayer->IsAlive() && IsRage() )
				{
					m_StoredStage = stage;

					int StageMinus = stage - 2;

					if ( StageMinus ) {
	
					}
					else {
						Engine_Prediction::Get( ).UpdateVelocity( );
					}
				}

				if ( stage == FRAME_NET_UPDATE_END )
				{
					if ( g_LocalPlayer->IsAlive() && IsRage() ) {
						Backtrack::Get().Run();
					}
				}
				else if ( stage == FRAME_RENDER_START )
				{
					if ( g_LocalPlayer->IsAlive() )
					{
						m_RateStored = ( int )std::round( 1.f / g_GlobalVars->interval_per_tick );
						LagComp::Get().FixLocalAnims();
					}

					bool ShouldLightModulate = g_Options.visuals.Active &&
						g_Options.visuals.world.modulate_light.rAmbient;

					if ( ShouldLightModulate )
					{
						if ( g_Options.visuals.world.modulate_light.rAmbient )
							rModelLight->SetValue( g_Options.visuals.world.modulate_light.AmbientAmount );

						else
							rModelLight->SetValue( mOld );

						for ( auto i = 1; i <= g_EntityList->GetHighestEntityIndex( ); ++i )
						{
							auto entity = reinterpret_cast < C_BasePlayer* > ( g_EntityList->GetClientEntity( i ) );

							if ( !entity )
								continue;

							if ( entity->GetClientClass( )->m_ClassID == 69 )
							{
								if ( g_Options.visuals.world.modulate_light.rAmbient )
								{
									entity->m_bUseCustomAutoExposureMin( ) = true;
									entity->m_bUseCustomAutoExposureMax( ) = true;
									entity->m_bUseCustomBloomScale( ) = true;
								}
								else
								{
									entity->m_bUseCustomAutoExposureMin( ) = false;
									entity->m_bUseCustomAutoExposureMax( ) = false;
									entity->m_bUseCustomBloomScale( ) = false;
								}

								entity->m_flCustomAutoExposureMin( ) = g_Options.visuals.world.modulate_light.Exposure;
								entity->m_flCustomAutoExposureMax( ) = g_Options.visuals.world.modulate_light.Exposure;
								entity->m_flCustomBloomScale( ) = g_Options.visuals.world.modulate_light.Bloom;
							}
						}
					}
					else
					{
						rModelLight->SetValue( mOld );
					}

					if ( g_Options.visuals.Active )
					{
						if ( g_Options.visuals.removals.recoil )
						{
							aim_punch = g_LocalPlayer->m_aimPunchAngle();
							view_punch = g_LocalPlayer->m_viewPunchAngle();

							g_LocalPlayer->m_aimPunchAngle() = QAngle{ };
							g_LocalPlayer->m_viewPunchAngle() = QAngle{ };
						}
					}

					bool m_Activate = g_Options.visuals.Active && g_Options.visuals.removals.post_process;
					static ConVar* PostProcVar = g_CVar->FindVar( "mat_postprocess_enable" );
					PostProcVar->SetValue( !m_Activate );
				}
				else if ( stage == FRAME_RENDER_END )
				{
					Visuals::Get( ).FogChanger( );

					if ( g_Options.visuals.Active && g_Options.visuals.removals.recoil )
					{
						if ( !aim_punch.IsZero() || !view_punch.IsZero() )
						{
							g_LocalPlayer->m_aimPunchAngle() = aim_punch;
							g_LocalPlayer->m_viewPunchAngle() = view_punch;

							aim_punch = QAngle{ };
							view_punch = QAngle{ };
						}
					}
				}

				if ( g_LocalPlayer->IsAlive() ) {
					LagComp::Get().AnimationFix( stage );
				}
			}
		}

		ofunc( g_CHLClient, edx, stage );
	}

	void __fastcall hkOverrideView( void* _this, int edx, CViewSetup* vsView )
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>( index::OverrideView );

		if ( g_LocalPlayer && g_EngineClient->IsInGame() && vsView )
		{
			static auto misc_viewmodel_fov = g_CVar->FindVar( "viewmodel_fov" );
			*( float* )(( DWORD )&misc_viewmodel_fov->m_fnChangeCallbacks + 0xC) = NULL;

			*reinterpret_cast< int* >(( DWORD )&g_CVar->FindVar( "viewmodel_offset_x" )->m_fnChangeCallbacks + 0xC) = 0;
			*reinterpret_cast< int* >(( DWORD )&g_CVar->FindVar( "viewmodel_offset_y" )->m_fnChangeCallbacks + 0xC) = 0;
			*reinterpret_cast< int* >(( DWORD )&g_CVar->FindVar( "viewmodel_offset_z" )->m_fnChangeCallbacks + 0xC) = 0;

			static auto viewmodel_offset_x = g_CVar->FindVar( "viewmodel_offset_x" );
			static auto viewmodel_offset_y = g_CVar->FindVar( "viewmodel_offset_y" );
			static auto viewmodel_offset_z = g_CVar->FindVar( "viewmodel_offset_z" );

			static float old; static bool once = false;
			if ( !once ) {
				once = true;
				old = misc_viewmodel_fov->GetFloat();
			}

			if ( g_Options.visuals.Active )
			{
				Visuals::Get( ).ThirdPerson( );

				if ( g_LocalPlayer->IsAlive( ) && g_Options.visuals.Active &&
					g_Options.visuals.view.grenade_prediction )
					Visuals::Get( ).View( vsView );

				if ( !g_LocalPlayer->m_bIsScoped() )
					vsView->fov = g_Options.visuals.view.fov_v;

				else if ( g_LocalPlayer->m_bIsScoped() && g_Options.visuals.removals.zoom && g_LocalPlayer->m_hActiveWeapon() )
					vsView->fov = g_Options.visuals.view.fov_v;

				misc_viewmodel_fov->SetValue( g_Options.visuals.view.view_v );
				viewmodel_offset_x->SetValue( g_Options.visuals.view.x_view );
				viewmodel_offset_y->SetValue( g_Options.visuals.view.y_view );
				viewmodel_offset_z->SetValue( g_Options.visuals.view.z_view );
			}
			else
			{
				misc_viewmodel_fov->SetValue( old );
				viewmodel_offset_x->SetValue( 0 );
				viewmodel_offset_y->SetValue( 0 );
				viewmodel_offset_z->SetValue( 0 );
			}

			if ( CMovement::Get().m_InDuck && g_LocalPlayer->IsAlive() )
				vsView->origin.z = g_LocalPlayer->GetAbsOrigin().z + 64.f;
		}

		ofunc( g_ClientMode, edx, vsView );
	}

	void __fastcall hkLockCursor( void* _this )
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>( index::LockCursor );

		if ( Gui::m_Details.GetMenuState() ) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}

		ofunc( g_VGuiSurface );
	}

	void __fastcall hkDrawModelExecute( void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld )
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>( index::DrawModelExecute );

		if ( !g_LocalPlayer || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() )
			return ofunc( _this, edx, ctx, state, pInfo, pCustomBoneToWorld );

		if ( g_MdlRender->IsForcedMaterialOverride() &&
			!strstr( pInfo.pModel->szName, "arms" ) &&
			!strstr( pInfo.pModel->szName, "weapons/v_" ) ) {
			return ofunc( _this, edx, ctx, state, pInfo, pCustomBoneToWorld );
		}

		Chams::Get().OnDrawModelExecute( ctx, state, pInfo, pCustomBoneToWorld );

		ofunc( _this, edx, ctx, state, pInfo, pCustomBoneToWorld );
		g_MdlRender->ForcedMaterialOverride( nullptr );
	}

	bool __fastcall hkSvCheatsGetBool( PVOID pConVar, void* edx )
	{
		static auto dwCAM_Think = Utils::PatternScan( GetModuleHandleW( L"client.dll" ), "85 C0 75 30 38 86" );
		static auto ofunc = sv_cheats.get_original<bool( __thiscall* )(PVOID)>( 13 );

		if ( !g_LocalPlayer || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() )
			return ofunc( pConVar );

		if ( !ofunc )
			return false;

		if ( reinterpret_cast< DWORD >(_ReturnAddress()) == reinterpret_cast< DWORD >(dwCAM_Think) )
			return true;
		return ofunc( pConVar );
	}
}
