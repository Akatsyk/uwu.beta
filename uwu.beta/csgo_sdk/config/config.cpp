#include "config.h"

#include "../functions/skins/skins.h"
#include "../functions/skins/kit_parser.h"
#include "../sdk/mem_init.h"

ConfigManager g_ConfigMaster;
ConfigList g_Options;

void ConfigManager::setup_skin_clr()
{
	if ( !skins_parsed )
		return;

	setup_item_skins( &g_Options.changers.enable_skins, false, "skins_active" );
	for ( auto& [key, val] : k_weapon_names ) {
		auto& option = g_Options.changers.skin.m_items[ key ];
		std::string m_vec = val;
		setup_item_skins( &option.definition_vector_index, 0, m_vec + "d_vec_index" );
		setup_item_skins( &option.definition_index, 0, m_vec + "d_index" );
		setup_item_skins( &option.paint_kit_vector_index, 0, m_vec + "pk_vec_index" );
		setup_item_skins( &option.paint_kit_index, 0, m_vec + "pk_index" );
		setup_item_skins( &option.definition_override_index, 0, m_vec + "do_index" );
		setup_item_skins( &option.definition_override_vector_index, 0, m_vec + "do_vec_index" );
		setup_item_skins( &option.use, true, m_vec + "do_use" );
	}

	for ( int i = 0; i < k_skins.size( ); i++ )
	{
		auto& option = g_Options.changers.skin.m_items_clr[ i ];
		std::string m_Add = std::to_string( i );

		setup_item_skins( &option.First, Color( 255, 255, 255, 255 ), "skclr0" + m_Add );
		setup_item_skins( &option.Second, Color( 255, 255, 255, 255 ), "skclr1" + m_Add );
		setup_item_skins( &option.Third, Color( 255, 255, 255, 255 ), "skclr2" + m_Add );
		setup_item_skins( &option.Fourth, Color( 255, 255, 255, 255 ), "skclr3" + m_Add );

		setup_item_skins( &option.FirstHue, 0.f, "skhue0" + m_Add );
		setup_item_skins( &option.SecondHue, 0.f, "skhue1" + m_Add );
		setup_item_skins( &option.ThirdHue, 0.f, "skhue2" + m_Add );
		setup_item_skins( &option.FourthHue, 0.f, "skhue3" + m_Add );

		setup_item_skins( &option.ShouldUse0, false, "skuse0" + m_Add );
		setup_item_skins( &option.ShouldUse1, false, "skuse1" + m_Add );
		setup_item_skins( &option.ShouldUse2, false, "skuse2" + m_Add );
		setup_item_skins( &option.ShouldUse3, false, "skuse3" + m_Add );
	}	
}

void ConfigManager::setup_config()
{
	setup_item(&g_Options.ragebot.Acitve, false, "rage_active");
	setup_item(&g_Options.ragebot.Enable, false, "rage_enable");

	setup_item(&g_Options.ragebot.AutoFire, false, "rage_autofire");
	setup_item(&g_Options.ragebot.SilentAim, false, "rage_silentaim");

	setup_item(&g_Options.ragebot.FieldOfView, 0, "rage_fov");
	setup_item(&g_Options.ragebot.TargetSelection, 0, "rage_target_sel");

	setup_item(&g_Options.ragebot.Backtracking, false, "rage_back");
	setup_item(&g_Options.ragebot.Resolver, false, "rage_aa_correct");

	setup_item(&g_Options.ragebot.ForceBaim, false, "rage_force");
	setup_item(&g_Options.ragebot.BaimKey, -1, "rage_force_key");

	setup_item(&g_Options.ragebot.PreferSafepoint, false, "rage_force_safe");
	setup_item(&g_Options.ragebot.SafeKey, -1, "rage_force_safe_key");

	for (int i = 0; i < 8; i++)
	{
		std::string m_ToAdd = "rage_value_";
		m_ToAdd.append(std::to_string(i));

		setup_item(&g_Options.ragebot.boost[i], 0, m_ToAdd + "_boost");
		setup_item(&g_Options.ragebot.PriorityHitbox[i], 0, m_ToAdd + "_priority_hit");
		setup_item(&g_Options.ragebot.HeadScale[i], 0, m_ToAdd + "_head_scale");
		setup_item(&g_Options.ragebot.BodyScale[i], 0, m_ToAdd + "_chest_scale");
		setup_item(&g_Options.ragebot.AutoScope[i], false, m_ToAdd + "_scope");
		setup_item(&g_Options.ragebot.AutoStop[i], false, m_ToAdd + "_stop");
		setup_item(&g_Options.ragebot.ShotPriority[i], false, m_ToAdd + "_onshot");
		setup_item(&g_Options.ragebot.PreferBaim[i], false, m_ToAdd + "_prefer_baim");
		setup_item(&g_Options.ragebot.LethalBaim[i], false, m_ToAdd + "_lethal");
		setup_item(&g_Options.ragebot.Hitboxes[i][0], false, m_ToAdd + "_hit_0");
		setup_item(&g_Options.ragebot.Hitboxes[i][1], false, m_ToAdd + "_hit_1");
		setup_item(&g_Options.ragebot.Hitboxes[i][2], false, m_ToAdd + "_hit_2");
		setup_item(&g_Options.ragebot.Hitboxes[i][3], false, m_ToAdd + "_hit_3");
		setup_item(&g_Options.ragebot.Hitboxes[i][4], false, m_ToAdd + "_hit_4");
		setup_item(&g_Options.ragebot.Hitboxes[i][5], false, m_ToAdd + "_hit_5");
		setup_item(&g_Options.ragebot.Hitboxes[i][6], false, m_ToAdd + "_hit_6");
		setup_item(&g_Options.ragebot.Hitboxes[i][7], false, m_ToAdd + "_hit_7");

		setup_item(&g_Options.ragebot.Prefers[i][0], false, m_ToAdd + "_pref_0");
		setup_item(&g_Options.ragebot.Prefers[i][1], false, m_ToAdd + "_pref_1");
		setup_item(&g_Options.ragebot.Prefers[i][2], false, m_ToAdd + "_pref_2");

		setup_item(&g_Options.ragebot.PrefersBo[i][0], false, m_ToAdd + "_pref_b_0");
		setup_item(&g_Options.ragebot.PrefersBo[i][1], false, m_ToAdd + "_pref_b_1");
		setup_item(&g_Options.ragebot.PrefersBo[i][2], false, m_ToAdd + "_pref_b_2");
	}

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.Active, false, "vis_active");
	setup_item(&g_Options.visuals.Enemy, false, "vis_enemy_active");
	setup_item(&g_Options.visuals.Local, false, "vis_local_active");

	setup_item(&g_Options.visuals.vis_enemy.Box, 0, "vis_enemy_box");
	setup_item(&g_Options.visuals.vis_enemy.Box_Clr, Color( 255, 255, 255, 255 ), "vis_enemy_box_clr");
	setup_item(&g_Options.visuals.vis_enemy.Box_hue, 0.f, "vis_enemy_box_clr_hue");

	setup_item(&g_Options.visuals.vis_enemy.HP, false, "vis_enemy_hp");
	setup_item(&g_Options.visuals.vis_enemy.HP_Percent, false, "vis_enemy_hp_percent");

	setup_item(&g_Options.visuals.vis_enemy.Ammo, false, "vis_enemy_ammo");
	setup_item(&g_Options.visuals.vis_enemy.Name, false, "vis_enemy_name");

	setup_item(&g_Options.visuals.vis_enemy.Ammo_Clr, Color(255, 255, 255, 255), "vis_enemy_ammo_clr");
	setup_item(&g_Options.visuals.vis_enemy.Ammo_hue, 0.f, "vis_enemy_ammo_clr_hue");

	setup_item(&g_Options.visuals.vis_enemy.Weapon[0], false, "vis_enemy_weapon_0");
	setup_item(&g_Options.visuals.vis_enemy.Weapon[1], false, "vis_enemy_weapon_1");

	setup_item(&g_Options.visuals.vis_enemy.Material, 0, "vis_enemy_material");
	setup_item(&g_Options.visuals.vis_enemy.Illuminate, 0, "vis_enemy_ill");
	setup_item(&g_Options.visuals.vis_enemy.Visibility, false, "vis_enemy_visibility");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.vis_local.Material, 0, "vis_local_material");
	setup_item(&g_Options.visuals.vis_local.Illuminate, 0, "vis_local_ill");

	setup_item(&g_Options.visuals.vis_local.Desync_Material, 0, "vis_local_desync_material");
	setup_item(&g_Options.visuals.vis_local.Desync_Illuminate, 0, "vis_local_desync_ill");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.colors.enemy_chams, Color(255, 255, 255, 255), "enemy_chams_clr");
	setup_item(&g_Options.visuals.colors.enemy_chams_xqz, Color(255, 255, 255, 255), "xqz_enemy_chams_clr");
	setup_item(&g_Options.visuals.colors.enemy_illuminate, Color(255, 255, 255, 255), "enemy_ill_clr");

	setup_item(&g_Options.visuals.colors.local_chams, Color(255, 255, 255, 255), "local_chams_clr");
	setup_item(&g_Options.visuals.colors.local_illuminate, Color(255, 255, 255, 255), "ill_local_chams_clr");

	setup_item(&g_Options.visuals.colors.desync_chams, Color(255, 255, 255, 255), "local_desync_chams_clr");
	setup_item(&g_Options.visuals.colors.desync_illuminate, Color(255, 255, 255, 255), "ill_desync_local_chams_clr");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.colors.enemy_chams_hue, 0.f, "enemy_chams_clr_hue");
	setup_item(&g_Options.visuals.colors.enemy_chams_hue_xqz, 0.f, "xqz_enemy_chams_clr_hue");
	setup_item(&g_Options.visuals.colors.enemy_illuminate_hue, 0.f, "enemy_ill_clr_hue");

	setup_item(&g_Options.visuals.colors.local_chams_hue, 0.f, "local_chams_clr_hue");
	setup_item(&g_Options.visuals.colors.local_illuminate_hue, 0.f, "ill_local_chams_clr_hue");

	setup_item(&g_Options.visuals.colors.desync_chams_hue, 0.f, "des_local_chams_clr_hue");
	setup_item(&g_Options.visuals.colors.desync_illuminate_hue, 0.f, "des_ill_local_chams_clr_hue");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.view.view_v, 63, "view_model_v");
	setup_item(&g_Options.visuals.view.fov_v, 90, "fov_model_v");

	setup_item(&g_Options.visuals.view.x_view, 0, "view_x");
	setup_item(&g_Options.visuals.view.y_view, 0, "view_y");
	setup_item(&g_Options.visuals.view.z_view, 0, "view_z");

	setup_item(&g_Options.visuals.removals.flash, false, "vis_removals_0");
	setup_item(&g_Options.visuals.removals.fog, false, "vis_removals_1");

	setup_item(&g_Options.visuals.removals.panorama_blue, false, "vis_removals_2");
	setup_item(&g_Options.visuals.removals.post_process, false, "vis_removals_3");

	setup_item(&g_Options.visuals.removals.recoil, false, "vis_removals_4");
	setup_item(&g_Options.visuals.removals.scope, false, "vis_removals_5");

	setup_item(&g_Options.visuals.removals.shadows, false, "vis_removals_6");
	setup_item(&g_Options.visuals.removals.smoke, false, "vis_removals_7");

	setup_item(&g_Options.visuals.removals.zoom, false, "vis_removals_8");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.vis_local.Glow, 0, "local_glow");
	setup_item(&g_Options.visuals.vis_enemy.Glow, 0, "enemy_glow");

	setup_item(&g_Options.visuals.world_weapons.ammo_clr, Color(255, 255, 255, 255), "weapon_ammo_clr");
	setup_item(&g_Options.visuals.world_weapons.ammo_clr_hue, 0.f, "weapon_ammo_clr_hue");

	setup_item(&g_Options.visuals.world_weapons.name_clr, Color(255, 255, 255, 255), "weapon_name_clr");
	setup_item(&g_Options.visuals.world_weapons.name_clr_hue, 0.f, "weapon_name_clr_hue");

	setup_item(&g_Options.visuals.world_weapons.box_clr, Color(255, 255, 255, 255), "weapon_box_clr");
	setup_item(&g_Options.visuals.world_weapons.box_clr_hue, 0.f, "weapon_box_clr_hue");

	setup_item(&g_Options.visuals.world.full_bright, false, "vis_full_bright");

	setup_item(&g_Options.visuals.world_weapons.ammo, false, "weapon_am");
	setup_item(&g_Options.visuals.world_weapons.name, false, "weapon_nam");
	setup_item(&g_Options.visuals.world_weapons.box, false, "weapon_bx");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.world.sky, false, "world_sky");
	setup_item(&g_Options.visuals.world.props, false, "world_props");
	setup_item(&g_Options.visuals.world.walls, false, "world_walls");
	setup_item(&g_Options.visuals.world.sky_mode, 0, "world_sky_type");

	setup_item(&g_Options.visuals.world.sky_clr, Color(255, 255, 255, 255), "world_sky_clr");
	setup_item(&g_Options.visuals.world.sky_clr_hue, 0.f, "world_sky_clr_hue");

	setup_item(&g_Options.visuals.world.props_clr, Color(255, 255, 255, 255), "world_props_clr");
	setup_item(&g_Options.visuals.world.props_clr_hue, 0.f, "world_props_clr_hue");

	setup_item(&g_Options.visuals.world.wall_clr, Color(255, 255, 255, 255), "world_wall_clr");
	setup_item(&g_Options.visuals.world.wall_clr_hue, 0.f, "world_wall_clr_hue");

	setup_item(&g_Options.visuals.colors.enemy_glow, Color(255, 255, 255, 255), "vis_ene_glow");
	setup_item(&g_Options.visuals.colors.enemy_glow_hue, 0.f, "vis_ene_glow_hue");

	setup_item(&g_Options.visuals.colors.local_glow, Color(255, 255, 255, 255), "vis_loc_glow");
	setup_item(&g_Options.visuals.colors.local_glow_hue, 0.f, "vis_loc_glow_hue");

	setup_item(&g_Options.visuals.vis_enemy.OOF, false, "vis_enemy_off");
	setup_item(&g_Options.visuals.vis_enemy.OOF_dist, 0, "vis_enemy_off_dist");
	setup_item(&g_Options.visuals.vis_enemy.OOF_size, 0, "vis_enemy_off_size");
	setup_item(&g_Options.visuals.vis_enemy.OOF_Clr, Color(255, 255, 255, 255), "vis_enemy_clr_off");
	setup_item(&g_Options.visuals.vis_enemy.OOF_hue, 0.f, "vis_enemy_off_hue");

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.misc_.thirdperson, false, "vis_tp");
	setup_item(&g_Options.visuals.misc_.tp_dist, 0, "vis_tp_dist");
	setup_item(&g_Options.visuals.misc_.key_tp, -1, "vis_tp_key");

	// ----------------------------------------------- //

	setup_item(&g_Options.ragebot.min_damage[0], 0, "rage_dmg_0");
	setup_item(&g_Options.ragebot.min_damage[1], 0, "rage_dmg_1");
	setup_item(&g_Options.ragebot.min_damage[2], 0, "rage_dmg_2");
	setup_item(&g_Options.ragebot.min_damage[3], 0, "rage_dmg_3");
	setup_item(&g_Options.ragebot.min_damage[4], 0, "rage_dmg_4");
	setup_item(&g_Options.ragebot.min_damage[5], 0, "rage_dmg_5");
	setup_item(&g_Options.ragebot.min_damage[6], 0, "rage_dmg_6");
	setup_item(&g_Options.ragebot.min_damage[7], 0, "rage_dmg_7");

	setup_item(&g_Options.ragebot.hitchance[0], 0, "rage_hit_0");
	setup_item(&g_Options.ragebot.hitchance[1], 0, "rage_hit_1");
	setup_item(&g_Options.ragebot.hitchance[2], 0, "rage_hit_2");
	setup_item(&g_Options.ragebot.hitchance[3], 0, "rage_hit_3");
	setup_item(&g_Options.ragebot.hitchance[4], 0, "rage_hit_4");
	setup_item(&g_Options.ragebot.hitchance[5], 0, "rage_hit_5");
	setup_item(&g_Options.ragebot.hitchance[6], 0, "rage_hit_6");
	setup_item(&g_Options.ragebot.hitchance[7], 0, "rage_hit_7");

	// ----------------------------------------------- //

	setup_item(&g_Options.antiaim.Pitch, 0, "aa_pitch");
	setup_item(&g_Options.antiaim.Yaw, 0, "aa_yaw");
	setup_item(&g_Options.antiaim.BaseYaw, 0, "aa_base");
	setup_item(&g_Options.antiaim.JitterMode, 0, "aa_jitter");

	setup_item(&g_Options.antiaim.JitterValue, 0, "aa_jitter_val");
	setup_item(&g_Options.antiaim.YawAdd, 0, "aa_yaw_add");
	setup_item(&g_Options.antiaim.YawAddInvert, 0, "aa_yaw_add_invert");

	setup_item(&g_Options.antiaim.FakeYaw, 0, "aa_fake_yaw");
	setup_item(&g_Options.antiaim.JiterSideFake, 0, "aa_jit_fake_yaw");

	setup_item(&g_Options.antiaim.FakeYawValue, 0, "aa_add_fake_yaw");
	setup_item(&g_Options.antiaim.FakeYawValueInvert, 0, "aa_inv_add_fake_yaw");

	setup_item(&g_Options.antiaim.KeyInvert, -1, "aa_switch_key");

	// ----------------------------------------------- //

	setup_item(&g_Options.fakelag.Activate, false, "lag_enabled");
	setup_item(&g_Options.fakelag.Mode, 0, "lag_mode");
	setup_item(&g_Options.fakelag.Amount, 1, "lag_value");

	setup_item(&g_Options.exploit.ActMode, false, "explt_enabled");
	setup_item(&g_Options.exploit.mExpltKey, -1, "explt_key");
	setup_item(&g_Options.exploit.mMode, 0, "explt_mode");
	setup_item(&g_Options.exploit.m_DoubleTapSpeed, 0, "explt_stop");
	setup_item( &g_Options.exploit.m_Tolerance, 0, "explt_tolerance" );

	setup_item(&g_Options.visuals.misc_.LogsOut[0], false, "vis_log_0");
	setup_item(&g_Options.visuals.misc_.LogsOut[1], false, "vis_log_1");
	setup_item(&g_Options.visuals.misc_.LogsOut[2], false, "vis_log_2");
	setup_item(&g_Options.visuals.misc_.LogsOut[3], false, "vis_log_3");

	setup_item(&g_Options.visuals.misc_.m_CrossType, 0, "vis_spread_cross_type");
	setup_item(&g_Options.visuals.misc_.Spread_Clr, Color(255, 255, 255, 255), "vis_spread_cross_clr");
	setup_item(&g_Options.visuals.misc_.Spread_hue, 0.f, "vis_spread_cr_hue");
	setup_item(&g_Options.visuals.misc_.SpreadCross, false, "vis_spread_cross_bool");

	// ----------------------------------------------- //

	setup_item(&g_Options.movement.unlimitDuck, false, "move_duck");
	setup_item(&g_Options.movement.fakeduck, false, "move_fake_duck");
	setup_item(&g_Options.movement.m_FdKey, -1, "move_fake_duck_key");
	setup_item(&g_Options.movement.bhop, false, "move_bhop");
	setup_item(&g_Options.movement.autostrafe, 0, "move_strafe");
	setup_item(&g_Options.movement.slowwalk, false, "move_walk");
	setup_item(&g_Options.movement.m_SlowKey, -1, "move_walk_key");
	setup_item(&g_Options.movement.slow_speed, 0, "move_walk_spe");

	// ------------------- GUI PART ----------------------- //

	setup_item(&g_Options.GUI_CLRS.Default_Clr, Color(200, 45, 90, 255), "gui_clrs_1");
	setup_item(&g_Options.GUI_CLRS.Default_Clr_Hue, 0.f, "gui_clrs_1_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_1_Clr, Color(37, 37, 37, 255), "gui_clrs_2");
	setup_item(&g_Options.GUI_CLRS.Frame_1_Clr_Hue, 0.f, "gui_clrs_2_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_2_Clr, Color(45, 45, 45, 255), "gui_clrs_3");
	setup_item(&g_Options.GUI_CLRS.Frame_2_Clr_Hue, 0.f, "gui_clrs_3_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_3_Clr, Color(27, 27, 27, 255), "gui_clrs_4");
	setup_item(&g_Options.GUI_CLRS.Frame_3_Clr_Hue, 0.f, "gui_clrs_4_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_4_Clr, Color(37, 37, 37, 255), "gui_clrs_5");
	setup_item(&g_Options.GUI_CLRS.Frame_4_Clr_Hue, 0.f, "gui_clrs_5_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_5_Clr, Color(27, 27, 27, 255), "gui_clrs_6");
	setup_item(&g_Options.GUI_CLRS.Frame_5_Clr_Hue, 0.f, "gui_clrs_6_hue");

	setup_item(&g_Options.GUI_CLRS.Frame_6_Clr, Color(37, 37, 37, 255), "gui_clrs_7");
	setup_item(&g_Options.GUI_CLRS.Frame_6_Clr_Hue, 0.f, "gui_clrs_7_hue");

	setup_item(&g_Options.GUI_CLRS.NameWater_Clr, Color(170, 170, 170, 255), "gui_clrs_8");
	setup_item(&g_Options.GUI_CLRS.NameWater_Hue, 0.f, "gui_clrs_8_hue");

	setup_item(&g_Options.GUI_CLRS.ElementsGradient_1_Clr, Color(0, 0, 0, 55), "gui_clrs_12");
	setup_item(&g_Options.GUI_CLRS.ElementsGradient_1_Hue, 0.f, "gui_clrs_12_hue");

	setup_item(&g_Options.GUI_CLRS.ElementsGradient_2_Clr, Color(45, 45, 45, 55), "gui_clrs_13");
	setup_item(&g_Options.GUI_CLRS.ElementsGradient_2_Hue, 0.f, "gui_clrs_13_hue");

	setup_item(&g_Options.GUI_CLRS.ElementsText_Clr, Color(170, 170, 170, 255), "gui_clrs_11");
	setup_item(&g_Options.GUI_CLRS.ElementsText_Hue, 0.f, "gui_clrs_11_hue");

	setup_item(&g_Options.GUI_CLRS.GroupBoxOuter_Clr, Color(27, 27, 27, 255), "gui_clrs_19");
	setup_item(&g_Options.GUI_CLRS.GroupBoxOuter_Hue, 0.f, "gui_clrs_19_hue");

	setup_item(&g_Options.GUI_CLRS.GroupBoxOuter_2_Clr, Color(45, 45, 45, 255), "gui_clrs_20");
	setup_item(&g_Options.GUI_CLRS.GroupBoxOuter_2_Hue, 0.f, "gui_clrs_20_hue");

	setup_item(&g_Options.GUI_CLRS.SelectorOuter_Clr, Color(37, 37, 37, 255), "gui_clrs_23");
	setup_item(&g_Options.GUI_CLRS.SelectorOuter_Hue, 0.f, "gui_clrs_23_hue");

	setup_item(&g_Options.GUI_CLRS.SelectorHovered_Clr, Color(27, 27, 27, 255), "gui_clrs_24");
	setup_item(&g_Options.GUI_CLRS.SelectorHovered_Hue, 0.f, "gui_clrs_24_hue");

	setup_item(&g_Options.GUI_CLRS.SelectorSelectedText_Clr, Color(65, 65, 65, 255), "gui_clrs_26");
	setup_item(&g_Options.GUI_CLRS.SelectorSelectedText_Hue, 0.f, "gui_clrs_26_hue");

	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_1_Clr, Color(45, 45, 45, 255), "gui_clrs_36");
	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_1_Hue, 0.f, "gui_clrs_36_hue");

	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_2_Clr, Color(27, 27, 27, 255), "gui_clrs_37");
	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_2_Hue, 0.f, "gui_clrs_37_hue");

	setup_item(&g_Options.GUI_CLRS.TabText_Clr, Color(215, 215, 215, 255), "gui_clrs_9");
	setup_item(&g_Options.GUI_CLRS.TabText_Hue, 0.f, "gui_clrs_9_hue");

	setup_item(&g_Options.GUI_CLRS.TabTextHovered_Clr, Color(110, 110, 110, 255), "gui_clrs_10");
	setup_item(&g_Options.GUI_CLRS.TabTextHovered_Hue, 0.f, "gui_clrs_10_hue");

	setup_item(&g_Options.GUI_CLRS.SubtabFilled_Clr, Color(185, 185, 185, 255), "gui_clrs_31");
	setup_item(&g_Options.GUI_CLRS.SubtabFilled_Hue, 0.f, "gui_clrs_31_hue");

	setup_item(&g_Options.GUI_CLRS.ListBoxLine_Clr, Color(60, 60, 60, 255), "gui_clrs_32");
	setup_item(&g_Options.GUI_CLRS.ListBoxLine_Hue, 0.f, "gui_clrs_32_hue");

	setup_item(&g_Options.GUI_CLRS.ListBoxLine_2_Clr, Color(27, 27, 27, 255), "gui_clrs_33");
	setup_item(&g_Options.GUI_CLRS.ListBoxLine_2_Hue, 0.f, "gui_clrs_33_hue");

	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_3_Clr, Color(160, 160, 160, 255), "gui_clrs_38");
	setup_item(&g_Options.GUI_CLRS.ColorPickerFrame_3_Hue, 0.f, "gui_clrs_38_hue");

	setup_item(&g_Options.GUI_CLRS.ColorPickerOutline_Clr, Color(255, 255, 255, 255), "gui_clrs_39");
	setup_item(&g_Options.GUI_CLRS.ColorPickerOutline_Hue, 0.f, "gui_clrs_39_hue");

	// ----------------------------------------------- //

	for (int i = 0; i < 17; i++)
	{
		std::string m_toAdd = std::to_string(i);
		setup_item(&g_Options.CustomFonts.m_VectorName[i], "", "m_font_name_vec_" + m_toAdd);
		setup_item(&g_Options.CustomFonts.m_VectorSize[i], "", "m_font_size_vec_" + m_toAdd);
		setup_item(&g_Options.CustomFonts.m_VectorWeight[i], "", "m_font_weight_vec_" + m_toAdd);
		setup_item(&g_Options.CustomFonts.m_VectorAA[i], false, "m_font_bool_0_vec_" + m_toAdd);
		setup_item(&g_Options.CustomFonts.m_VectorOutline[i], false, "m_font_bool_1_vec_" + m_toAdd);
		setup_item(&g_Options.CustomFonts.m_VectorDropShadow[i], false, "m_font_bool_2_vec_" + m_toAdd);
	}

	setup_item(&g_Options.CustomVisualsFont.HpNumberFont, 0, "vis_custom_font_0");
	setup_item(&g_Options.CustomVisualsFont.LogsFont, 0, "vis_custom_font_1");
	setup_item(&g_Options.CustomVisualsFont.NameFont, 0, "vis_custom_font_2");
	setup_item(&g_Options.CustomVisualsFont.WeaponFont, 0, "vis_custom_font_3");
	setup_item(&g_Options.CustomVisualsFont.WorldNameFont, 0, "vis_custom_font_4");

	// ----------------------------------------------- //

	setup_item(&g_Options.CustomVisals.ScaleX, 8, "vis_custom_scaling_0");
	setup_item(&g_Options.CustomVisals.ScaleY, 10, "vis_custom_scaling_1");

	setup_item(&g_Options.CustomVisals.ScaleZ, 3.35f, "vis_custom_scaling_2");
	setup_item(&g_Options.CustomVisals.ScaleA, 2, "vis_custom_scaling_3");

	setup_item( &g_Options.CustomVisals.SizeHealthBar, 0.f, "vis_custom_value_0" );
	setup_item( &g_Options.CustomVisals.LeftSideHealthBar, 0.f, "vis_custom_value_1" );

	setup_item( &g_Options.CustomVisals.OneLineFrame, 0.f, "vis_custom_value_2" );
	setup_item( &g_Options.CustomVisals.SecondLineFrame, 0.f, "vis_custom_value_3" );

	setup_item( &g_Options.CustomVisals.NamePosX, 0.f, "vis_custom_value_4" );
	setup_item( &g_Options.CustomVisals.NamePosY, 0.f, "vis_custom_value_5" );

	setup_item( &g_Options.CustomVisals.AmmoPos, 0.f, "vis_custom_value_6" );
	setup_item( &g_Options.CustomVisals.WeaponNamePos1, 0.f, "vis_custom_value_7" );

	setup_item( &g_Options.CustomVisals.WeaponNamePos2, 0.f, "vis_custom_value_8" );

	setup_item( &g_Options.CustomVisals.WeaponIcon[ 0 ], 0.f, "vis_custom_value_9" );
	setup_item( &g_Options.CustomVisals.WeaponIcon[ 1 ], 0.f, "vis_custom_value_10" );
	setup_item( &g_Options.CustomVisals.WeaponIcon[ 2 ], 0.f, "vis_custom_value_11" );
	setup_item( &g_Options.CustomVisals.WeaponIcon[ 3 ], 0.f, "vis_custom_value_12" );
	setup_item( &g_Options.CustomVisals.WeaponIcon[ 4 ], 0.f, "vis_custom_value_13" );

	setup_item( &g_Options.CustomVisals.HealthTextPos, 0.f, "vis_custom_value_14" );
	setup_item( &g_Options.CustomVisals.AmmoSize, 0.f, "vis_custom_value_15" );

	// ----------------------------------------------- //

	setup_item(&g_Options.m_InitScripts, ("Scripts.loaded"));

	// ----------------------------------------------- //

	setup_item(&g_Options.visuals.misc_.watermark, true, "misc_watermark");
	setup_item(&g_Options.visuals.misc_.spectator_list, false, "misc_specs");
	setup_item(&g_Options.visuals.misc_.keybind_list, false, "misc_bind_list");
	setup_item(&g_Options.MasterSwitch, 0, "m_switch_rl");

	// ----------------------------------------------- //

	setup_item( &g_Options.active_legit, false, "legit_active" );
	setup_item( &g_Options.awall_key, -1, "legit_awall_key" );

	for ( int i = 0; i < 8; i++ )
	{
		std::string m_ToAdd = "legitmode";
		m_ToAdd.append( std::to_string( i ) );

		setup_item( &g_Options.m_legit[ i ].enabled, false, m_ToAdd + "_enabled" );
		setup_item( &g_Options.m_legit[ i ].flash_check, false, m_ToAdd + "_flash_check" );
		setup_item( &g_Options.m_legit[ i ].smoke_check, false, m_ToAdd + "_smoke_check" );
		setup_item( &g_Options.m_legit[ i ].auto_pistol, false, m_ToAdd + "_auto_pistol" );
		setup_item( &g_Options.m_legit[ i ].hitbox.head, false, m_ToAdd + "_hitbox_0" );
		setup_item( &g_Options.m_legit[ i ].hitbox.chest, false, m_ToAdd + "_hitbox_1" );
		setup_item( &g_Options.m_legit[ i ].hitbox.hands, false, m_ToAdd + "_hitbox_2" );
		setup_item( &g_Options.m_legit[ i ].hitbox.legs, false, m_ToAdd + "_hitbox_3" );
		setup_item( &g_Options.m_legit[ i ].fov, 0.f, m_ToAdd + "_field" );
		setup_item( &g_Options.m_legit[ i ].smooth, 0.f, m_ToAdd + "_smooth" );
		setup_item( &g_Options.m_legit[ i ].shot_delay, 0.f, m_ToAdd + "_shot_delay" );
		setup_item( &g_Options.m_legit[ i ].kill_delay, 0.f, m_ToAdd + "_kill_delay" );
		setup_item( &g_Options.m_legit[ i ].silent_fov, 0.f, m_ToAdd + "_silent_field" );
		setup_item( &g_Options.m_legit[ i ].silent_mode, 0, m_ToAdd + "_silent_mode" );
		setup_item( &g_Options.m_legit[ i ].rcs, false, m_ToAdd + "_rcs_ena" );
		setup_item( &g_Options.m_legit[ i ].rcs_x, 0.f, m_ToAdd + "_rcs_x" );
		setup_item( &g_Options.m_legit[ i ].rcs_y, 0.f, m_ToAdd + "_rcs_y" );
		setup_item( &g_Options.m_legit[ i ].rcs_mode, 0, m_ToAdd + "_rcs_mode" );
		setup_item( &g_Options.m_legit[ i ].awall, false, m_ToAdd + "_awl" );
		setup_item( &g_Options.m_legit[ i ].awall_mode, 0, m_ToAdd + "_awl_md" );
		setup_item( &g_Options.m_legit[ i ].awall_damage, 0.f, m_ToAdd + "_awl_dmg" );
		setup_item( &g_Options.m_legit[ i ].vis, false, m_ToAdd + "_leg_vis" );
	}

	// ----------------------------------------------- //

	setup_item( &g_Options.visuals.vis_enemy.BackgroundType, 0, "vis_enemy_background_type" );
	setup_item( &g_Options.visuals.vis_enemy.Background[ 0 ], false, "vis_enemy_background_0" );
	setup_item( &g_Options.visuals.vis_enemy.Background[ 1 ], false, "vis_enemy_background_1" );

	setup_item( &g_Options.visuals.vis_enemy.BackColor, Color( 255, 255, 255, 255 ), "vis_en_back_clr" );
	setup_item( &g_Options.visuals.vis_enemy.BackHue, 0.f, "vis_en_back_hue" );

	setup_item( &g_Options.visuals.vis_enemy.BackLineColor, Color( 255, 255, 255, 255 ), "vis_en_back_line_clr" );
	setup_item( &g_Options.visuals.vis_enemy.BackLineHue, 0.f, "vis_en_back_line_hue" );

	// ----------------------------------------------- //

	setup_item( &g_Options.visuals.misc_.auto_peek, false, "misc_peek_helper" );
	setup_item( &g_Options.visuals.misc_.peek_key, -1, "misc_peek_helper_key" );

	setup_item( &g_Options.visuals.misc_.force_aspect, false, "misc_aspect_ratio" );
	setup_item( &g_Options.visuals.misc_.aspect_rat, 1, "misc_aspect_ratio_value" );
	
	setup_item( &g_Options.visuals.misc_.hitsound, 0, "misc_sound" );

	// ----------------------------------------------- //

	setup_item( &g_Options.visuals.world.modulate_light.rAmbient, false, "modulate_light" );
	setup_item( &g_Options.visuals.world.modulate_light.Exposure, 0.f, "modulate_light_exp" );
	setup_item( &g_Options.visuals.world.modulate_light.Bloom, 0.f, "modulate_light_bloom" );
	setup_item( &g_Options.visuals.world.modulate_light.AmbientAmount, 0.f, "modulate_light_amb" );

	setup_item( &g_Options.visuals.world.fog.Enabled, false, "fog_start" );
	setup_item( &g_Options.visuals.world.fog.Density, 0.f, "fog_density" );
	setup_item( &g_Options.visuals.world.fog.Dist, 0.f, "fog_dist" );

	setup_item( &g_Options.visuals.world.fog.FogClr, Color( 255, 255, 255, 255 ), "fog_clr" );
	setup_item( &g_Options.visuals.world.fog.FogHue, 0.f, "fog_hue" );

	setup_item( &g_Options.visuals.world.ModulateFire, false, "world_modulate_fire" );
	setup_item( &g_Options.visuals.world.FireHue, 0, "world_modulate_fire_hue" );
	setup_item( &g_Options.visuals.misc_.peek_hue, 0.f, "misc_peek_hu" );

	// ----------------------------------------------- //

	setup_item( &g_Options.visuals.view.DimCrosshair, false, "crosshair_3d" );
	setup_item( &g_Options.visuals.view.DimColor, Color( 255, 255, 255, 255 ), "crosshair_3d_clr" );
	setup_item( &g_Options.visuals.view.DimHue, 0.f, "crosshair_3d_clr_hue" );
	setup_item( &g_Options.visuals.view.DimSize, 1, "crosshair_3d_size" );

	setup_item( &g_Options.visuals.view.NadeEsp, false, "vis_na_esp" );
	setup_item( &g_Options.visuals.view.NadeColor, Color( 255, 255, 255, 255 ), "vis_na_esp_clr" );
	setup_item( &g_Options.visuals.view.NadeEspHue, 0.f, "vis_na_esp_hue" );

	setup_item( &g_Options.visuals.view.grenade_prediction, false, "vis_nade_pred_esp" );
	setup_item( &g_Options.visuals.view.nade_line, Color( 255, 255, 255, 255 ), "vis_nade_pred_esp_clr" );
	setup_item( &g_Options.visuals.view.nade_hue, 0.f, "vis_nade_pred_esp_hue" );

	setup_item( &g_Options.visuals.view.corners, false, "vis_nade_pred_esp_corn" );
	setup_item( &g_Options.visuals.view.corn_clr, Color( 255, 255, 255, 255 ), "vis_nade_pred_esp_clr_corn" );
	setup_item( &g_Options.visuals.view.corn_hue, 0.f, "vis_nade_pred_esp_hue_corn" );
}