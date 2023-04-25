#include "../../csgo.hpp"

const char* enemy_chams_type [ ] = {
    "regular",
    "flat",
    "glow",
    "glow overlay",
    "metallic",
};

const char* stop_type_type[ ] = {
    "slow-motion",
    "full stop"
};

const char* stop_type_dt[] = {
    "default",
    "slow-motion",
    "full stop"
};


 const char* pitch_type[ ] = {
    "none",
    "down",
    "up"
};

 const char* force_head_conditions[ ] = {
     "body update",
     "walking",
     "resolved"
 };

 const char* auto_stop_modifiers[] = {
     "early",
     "dynamic"
 };


 const char* body_conditions[ ] = {
     "in air",
     "no move data",
     "unresolved",
     "two shot",
     "double tapping"
 };

 const char* knife_type[ ] = {
     "none",
     "bayonet",
     "flip knife",
     "gut knife",
     "karambit",
     "m9 bayonet",
     "huntsman"
 };

 const char* cfg_slots[ ] = {
     "primary",
     "secondary",
     "additional",
     "additional_2",
     "additional_3",
     "extra"
 };

 const char* tabs[] = {
    "rage",
    "anti-aim",
    "visuals",
    "movement",
    "misc",
    "config"
 };

 
 const char* visuals_tabs[] = {
    "esp",
    "chams",
    "world",
 };


 const char* skins_arr[ ] = {
     "none",
     "groundwater",
     "candy apple",
     "forest ddpat",
     "arctic camo",
     "desert storm",
     "bengal tiger",
     "copperhead",
     "skulls",
     "crimson web",
     "blue streak",
     "red laminate",
     "gunsmoke",
     "jungle tiger",
     "urban ddpat",
     "virus",
     "granite marbleized",
     "contrast spray",
     "forest leaves",
     "lichen dashed",
     "bone mask",
     "anodized navy",
     "snake camo",
     "silver",
     "hot rod",
     "metallic ddpat", // 25 idx
     "ossified",
     "blaze",
     "fade",
     "bulldozer",
     "night",
     "copper",
     "blue steel",
     "stained",
     "case hardened", // 34
     "contractor",
     "colony",
     "dragon tattoo", // 37
     "lighting strike"
 };
 
const char* skybox_list [ ] =
{
( "none" ),
( "tibet" ),
( "baggage" ),
( "italy" ),
( "aztec" ),
( "vertigo" ),
( "daylight" ),
( "daylight 2" ),
( "clouds" ),
( "clouds 2" ),
( "gray" ),
( "clear" ),
( "canals" ),
( "cobblestone" ),
( "assault" ),
( "clouds dark" ),
( "night" ),
( "night 2" ),
( "night flat" ),
( "dusty" ),
( "rainy" )
};

inline const char* hitboxes [ ] =
{
    "head",
    "chest",
    "stomach",
    "arms",
    "legs"
};

inline const char* esp_flags[ ] = {
    "money",
    "ping",
    "armor",
    "scoped",
    "lc"
};

inline const char* removals[ ] = {
   "scope",
   "second zoom",
   "flash effect",
   "smoke",
   "hands shaking",
   "post processing",
   "view kick",
   "view punch",
   "landing bob"
};
 
const char* snipers_arr[ ] = {
    "none",
    "auto",
    "awp",
    "ssg 08"
};

inline const char* sounds_arr[ ] = {
    "neverlose",
    "phonk",
    "skeet",
    "primordial",
    "cock",
    "bepis"
};

const char* pistols_arr[ ] = {
    "none",
    "dual berettas",
    "heavy pistol"
};

const char* additional_arr[ ] = {
    "grenades",
    "armor",
    "taser",
    "defuser"
};

inline const char* wpns[ ] = {
    "scar",
    "scout",
    "awp",
    "heavy pistols",
    "pistols",
    "other"
};

inline const char* scan_intensity[] = {
    "off",
    "low",
    "medium",
    "high",
    "maximum"
};

inline const char* intensity[] = {
    "off",
    "medium",
    "high"
};

inline const char* wpns_for_skins[ ] = {
    "scar",
    "g3sg1",
    "deagle",
    "elite",
    "five-seven",
    "glock",
    "ak-47",
    "aug",
    "awp",
    "famas",
    "galil",
    "m249",
    "m4a4",
    "mac-10",
    "p90",
    "ump-45",
    "xm-1014",
    "bizon",
    "mag-7",
    "negev",
    "sawed-off",
    "tec-9",
    "p2000",
    "mp-7",
    "mp-9",
    "nova",
    "p250",
    "sg-553",
    "ssg-08",
    "m4a1-s",
    "usp-s",
    "cz75a",
    "revolver",
    "knife"
};

int cur_weapon { };
int cur_wpn_for_skins{ };

void draw_misc( ) {

    auto& misc_cfg = csgo::hacks::g_misc->cfg( );

    // miscellaneous
    ImGui::Checkbox( "buy bot", &csgo::hacks::g_misc->cfg( ).m_buy_bot );

    ImGui::Combo( xor_str( "primary" ), &csgo::hacks::g_misc->cfg( ).m_buy_bot_snipers, snipers_arr, IM_ARRAYSIZE( snipers_arr ) );

    ImGui::Combo( xor_str( "secondary" ), &csgo::hacks::g_misc->cfg( ).m_buy_bot_pistols, pistols_arr, IM_ARRAYSIZE( pistols_arr ) );

    if ( ImGui::BeginCombo( xor_str( "additionals" ), "" ) ) {
        static bool hitgroups_vars[ IM_ARRAYSIZE( additional_arr ) ]{};

        for ( std::size_t i{}; i < IM_ARRAYSIZE( additional_arr ); ++i ) {
            hitgroups_vars[ i ] = csgo::hacks::g_misc->cfg( ).m_buy_bot_additional & ( 1 << i );

            ImGui::Selectable(
                additional_arr[ i ], &hitgroups_vars[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if ( hitgroups_vars[ i ] )
                csgo::hacks::g_misc->cfg( ).m_buy_bot_additional |= ( 1 << i );
            else
                csgo::hacks::g_misc->cfg( ).m_buy_bot_additional &= ~( 1 << i );
        }

        ImGui::EndCombo( );
    }

    ImGui::SliderInt( "fov amount", &misc_cfg.m_camera_distance, 45, 130 );
    ImGui::Checkbox( "clantag spammer##misc", &misc_cfg.m_clan_tag );
    g_key_binds->add_keybind( xor_str( "ping spike##misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_ping_spike_key, false, 140 );
    ImGui::SliderFloat( xor_str( "##fake_latency_value_misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_to_spike, 50.f, 600.f, "%.1f" );
    g_key_binds->add_keybind( "third person##misc", &misc_cfg.m_third_person_key, false, 140.f );
    ImGui::SliderFloat( "##third_person_dist_misc", &misc_cfg.m_third_person_dist, 45.f, 150.f );
    ImGui::Checkbox( "force thirdperson when spectating##misc", &misc_cfg.m_force_thirdperson_dead );

    ImGui::Checkbox( "aspect ratio##misc", &misc_cfg.m_aspect_ratio );

    if( misc_cfg.m_aspect_ratio )
        ImGui::SliderFloat( "##aspect_ratio_amt_misc", &misc_cfg.m_aspect_ratio_value, 0.f, 2.f );

    ImGui::Checkbox( "hitsound", &misc_cfg.m_hit_marker_sound );

    if( misc_cfg.m_hit_marker_sound )
        ImGui::Combo( "##hitsound_value_misc", &misc_cfg.m_hit_marker_sound_val, sounds_arr, IM_ARRAYSIZE( sounds_arr ) );

    ImGui::Checkbox( "show spectators", &misc_cfg.m_spectators );
    ImGui::Checkbox( "indicators", &misc_cfg.m_key_binds );

    ImGui::Checkbox( xor_str( "preserve killfeed" ), &misc_cfg.m_kill_feed );
}

#pragma region rage
void rage_hitbox( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_auto" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_scar_hitboxes &( 1 << i );
                ImGui::Selectable( hitboxes[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_scar_hitboxes |= ( 1 << i );
                else
                    cfg.m_scar_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_scout_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_scout_hitboxes |= ( 1 << i );
                else
                    cfg.m_scout_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_awp_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_awp_hitboxes |= ( 1 << i );
                else
                    cfg.m_awp_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_heavy_pistols" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_heavy_pistol_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_heavy_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_heavy_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "hitboxes##rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_pistol_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "hitboxes##rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_other_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_other_hitboxes |= ( 1 << i );
                else
                    cfg.m_other_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    default:
        break;
    }
}
void rage_pointscale( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_scar_point_scale, 30.f, 100.f, cfg.m_scar_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_scout_point_scale, 30.f, 100.f, cfg.m_scout_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_awp_point_scale, 30.f, 100.f, cfg.m_awp_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_heavy_pistol_point_scale, 30.f, 100.f, cfg.m_heavy_pistol_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_pistol_point_scale, 30.f, 100.f, cfg.m_pistol_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "point scale" ), &cfg.m_other_point_scale, 30.f, 100.f, cfg.m_other_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    default:
        break;
    }
}
void rage_damage_ovr( ) {



    auto& cfg = csgo::hacks::g_aim_bot->cfg( );


    switch ( cur_weapon ) {
    case 0:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_scar_dmg_key, false, 175.f );
        break;
    case 1:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_scout_dmg_key, false, 175.f );
        break;
    case 2:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_awp_dmg_key, false, 175.f );
        break;
    case 3:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_heavy_pistol_dmg_key, false, 175.f );
        break;
    case 4:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_pistol_dmg_key, false, 175.f );
        break;
    case 5:
        g_key_binds->add_keybind( "damage override", &cfg.m_min_other_dmg_key, false, 175.f );
        break;
    default:
        break;
    }

    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_scar" ), &cfg.m_scar_min_dmg_on_key, 1, 126 );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_scout" ), &cfg.m_scout_min_dmg_on_key, 1, 126 );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_awp" ), &cfg.m_awp_min_dmg_on_key, 1, 126 );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_heavy_pistol" ), &cfg.m_heavy_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_pistol" ), &cfg.m_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_other" ), &cfg.m_other_min_dmg_on_key, 1, 126 );
        break;
    default:
        break;
    }

    g_key_binds->add_keybind( xor_str( "force body-aim##rage" ), &cfg.m_baim_key, false, 160.f );

}
void rage_autostop( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) {
    case 0:
        ImGui::Combo( xor_str( "auto stop##rage_scar" ), &cfg.m_auto_stop_type_scar, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 1:
        ImGui::Combo( xor_str( "auto stop##rage_scout" ), &cfg.m_auto_stop_type_scout, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 2:
        ImGui::Combo( xor_str( "auto stop##rage_awp" ), &cfg.m_auto_stop_type_awp, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 3:
        ImGui::Combo( xor_str( "auto stop##rage_heavy_pistol" ), &cfg.m_auto_stop_type_heavy_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 4:
        ImGui::Combo( xor_str( "auto stop##rage_pistol" ), &cfg.m_auto_stop_type_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 5:
        ImGui::Combo( xor_str( "auto stop##rage_other" ), &cfg.m_auto_stop_type_other, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    default:
        break;
    }

    if( ImGui::BeginCombo( xor_str( "stop modifiers##rage" ), "" ) ) {
        static bool stop_modifiers[ IM_ARRAYSIZE( auto_stop_modifiers ) ]{ };

        for( std::size_t i{ }; i < IM_ARRAYSIZE( auto_stop_modifiers ); ++i ) {
            stop_modifiers[ i ] = cfg.m_stop_modifiers & ( 1 << i );

            ImGui::Selectable( 
                auto_stop_modifiers[ i ], &stop_modifiers[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if( stop_modifiers[ i ] )
                cfg.m_stop_modifiers |= ( 1 << i );
            else
                cfg.m_stop_modifiers &= ~( 1 << i );
        }

        ImGui::EndCombo( );
    }

}
void rage_hitchance( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch ( cur_weapon ) {
    case 0:
        ImGui::SliderFloat( xor_str( "hit chance##rage_scar" ), &cfg.m_hit_chance_scar, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "hit chance##rage_scout" ), &cfg.m_hit_chance_scout, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "hit chance##rage_awp" ), &cfg.m_hit_chance_awp, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "hit chance##rage_heavy_pistol" ), &cfg.m_hit_chance_heavy_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "hit chance##rage_pistol" ), &cfg.m_hit_chance_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "hit chance##rage_other" ), &cfg.m_hit_chance_other, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }
}

void rage_body_aim( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_scar" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scar & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scout & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scout &= ~ ( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[i] = cfg.m_force_body_conditions_awp & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[i] )
                    cfg.m_force_body_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_heavy_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_heavy_pistol & ( 1 << i );
                ImGui::Selectable( body_conditions[i], &hitgroups_vars[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_heavy_pistol |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_heavy_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_pistol & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_other & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    default:
        break;
    }
}
void rage_head_aim( ) {
    
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_scar" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scar &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scout &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scout &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_awp &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_heavy_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_heavy_pistol &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_heavy_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_heavy_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_pistol &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_other &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    default:
        break;
    }

}
void rage_damage( ) {
    
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str( "minimum damage##rage_scar" ), &cfg.m_min_dmg_scar, 1.f, 126.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "minimum damage##rage_scout" ), &cfg.m_min_dmg_scout, 1.f, 126.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "minimum damage##rage_awp" ), &cfg.m_min_dmg_awp, 1.f, 126.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "minimum damage##rage_heavy_pistol" ), &cfg.m_min_dmg_heavy_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "minimum damage##rage_pistol" ), &cfg.m_min_dmg_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "minimum damage##rage_other" ), &cfg.m_min_dmg_other, 1.f, 126.f, "%.1f" );
        break;
    default:
        break;
    }
}

void rage_dt_hitchance( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch ( cur_weapon ) {
    case 0:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_scar" ), &cfg.m_dt_hit_chance_scar, 0.f, 75.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_scout" ), &cfg.m_dt_hit_chance_scout, 0.f, 75.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_awp" ), &cfg.m_dt_hit_chance_awp, 0.f, 75.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_heavy_pistol" ), &cfg.m_dt_hit_chance_heavy_pistol, 0.f, 75.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_pistol" ), &cfg.m_dt_hit_chance_pistol, 0.f, 75.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "double tap hit chance##rage_other" ), &cfg.m_dt_hit_chance_other, 0.f, 75.f, "%.1f" );
        break;
    default:
        break;
    }
}


void rage_dt_stop( ) {

     auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) {
    case 0:
        ImGui::Combo( xor_str( "double tap auto stop##rage_scar" ), &cfg.m_auto_stop_type_dt_scar, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 1:
        ImGui::Combo( xor_str( "double tap auto stop##rage_scout" ), &cfg.m_auto_stop_type_dt_scout, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 2:
        ImGui::Combo( xor_str( "double tap auto stop##rage_awp" ), &cfg.m_auto_stop_type_dt_awp, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 3:
        ImGui::Combo( xor_str( "double tap auto stop##rage_heavy_pistol" ), &cfg.m_auto_stop_type_dt_heavy_pistol, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 4:
        ImGui::Combo( xor_str( "double tap auto stop##rage_pistol" ), &cfg.m_auto_stop_type_dt_pistol, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 5:
        ImGui::Combo( xor_str( "double tap auto stop##rage_other" ), &cfg.m_auto_stop_type_dt_other, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    default:
        break;
    }
}

void draw_rage( ) {
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    ImGui::Combo( xor_str( "current weapon##rage" ), &cur_weapon, wpns, IM_ARRAYSIZE( wpns ) );
    ImGui::Checkbox( xor_str( "master switch##rage" ), &cfg.m_rage_bot );
    ImGui::Checkbox( xor_str( "auto scope##rage" ), &cfg.m_auto_scope );
    ImGui::Checkbox( xor_str( "limit records per tick##rage" ), &cfg.m_limit_records_per_tick );
    ImGui::Combo( xor_str( "backtrack intensity##rage" ), &cfg.m_backtrack_intensity, scan_intensity, IM_ARRAYSIZE( scan_intensity ) );

    rage_hitbox( );
    rage_pointscale( );
    rage_hitchance( ); 
    rage_damage( );
    rage_damage_ovr( );
    rage_body_aim( );
    rage_head_aim( );
    rage_autostop( );

    auto& exploit_cfg = csgo::hacks::g_exploits->cfg( );
    g_key_binds->add_keybind( xor_str( "double tap##rage_exploits" ), &exploit_cfg.m_dt_key, false, 140.f );
 
    rage_dt_stop( );
    rage_dt_hitchance( );
}

#pragma endregion

int visual_sub_tab = 0;

void draw_visuals( ) {

    auto& cfg = csgo::hacks::g_visuals->cfg( );
    auto& chams_cfg = csgo::hacks::g_chams->cfg( );
    ImGui::Combo( "current visuals tab", &visual_sub_tab, visuals_tabs, IM_ARRAYSIZE( visuals_tabs ) );

    if( visual_sub_tab == 0 ) {
        ImGui::Checkbox( xor_str( "name##esp" ), &cfg.m_draw_name );
        ImGui::Checkbox( xor_str( "health bar" ), &cfg.m_draw_health );
        ImGui::Checkbox( xor_str( "bounding box" ), &cfg.m_draw_box );
        ImGui::Checkbox( xor_str( "flags" ), &cfg.m_draw_flags );

        if ( ImGui::BeginCombo( xor_str( "show flag" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( esp_flags ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( esp_flags ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_visuals->cfg( ).m_player_flags & ( 1 << i );

                ImGui::Selectable(
                    esp_flags[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_visuals->cfg( ).m_player_flags |= ( 1 << i );
                else
                    csgo::hacks::g_visuals->cfg( ).m_player_flags &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }

        ImGui::Checkbox( xor_str( "weapon icon" ), &cfg.m_wpn_icon );
        ImGui::Checkbox( xor_str( "weapon text" ), &cfg.m_wpn_text );
        ImGui::Checkbox( xor_str( "ammo bar" ), &cfg.m_wpn_ammo ); ImGui::SameLine( );
        ImGui::ColorEdit4( xor_str( "ammo color" ), cfg.m_wpn_ammo_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::Checkbox( xor_str( "out of fov arrows" ), &cfg.m_oof_indicator ); ImGui::SameLine( );
        ImGui::ColorEdit4( xor_str( "oof color" ), cfg.m_oof_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::Checkbox( xor_str( "body update" ), &cfg.m_draw_lby ); ImGui::SameLine( );
        ImGui::ColorEdit4( xor_str( "update bar color" ), cfg.m_lby_upd_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::Checkbox( xor_str( "glow" ), &cfg.m_glow ); ImGui::SameLine( );
        ImGui::ColorEdit4( xor_str( "glow color" ), cfg.m_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
    }
    else if( visual_sub_tab == 1 ) {
       
        ImGui::Checkbox( xor_str( "player##chams" ), &chams_cfg.m_enemy_chams );

        if( chams_cfg.m_enemy_chams ) {
            ImGui::Combo( xor_str( "##player_chams_type" ), &chams_cfg.m_enemy_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##player_chams_color" ), chams_cfg.m_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
          
            ImGui::Checkbox( xor_str( "player behind walls" ), &chams_cfg.m_enemy_chams_invisible );

            if( chams_cfg.m_enemy_chams_invisible ){
                ImGui::Combo( xor_str( "##player_hidden_chams_type_" ), &chams_cfg.m_invisible_enemy_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
                ImGui::ColorEdit4( xor_str( "##player_hidden_chams_color" ), chams_cfg.m_invisible_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
            }
         }

        ImGui::Checkbox( xor_str( "viewmodel" ), &chams_cfg.m_arms_chams );

        if( chams_cfg.m_arms_chams ) {
            ImGui::Combo( xor_str( "##viewmodel_chams_type" ), &chams_cfg.m_arms_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##viewmodel_chams_color" ), chams_cfg.m_arms_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        ImGui::Checkbox( xor_str( "weapon" ), &chams_cfg.m_wpn_chams );

        if( chams_cfg.m_wpn_chams ) {
            ImGui::Combo( xor_str( "##weapon_chams_type" ), &chams_cfg.m_wpn_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##weapon_chams_color" ), chams_cfg.m_wpn_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        ImGui::Checkbox( xor_str( "shot record" ), &chams_cfg.m_shot_chams );

        if( chams_cfg.m_shot_chams ) {
            ImGui::Combo( xor_str( "##shot_chams_type" ), &chams_cfg.m_shot_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##shot_chams_color" ), chams_cfg.m_shot_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        ImGui::Checkbox( xor_str( "history" ), &chams_cfg.m_history_chams );

        if( chams_cfg.m_history_chams ) {
            ImGui::Combo( xor_str( "##history_chams_type" ), &chams_cfg.m_history_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##history_chams_color" ), chams_cfg.m_history_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }


        ImGui::Checkbox( xor_str( "local chams" ), &chams_cfg.m_local_chams );

        if( chams_cfg.m_local_chams ) {
            ImGui::Combo( xor_str( "##local_chams_type" ), &chams_cfg.m_local_chams_type, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );
            ImGui::ColorEdit4( xor_str( "##local_chams_color" ), chams_cfg.m_local_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        ImGui::Checkbox( xor_str( "local player transparency in scope" ), &cfg.m_blend_in_scope );

        if( cfg.m_blend_in_scope )
            ImGui::SliderInt( xor_str( "##transparency_amount" ), &cfg.m_blend_in_scope_val, 0, 100 );
    }
    else if( visual_sub_tab == 2 ) {
         if ( ImGui::BeginCombo( xor_str( "removals" ), "" ) ) {
            static bool removal_vars[ IM_ARRAYSIZE( removals ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( removals ); ++i ) {
                removal_vars[ i ] = csgo::hacks::g_visuals->cfg( ).m_removals & ( 1 << i );

                ImGui::Selectable(
                    removals[ i ], &removal_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( removal_vars[ i ] )
                    csgo::hacks::g_visuals->cfg( ).m_removals |= ( 1 << i );
                else
                    csgo::hacks::g_visuals->cfg( ).m_removals &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }

        ImGui::Combo( xor_str( "sky box" ), &cfg.m_skybox_type, skybox_list, IM_ARRAYSIZE( skybox_list ) );

        ImGui::Checkbox( xor_str( "map effects modulation" ), &cfg.m_tone_map_modulation );

        if( cfg.m_tone_map_modulation ) {
            ImGui::SliderInt( xor_str( "bloom amount" ), &cfg.m_bloom, 0, 750 );

            ImGui::SliderInt( xor_str( "exposure amount" ), &cfg.m_exposure, 0, 2000 );
        }

        ImGui::ColorEdit4( xor_str( "modulate world color##ye" ), cfg.m_world_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::ColorEdit4( xor_str( "modulate props color##ye" ), cfg.m_props_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::ColorEdit4( xor_str( "modulate sky color##ye" ), cfg.m_sky_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::Checkbox( xor_str( "indicate manual anti-aim" ), &cfg.m_manuals_indication );

        ImGui::Checkbox( xor_str( "show weapon in scope" ), &cfg.m_show_weapon_in_scope );

        ImGui::Checkbox( xor_str( "enemy bullet tracers" ), &cfg.m_enemy_bullet_tracers );

        ImGui::ColorEdit4( xor_str( "enemy trace color" ), cfg.m_enemy_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        
        ImGui::Checkbox( xor_str( "local bullet tracers" ), &cfg.m_bullet_tracers );

        ImGui::ColorEdit4( xor_str( "local trace color" ), cfg.m_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::Checkbox( xor_str( "world hitmarker" ), &cfg.m_hit_markers );
        ImGui::Checkbox( xor_str( "bullet impacts" ), &cfg.m_bullet_impacts );

        ImGui::Checkbox( xor_str( "molotov timer" ), &cfg.m_molotov_timer );

        ImGui::Checkbox( xor_str( "smoke timer" ), &cfg.m_smoke_timer );

        ImGui::Checkbox( xor_str( "grenade projectiles" ), &cfg.m_grenade_projectiles );

        ImGui::Checkbox( xor_str( "draw dropped weapon text" ), &cfg.m_proj_wpn );

        ImGui::Checkbox( xor_str( "draw dropped weapon icon" ), &cfg.m_proj_icon );

        ImGui::Checkbox( xor_str( "modulate shadows direction" ), &cfg.m_shadows_modulation );

        ImGui::SliderFloat( "x dir", &cfg.m_x_dir, -100.f, 100.f, "%.1f" );
        ImGui::SliderFloat( "y dir", &cfg.m_y_dir, -100.f, 100.f, "%.1f" );
        ImGui::SliderFloat( "z dir", &cfg.m_z_dir, -100.f, 100.f, "%.1f" );

        ImGui::Checkbox( xor_str( "modulate world fog" ), &cfg.m_fog );

        ImGui::ColorEdit4( xor_str( "fog color" ), cfg.m_fog_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::SliderInt( "fog start", &cfg.m_fog_start, 0, 1000 );
        ImGui::SliderInt( "fog end", &cfg.m_fog_end, 100, 1100 );
        ImGui::SliderInt( "fog density", &cfg.m_fog_density, 0, 100 );
    }
}

void draw_movement( ) {
    auto& move_cfg = csgo::hacks::g_move->cfg( );

    // movement
    ImGui::Checkbox( "bunny hop##misc", &move_cfg.m_bhop );
    ImGui::Checkbox( "automatic strafe##misc", &move_cfg.m_auto_strafe );
    ImGui::Checkbox( "standalone quick stop##misc", &move_cfg.m_fast_stop );
    ImGui::Checkbox( "infinite duck##misc", &move_cfg.m_infinity_duck ); 
    g_key_binds->add_keybind( "slow motion##misc", &move_cfg.m_slow_walk, false, 140.f );
    g_key_binds->add_keybind( xor_str( "auto peek" ), &csgo::hacks::g_move->cfg( ).m_auto_peek_key, false );
}

void draw_anti_aim( ) {

    auto& cfg = csgo::hacks::g_anti_aim->cfg( );
    ImGui::Checkbox( xor_str( "master switch##antiaim" ), &cfg.m_master_switch );
    ImGui::Combo( xor_str( "pitch##antiaim" ), &cfg.m_pitch, pitch_type, IM_ARRAYSIZE( pitch_type ) );
    ImGui::SliderFloat( xor_str( "yaw##antiaim" ), &cfg.m_yaw, -180.f, 180.f, "%.1f" );
    ImGui::SliderFloat( xor_str( "yaw jitter##antiaim" ), &cfg.m_jitter_yaw, -180.f, 180.f, "%.1f" );
    ImGui::Checkbox( xor_str( "fake body##antiaim" ), &cfg.m_body_yaw );
  
    if( cfg.m_body_yaw ) {
        ImGui::SliderFloat( xor_str( "##antiaim_body_yaw_angle" ), &cfg.m_body_yaw_angle, -180.f, 180.f, "%.1f" );
        ImGui::Checkbox( xor_str( "dynamic fake body##antiaim" ), &cfg.m_dynamic_body_yaw );
    }

    ImGui::Checkbox( xor_str( "manual anti-aim" ), &cfg.m_manual_antiaim );

    if( cfg.m_manual_antiaim ) {
        g_key_binds->add_keybind( xor_str( "manual left" ), &cfg.m_left_manual, false );
        g_key_binds->add_keybind( xor_str( "manual right" ), &cfg.m_right_manual, false );
        g_key_binds->add_keybind( xor_str( "manual back" ), &cfg.m_back_manual, false );
        g_key_binds->add_keybind( xor_str( "manual forwards" ), &cfg.m_forward_manual, false );
        ImGui::Checkbox( xor_str( "ignore distortion on manual" ), &cfg.m_ignore_distortion_manual );
    }

    ImGui::Checkbox( xor_str( "distortion##antiaim" ), &cfg.m_should_distort );

    if( cfg.m_should_distort ) {
        ImGui::SliderFloat( xor_str( "speed##antiaim" ), &cfg.m_distort_speed, 0.f, 100.f, "%.1f" );
        ImGui::SliderFloat( xor_str( "factor##antiaim" ), &cfg.m_distort_factor, 0.f, 100.f, "%.1f" );
        ImGui::Checkbox( xor_str( "force turn##antiaim" ), &cfg.m_force_turn );
        ImGui::Checkbox( xor_str( "shift##antiaim" ), &cfg.m_shift );

        if( cfg.m_shift ) {
            ImGui::SliderInt( xor_str( "await##antiaim" ), &cfg.m_await_shift, 0, 14, "%ix" );
            ImGui::SliderFloat( xor_str( "factor##antiaim_shift" ), &cfg.m_shift_factor, 0.f, 100.f, "%.1f" );
        }
    }
   
    g_key_binds->add_keybind( xor_str( "freestanding" ), &cfg.m_freestand, true );
    ImGui::Checkbox( "ignore distortion when freestanding##antiaim", &cfg.m_ignore_distortion_freestand );

    g_key_binds->add_keybind( xor_str( "fake flick" ), &cfg.m_fake_flick, true );

    ImGui::Checkbox( xor_str( "fake lag##antiaim" ), &cfg.m_should_fake_lag );

    if( cfg.m_should_fake_lag )
        ImGui::SliderInt( xor_str( "##antiaim_ticks_to_choke" ), &cfg.m_ticks_to_choke, 2, 15 );
}

void draw_config( ) {

    int& slot = csgo::g_ctx->cur_cfg_slot( );
    ImGui::Combo( xor_str( "cfg slot" ), &slot, cfg_slots, IM_ARRAYSIZE( cfg_slots ) );

    if( ImGui::Button( "load" ) )
        sdk::g_cfg->load( cfg_slots[slot] );

    if( ImGui::Button( "save" ) )
        sdk::g_cfg->save( cfg_slots[slot] );

    ImGui::Combo( xor_str( "knife changer" ), &csgo::hacks::g_skins->cfg( ).m_knife_type, knife_type, IM_ARRAYSIZE( knife_type ) );
    ImGui::Combo( xor_str( "weapon" ), &cur_wpn_for_skins, wpns_for_skins, IM_ARRAYSIZE( wpns_for_skins ) );

    /* skins via cur_wpn_for_skins idx */

    switch( cur_wpn_for_skins ) {
    case 0:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_scar20, 1, 600 );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_g3sg1, 1, 600 );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_deagle, 1, 600 );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_elite, 1, 600 );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_five_seven, 1, 600 );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_glock, 1, 600 );
        break;
    case 6:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ak47, 1, 600 );
        break;
    case 7:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_aug, 1, 600 );
        break;
    case 8:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_awp, 1, 600 );
        break;
    case 9:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_famas, 1, 600 );
        break;
    case 10:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_galil, 1, 600 );
        break;
    case 11:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m249, 1, 600 );
        break;
    case 12:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4a4, 1, 600 );
        break;
    case 13:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mac10, 1, 600 );
        break;
    case 14:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p90, 1, 600 );
        break;
    case 15:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ump45, 1, 600 );
        break;
    case 16:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_xm1014, 1, 600 );
        break;
    case 17:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_bizon, 1, 600 );
        break;
    case 18:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mag7, 1, 600 );
        break;
    case 19:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_negev, 1, 600 );
        break;
    case 20:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sawedoff, 1, 600 );
        break;
    case 21:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_tec9, 1, 600 );
        break;
    case 22:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p2000, 1, 600 );
        break;
    case 23:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp7, 1, 600 );
        break;
    case 24:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp9, 1, 600 );
        break;
    case 25:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_nova, 1, 600 );
        break;
    case 26:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p250, 1, 600 );
        break;
    case 27:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sg553, 1, 600 );
        break;
    case 28:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ssg08, 1, 600 );
        break;
    case 29:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4_s, 1, 600 );
        break;
    case 30:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_usp_s, 1, 600 );
        break;
    case 31:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_cz75a, 1, 600 );
        break;
    case 32:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_revolver, 1, 600 );
        break;
    case 33:
        ImGui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_knifes, 1, 600 );
        break;
    }
}

namespace csgo {

    void c_menu::render( ) {

        if( m_main.m_hidden )
            return;

        ImGui::Begin( "hehe", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse );
        ImGui::StyleColorsClassic( );

        ImGui::Combo( "current tab", &m_main.m_current_tab, tabs, IM_ARRAYSIZE( tabs ) );

        switch( m_main.m_current_tab ) {
        case 0:
            draw_rage( );
            break;
        case 1:
            draw_anti_aim( );
            break;
        case 2:
            draw_visuals( );
            break;
        case 3:
            draw_movement( );
            break;
        case 4:
            draw_misc( );
            break;
        case 5:
            draw_config( );
            break;
        default:
            break;
        }

        ImGui::End( );
    }
}