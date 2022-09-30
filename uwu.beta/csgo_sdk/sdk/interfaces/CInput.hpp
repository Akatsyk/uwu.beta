#pragma once

#include "../Misc/CUserCmd.hpp"

#define MULTIPLAYER_BACKUP 150

class bf_write;
class bf_read;

class CInput
{
public:
	std::byte			pad0[0xC];			// 0x0
	bool				bTrackIRAvailable;		// 0xC
	bool				bMouseInitialized;		// 0xD
	bool				bMouseActive;			// 0xE
	std::byte			pad1[0x9A];			// 0xF
	bool				bCameraInThirdPerson;	// 0xA9
	std::byte			pad2[0x2];			// 0xAA
	Vector				vecCameraOffset;		// 0xAC
	std::byte			pad3[0x38];			// 0xB8
	CUserCmd* m_pCommands;			// 0xF0
	CVerifiedUserCmd* m_pVerifiedCommands;	// 0xF4       

	inline CUserCmd* GetUserCmd(int sequence_number);
	inline CUserCmd * GetUserCmd(int nSlot, int sequence_number);
	inline CVerifiedUserCmd* GetVerifiedCmd(int sequence_number);
};

CUserCmd* CInput::GetUserCmd(int sequence_number)
{
	using OriginalFn = CUserCmd * (__thiscall *)(void *, int, int);
	return CallVFunction<OriginalFn>(this, 8)(this, 0, sequence_number);
}

CUserCmd *CInput::GetUserCmd(int nSlot, int sequence_number)
{
	typedef CUserCmd*(__thiscall *GetUserCmd_t)(void*, int, int);
	return CallVFunction<GetUserCmd_t>(this, 8)(this, nSlot, sequence_number);
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
	auto verifiedCommands = *(CVerifiedUserCmd **)(reinterpret_cast<uint32_t>(this) + 0xF4);
	return &verifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
}
