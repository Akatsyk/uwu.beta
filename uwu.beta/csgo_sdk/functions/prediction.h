#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/singleton.hpp"

#define ZERO Vector(0.0f, 0.0f, 0.0f)
#define ZEROQ QAngle(0.0f, 0.0f, 0.0f)

enum Prediction_stage
{
	SETUP,
	PREDICT,
	FINISH
};

class Engine_Prediction : public Singleton<Engine_Prediction>
{
	friend class Singleton<Engine_Prediction>;

	float _curtime_backup;
	float _frametime_backup;
	CMoveData _movedata;
	CUserCmd* _prevcmd;
	int _fixedtick;

	int32_t* _prediction_seed;
	C_BasePlayer*** _prediction_player;

public:

	struct Netvars_data
	{
		int tickbase = INT_MIN;

		QAngle m_aimPunchAngle = ZEROQ;
		Vector m_aimPunchAngleVel = ZERO;
		QAngle m_viewPunchAngle = ZEROQ;
		Vector m_vecViewOffset = ZERO;
	};

	struct Viewmodel_data
	{
		C_BaseCombatWeapon* weapon = nullptr;

		int viewmodel_index = 0;
		int sequence = 0;
		int animation_parity = 0;

		float cycle = 0.0f;
		float animation_time = 0.0f;
	};

public:

	Netvars_data netvars_data[ MULTIPLAYER_BACKUP ];
	Viewmodel_data viewmodel_data;

	void store_netvars( );
	void restore_netvars( );

	void UpdateVelocity( );

	void Begin( CUserCmd* cmd )
	{
		_curtime_backup = g_GlobalVars->curtime;
		_frametime_backup = g_GlobalVars->frametime;

		if ( !_prediction_seed || !_prediction_player )
		{
			auto client = GetModuleHandle( TEXT( "client.dll" ) );
			_prediction_seed = *( int32_t** )(Utils::PatternScan( client, "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04" ) + 0x2);
			_prediction_player = ( C_BasePlayer*** )(Utils::PatternScan( client, "89 35 ? ? ? ? F3 0F 10 48 20" ) + 0x2);
		}

		if ( g_ClientState->m_nDeltaTick > 0 )
			g_Prediction->Update( g_ClientState->m_nDeltaTick, true, g_ClientState->m_nLastCommandAck,
				g_ClientState->m_nLastOutgoingCommand + g_ClientState->m_nChokedCommands );

		if ( _prediction_seed )
			*_prediction_seed = MD5_PseudoRandom( cmd->command_number ) & 0x7FFFFFFF;

		if ( _prediction_player )
			**_prediction_player = g_LocalPlayer;

		g_LocalPlayer->m_pCurrentCommand( ) = cmd;

		g_GlobalVars->curtime = TICKS_TO_TIME( CSGO::m_TicksAllowed );
		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

		bool _inpred_backup = *( bool* )(( uintptr_t )g_Prediction + 0x8);
		bool _in_backup = g_Prediction->IsFirstTimePredicted;

		memset( &_movedata, 0, sizeof( CMoveData ) );

		*( bool* )(( uintptr_t )g_Prediction + 0x8) = true;
		g_Prediction->IsFirstTimePredicted = false;

		g_MoveHelper->SetHost( g_LocalPlayer );
		g_GameMovement->StartTrackPredictionErrors( g_LocalPlayer );
		g_Prediction->SetupMove( g_LocalPlayer, cmd, g_MoveHelper, &_movedata );
		g_GameMovement->ProcessMovement( g_LocalPlayer, &_movedata );
		g_Prediction->FinishMove( g_LocalPlayer, cmd, &_movedata );
		g_GameMovement->FinishTrackPredictionErrors( g_LocalPlayer );
		g_MoveHelper->SetHost( nullptr );

		*( bool* )(( uintptr_t )g_Prediction + 0x8) = _inpred_backup;
		g_Prediction->IsFirstTimePredicted = _in_backup;

		auto viewmodel = g_LocalPlayer->m_hViewModel( ).Get( );

		if ( viewmodel )
		{
			viewmodel_data.weapon = viewmodel->m_hWeapon( ).Get( );

			viewmodel_data.viewmodel_index = viewmodel->m_nViewModelIndex( );
			viewmodel_data.sequence = viewmodel->m_nSequenceVar( );
			viewmodel_data.animation_parity = viewmodel->m_nAnimationParity( );

			viewmodel_data.cycle = viewmodel->m_flCycleData( );
			viewmodel_data.animation_time = viewmodel->m_flAnimTimeData( );
		}
	}

	void End( )
	{
		if ( _prediction_player )
			**_prediction_player = nullptr;

		if ( _prediction_seed )
			*_prediction_seed = -1;

		g_GlobalVars->curtime = _curtime_backup;
		g_GlobalVars->frametime = _frametime_backup;
	}
};