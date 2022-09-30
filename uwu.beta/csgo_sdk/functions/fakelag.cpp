#include "fakelag.h"
#include "../sdk/utils/math.hpp"

#include <algorithm>
#include "../config/config.h"

#include "movement.h"
#include "tickbase.h"

bool CFakeLag::IsInit( )
{
	if ( g_Options.ragebot.Acitve && g_Options.antiaim.FakeYaw > 0 && !g_Options.fakelag.Activate )
		return true;

	return false;
}

void CFakeLag::OnCreateMove( CUserCmd* m )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	int m_Choke = 0;
	int m_Lag = ( int )g_Options.fakelag.Amount;

	auto m_Time = (( int )(1.0f / g_GlobalVars->interval_per_tick)) / 64;
	bool isExploit = g_Options.exploit.ActMode && GetKeyState( g_Options.exploit.mExpltKey );

	switch ( g_Options.fakelag.Mode ) {
	case 0:
		m_Choke = m_Lag;
		break;
	case 1:

		if ( g_LocalPlayer->m_vecVelocity( ).Length2D( ) >= 5.0f )
		{
			auto dynamic_factor = std::ceilf( 64.0f / (g_LocalPlayer->m_vecVelocity( ).Length2D( ) * g_GlobalVars->interval_per_tick) );

			if ( dynamic_factor > 14 )
				dynamic_factor = m_Lag;

			m_Choke = dynamic_factor;
		}
		else
			m_Choke = m_Lag;

		break;
	}

	auto m_Packet = g_ClientState->m_nChokedCommands;

	if ( !CMovement::Get( ).m_InDuck )
	{
		if ( g_Options.ragebot.Acitve && g_Options.fakelag.Activate ) {

			if ( !isExploit )
			{
				if ( m_Packet < m_Choke )
					CTX::m_SendPacket = false;
				else
					CTX::m_SendPacket = true;
			}

			else
			{
				if ( !TickBaseSys::Get( ).Key )
				{
					if ( m_Packet < 1 )
						CTX::m_SendPacket = false;
					else
						CTX::m_SendPacket = true;
				}
				else
				{
					if ( g_Options.exploit.m_Tolerance < 1 )
					{
						if ( m_Packet < 14 )
							CTX::m_SendPacket = false;
						else
							CTX::m_SendPacket = true;
					}
					else
					{
						if ( TickBaseSys::Get( ).ShiftRelloc < 20 - g_Options.exploit.m_Tolerance )
						{
							if ( m_Packet < 14 )
								CTX::m_SendPacket = false;
							else
								CTX::m_SendPacket = true;
						}
					}
				}
			}

			isLag = true;
		}

		else if ( !g_Options.ragebot.Acitve && g_Options.fakelag.Activate )
		{
			if ( m_Packet < m_Choke )
				CTX::m_SendPacket = false;
			else
				CTX::m_SendPacket = true;

			isLag = true;
		}

		else if ( g_Options.ragebot.Acitve && !g_Options.fakelag.Activate ) {

			if ( !isExploit )
			{
				if ( m_Packet < 1 )
					CTX::m_SendPacket = false;
				else
					CTX::m_SendPacket = true;
			}
			else
			{
				if ( !TickBaseSys::Get( ).Key )
				{
					if ( m_Packet < 1 )
						CTX::m_SendPacket = false;
					else
						CTX::m_SendPacket = true;
				}
				else
				{
					if ( g_Options.exploit.m_Tolerance < 1 )
					{
						if ( m_Packet < 14 )
							CTX::m_SendPacket = false;
						else
							CTX::m_SendPacket = true;
					}
					else
					{
						if ( TickBaseSys::Get( ).ShiftRelloc < 20 - g_Options.exploit.m_Tolerance )
						{
							if ( m_Packet < m_Choke )
								CTX::m_SendPacket = false;
							else
								CTX::m_SendPacket = true;
						}
					}
				}
			}

			isLag = true;
		}

		else
			isLag = false;
	}
	else
	{
		//if ( m_Packet < 14 )
		//	CTX::m_SendPacket = false;
		//else
		//	CTX::m_SendPacket = true;

		CTX::m_SendPacket = (g_ClientState->m_nChokedCommands >= 14);

		isLag = true;
	}
}