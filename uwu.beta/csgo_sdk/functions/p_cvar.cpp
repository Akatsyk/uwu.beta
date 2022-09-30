#include "p_cvar.h"

void CVarSys::CallUpdate()
{
	if ( isInit )
		return;

	m_StoredVars[ "mp_damage_scale_ct_head" ] = g_CVar->FindVar( "mp_damage_scale_ct_head" );
	m_StoredVars[ "mp_damage_scale_t_head" ] = g_CVar->FindVar( "mp_damage_scale_t_head" );

	m_StoredVars[ "mp_damage_scale_ct_body" ] = g_CVar->FindVar( "mp_damage_scale_ct_body" );
	m_StoredVars[ "mp_damage_scale_t_body" ] = g_CVar->FindVar( "mp_damage_scale_t_body" );

	m_StoredVars[ "sv_penetration_type" ] = g_CVar->FindVar( "sv_penetration_type" );
	m_StoredVars[ "ff_damage_reduction_bullets" ] = g_CVar->FindVar( "ff_damage_reduction_bullets" );
	m_StoredVars[ "ff_damage_bullet_penetration" ] = g_CVar->FindVar( "ff_damage_bullet_penetration" );

	m_StoredVars[ "r_jiggle_bones" ] = g_CVar->FindVar( "r_jiggle_bones" );
	m_StoredVars[ "sv_gravity" ] = g_CVar->FindVar( "sv_gravity" );

	m_StoredVars[ "sv_jump_impulse" ] = g_CVar->FindVar( "sv_jump_impulse" );
	m_StoredVars[ "sv_maxunlag" ] = g_CVar->FindVar( "sv_maxunlag" );

	m_StoredVars[ "cl_interp" ] = g_CVar->FindVar( "cl_interp" );
	m_StoredVars[ "cl_interp_ratio" ] = g_CVar->FindVar( "cl_interp_ratio" );

	m_StoredVars[ "r_shadows" ] = g_CVar->FindVar( "r_shadows" );
	m_StoredVars[ "cl_csm_shadows" ] = g_CVar->FindVar( "cl_csm_shadows" );
	m_StoredVars[ "cl_foot_contact_shadows" ] = g_CVar->FindVar( "cl_foot_contact_shadows" );
	m_StoredVars[ "cl_csm_viewmodel_shadows" ] = g_CVar->FindVar( "cl_csm_viewmodel_shadows" );
	m_StoredVars[ "cl_csm_rope_shadows" ] = g_CVar->FindVar( "cl_csm_rope_shadows" );
	m_StoredVars[ "mat_fullbright" ] = g_CVar->FindVar( "mat_fullbright" );
	m_StoredVars[ "cl_csm_enabled" ] = g_CVar->FindVar( "cl_csm_enabled" );

	m_StoredVars[ "r_modelAmbientMin" ] = g_CVar->FindVar( "r_modelAmbientMin" );

	isInit = true;
}