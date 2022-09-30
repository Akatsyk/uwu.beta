#include "glow.h"

#include "../sdk/csgostructs.hpp"
#include "../config/config.h"

Glow::Glow( )
{
}

Glow::~Glow( )
{

}

void Glow::Shutdown( )
{
    for ( auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count( ); i++ ) {
        auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[ i ];
        auto entity = reinterpret_cast< C_BasePlayer* >(glowObject.m_pEntity);

        if ( glowObject.IsUnused( ) )
            continue;

        if ( !entity || entity->IsDormant( ) )
            continue;

        glowObject.m_flAlpha = 0.0f;
    }
}

void Glow::Run( )
{
    for ( auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count( ); i++ ) {
        auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[ i ];
        auto entity = reinterpret_cast< C_BasePlayer* >(glowObject.m_pEntity);

        if ( glowObject.IsUnused( ) )
            continue;

        if ( !entity || entity->IsDormant( ) )
            continue;

        auto class_id = entity->GetClientClass( )->m_ClassID;
        auto color = Color{ };

        auto is_enemy = entity->m_iTeamNum( ) != g_LocalPlayer->m_iTeamNum( );
        auto is_local = g_LocalPlayer->EntIndex( ) == entity->EntIndex( );

        switch ( class_id ) {
        case ClassId_CCSPlayer:
        {
            if ( !entity->IsAlive( ) )
                continue;

            if ( is_enemy && !g_Options.visuals.Enemy )
                continue;

            if ( is_enemy && g_Options.visuals.vis_enemy.Glow < 1 )
                continue;

            if ( is_local && !g_Options.visuals.Local )
                continue;

            if ( is_local && g_Options.visuals.vis_local.Glow < 1 )
                continue;

            if ( is_enemy ) {
                color = g_Options.visuals.colors.enemy_glow;
            }
            else if ( is_local ) {
                color = g_Options.visuals.colors.local_glow;
            }

            break;
        }
        }

        glowObject.m_flRed = color.r( ) / 255.0f;
        glowObject.m_flGreen = color.g( ) / 255.0f;
        glowObject.m_flBlue = color.b( ) / 255.0f;

        if ( is_enemy )
            glowObject.m_flAlpha = (g_Options.visuals.vis_enemy.Glow * 2.55) / 255.0f;

        else if ( is_local )
            glowObject.m_flAlpha = (g_Options.visuals.vis_local.Glow * 2.55) / 255.0f;

        glowObject.m_bRenderWhenOccluded = true;
        glowObject.m_bRenderWhenUnoccluded = false;
    }
}