#include "tickbase.h"
#include "../sdk/utils/math.hpp"

#include "ragebot.h"
#include "movement.h"
#include "autowall.h"

#include "../config/config.h"

void TickBaseSys::WriteUserCmd( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd )
{
	using WriteUsercmd_t = void( __fastcall* )(void*, CUserCmd*, CUserCmd*);
	static WriteUsercmd_t fnWriteUserCmd = reinterpret_cast< WriteUsercmd_t >(Utils::FindSig( "client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ));

	__asm {
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    fnWriteUserCmd
		add     esp, 4
	}
}

bool CanExploit( int tickbase_shift )
{
	auto weapon = LocalPlayerData::m_Weapon;

	if ( !weapon )
		return false;

	if ( weapon->IsGrenadeIDX( ) )
		return false;

	if ( weapon->IsKnifeIDX( ) )
		return false;

	auto m_It = weapon->m_Item( ).m_iItemDefinitionIndex( );

	if ( m_It == WEAPON_REVOLVER || m_It == WEAPON_TASER || m_It == WEAPON_HEALTHSHOT )
		return false;

	const auto info = (weapon->GetCSWeaponData( ));

	if ( !(info) )
		return false;

	float curtime = 0;

	if ( tickbase_shift != 0 )
		curtime = TICKS_TO_TIME( CSGO::m_TicksAllowed - tickbase_shift );

	else
		curtime = TICKS_TO_TIME( CSGO::m_TicksAllowed );

	if ( curtime < g_LocalPlayer->m_flNextAttack( ) )
		return false;

	if ( curtime < weapon->m_flNextPrimaryAttack( ) )
		return false;

	return true;
}

void TickBaseSys::TryDefensive( CUserCmd* m ) {

}

void TickBaseSys::CallExploit( CUserCmd* m )
{
	if ( GetAsyncKeyState( VK_LBUTTON ) || g_Options.exploit.mMode == 1 )
		Key = false;

	static bool DidShift = false;
	static int PrevTicks = 0;

	IsShotDT = false;
	bool AchieveDt = g_Options.ragebot.Acitve && g_Options.exploit.ActMode
		&& GetKeyState( g_Options.exploit.mExpltKey );

	if ( Key )
	{
		--ShiftRelloc;

		if ( ShiftRelloc == 0 )
			Key = false;
	}
	else
		ShiftRelloc = 20;

	if ( ShiftRelloc > 20 )
		ShiftRelloc = 20;

	if ( ShiftRelloc < 0 )
		ShiftRelloc = 0;

	std::clamp( ShiftRelloc, 0, 20 );

	if ( !CMovement::Get( ).m_InDuck )
	{
		if ( AchieveDt )
		{
			PrevTicks = 0;

			auto CanShift = CanExploit( m_GoalShift );
			auto CanShot = CanExploit( 0 );

			if ( CanShift || (!CanShot) && !DidShift ) {
				PrevTicks = m_GoalShift;
			}
			else {
				PrevTicks = 0;
			}

			if ( PrevTicks > 0 )
			{
				if ( CanExploit( PrevTicks ) )
				{
					IsShotDT = true;

					if ( m->buttons & IN_ATTACK )
					{
						ShiftedCommand = m->command_number;
						OriginalTickBase = g_LocalPlayer->m_nTickBase( );
						ValueToShift = PrevTicks;
						DidShot = true;
						Key = true;
						CSGO::m_CallShift = true;
					}
					else
					{
						if ( (!(m->buttons & IN_ATTACK) || !CRage::Get( ).RageShot) && DidShot ) {
							IsCharged = false;
							TicksToSkip = m_GoalShift;
							DidShot = false;
							CSGO::m_CallShift = false;
						}
					}
				}
				else
				{
					CSGO::m_CallShift = false;
					IsShotDT = false;
				}
			}

			DidShift = PrevTicks != 0;
		}
	}
}