#include "prediction.h"

void Engine_Prediction::store_netvars( )
{
	auto data = &netvars_data[ g_ClientState->m_nCommandAck % MULTIPLAYER_BACKUP ];

	data->tickbase = g_LocalPlayer->m_nTickBase( ); //-V807
	data->m_aimPunchAngle = g_LocalPlayer->m_aimPunchAngle( );
	data->m_aimPunchAngleVel = g_LocalPlayer->m_aimPunchAngleVel( );
	data->m_viewPunchAngle = g_LocalPlayer->m_viewPunchAngle( );
	data->m_vecViewOffset = g_LocalPlayer->m_vecViewOffset( );
}

void Engine_Prediction::restore_netvars( )
{
	auto data = &netvars_data[ (g_ClientState->m_nCommandAck - 1) % MULTIPLAYER_BACKUP ];

	if ( data->tickbase != g_LocalPlayer->m_nTickBase( ) )
		return;

	auto aim_punch_angle_delta = g_LocalPlayer->m_aimPunchAngle( ) - data->m_aimPunchAngle;
	auto aim_punch_angle_vel_delta = g_LocalPlayer->m_aimPunchAngleVel( ) - data->m_aimPunchAngleVel;
	auto view_punch_angle_delta = g_LocalPlayer->m_viewPunchAngle( ) - data->m_viewPunchAngle;
	auto view_offset_delta = g_LocalPlayer->m_vecViewOffset( ) - data->m_vecViewOffset;

	if ( fabs( aim_punch_angle_delta.pitch ) < 0.03125f && fabs( aim_punch_angle_delta.yaw ) < 0.03125f && fabs( aim_punch_angle_delta.roll ) < 0.03125f )
		g_LocalPlayer->m_aimPunchAngle( ) = data->m_aimPunchAngle;

	if ( fabs( aim_punch_angle_vel_delta.x ) < 0.03125f && fabs( aim_punch_angle_vel_delta.y ) < 0.03125f && fabs( aim_punch_angle_vel_delta.z ) < 0.03125f )
		g_LocalPlayer->m_aimPunchAngleVel( ) = data->m_aimPunchAngleVel;

	if ( fabs( view_punch_angle_delta.pitch ) < 0.03125f && fabs( view_punch_angle_delta.yaw ) < 0.03125f && fabs( view_punch_angle_delta.roll ) < 0.03125f )
		g_LocalPlayer->m_viewPunchAngle( ) = data->m_viewPunchAngle;

	if ( fabs( view_offset_delta.x ) < 0.03125f && fabs( view_offset_delta.y ) < 0.03125f && fabs( view_offset_delta.z ) < 0.03125f )
		g_LocalPlayer->m_vecViewOffset( ) = data->m_vecViewOffset;
}

void Engine_Prediction::UpdateVelocity( )
{
	static int m_iLastCmdAck = 0;
	static float m_flNextCmdTime = 0.f;

	if ( g_ClientState && (m_iLastCmdAck != g_ClientState->m_nLastCommandAck || m_flNextCmdTime != g_ClientState->m_flNextCmdTime) )
	{
		if ( m_VelocityMod != g_LocalPlayer->m_bVelMod( ) )
		{
			*reinterpret_cast< int* >(reinterpret_cast< uintptr_t >(g_Prediction + 0x24)) = 1;
			m_VelocityMod = g_LocalPlayer->m_bVelMod( );
		}

		m_iLastCmdAck = g_ClientState->m_nLastCommandAck;
		m_flNextCmdTime = g_ClientState->m_flNextCmdTime;
	}
}