#pragma once

#include "sdk/csgostructs.hpp"
#include "sdk/utils/vfunc_hook.hpp"

#include <d3d9.h>

namespace index
{
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
	constexpr auto Present					= 17;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 22;
    constexpr auto PlaySound                = 82;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
	constexpr auto PaintV					= 14;
}

namespace Hooks
{
    void Initialize();
    void Shutdown();

	using GetClrFN = void(__thiscall*)(void*, float*, float*, float*);
	extern GetClrFN GetClrOriginal;

	using ModifyEyePosFn = void( __thiscall* )(void*, void*, Vector&);
	extern ModifyEyePosFn OriginalModEyePos;

	using CalculateViewFn = void( __fastcall* )(void*, void*, Vector&, QAngle&, float&, float&, float&);
	extern CalculateViewFn OriginalCalcView;

    inline vfunc_hook hlclient_hook;
	inline vfunc_hook bsp_hook;
	inline vfunc_hook direct3d_hook;
	inline vfunc_hook vguipanel_hook;
	inline vfunc_hook vguisurf_hook;
	inline vfunc_hook mdlrender_hook;
	inline vfunc_hook viewrender_hook;
	inline vfunc_hook sound_hook;
	inline vfunc_hook clientmode_hook;
	inline vfunc_hook sv_cheats;
	inline vfunc_hook enginegui_hook;
	inline vfunc_hook engine_hook;
	inline vfunc_hook m_renderable_hook;
	inline vfunc_hook predict_hook;
	inline vfunc_hook clientstate_hook;
	inline vfunc_hook traced_hook;
	inline vfunc_hook player_hook;
	inline vfunc_hook file_hook;
	inline vfunc_hook mat_sys_hook;

	static auto cl_move = (DWORD)(Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 53 56 8A"));
	void __cdecl hkCL_Move(float_t flFrametime, bool bIsFinalTick);
    long __stdcall hkEndScene(IDirect3DDevice9* device);
    long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
    void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
	void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	static void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
    void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
	void __fastcall hkOverrideView(void* _this, int, CViewSetup * vsView);
	void __fastcall hkLockCursor(void* _this);
    int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx);
	long __stdcall hkPresent(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);
	void __stdcall hkPaint(int mode);
	int __fastcall ListLeavesInBox(std::uintptr_t ecx, std::uintptr_t edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max);

	static void __fastcall hkModifyEyePos( void* ecx, void* edx, Vector& input_eye_pos );
	static void __fastcall hkCalcView( void* ecx, void* edx, Vector& eye_origin, QAngle& eye_angles, float& z_near, float& z_far, float& fov );

	typedef void(__thiscall* UpdateAnimation_t)(C_BasePlayer*);
	extern UpdateAnimation_t originAnimation;
	void __fastcall UpdateAnimationHook(C_BasePlayer* player, uint32_t);

	void __cdecl SendMove(float_t m1, bool m2);
	extern decltype(&SendMove) original_cl_move;
}
