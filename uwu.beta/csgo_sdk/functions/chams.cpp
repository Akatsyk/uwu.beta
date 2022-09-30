#include "chams.h"
#include "../config/config.h"
#include "lag_comp.h"

Chams::Chams( )
{
	std::ofstream( "csgo\\materials\\metallic.vmt" ) << R"#("VertexLitGeneric" 
	{ 
	"$basetexture" "vgui/white_additive" 
	"$envmap" "editor/cube_vertigo" 
	"$normalmapalphaenvmapmask" "1" 
	"$envmapcontrast" "1" 
	"$reflectivity" "[1.0 1.0 1.0]" 
	"$nofog" "1" 
	"$model" "1" 
	"$nocull" "0" 
	"$selfillum" "1" 
	"$halflambert" "1" 
	"$znearer" "0" 
	"$flat" "1" 
	} 
	)#";

	std::ofstream( "csgo/materials/glowOverlay.vmt" ) << R"#("VertexLitGeneric" 
	{
	"$additive" "1"
	"$envmap" "models/effects/cube_white"
	"$envmaptint" "[1 1 1]"
	"$envmapfresnel" "1"
	"$envmapfresnelminmaxexp" "[0 1 2]"
	"$alpha" "1.0"
	})#";

	this->material_circuit = g_MatSystem->FindMaterial( "dev/glow_armsrace", TEXTURE_GROUP_MODEL );
	this->material_default = g_MatSystem->FindMaterial( "debug/debugambientcube", TEXTURE_GROUP_MODEL );
	this->material_flat = g_MatSystem->FindMaterial( "debug/debugdrawflat", TEXTURE_GROUP_MODEL );
	this->material_glow = g_MatSystem->FindMaterial( "glowOverlay", TEXTURE_GROUP_MODEL );
	this->material_metallic = g_MatSystem->FindMaterial( "metallic", TEXTURE_GROUP_MODEL );

	Smoke_Materials.material_1 = g_MatSystem->FindMaterial( "particle/vistasmokev1/vistasmokev1_emods", TEXTURE_GROUP_OTHER );
	Smoke_Materials.material_2 = g_MatSystem->FindMaterial( "particle/vistasmokev1/vistasmokev1_emods_impactdust", TEXTURE_GROUP_OTHER );
	Smoke_Materials.material_3 = g_MatSystem->FindMaterial( "particle/vistasmokev1/vistasmokev1_smokegrenade", TEXTURE_GROUP_OTHER );
	Smoke_Materials.material_4 = g_MatSystem->FindMaterial( "particle/vistasmokev1/vistasmokev1_fire", TEXTURE_GROUP_OTHER );
}

Chams::~Chams( )
{

}

void Chams::OnDrawModelExecute( IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix )
{
	typedef void( __thiscall* m_Proto )(IVModelRender*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>( index::DrawModelExecute );

	const auto mdl = info.pModel;
	bool is_player = strstr( mdl->szName, "models/player" ) != nullptr;

	auto player = C_BasePlayer::GetPlayerByIndex( info.entity_index );
	bool isIn = g_Options.visuals.Active && g_Options.visuals.removals.smoke;

	if ( g_LocalPlayer && Smoke_Materials.material_1 && Smoke_Materials.material_2 &&
		Smoke_Materials.material_3 && Smoke_Materials.material_4 ) {

		Smoke_Materials.material_1->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, isIn );
		Smoke_Materials.material_2->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, isIn );
		Smoke_Materials.material_3->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, isIn );
		Smoke_Materials.material_4->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, isIn );
	}

	static auto SmokeCount = *( int* )(Utils::FindSig( "client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0" ) + 8);
	if ( isIn )
		*( int* )SmokeCount = 0;

	if ( !g_Options.visuals.Active )
		return;

	if ( g_Options.visuals.vis_local.Desync_Material > 0 && g_Options.visuals.Local && player && player->IsAlive( ) )
	{
		if ( player->EntIndex( ) == g_LocalPlayer->EntIndex( ) )
		{
			if ( LagComp::Get( ).fake_matrix )
			{
				for ( auto& i : LagComp::Get( ).fake_matrix )
				{
					i[ 0 ][ 3 ] += info.origin.x;
					i[ 1 ][ 3 ] += info.origin.y;
					i[ 2 ][ 3 ] += info.origin.z;
				}

				OverrideMaterial( false, false, g_Options.visuals.colors.desync_chams, g_Options.visuals.vis_local.Desync_Material == 2 ? this->material_flat : this->material_default, g_Options.visuals.colors.desync_chams.a( ) );
				fnDME( g_MdlRender, 0, ctx, state, info, LagComp::Get( ).fake_matrix );

				if ( g_Options.visuals.vis_local.Desync_Illuminate > 0 )
				{
					OverrideMaterial( false, true, g_Options.visuals.colors.desync_illuminate, g_Options.visuals.vis_local.Desync_Illuminate == 2 ? this->material_glow : this->material_circuit, g_Options.visuals.colors.desync_illuminate.a( ) );
					fnDME( g_MdlRender, 0, ctx, state, info, LagComp::Get( ).fake_matrix );
				}

				g_MdlRender->ForcedMaterialOverride( nullptr );

				for ( auto& i : LagComp::Get( ).fake_matrix )
				{
					i[ 0 ][ 3 ] -= info.origin.x;
					i[ 1 ][ 3 ] -= info.origin.y;
					i[ 2 ][ 3 ] -= info.origin.z;
				}
			}
		}
	}

	if ( player && player->IsAlive( ) && !player->IsDormant( ) && is_player )
	{
		if ( g_Options.visuals.vis_enemy.Material > 0 && g_Options.visuals.Enemy && player->m_iTeamNum( ) != g_LocalPlayer->m_iTeamNum( ) )
		{
			if ( !g_Options.visuals.vis_enemy.Visibility )
			{
				OverrideMaterial( true, false, g_Options.visuals.colors.enemy_chams_xqz, g_Options.visuals.vis_enemy.Material == 2 ? this->material_flat : this->material_default, g_Options.visuals.colors.enemy_chams_xqz.a( ) );
				fnDME( g_MdlRender, 0, ctx, state, info, matrix );
			}

			OverrideMaterial( false, false, g_Options.visuals.colors.enemy_chams, g_Options.visuals.vis_enemy.Material == 2 ? this->material_flat : this->material_default, g_Options.visuals.colors.enemy_chams.a( ) );
			fnDME( g_MdlRender, 0, ctx, state, info, matrix );

			if ( g_Options.visuals.vis_enemy.Illuminate > 0 )
			{
				OverrideMaterial( !g_Options.visuals.vis_enemy.Visibility, true, g_Options.visuals.colors.enemy_illuminate, g_Options.visuals.vis_enemy.Illuminate == 2 ? this->material_glow : this->material_circuit, g_Options.visuals.colors.enemy_illuminate.a( ) );
				fnDME( g_MdlRender, 0, ctx, state, info, matrix );
			}
		}
		else if ( g_LocalPlayer->IsAlive( ) && g_Options.visuals.vis_local.Material > 0 && g_Options.visuals.Local && player->EntIndex( ) == g_LocalPlayer->EntIndex( ) )
		{
			OverrideMaterial( false, false, g_Options.visuals.colors.local_chams, g_Options.visuals.vis_local.Material == 2 ? this->material_flat : this->material_default, g_Options.visuals.colors.local_chams.a( ) );
			fnDME( g_MdlRender, 0, ctx, state, info, matrix );

			if ( g_Options.visuals.vis_local.Illuminate > 0 )
			{
				OverrideMaterial( false, true, g_Options.visuals.colors.local_illuminate, g_Options.visuals.vis_local.Illuminate == 2 ? this->material_glow : this->material_circuit, g_Options.visuals.colors.local_illuminate.a( ) );
				fnDME( g_MdlRender, 0, ctx, state, info, matrix );
			}
		}
	}
}

void Chams::OverrideMaterial( bool ignoreZ, bool use_env, const Color& rgba, IMaterial* material, float alpha )
{
	if ( !material )
		return;

	if ( material->IsErrorMaterial( ) )
		return;

	material->IncrementReferenceCount( );
	material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, ignoreZ );

	if ( alpha == 255 )
		material->AlphaModulate( 255.0f / alpha );
	else
		material->AlphaModulate( alpha * 0.01f );

	if ( !use_env )
		material->ColorModulate( rgba.r( ) / 255.0f, rgba.g( ) / 255.0f, rgba.b( ) / 255.0f );
	else
	{
		bool bFound = false;
		IMaterialVar* pMatVar = material->FindVar( "$envmaptint", &bFound );
		if ( bFound )
			(*(void( __thiscall** )(int, float, float, float))(*( DWORD* )pMatVar + 0x2c))(( uintptr_t )pMatVar, rgba.r( ) / 255.0f, rgba.g( ) / 255.0f, rgba.b( ) / 255.0f);
	}

	g_MdlRender->ForcedMaterialOverride( material );
}