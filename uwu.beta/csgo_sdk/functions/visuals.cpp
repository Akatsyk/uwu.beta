#include "visuals.h"
#include "../config/config.h"

#include "../render/engine/engine_draw.h"
#include "logs.h"
#include "../render/directx render/directx.h"

#include "../gui/gui.h"
#include "tickbase.h"

#include "../sdk/mem_init.h"
#include "movement.h"

#include "skins/kit_parser.h"
#include "skins/skins.h"

int DormantAlpha[ 65 ];
float StoredCurtime[ 65 ];

bool VecIsZero( Vector m_Rec )
{
	return m_Rec.x == 0.0f && m_Rec.y == 0.0f
		&& m_Rec.z == 0.0f;
}

bool Visuals::W2S( const Vector& origin, Vector2D& screen )
{
	const auto screenTransform = [ &origin, &screen ]( ) -> bool
	{
		static std::uintptr_t pViewMatrix;
		if ( !pViewMatrix )
		{
			pViewMatrix = static_cast< std::uintptr_t >(Utils::FindSig( "client.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9" ));
			pViewMatrix += 3;
			pViewMatrix = *reinterpret_cast< std::uintptr_t* >(pViewMatrix);
			pViewMatrix += 176;
		}

		const VMatrix& w2sMatrix = *reinterpret_cast< VMatrix* >(pViewMatrix);
		screen.x = w2sMatrix.m[ 0 ][ 0 ] * origin.x + w2sMatrix.m[ 0 ][ 1 ] * origin.y + w2sMatrix.m[ 0 ][ 2 ] * origin.z + w2sMatrix.m[ 0 ][ 3 ];
		screen.y = w2sMatrix.m[ 1 ][ 0 ] * origin.x + w2sMatrix.m[ 1 ][ 1 ] * origin.y + w2sMatrix.m[ 1 ][ 2 ] * origin.z + w2sMatrix.m[ 1 ][ 3 ];

		float w = w2sMatrix.m[ 3 ][ 0 ] * origin.x + w2sMatrix.m[ 3 ][ 1 ] * origin.y + w2sMatrix.m[ 3 ][ 2 ] * origin.z + w2sMatrix.m[ 3 ][ 3 ];

		if ( w < 0.001f )
		{
			screen.x *= 100000;
			screen.y *= 100000;
			return true;
		}

		float invw = 1.f / w;
		screen.x *= invw;
		screen.y *= invw;

		return false;
	};

	if ( !screenTransform( ) )
	{
		int iScreenWidth, iScreenHeight;
		g_EngineClient->GetScreenSize( iScreenWidth, iScreenHeight );

		screen.x = (iScreenWidth * 0.5f) + (screen.x * iScreenWidth) * 0.5f;
		screen.y = (iScreenHeight * 0.5f) - (screen.y * iScreenHeight) * 0.5f;

		return true;
	}

	return false;
}

class RadarPlayer_t
{
public:
	Vector pos;
	Vector angle;
	Vector spotted_map_angle_related;
	DWORD tab_related;
	char pad_0x0028[ 0xC ];
	float spotted_time;
	float spotted_fraction;
	float time;
	char pad_0x0040[ 0x4 ];
	__int32 player_index;
	__int32 entity_index;
	char pad_0x004C[ 0x4 ];
	__int32 health;
	char name[ 32 ];
	char pad_0x0074[ 0x75 ];
	unsigned char spotted;
	char pad_0x00EA[ 0x8A ];
};

class CCSGO_HudRadar
{
public:
	char pad_0x0000[ 0x14C ];
	RadarPlayer_t radar_info[ 65 ];
};

void Visuals::Run( )
{
	if ( !g_LocalPlayer || !g_EngineClient->IsConnected( ) || !g_EngineClient->IsInGame( ) )
		return;

	DormantEsp::Get( ).start( );
	RunWorld( );

	static auto FindHudElement = (DWORD( __thiscall* )(void*, const char*))Utils::FindSig( "client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28" );
	static auto hud_ptr = *( DWORD** )(Utils::FindSig( "client.dll", "81 25 ? ? ? ? ? ? ? ? 8B 01" ) + 0x2);

	auto radar_base = FindHudElement( hud_ptr, "CCSGO_HudRadar" );
	auto hud_radar = ( CCSGO_HudRadar* )(radar_base - 0x14);

	if ( g_Options.visuals.Enemy )
	{
		for ( int i = 1; i < g_GlobalVars->maxClients; ++i )
		{
			C_BasePlayer* e = reinterpret_cast< C_BasePlayer* > (g_EntityList->GetClientEntity( i ));

			if ( !e || !e->IsAlive( ) || e->GetClientClass( )->m_ClassID != 40 )
				continue;

			if ( e->EntIndex( ) == g_LocalPlayer->EntIndex( ) )
				continue;

			if ( e->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) )
				continue;

			auto valid_dormant = false;
			auto backup_flags = e->m_fFlags( );
			auto backup_origin = e->GetAbsOrigin( );

			if ( e->IsDormant( ) )
				valid_dormant = DormantEsp::Get( ).adjust_sound( e );
			else
			{
				health[ i ] = e->m_iHealth( );
				DormantEsp::Get( ).m_cSoundPlayers[ i ].reset( true, e->GetAbsOrigin( ), e->m_fFlags( ) );
			}

			if ( radar_base && hud_radar && e->IsDormant( ) && e->m_bSpotted( ) )
				health[ i ] = hud_radar->radar_info[ i ].health;

			if ( !health[ i ] )
			{
				if ( e->IsDormant( ) )
				{
					e->m_fFlags( ) = backup_flags;
					e->SetAbsOrig( backup_origin );
				}

				continue;
			}

			auto fast = 2.5f * g_GlobalVars->frametime;
			auto slow = 0.25f * g_GlobalVars->frametime;

			if ( e->IsDormant( ) )
			{
				auto origin = e->GetAbsOrigin( );

				if ( origin.IsZero( ) )
					esp_alpha_fade[ i ] = 0.0f;
				else if ( !valid_dormant && esp_alpha_fade[ i ] > 0.0f )
					esp_alpha_fade[ i ] -= slow;
				else if ( valid_dormant && esp_alpha_fade[ i ] < 1.0f )
					esp_alpha_fade[ i ] += fast;
			}
			else if ( esp_alpha_fade[ i ] < 1.0f )
				esp_alpha_fade[ i ] += fast;

			esp_alpha_fade[ i ] = Math::clamp( esp_alpha_fade[ i ], 0.0f, 1.0f );

			BoundBox( e );

			if ( bbox.bottom == 0 )
				continue;

			Box( e );

			if ( g_Options.visuals.vis_enemy.HP )
				Health( e );

			if ( g_Options.visuals.vis_enemy.Name )
				Name( e );

			if ( g_Options.visuals.vis_enemy.Ammo )
				Ammo( e );

			if ( g_Options.visuals.vis_enemy.Weapon[ 0 ] )
				Weapon( e );

			if ( g_Options.visuals.vis_enemy.Weapon[ 1 ] )
				Icon( e );

			if (g_Options.visuals.vis_enemy.OOF)
				OOF(e);

			if ( e->IsDormant( ) )
			{
				e->m_fFlags( ) = backup_flags;
				e->SetAbsOrig( backup_origin );
			}
		}
	}

	if ( g_Options.visuals.removals.scope )
	{
		auto m_weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );

		if ( !m_weapon )
			return;

		auto id = m_weapon->m_Item( ).m_iItemDefinitionIndex( );

		if ( id == WEAPON_G3SG1 || id == WEAPON_SCAR20 || id == WEAPON_SSG08 || id == WEAPON_AWP )
		{
			int w, h = 0;
			g_EngineClient->GetScreenSize( w, h );

			if ( g_LocalPlayer->m_bIsScoped( ) )
			{
				EngineDraw::Get( ).Line( w / 2, 0, w / 2, h, Color( 0, 0, 0, 255 ) );
				EngineDraw::Get( ).Line( 0, h / 2, w, h / 2, Color( 0, 0, 0, 255 ) );
			}
		}
	}

	DrawAutoPeekIndicator( );

	if ( g_LocalPlayer->IsAlive( ) && g_Options.visuals.Active &&
		g_Options.visuals.view.grenade_prediction )
		Paint( );

	if ( g_LocalPlayer->IsAlive( ) && g_Options.visuals.view.DimCrosshair )
		DimensionCrosshair( );
}

VMatrix& GetBoundTrans( C_BasePlayer* m_entity )
{
	return *reinterpret_cast< VMatrix* >(( DWORD )m_entity + 0x0444);
}

void Visuals::RunWorld( )
{
	for ( int i = 1; i < g_EntityList->GetHighestEntityIndex( ); i++ )
	{
		auto e = C_BaseEntity::GetEntityByIndex( i );
		
		if ( !e )
			continue;

		auto mdl = g_MdlInfo->GetModelName( e->GetModel( ) );

		if ( e->IsPlayer( ) )
			continue;

		if ( e->IsDormant( ) )
			continue;

		auto client_class = e->GetClientClass( );

		if ( !client_class )
			continue;

		if ( g_Options.visuals.view.NadeEsp )
			GrenadeEsp( e );

		if ( e->IsWeapon( ) )
		{
			EntBoxInfoT& box = infos[ i - 1 ];

			if ( !strstr( mdl, "w_" ) )
				continue;

			if ( BoundBoxWeapon( ( C_BasePlayer* )e, box, true ) )
			{
				if ( g_LocalPlayer->IsAlive( ) )
				{
					auto dist = g_LocalPlayer->m_vecOrigin( ).DistTo( e->m_vecOrigin( ) );
					box.alpha = 255.f;

					const auto cl_dist = std::clamp( dist - 300.f, 0.f, 510.f );
					box.alpha = 255.f - cl_dist / 2;
				}
				else
					box.alpha = 255.f;

				if ( !box.alpha )
					continue;

				if ( g_Options.visuals.world_weapons.name )
					DrawNameWeapon( box );

				if ( g_Options.visuals.world_weapons.box )
					DrawBoxWPN( box );

				if ( g_Options.visuals.world_weapons.ammo )
					DrawAmmoWpn( box );
			}
		}
	}
}

void Visuals::DrawNameWeapon( EntBoxInfoT inf )
{
	auto clean_item_name = [ ]( const char* name ) -> const char* {
		if ( name[ 0 ] == 'C' )
			name++;

		auto start = strstr( name, "Weapon" );
		if ( start != nullptr )
			name = start + 6;

		return name;
	};

	std::string name = clean_item_name( inf.ent->GetClientClass( )->m_pNetworkName );
	std::transform( name.begin( ), name.end( ), name.begin( ), toupper );

	int x = inf.x;

	if ( g_Options.CustomVisualsFont.WorldNameFont == 0 )
	{
		int textWidth = EngineDraw::Get( ).GetTextSize( EngineDraw::Get( ).fonts.pixel_font, name.c_str( ) ).right;
		EngineDraw::Get( ).DrawTextA( inf.x + (inf.w / 2) - (textWidth / 2), inf.y - 13, Color( g_Options.visuals.world_weapons.name_clr.r( ), g_Options.visuals.world_weapons.name_clr.g( ), g_Options.visuals.world_weapons.name_clr.b( ), inf.alpha * 0.85 ), EngineDraw::Get( ).fonts.pixel_font, false, name.c_str( ) );
	}
	else {
		int textWidth = EngineDraw::Get( ).GetTextSize( g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.WorldNameFont - 1 ], name.c_str( ) ).right;
		EngineDraw::Get( ).DrawTextA( inf.x + (inf.w / 2) - (textWidth / 2), inf.y - 13, Color( g_Options.visuals.world_weapons.name_clr.r( ),
			g_Options.visuals.world_weapons.name_clr.g( ), g_Options.visuals.world_weapons.name_clr.b( ), inf.alpha * 0.85 ), g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.WorldNameFont - 1 ], false, name.c_str( ) );
	}
}

void Visuals::DrawBoxWPN( EntBoxInfoT inf )
{
	EngineDraw::Get( ).OutlineRect( inf.x - 1, inf.y - 1, inf.w + 2, inf.h + 2, Color( 0, 0, 0, inf.alpha * 0.85 ) );
	EngineDraw::Get( ).OutlineRect( inf.x, inf.y, inf.w, inf.h, Color( g_Options.visuals.world_weapons.box_clr.r( ), g_Options.visuals.world_weapons.box_clr.g( ), g_Options.visuals.world_weapons.box_clr.b( ), inf.alpha * 0.85 ) );
	EngineDraw::Get( ).OutlineRect( inf.x + 1, inf.y + 1, inf.w - 2, inf.h - 2, Color( 0, 0, 0, inf.alpha * 0.85 ) );
}

void Visuals::DrawAmmoWpn( EntBoxInfoT inf )
{
	if ( inf.ent->GetClientClass( )->m_ClassID == 1 || inf.ent->GetClientClass( )->m_ClassID == 46 ||
		inf.ent->GetClientClass( )->m_ClassID >= 232 && inf.ent->GetClientClass( )->m_ClassID <= 274)
	{
		std::vector<bottom_info_t> botInfos;
		auto weapon = ( C_BaseCombatWeapon* )inf.ent;

		if ( weapon )
		{
			auto m_Data = weapon->GetCSWeaponData( );

			if ( m_Data )
			{
				int maxAmmo = m_Data->iMaxClip1;
				int curAmmo = weapon->m_iClip1( );
				botInfos.push_back( { false, "", ( double )maxAmmo, ( double )curAmmo, Color( g_Options.visuals.world_weapons.ammo_clr.r( ), g_Options.visuals.world_weapons.ammo_clr.g( ), g_Options.visuals.world_weapons.ammo_clr.b( ), inf.alpha * 0.85 ) } );
			}
		}

		int yOffset = 0;

		for ( auto i : botInfos )
		{
			EngineDraw::Get( ).FilledRect( inf.x - 1, inf.y + inf.h + 2 + yOffset, inf.w + 2, 4, Color( 0, 0, 0, inf.alpha * 0.85 ) );
			EngineDraw::Get( ).FilledRect( inf.x, inf.y + inf.h + 2 + yOffset + 1, ceil( ( double )inf.w * (i.percentage / i.maxValue) ), 2, i.color );
			yOffset += 5;
		}
	}
}

bool Visuals::BoundBoxWeapon( C_BasePlayer* m_entity, EntBoxInfoT& box, bool dynamic )
{
	if ( !m_entity )
		return false;

	const VMatrix& trans = GetBoundTrans( m_entity );
	ICollideable* collision = m_entity->GetCollideable( );

	if ( !collision )
		return false;

	Vector min, max;
	min = collision->OBBMins( );
	max = collision->OBBMaxs( );

	if ( dynamic )
	{
		Vector points[ ] = { Vector( min.x, min.y, min.z ),
			Vector( min.x, max.y, min.z ),
			Vector( max.x, max.y, min.z ),
			Vector( max.x, min.y, min.z ),
			Vector( max.x, max.y, max.z ),
			Vector( min.x, max.y, max.z ),
			Vector( min.x, min.y, max.z ),
			Vector( max.x, min.y, max.z )
		};
		auto vector_transform = [ ]( const Vector in1, const VMatrix& in2 )
		{
			auto dot_product = [ ]( const Vector& v1, const float* v2 )
			{
				float ret = v1.x * v2[ 0 ] + v1.y * v2[ 1 ] + v1.z * v2[ 2 ];
				return ret;
			};
			auto out = Vector( );
			out.x = dot_product( in1, in2[ 0 ] ) + in2[ 0 ][ 3 ];
			out.y = dot_product( in1, in2[ 1 ] ) + in2[ 1 ][ 3 ];
			out.z = dot_product( in1, in2[ 2 ] ) + in2[ 2 ][ 3 ];
			return out;
		};

		Vector pointsTransformed[ 8 ];

		for ( int i = 0; i < 8; i++ )
			pointsTransformed[ i ] = vector_transform( points[ i ], trans );

		Vector pos = m_entity->m_vecOrigin( );
		Vector2D flb;
		Vector2D brt;
		Vector2D blb;
		Vector2D frt;
		Vector2D frb;
		Vector2D brb;
		Vector2D blt;
		Vector2D flt;

		if ( !W2S( pointsTransformed[ 3 ], flb ) || !W2S( pointsTransformed[ 5 ], brt )
			|| !W2S( pointsTransformed[ 0 ], blb ) || !W2S( pointsTransformed[ 4 ], frt )
			|| !W2S( pointsTransformed[ 2 ], frb ) || !W2S( pointsTransformed[ 1 ], brb )
			|| !W2S( pointsTransformed[ 6 ], blt ) || !W2S( pointsTransformed[ 7 ], flt ) )
			return false;

		Vector2D arr[ ] = { flb, brt, blb, frt, frb, brb, blt, flt };
		float left = flb.x;
		float top = flb.y;
		float right = flb.x;
		float bottom = flb.y;

		for ( int i = 1; i < 8; i++ )
		{
			if ( left > arr[ i ].x )
				left = arr[ i ].x;

			if ( top < arr[ i ].y )
				top = arr[ i ].y;

			if ( right < arr[ i ].x )
				right = arr[ i ].x;

			if ( bottom > arr[ i ].y )
				bottom = arr[ i ].y;
		}

		Vector BotCenter = Vector( right - ((right - left) / 2), top, 0 );
		Vector TopCenter = Vector( right - ((right - left) / 2), bottom, 0 );
		box.x = left;
		box.y = TopCenter.y;
		box.w = right - left;
		box.h = BotCenter.y - TopCenter.y;
	}
	else
	{
		Vector pos3D, top3D;
		Vector2D pos, top;

		pos3D = m_entity->m_vecOrigin( );
		top3D = pos3D + Vector( 0, 0, max.z );

		if ( W2S( pos3D, pos ) && W2S( top3D, top ) )
		{
			int height = (pos.y - top.y);
			int width = height / 2;
			box.x = (pos.x - width / 2);
			box.y = top.y;
			box.w = width;
			box.h = height;
		}
	}

	box.ent = m_entity;
	return true;
}

void Visuals::SkyBox( )
{
	static auto skybox = g_CVar->FindVar( "sv_skyname" );
	*( float* )(( DWORD )&skybox->m_fnChangeCallbacks + 0xC) = NULL;

	static auto backup_skybox = skybox->GetName( );
	static auto in_game = false;

	if ( !in_game && g_EngineClient->IsInGame( ) )
	{
		in_game = true;
		backup_skybox = skybox->GetName( );
	}
	else if ( !g_EngineClient->IsInGame( ) )
	{
		in_game = false;
		skybox->SetValue( backup_skybox );
	}

	auto skybox_name = backup_skybox;

	switch ( g_Options.visuals.world.sky_mode )
	{
	case 0:
		skybox_name = backup_skybox;
		break;
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	}

	if ( skybox_name != skybox->GetName( ) )
		skybox->SetValue( skybox_name );
}

void Visuals::Box( C_BasePlayer* e )
{
	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_CurAlphaNext = 155.0f * esp_alpha_fade[ e->EntIndex( ) ];

	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( g_Options.visuals.vis_enemy.Box_Clr.r( ),
		g_Options.visuals.vis_enemy.Box_Clr.g( ), g_Options.visuals.vis_enemy.Box_Clr.b( ), ( int )m_CurAlpha );

	auto m_Second = e->IsDormant( ) ? Color( 0, 0, 0, 130 ) : Color( 0,
		0, 0, ( int )m_CurAlphaNext );

	if ( e->m_iTeamNum( ) != g_LocalPlayer->m_iTeamNum( ) )
	{
		if ( g_Options.visuals.vis_enemy.Box > 0 )
			EngineDraw::Get( ).OutlineRect( bbox.left, bbox.top, bbox.right, bbox.bottom, m_Press );

		if ( g_Options.visuals.vis_enemy.Box == 2 )
		{
			EngineDraw::Get( ).OutlineRect( bbox.left + 1, bbox.top + 1, bbox.right - 2, bbox.bottom - 2, m_Second );
			EngineDraw::Get( ).OutlineRect( bbox.left - 1, bbox.top - 1, bbox.right + 2, bbox.bottom + 2, m_Second );
		}
	}
}

void Visuals::BoundBox( C_BasePlayer* player )
{
	bbox.bottom = -1;
	bbox.top = -1;
	bbox.left = -1;
	bbox.right = -1;

	Vector2D w2sBottom, w2sTop;

	W2S( player->GetAbsOrigin( ) - Vector( 0, 0, g_Options.CustomVisals.ScaleX ), w2sBottom );
	W2S( player->GetHitboxPos( 0 ) + Vector( 0, 0, g_Options.CustomVisals.ScaleY ), w2sTop );

	int Middle = w2sBottom.y - w2sTop.y;
	int Width = Middle / g_Options.CustomVisals.ScaleZ;

	bbox.bottom = Middle;
	bbox.top = w2sTop.y;
	bbox.left = w2sBottom.x - Width;
	bbox.right = Width * g_Options.CustomVisals.ScaleA;
}

void Visuals::Health( C_BasePlayer* e )
{
	float box_h = ( float )fabs( bbox.bottom - bbox.top );
	int height = (box_h * e->m_iHealth( )) / 100;
	int green = int( e->m_iHealth( ) * 2.55f );
	int red = 255 - green;

	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_CurAlphaNext = 155.0f * esp_alpha_fade[ e->EntIndex( ) ];

	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( red,
		green, 0, ( int )m_CurAlpha );

	auto m_Second = e->IsDormant( ) ? Color( 0, 0, 0, 130 ) : Color( 0,
		0, 0, ( int )m_CurAlphaNext );

	EngineDraw::Get( ).FilledRect( bbox.left - 7 - g_Options.CustomVisals.LeftSideHealthBar, 
		bbox.top - 1, 3 + g_Options.CustomVisals.SizeHealthBar, bbox.bottom + 2, m_Second );

	int pixelValue = e->m_iHealth( ) * bbox.bottom / 100;

	EngineDraw::Get( ).FilledRect( bbox.left - 6 - g_Options.CustomVisals.LeftSideHealthBar, 
		bbox.top + bbox.bottom - pixelValue, 1 + g_Options.CustomVisals.SizeHealthBar, pixelValue, m_Press );

	std::string Health = std::to_string( e->m_iHealth( ) );

	auto m_Text = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( 255,
		255, 255, ( int )m_CurAlpha );

	if ( e->m_iHealth( ) < 100 && (g_Options.visuals.vis_enemy.HP_Percent) )
	{
		if ( g_Options.CustomVisualsFont.HpNumberFont == 0 )
		{
			EngineDraw::Get( ).DrawTextA( bbox.left - 9 + g_Options.CustomVisals.HealthTextPos, bbox.top + bbox.bottom - pixelValue - 5, m_Text, EngineDraw::Get( ).fonts.pixel_font, false, Health.c_str( ) );
		}
		else
			EngineDraw::Get( ).DrawTextA( bbox.left - 9 + g_Options.CustomVisals.HealthTextPos, bbox.top + bbox.bottom - pixelValue - 5, m_Text,
				g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.HpNumberFont - 1 ], false, Health.c_str( ) );
	}
}

bool IsNonAim( C_BaseCombatWeapon* m_Weapon )
{
	if ( !m_Weapon )
		return true;

	auto idx = m_Weapon->m_iItemDefinitionIndex( );

	if ( idx == WEAPON_C4 || idx == WEAPON_HEALTHSHOT )
		return true;

	if ( m_Weapon->IsKnife( ) )
		return true;

	if ( m_Weapon->IsGrenade( ) )
		return true;

	if ( idx == WEAPON_KNIFE )
		return true;

	return false;
}

void Visuals::Ammo( C_BasePlayer* e )
{
	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_CurAlphaNext = 155.0f * esp_alpha_fade[ e->EntIndex( ) ];

	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( g_Options.visuals.vis_enemy.Ammo_Clr.r( ),
		g_Options.visuals.vis_enemy.Ammo_Clr.g( ), g_Options.visuals.vis_enemy.Ammo_Clr.b( ), ( int )m_CurAlpha );

	auto m_Second = e->IsDormant( ) ? Color( 0, 0, 0, 130 ) : Color( 0,
		0, 0, ( int )m_CurAlphaNext );

	auto weapon = e->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	auto WpnINFO = weapon->GetCSWeaponData( );

	if ( !WpnINFO )
		return;

	if ( IsNonAim( weapon ) )
		return;

	int ammo = weapon->m_iClip1( );
	float bar_width;

	if ( WpnINFO->iMaxClip1 == 0 )
		bar_width = ammo * bbox.right / 1;

	else
		bar_width = ammo * bbox.right / WpnINFO->iMaxClip1;

	auto reloading = false;
	auto animlayer = e->GetAnimOverlays( )[ 1 ];

	if ( animlayer.m_nSequence )
	{
		auto activity = e->GetSequenceActivity( animlayer.m_nSequence );

		reloading = activity == 967 && animlayer.m_flWeight;

		if ( reloading && animlayer.m_flCycle < 1.0f )
			bar_width = animlayer.m_flCycle * bbox.right;
	}

	EngineDraw::Get( ).FilledRect( bbox.left - 1, bbox.top + bbox.bottom + 3 + g_Options.CustomVisals.AmmoPos, bbox.right + 2, 3 + g_Options.CustomVisals.AmmoSize, m_Second );
	EngineDraw::Get( ).FilledRect( bbox.left, bbox.top + bbox.bottom + 4 + g_Options.CustomVisals.AmmoPos, bar_width, 1 + g_Options.CustomVisals.AmmoSize, m_Press );
}

void Visuals::Name( C_BasePlayer* e )
{
	player_info_t pInfo;
	g_EngineClient->GetPlayerInfo( e->EntIndex( ), &pInfo );

	if ( !pInfo.szName )
		return;

	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( 255,
		255, 255, ( int )m_CurAlpha );

	if ( g_Options.CustomVisualsFont.NameFont == 0 )
	{
		auto m_TextSize = EngineDraw::Get( ).GetTextSize( EngineDraw::Get( ).fonts.default_font, pInfo.szName );

		int mid = bbox.left + (bbox.right / 2);
		int _x = mid - m_TextSize.right / 2 - 1;

		if ( g_Options.visuals.vis_enemy.Background[ 0 ] )
		{
			if ( g_Options.visuals.vis_enemy.BackgroundType == 1 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 2 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + g_Options.CustomVisals.OneLineFrame, m_TextSize.right + 1, 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 3 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
		}

		EngineDraw::Get( ).DrawTextA( bbox.left + (bbox.right / 2), bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_Press, EngineDraw::Get( ).fonts.default_font, true, pInfo.szName );
	}
	else
	{
		auto m_TextSize = EngineDraw::Get( ).GetTextSize(
			g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.NameFont - 1 ], pInfo.szName );

		if ( g_Options.visuals.vis_enemy.Background[ 0 ] )
		{
			int mid = bbox.left + (bbox.right / 2);
			int _x = mid - m_TextSize.right / 2 - 1;

			if ( g_Options.visuals.vis_enemy.BackgroundType == 1 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 2 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + g_Options.CustomVisals.OneLineFrame, m_TextSize.right + 1, 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 3 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_TextSize.right + 1, m_TextSize.bottom + 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
		}

		EngineDraw::Get( ).DrawTextA( bbox.left + (bbox.right / 2), bbox.top - 13 + g_Options.CustomVisals.NamePosY, m_Press,
			g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.NameFont - 1 ], true, pInfo.szName );
	}
}

const char* Visuals::GetWeaponIC( C_BaseCombatWeapon* wep )
{
	int WeapID = wep->m_iItemDefinitionIndex( );

	switch ( WeapID )
	{
	case 500:
		return "1";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "7";
	case WEAPON_KNIFE_BUTTERFLY:
		return "8";
	case WEAPON_KNIFE_FALCHION:
		return "0";
	case WEAPON_KNIFE_FLIP:
		return "2";
	case WEAPON_KNIFE_GUT:
		return "3";
	case WEAPON_KNIFE_KARAMBIT:
		return "4";
	case WEAPON_KNIFE_M9_BAYONET:
		return "5";
	case WEAPON_KNIFE_TACTICAL:
		return "6";
	case WEAPON_KNIFE_PUSH:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG556:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	case WEAPON_CZ75A:
		return "I";
	default:
		return " ";
	}
}

std::string str_toupper( std::string s )
{
	std::transform( s.begin( ), s.end( ), s.begin( ),
		[ ]( unsigned char c ) { return toupper( c ); }
	);
	return s;
}

void Visuals::Weapon( C_BasePlayer* e )
{
	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( 255,
		255, 255, ( int )m_CurAlpha );

	auto weapon = e->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	if ( !weapon->GetCSWeaponData( ) )
		return;

	char wpn_name[ 100 ] = "";
	sprintf_s( wpn_name, "%s", str_toupper( weapon->GetCSWeaponData( )->szWeaponName ).c_str( ) + 7 );

	if ( weapon->m_Item( ).m_iItemDefinitionIndex( ) == 64 )
		strcpy_s( wpn_name, "REVOLVER" );

	int correct_enemy;

	if ( g_Options.visuals.vis_enemy.Ammo )
		correct_enemy = 9 + g_Options.CustomVisals.WeaponNamePos2;

	else
		correct_enemy = 3 + g_Options.CustomVisals.WeaponNamePos1;

	int Middle = bbox.left + (bbox.right / 2);

	if ( g_Options.CustomVisualsFont.WeaponFont == 0 )
	{
		auto m_TextSize = EngineDraw::Get( ).GetTextSize( 
			EngineDraw::Get( ).fonts.pixel_font, wpn_name );

		int _x = Middle - m_TextSize.right / 2 - 1;

		if ( g_Options.visuals.vis_enemy.Background[ 1 ] )
		{
			if ( g_Options.visuals.vis_enemy.BackgroundType == 1 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 2 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + bbox.bottom + correct_enemy 
					+ g_Options.CustomVisals.SecondLineFrame, m_TextSize.right + 2, 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 3 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackLineColor );
			}
		}

		EngineDraw::Get( ).DrawTextA( bbox.left + bbox.right / 2, bbox.top + bbox.bottom + correct_enemy, m_Press, EngineDraw::Get( ).fonts.pixel_font, true, wpn_name );
	}
	else
	{
		auto m_TextSize = EngineDraw::Get( ).GetTextSize(
			g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.WeaponFont - 1 ], wpn_name );

		int _x = Middle - m_TextSize.right / 2 - 1;

		if ( g_Options.visuals.vis_enemy.Background[ 1 ] )
		{
			if ( g_Options.visuals.vis_enemy.BackgroundType == 1 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 2 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + bbox.bottom + correct_enemy 
					+ g_Options.CustomVisals.SecondLineFrame, m_TextSize.right + 2, 1, g_Options.visuals.vis_enemy.BackLineColor );
			}
			else if ( g_Options.visuals.vis_enemy.BackgroundType == 3 ) {
				EngineDraw::Get( ).FilledRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackColor );
				EngineDraw::Get( ).OutlineRect( _x, bbox.top + bbox.bottom + correct_enemy, m_TextSize.right + 2, m_TextSize.bottom, g_Options.visuals.vis_enemy.BackLineColor );
			}
		}

		EngineDraw::Get( ).DrawTextA( bbox.left + bbox.right / 2, bbox.top + bbox.bottom + correct_enemy, m_Press, g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.WeaponFont - 1 ], true, wpn_name );
	}
}

void Visuals::Icon( C_BasePlayer* e )
{
	auto weapon = e->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	auto m_CurAlpha = 255.0f * esp_alpha_fade[ e->EntIndex( ) ];
	auto m_Press = e->IsDormant( ) ? Color( 255, 255, 255, 130 ) : Color( 255,
		255, 255, ( int )m_CurAlpha );

	int correct;

	if ( g_Options.visuals.vis_enemy.Ammo && !g_Options.visuals.vis_enemy.Weapon[ 0 ] )
		correct = 10 
		+ g_Options.CustomVisals.WeaponIcon[ VISUALS_POS::AMMO_MINUS_NAME ];

	else if ( g_Options.visuals.vis_enemy.Ammo && g_Options.visuals.vis_enemy.Weapon[ 0 ] )
		correct = 23 
		+ g_Options.CustomVisals.WeaponIcon[ VISUALS_POS::AMMO_PLUS_NAME ];

	else if ( !g_Options.visuals.vis_enemy.Ammo && g_Options.visuals.vis_enemy.Weapon[ 0 ] )
		correct = 16 
		+ g_Options.CustomVisals.WeaponIcon[ VISUALS_POS::MINUS_AMMO_NAME ];

	else
		correct = 6 
		+ g_Options.CustomVisals.WeaponIcon[ VISUALS_POS::NOTHING ];

	EngineDraw::Get( ).DrawTextA( bbox.left + (bbox.right / 2), bbox.top + bbox.bottom + correct, m_Press, EngineDraw::Get( ).fonts.weapon_font, true, GetWeaponIC( weapon ) );
}

void Rotate( std::array< Vector2D, 3 >& points, float rotation )
{
	const auto points_center = (points.at( 0 ) + points.at( 1 ) + points.at( 2 )) / 3;
	for ( auto& point : points ) {
		point -= points_center;

		const auto temp_x = point.x;
		const auto temp_y = point.y;

		const auto theta = DEG2RAD( rotation );
		const auto c = cos( theta );
		const auto s = sin( theta );

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point += points_center;
	}
}

void Visuals::OOF( C_BasePlayer* player )
{
	if ( player->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) )
		return;

	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	if ( player->IsDormant( ) )
		return;

	Vector position;

	if ( Math::WorldToScreen( player->GetHitboxPos( 2 ), position ) )
		return;

	QAngle viewangles;
	int width, height;

	g_EngineClient->GetViewAngles( &viewangles );
	g_EngineClient->GetScreenSize( width, height );

	const auto screen_center = Vector2D( width * .5f, height * .5f );
	const auto angle_yaw_rad = DEG2RAD( viewangles.yaw - Math::CalcAngle( g_LocalPlayer->GetEyePos( ), player->GetHitboxPos( 2 ) ).yaw - 90 );

	int radius = g_Options.visuals.vis_enemy.OOF_dist;
	int size = g_Options.visuals.vis_enemy.OOF_size;

	const auto new_point_x = screen_center.x + ((((width - (size * 3)) * .5f) * (radius / 100.0f)) * cos( angle_yaw_rad )) + ( int )(6.0f * ((( float )size - 4.f) / 16.0f));
	const auto new_point_y = screen_center.y + ((((height - (size * 3)) * .5f) * (radius / 100.0f)) * sin( angle_yaw_rad ));

	Color esp = g_Options.visuals.vis_enemy.OOF_Clr;

	std::array< Vector2D, 3 >points{ Vector2D( new_point_x - size, new_point_y - size ),
		Vector2D( new_point_x + size, new_point_y ),
		Vector2D( new_point_x - size, new_point_y + size ) };

	Rotate( points, viewangles.yaw - Math::CalcAngle( g_LocalPlayer->GetEyePos( ), player->GetHitboxPos( 2 ) ).yaw - 90 );
	EngineDraw::Get( ).DrawTriangle( points, esp );
}

void UtilTraceHull( const Vector& vec_start, const Vector& vec_end, const unsigned int n_mask, const Vector& extents, trace_t* p_trace )
{
	CTraceFilterWorldAndPropsOnly Filter;

	Ray_t ray;
	ray.Init( vec_start, vec_end );
	ray.m_Extents = extents;
	ray.m_IsRay = false;

	g_EngineTrace->TraceRay( ray, n_mask, &Filter, p_trace );
}

void Visuals::ThirdPerson( ) {

	if ( !g_Options.visuals.misc_.thirdperson ) {
		return;
	}

	if ( !g_LocalPlayer->m_hActiveWeapon( ).Get( ) )
		return;

	if ( !g_LocalPlayer->m_hActiveWeapon( ).Get( )->GetCSWeaponData( ) )
		return;

	if ( GetKeyState( g_Options.visuals.misc_.key_tp ) && g_LocalPlayer->IsAlive( ) )
	{
		float dist = g_Options.visuals.misc_.tp_dist;

		QAngle* view = g_LocalPlayer->GetVAngles( );
		trace_t tr;
		Ray_t ray;

		Vector desiredCamOffset = Vector( cos( DEG2RAD( view->yaw ) ) * dist,
			sin( DEG2RAD( view->yaw ) ) * dist,
			sin( DEG2RAD( -view->pitch ) ) * dist
		);

		ray.Init( g_LocalPlayer->GetEyePos( ), (g_LocalPlayer->GetEyePos( ) - desiredCamOffset) );
		CTraceFilter traceFilter;
		traceFilter.pSkip = g_LocalPlayer;
		g_EngineTrace->TraceRay( ray, MASK_SHOT, &traceFilter, &tr );

		Vector diff = g_LocalPlayer->GetEyePos( ) - tr.endpos;

		float distance2D = sqrt( abs( diff.x * diff.x ) + abs( diff.y * diff.y ) );

		bool horOK = distance2D > (dist - 2.0f);
		bool vertOK = (abs( diff.z ) - abs( desiredCamOffset.z ) < 3.0f);

		float cameraDistance;

		if ( horOK && vertOK )
		{
			cameraDistance = dist;
		}
		else
		{
			if ( vertOK )
			{
				cameraDistance = distance2D * 0.95f;
			}
			else
			{
				cameraDistance = abs( diff.z ) * 0.95f;
			}
		}

		g_Input->bCameraInThirdPerson = true;
		g_Input->vecCameraOffset.z = cameraDistance;
	}
	else
	{
		g_Input->bCameraInThirdPerson = false;
	}
}

void Visuals::Spread( IDirect3DDevice9* m_device )
{
	if ( !g_Options.visuals.Active )
		return;

	if ( !g_LocalPlayer || !g_EngineClient->IsConnected( ) || !g_EngineClient->IsInGame( ) )
		return;

	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	if ( !g_Options.visuals.misc_.SpreadCross )
		return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	static float rot = 0.f;
	int w, h;

	g_EngineClient->GetScreenSize( w, h );

	w /= 2, h /= 2;

	int r, g, b;
	r = g_Options.visuals.misc_.Spread_Clr.r( );
	g = g_Options.visuals.misc_.Spread_Clr.g( );
	b = g_Options.visuals.misc_.Spread_Clr.b( );

	switch ( g_Options.visuals.misc_.m_CrossType ) {
	case 0:
		DrawList.CircleDualColor( w, h, weapon->GetInaccuracy( ) * 500.0f, 0, 1, 50, D3DCOLOR_RGBA( r, g, b, g_Options.visuals.misc_.Spread_Clr.a( ) ), D3DCOLOR_RGBA( 0, 0, 0, 0 ), m_device );
		break;
	case 1:
		DrawList.CircleDualColor( w, h, weapon->GetInaccuracy( ) * 500.0f, rot, 1, 50, m_device );
		break;
	}

	rot += 0.5f;
	if ( rot > 360.f )
		rot = 0.f;
}

struct Color ConverDefault( int hexValue )
{
	Color rgbColor = Color
	(
		((hexValue) & 0xFF) / 255, // r
		((hexValue >> 8) & 0xFF) / 255, // g
		((hexValue >> 16) & 0xFF) / 255 // b
	);

	return rgbColor;
}

void Visuals::DrawWatermark( )
{
	if ( !g_VGuiSurface )
		return;

	int width, height;
	g_EngineClient->GetScreenSize( width, height );

	EngineDraw::Get( ).Gradient( width - 100, 0, 100, 30, Color( 0, 0, 0, 0 ), Color( 0, 0, 0, 255 ), GradientType::GRADIENT_HORIZONTAL );
	EngineDraw::Get( ).Gradient( width - 100, 30, 100, 1, Color( 0, 0, 0, 0 ), Gui::m_Details.m_DefaultColor, GradientType::GRADIENT_HORIZONTAL );
	EngineDraw::Get( ).DrawTextA( width - 85, 9, Color::White, EngineDraw::Get( ).fonts.indicators_font, false, "uwu framework" );
}

void Visuals::DrawSpectatorList( )
{
	// todo
}

void Visuals::DrawAutoPeekIndicator( )
{
	if ( !g_LocalPlayer->IsAlive( ) )
		return;

	if ( !(g_Options.visuals.misc_.auto_peek && GetKeyState( g_Options.visuals.misc_.peek_key )) )
		return;

	auto m_Weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );

	if ( !m_Weapon )
		return;

	bool m_NonAim = m_Weapon->IsKnifeIDX( ) || m_Weapon->IsGrenadeIDX( )
		|| m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_HEALTHSHOT || m_Weapon->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_TASER;

	if ( m_NonAim )
		return;

	static auto position = Vector( 0, 0, 0 );

	if ( !VecIsZero( CMovement::Get( ).m_StartPos ) )
		position = CMovement::Get( ).m_StartPos;

	if ( VecIsZero( position ) )
		return;

	static auto prevScreenPos = Vector( 0, 0, 0 );

	auto step = M_PI * 2.0f / 72.0f;
	auto screenPos = Vector( 0, 0, 0 );

	static float hue_offset = 0;

	for ( auto rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step ) //-V1034
	{
		Vector pos( 20 * cos( rotation ) + position.x, 20 * sin( rotation ) + position.y, position.z );

		if ( Math::WorldToScreen( pos, screenPos ) )
		{
			int hue = RAD2DEG( rotation ) + hue_offset;
			Color color = Color::FromHSB( g_Options.visuals.misc_.peek_hue / 255, hue / 360.f,
				1 );

			if ( !VecIsZero( prevScreenPos ) )
				EngineDraw::Get( ).Line( prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color );

			prevScreenPos = screenPos;
		}
	}

	hue_offset -= 0.5;
}

void Visuals::DrawKeyBinds( )
{
	int width, height;
	g_EngineClient->GetScreenSize( width, height );

	std::vector<std::string> m_Binds;

	if ( g_Options.ragebot.Acitve && GetKeyState( g_Options.exploit.mExpltKey ) )
		m_Binds.push_back( "exploit" );

	if ( g_Options.ragebot.Acitve && g_Options.ragebot.ForceBaim && GetKeyState( g_Options.ragebot.BaimKey ) )
		m_Binds.push_back( "force body" );

	if ( g_Options.ragebot.Acitve && g_Options.ragebot.PreferSafepoint && GetKeyState( g_Options.ragebot.SafeKey ) )
		m_Binds.push_back( "optimal point" );

	if ( g_Options.ragebot.Acitve && GetKeyState( g_Options.antiaim.KeyInvert ) )
		m_Binds.push_back( "invert" );

	if ( g_Options.movement.fakeduck && GetAsyncKeyState( g_Options.movement.m_FdKey ) )
		m_Binds.push_back( "fake duck" );

	if ( g_Options.movement.slowwalk && GetAsyncKeyState( g_Options.movement.m_SlowKey ) )
		m_Binds.push_back( "slow walk" );

	if ( g_Options.visuals.misc_.auto_peek && GetKeyState( g_Options.visuals.misc_.peek_key ) )
		m_Binds.push_back( "auto peek" );

	if ( g_Options.visuals.misc_.thirdperson && g_Options.visuals.Active && GetKeyState( g_Options.visuals.misc_.key_tp ) )
		m_Binds.push_back( "thirdperson" );

	if ( m_Binds.empty( ) )
		return;

	static int m_Value = 0;

	for ( size_t i = 0; i < m_Binds.size( ); i++ ) {
		m_Value = i + 1;
	}

	EngineDraw::Get( ).Gradient( 0, height / 2, 100, 20 * m_Value, Color( 0, 0, 0, 255 ), Color( 0, 0, 0, 0 ), GradientType::GRADIENT_HORIZONTAL );
	EngineDraw::Get( ).Gradient( 0, (height / 2) + (20 * m_Value), 100, 1, Gui::m_Details.m_DefaultColor, Color( 0, 0, 0, 0 ), GradientType::GRADIENT_HORIZONTAL );

	for ( size_t i = 0; i < m_Binds.size( ); i++ ) {
		EngineDraw::Get( ).DrawTextA( 10, ((height / 2) + 3) + 19 * i, Color::White,
			EngineDraw::Get( ).fonts.indicators_font, false, m_Binds.at( i ).c_str( ) );
	}
}

bool IsPlayerValid( C_BasePlayer* pl, bool check_team, bool check_dormant )
{
	if ( !pl )
		return false;

	if ( !g_LocalPlayer )
		return false;

	if ( pl->GetClientClass( )->m_ClassID != 40 )
		return false;

	if ( !pl->IsAlive( ) )
		return false;

	if ( pl->IsDormant( ) && check_dormant )
		return false;

	if ( check_team && g_LocalPlayer->m_iTeamNum( ) == pl->m_iTeamNum( ) )
		return false;

	return true;
}

void DormantEsp::start( )
{
	m_utlCurSoundList.RemoveAll( );
	g_EngineSound->GetActiveSounds( m_utlCurSoundList );

	if ( !m_utlCurSoundList.Count( ) )
		return;

	for ( auto i = 0; i < m_utlCurSoundList.Count( ); i++ )
	{
		auto& sound = m_utlCurSoundList[ i ];

		if ( sound.m_nSoundSource < 1 || sound.m_nSoundSource > 64 )
			continue;

		if ( sound.m_pOrigin->IsZero( ) )
			continue;

		if ( !valid_sound( sound ) )
			continue;

		auto player = static_cast< C_BasePlayer* >(g_EntityList->GetClientEntity( sound.m_nSoundSource ));

		if ( !IsPlayerValid( player, true, false ) )
			continue;

		setup_adjust( player, sound );
		m_cSoundPlayers[ sound.m_nSoundSource ].Override( sound );
	}

	m_utlvecSoundBuffer = m_utlCurSoundList;
}

void DormantEsp::setup_adjust( C_BasePlayer* player, SndInfo_t& sound )
{
	Vector src3D, dst3D;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	src3D = *sound.m_pOrigin + Vector( 0.0f, 0.0f, 1.0f );
	dst3D = src3D - Vector( 0.0f, 0.0f, 100.0f );

	filter.pSkip = player;
	ray.Init( src3D, dst3D );

	g_EngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

	if ( tr.allsolid )
		m_cSoundPlayers[ sound.m_nSoundSource ].m_iReceiveTime = -1;

	*sound.m_pOrigin = tr.fraction <= 0.97f ? tr.endpos : *sound.m_pOrigin;

	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags = player->m_fFlags( );
	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags |= (tr.fraction < 0.50f ? FL_DUCKING : 0) | (tr.fraction < 1.0f ? FL_ONGROUND : 0);
	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags &= (tr.fraction >= 0.50f ? ~FL_DUCKING : 0) | (tr.fraction >= 1.0f ? ~FL_ONGROUND : 0);
}

bool DormantEsp::adjust_sound( C_BasePlayer* entity )
{
	auto i = entity->EntIndex( );
	auto sound_player = m_cSoundPlayers[ i ];

	auto expired = false;

	if ( fabs( g_GlobalVars->realtime - sound_player.m_iReceiveTime ) > 10.0f )
		expired = true;

	entity->m_bSpotted( ) = true;
	entity->m_fFlags( ) = sound_player.m_nFlags;
	entity->SetAbsOrig( sound_player.m_vecOrigin );

	return !expired;
}

bool DormantEsp::valid_sound( SndInfo_t& sound )
{
	for ( auto i = 0; i < m_utlvecSoundBuffer.Count( ); i++ )
		if ( m_utlvecSoundBuffer[ i ].m_nGuid == sound.m_nGuid )
			return false;

	return true;
}

void Visuals::FogChanger( )
{
	static auto fog_override = g_CVar->FindVar( "fog_override" );
	bool m_Reason = g_Options.visuals.Active && g_Options.visuals.world.fog.Enabled;

	if ( !m_Reason )
	{
		if ( fog_override->GetBool( ) )
			fog_override->SetValue( FALSE );

		return;
	}

	if ( !fog_override->GetBool( ) )
		fog_override->SetValue( TRUE );

	static auto fog_start = g_CVar->FindVar( "fog_start" );

	if ( fog_start->GetInt( ) )
		fog_start->SetValue( 0 );

	static auto fog_end = g_CVar->FindVar( "fog_end" );

	if ( fog_end->GetInt( ) != g_Options.visuals.world.fog.Dist )
		fog_end->SetValue( g_Options.visuals.world.fog.Dist );

	static auto fog_maxdensity = g_CVar->FindVar( "fog_maxdensity" );

	if ( fog_maxdensity->GetFloat( ) != ( float )g_Options.visuals.world.fog.Density * 0.01f )
		fog_maxdensity->SetValue( ( float )g_Options.visuals.world.fog.Density * 0.01f );

	char buffer_color[ 12 ];
	sprintf_s( buffer_color, 12, "%i %i %i", g_Options.visuals.world.fog.FogClr.r( ),
		g_Options.visuals.world.fog.FogClr.g( ), g_Options.visuals.world.fog.FogClr.b( ) );

	static auto fog_color = g_CVar->FindVar( "fog_color" );

	if ( strcmp( fog_color->GetString( ), buffer_color ) )
		fog_color->SetValue( buffer_color );
}

// --------------------------------------------------------------- //

void Visuals::Paint( )
{
	if ( (type) && path.size( ) > 1 )
	{
		Vector nadeStart, nadeEnd;
		Vector nadeStart2, nadeEnd2;
		Vector prev = path[ 0 ];
		Vector prevoth = others[ 0 ].first;

		bool in_attack 
			= g_pCmd->buttons & IN_ATTACK, in_attack2 = g_pCmd->buttons & IN_ATTACK2;

		if ( !(in_attack || in_attack2) ) return;

		for ( auto it = path.begin( ), end = path.end( ); it != end; ++it )
		{
			if ( Math::WorldToScreen( prev, nadeStart ) && Math::WorldToScreen( *it, nadeEnd ) )
				EngineDraw::Get().Line( ( int )nadeStart.x, ( int )nadeStart.y, ( int )nadeEnd.x, ( int )nadeEnd.y, g_Options.visuals.view.nade_line );

			prev = *it;
		}
		for ( auto it = others.begin( ), end = others.end( ); it != end; ++it ) {

			if ( g_Options.visuals.view.corners )
			{
				if ( Math::WorldToScreen( prevoth, nadeStart2 ) && Math::WorldToScreen( it->first, nadeEnd2 ) ) {
					EngineDraw::Get( ).FilledRect( ( int )nadeStart2.x - 2, ( int )nadeStart2.y - 2, 4, 4, g_Options.visuals.view.corn_clr );
				}
			}
			prevoth = it->first;
		}
	}
}

void Visuals::Setup( Vector& vecSrc, Vector& vecThrow, Vector viewangles )
{
	Vector angThrow = viewangles;
	float pitch = angThrow.x;

	if ( pitch <= 90.0f )
	{
		if ( pitch < -90.0f )
			pitch += 360.0f;
	}
	else
		pitch -= 360.0f;

	float a = pitch - (90.0f - fabs( pitch )) * 10.0f / 90.0f;
	angThrow.x = a;

	float flVel = 750.0f * 0.9f;
	static const float power[ ] = { 1.0f, 1.0f, 0.5f, 0.0f };
	float b = power[ act ];
	b = b * 0.7f; b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	Math::AngVec( angThrow, &vForward, &vRight, &vUp );

	vecSrc = g_LocalPlayer->GetEyePos( );
	float off = (power[ act ] * 12.0f) - 12.0f;
	vecSrc.z += off;

	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f;

	TraceHull( vecSrc, vecDest, tr );

	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	vecThrow = g_LocalPlayer->m_vecVelocity( ); vecThrow *= 1.25f;
	vecThrow += vForward * flVel;
}

void Visuals::Simulate( CViewSetup* setup )
{
	Vector vecSrc, vecThrow;
	QAngle angles; g_EngineClient->GetViewAngles( &angles );
	Setup( vecSrc, vecThrow, Vector( angles.pitch, angles.yaw, angles.roll ) );

	float interval = g_GlobalVars->interval_per_tick;
	int logstep = ( int )(0.05f / interval);
	int logtimer = 0;

	path.clear( ); others.clear( );
	for ( unsigned int i = 0; i < path.max_size( ) - 1; ++i )
	{
		if ( !logtimer ) path.push_back( vecSrc );

		int s = Step( vecSrc, vecThrow, i, interval );
		if ( (s & 1) ) break;
		if ( (s & 2) || logtimer >= logstep ) logtimer = 0;
		else ++logtimer;
	}
	path.push_back( vecSrc );
}

#define PI 3.14159265358979323846f

void VectorAngles( const Vector& forward, QAngle& angles )
{
	if ( forward[ 1 ] == 0.0f && forward[ 0 ] == 0.0f )
	{
		angles[ 0 ] = (forward[ 2 ] > 0.0f) ? 270.0f : 90.0f;
		angles[ 1 ] = 0.0f;
	}
	else
	{
		angles[ 0 ] = atan2( -forward[ 2 ], forward.Length2D( ) ) * -180 / PI;
		angles[ 1 ] = atan2( forward[ 1 ], forward[ 0 ] ) * 180 / PI;

		if ( angles[ 1 ] > 90 ) angles[ 1 ] -= 180;
		else if ( angles[ 1 ] < 90 ) angles[ 1 ] += 180;
		else if ( angles[ 1 ] == 90 ) angles[ 1 ] = 0;
	}

	angles[ 2 ] = 0.0f;
}

int Visuals::Step( Vector& vecSrc, Vector& vecThrow, int tick, float interval )
{
	Vector move; AddGravityMove( move, vecThrow, interval, false );
	trace_t tr; PushEntity( vecSrc, move, tr );

	int result = 0;

	auto weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !weapon ) 
		return 0;

	if ( !weapon->IsGrenadeIDX( ) ) 
		return 0;

	if ( CheckDetonate( vecThrow, tr, tick, interval ) )
		result |= 1;

	if ( tr.fraction != 1.0f )
	{
		result |= 2;
		ResolveFlyCollisionCustom( tr, vecThrow, interval );

		QAngle angles;
		VectorAngles( (tr.endpos - tr.startpos).Normalized( ), angles );
		others.push_back( std::make_pair( tr.endpos, angles ) );
	}

	vecSrc = tr.endpos;
	return result;
}

bool Visuals::CheckDetonate( const Vector& vecThrow, const trace_t& tr, int tick, float interval )
{
	switch ( type )
	{
	case 45:
	case 47:
		if ( vecThrow.Length2D( ) < 0.1f )
		{
			int det_tick_mod = ( int )(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;

	case 46:
	case 48:
		if ( tr.fraction != 1.0f && tr.plane.normal.z > 0.7f )
			return true;

	case 43:
	case 44:
		return ( float )tick * interval > 1.5f && !(tick % ( int )(0.2f / interval));
	default:
		assert( false );
		return false;
	}
}

void Visuals::TraceHull( Vector& src, Vector& end, trace_t& tr )
{
	CTraceFilterWorldOnly filter;
	g_EngineTrace->TraceRay( Ray_t( src, end, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ) ), 0x200400B, &filter, &tr );
}

void Visuals::AddGravityMove( Vector& move, Vector& vel, float frametime, bool onground )
{
	Vector basevel( 0.0f, 0.0f, 0.0f );
	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if ( onground )
		move.z = (vel.z + basevel.z) * frametime;
	else
	{
		float gravity = 800.0f * 0.4f;
		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;
		vel.z = newZ;
	}
}

void Visuals::PushEntity( Vector& src, const Vector& move, trace_t& tr )
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	TraceHull( src, vecAbsEnd, tr );
}

void Visuals::ResolveFlyCollisionCustom( trace_t& tr, Vector& vecVelocity, float interval )
{
	float flSurfaceElasticity = 1.0, flGrenadeElasticity = 0.45f;
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if ( flTotalElasticity > 0.9f ) flTotalElasticity = 0.9f;
	if ( flTotalElasticity < 0.0f ) flTotalElasticity = 0.0f;

	Vector vecAbsVelocity;
	PhysicsClipVelocity( vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	float flSpeedSqr = vecAbsVelocity.LengthSqr( );
	static const float flMinSpeedSqr = 20.0f * 20.0f;

	if ( flSpeedSqr < flMinSpeedSqr )
	{
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	if ( tr.plane.normal.z > 0.7f )
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
		PushEntity( tr.endpos, vecAbsVelocity, tr );
	}
	else
		vecVelocity = vecAbsVelocity;
}

int Visuals::PhysicsClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce )
{
	static const float STOP_EPSILON = 0.1f;

	float backoff, change, angle;
	int   i, blocked;

	blocked = 0;
	angle = normal[ 2 ];

	if ( angle > 0 ) blocked |= 1;
	if ( !angle ) blocked |= 2;

	backoff = in.Dot( normal ) * overbounce;
	for ( i = 0; i < 3; i++ )
	{
		change = normal[ i ] * backoff;
		out[ i ] = in[ i ] - change;
		if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON )
			out[ i ] = 0;
	}
	return blocked;
}

void Visuals::Tick( int buttons )
{
	bool in_attack = buttons & IN_ATTACK, in_attack2 = buttons & IN_ATTACK2;

	act = (in_attack && in_attack2) ? 2 :
		(in_attack2) ? 3 :
		(in_attack) ? 1 :
		0;
}

void Visuals::View( CViewSetup* setup )
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !weapon ) 
		return;

	if ( weapon->IsGrenadeIDX( ) )
	{
		type = weapon->m_Item().m_iItemDefinitionIndex( );
		Simulate( setup );
	}
	else
		type = 0;
}

// -------------------------------------------------------------- //

void Visuals::GrenadeEsp( C_BaseEntity* entity )
{
	auto client_class = entity->GetClientClass( );

	if ( !client_class )
		return;

	auto model = entity->GetModel( );

	if ( !model )
		return;

	auto studio_model = g_MdlInfo->GetStudiomodel( model );

	if ( !studio_model )
		return;

	auto name = ( std::string )studio_model->szName;

	if ( name.find( "thrown" ) != std::string::npos ||
		client_class->m_ClassID == 9 || client_class->m_ClassID == 48 || client_class->m_ClassID == 114 )
	{
		auto grenade_origin = entity->GetAbsOrigin( );
		auto grenade_position = Vector( 0, 0, 0 );

		if ( !Math::WorldToScreen( grenade_origin, grenade_position ) )
			return;

		std::string grenade_name;

		if ( name.find( "flashbang" ) != std::string::npos )
		{
			grenade_name = "FLASHBANG";
		}
		else if ( name.find( "smokegrenade" ) != std::string::npos )
		{
			grenade_name = "SMOKE";
		}
		else if ( name.find( "incendiarygrenade" ) != std::string::npos )
		{
			grenade_name = "INCENDIARY";
		}
		else if ( name.find( "molotov" ) != std::string::npos )
		{
			grenade_name = "MOLOTOV";
		}
		else if ( name.find( "fraggrenade" ) != std::string::npos )
		{
			grenade_name = "HE GRENADE";
		}
		else if ( name.find( "decoy" ) != std::string::npos )
		{
			grenade_name = "DECOY";
		}
		else
			return;

		if ( g_Options.CustomVisualsFont.WorldNameFont == 0 )
		{
			EngineDraw::Get( ).DrawTextA( grenade_position.x, grenade_position.y, g_Options.visuals.view.NadeColor, 
				EngineDraw::Get( ).fonts.pixel_font, false, grenade_name.c_str( ) );
		}
		else {
			EngineDraw::Get( ).DrawTextA( grenade_position.x, grenade_position.y, g_Options.visuals.view.NadeColor, 
				g_Options.CustomFonts.m_Font[ g_Options.CustomVisualsFont.WorldNameFont - 1 ], false, grenade_name.c_str( ) );
		}
	}
}

void NormalizeInOut( Vector& vIn, Vector& vOut )
{
	float flLen = vIn.Length( );
	if ( flLen == 0 ) {
		vOut.Init( 0, 0, 1 );
		return;
	}
	flLen = 1 / flLen;
	vOut.Init( vIn.x * flLen, vIn.y * flLen, vIn.z * flLen );
}

void Visuals::DimensionCrosshair( )
{
	QAngle viewangles;
	g_EngineClient->GetViewAngles( &viewangles );
	QAngle diff = QAngle( 0, 0, 0 );
	auto tmpviewangles = viewangles + diff;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;
	Vector begin = g_LocalPlayer->GetEyePos( ), t, end;
	Math::AngleVectors( tmpviewangles, t );
	NormalizeInOut( t, end );
	end *= 8192.0f;
	end += begin;
	ray.Init( begin, end );
	g_EngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );
	EngineDraw::Get().Draw3DBox( tr.endpos, g_Options.visuals.view.DimSize, g_Options.visuals.view.DimSize, g_Options.visuals.view.DimColor );
}