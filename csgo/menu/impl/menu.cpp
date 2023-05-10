#include "../../csgo.hpp"

int cur_weapon { };
int cur_wpn_for_skins{ };

void draw_misc( ) { 

    auto& misc_cfg = csgo::hacks::g_misc->cfg( );
    auto& visuals_cfg = csgo::hacks::g_visuals->cfg( );

    if( gui::BeginCombo( xor_str( "notification logs" ), "" ) ) { 
        static bool notification_vars[ IM_ARRAYSIZE( notification_logs ) ]{ };

        for( std::size_t i{ }; i < IM_ARRAYSIZE( notification_logs ); ++i ) { 
            notification_vars[ i ] = misc_cfg.m_notification_logs & ( 1 << i );

            gui::Selectable(
                notification_logs[ i ], &notification_vars[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if( notification_vars[ i ] )
                misc_cfg.m_notification_logs |= ( 1 << i );
            else
                misc_cfg.m_notification_logs &= ~( 1 << i );
        }

        gui::EndCombo( );
    }

    gui::SliderInt( "view-model x", &misc_cfg.m_view_model_x, -10, 10 );
    gui::SliderInt( "view-model y", &misc_cfg.m_view_model_y, -10, 10 );
    gui::SliderInt( "view-model z", &misc_cfg.m_view_model_z, -10, 10 );
    gui::SliderFloat( "view-model fov amount", &visuals_cfg.m_view_model_fov, 60.f, 130.f );
    gui::SliderInt( "fov amount", &misc_cfg.m_camera_distance, 45, 130 );

    // miscellaneous
    gui::Checkbox( "buy bot", &misc_cfg.m_buy_bot );

    gui::Combo( xor_str( "primary" ), &misc_cfg.m_buy_bot_snipers, snipers_arr, IM_ARRAYSIZE( snipers_arr ) );

    gui::Combo( xor_str( "secondary" ), &misc_cfg.m_buy_bot_pistols, pistols_arr, IM_ARRAYSIZE( pistols_arr ) );

    if( gui::BeginCombo( xor_str( "additionals" ), "" ) ) { 
        static bool additional_vars[ IM_ARRAYSIZE( additional_arr ) ]{ };

        for( std::size_t i{ }; i < IM_ARRAYSIZE( additional_arr ); ++i ) { 
            additional_vars[ i ] = misc_cfg.m_buy_bot_additional & ( 1 << i );

            gui::Selectable(
                additional_arr[ i ], &additional_vars[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if( additional_vars[ i ] )
                misc_cfg.m_buy_bot_additional |= ( 1 << i );
            else
                misc_cfg.m_buy_bot_additional &= ~( 1 << i );
        }

        gui::EndCombo( );
    }

    gui::Checkbox( "clantag spammer##misc", &misc_cfg.m_clan_tag );
    g_key_binds->add_keybind( xor_str( "ping spike##misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_ping_spike_key, false, 140 );
    gui::SliderFloat( xor_str( "##fake_latency_value_misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_to_spike, 50.f, 800.f, "%.1f" );
    g_key_binds->add_keybind( "third person##misc", &misc_cfg.m_third_person_key, false, 140.f );
    gui::SliderFloat( "##third_person_dist_misc", &misc_cfg.m_third_person_dist, 45.f, 150.f );
    gui::Checkbox( "force thirdperson when spectating##misc", &misc_cfg.m_force_thirdperson_dead );

    gui::Checkbox( "aspect ratio##misc", &misc_cfg.m_aspect_ratio );

    if( misc_cfg.m_aspect_ratio )
        gui::SliderFloat( "##aspect_ratio_amt_misc", &misc_cfg.m_aspect_ratio_value, 0.f, 2.f );

    gui::Checkbox( "hitsound", &misc_cfg.m_hit_marker_sound );

    gui::Checkbox( "show spectators", &misc_cfg.m_spectators );
    gui::Checkbox( "indicators", &misc_cfg.m_key_binds );

    gui::Checkbox( xor_str( "preserve killfeed" ), &misc_cfg.m_kill_feed );
    gui::Checkbox( xor_str( "filter console" ), &misc_cfg.m_filter_console );
    gui::Checkbox( xor_str( "force crosshair" ), &misc_cfg.m_force_crosshair );
    
}

#pragma region rage
void rage_hitbox( ) { 

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) { 
    case 0:
        if( gui::BeginCombo( xor_str( "target hitboxes##rage_auto" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_scar_hitboxes & ( 1 << i );
                gui::Selectable( hitboxes[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_scar_hitboxes |= ( 1 << i );
                else
                    cfg.m_scar_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 1:
        if( gui::BeginCombo( xor_str( "target hitboxes##rage_scout" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_scout_hitboxes & ( 1 << i );

                gui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_scout_hitboxes |= ( 1 << i );
                else
                    cfg.m_scout_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 2:
        if( gui::BeginCombo( xor_str( "target hitboxes##rage_awp" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_awp_hitboxes & ( 1 << i );

                gui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_awp_hitboxes |= ( 1 << i );
                else
                    cfg.m_awp_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 3:
        if( gui::BeginCombo( xor_str( "target hitboxes##rage_heavy_pistols" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_heavy_pistol_hitboxes & ( 1 << i );

                gui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_heavy_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_heavy_pistol_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 4:
        if( gui::BeginCombo( xor_str( "hitboxes##rage_pistol" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_pistol_hitboxes & ( 1 << i );

                gui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_pistol_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 5:
        if( gui::BeginCombo( xor_str( "hitboxes##rage_other" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_other_hitboxes & ( 1 << i );

                gui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_other_hitboxes |= ( 1 << i );
                else
                    cfg.m_other_hitboxes &= ~( 1 << i );
            }

            gui::EndCombo( );
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
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_scar_point_scale, 30.f, 100.f, cfg.m_scar_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 1:
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_scout_point_scale, 30.f, 100.f, cfg.m_scout_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 2:
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_awp_point_scale, 30.f, 100.f, cfg.m_awp_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 3:
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_heavy_pistol_point_scale, 30.f, 100.f, cfg.m_heavy_pistol_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 4:
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_pistol_point_scale, 30.f, 100.f, cfg.m_pistol_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    case 5:
        gui::SliderInt( xor_str( "point scale" ), &cfg.m_other_point_scale, 30.f, 100.f, cfg.m_other_point_scale <= 30 ? "auto" : "%.1f" );
        break;
    default:
        break;
    }
}
void rage_damage_ovr( ) { 



    auto& cfg = csgo::hacks::g_aim_bot->cfg( );


    switch( cur_weapon ) { 
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
        gui::SliderInt( xor_str( "##rage_dmg_override_scar" ), &cfg.m_scar_min_dmg_on_key, 1, 126 );
        break;
    case 1:
        gui::SliderInt( xor_str( "##rage_dmg_override_scout" ), &cfg.m_scout_min_dmg_on_key, 1, 126 );
        break;
    case 2:
        gui::SliderInt( xor_str( "##rage_dmg_override_awp" ), &cfg.m_awp_min_dmg_on_key, 1, 126 );
        break;
    case 3:
        gui::SliderInt( xor_str( "##rage_dmg_override_heavy_pistol" ), &cfg.m_heavy_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 4:
        gui::SliderInt( xor_str( "##rage_dmg_override_pistol" ), &cfg.m_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 5:
        gui::SliderInt( xor_str( "##rage_dmg_override_other" ), &cfg.m_other_min_dmg_on_key, 1, 126 );
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
        gui::Combo( xor_str( "auto stop##rage_scar" ), &cfg.m_auto_stop_type_scar, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 1:
        gui::Combo( xor_str( "auto stop##rage_scout" ), &cfg.m_auto_stop_type_scout, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 2:
        gui::Combo( xor_str( "auto stop##rage_awp" ), &cfg.m_auto_stop_type_awp, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 3:
        gui::Combo( xor_str( "auto stop##rage_heavy_pistol" ), &cfg.m_auto_stop_type_heavy_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 4:
        gui::Combo( xor_str( "auto stop##rage_pistol" ), &cfg.m_auto_stop_type_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    case 5:
        gui::Combo( xor_str( "auto stop##rage_other" ), &cfg.m_auto_stop_type_other, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1 );
        break;
    default:
        break;
    }

    if( gui::BeginCombo( xor_str( "stop modifiers##rage" ), "" ) ) { 
        static bool stop_modifiers[ IM_ARRAYSIZE( auto_stop_modifiers ) ]{ };

        for( std::size_t i{ }; i < IM_ARRAYSIZE( auto_stop_modifiers ); ++i ) { 
            stop_modifiers[ i ] = cfg.m_stop_modifiers & ( 1 << i );

            gui::Selectable( 
                auto_stop_modifiers[ i ], &stop_modifiers[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if( stop_modifiers[ i ] )
                cfg.m_stop_modifiers |= ( 1 << i );
            else
                cfg.m_stop_modifiers &= ~( 1 << i );
        }

        gui::EndCombo( );
    }

}
void rage_hitchance( ) { 

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) { 
    case 0:
        gui::SliderFloat( xor_str( "hit chance##rage_scar" ), &cfg.m_hit_chance_scar, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        gui::SliderFloat( xor_str( "hit chance##rage_scout" ), &cfg.m_hit_chance_scout, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        gui::SliderFloat( xor_str( "hit chance##rage_awp" ), &cfg.m_hit_chance_awp, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        gui::SliderFloat( xor_str( "hit chance##rage_heavy_pistol" ), &cfg.m_hit_chance_heavy_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        gui::SliderFloat( xor_str( "hit chance##rage_pistol" ), &cfg.m_hit_chance_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        gui::SliderFloat( xor_str( "hit chance##rage_other" ), &cfg.m_hit_chance_other, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }
}

void rage_body_aim( ) { 

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) { 
    case 0:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_scar" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scar & ( 1 << i );
                gui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scar &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 1:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_scout" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scout & ( 1 << i );
                gui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scout &= ~ ( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 2:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_awp" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[i] = cfg.m_force_body_conditions_awp & ( 1 << i );
                gui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[i] )
                    cfg.m_force_body_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_awp &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 3:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_heavy_pistol" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_heavy_pistol & ( 1 << i );
                gui::Selectable( body_conditions[i], &hitgroups_vars[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_heavy_pistol |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_heavy_pistol &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 4:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_pistol" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_pistol & ( 1 << i );
                gui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_pistol &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 5:
        if( gui::BeginCombo( xor_str( "prefer body-aim##rage_other" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) { 

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_other & ( 1 << i );
                gui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_other &= ~( 1 << i );
            }

            gui::EndCombo( );
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
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_scar" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scar & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scar &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 1:
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_scout" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scout & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scout &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 2:
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_awp" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_awp & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_awp &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 3:
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_heavy_pistol" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_heavy_pistol & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_heavy_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_heavy_pistol &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 4:
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_pistol" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_pistol & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_pistol &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        break;
    case 5:
        if( gui::BeginCombo( xor_str( "prefer head-aim##rage_other" ), "" ) ) { 
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) { 
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_other & ( 1 << i );

                gui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_other &= ~( 1 << i );
            }

            gui::EndCombo( );
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
        gui::SliderInt( xor_str( "minimum damage##rage_scar" ), &cfg.m_min_dmg_scar, 1.f, 126.f, "%.1f" );
        break;
    case 1:
        gui::SliderInt( xor_str( "minimum damage##rage_scout" ), &cfg.m_min_dmg_scout, 1.f, 126.f, "%.1f" );
        break;
    case 2:
        gui::SliderInt( xor_str( "minimum damage##rage_awp" ), &cfg.m_min_dmg_awp, 1.f, 126.f, "%.1f" );
        break;
    case 3:
        gui::SliderInt( xor_str( "minimum damage##rage_heavy_pistol" ), &cfg.m_min_dmg_heavy_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 4:
        gui::SliderInt( xor_str( "minimum damage##rage_pistol" ), &cfg.m_min_dmg_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 5:
        gui::SliderInt( xor_str( "minimum damage##rage_other" ), &cfg.m_min_dmg_other, 1.f, 126.f, "%.1f" );
        break;
    default:
        break;
    }
}

void rage_dt_hitchance( ) { 

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) { 
    case 0:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_scar" ), &cfg.m_dt_hit_chance_scar, 0.f, 75.f, "%.1f" );
        break;
    case 1:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_scout" ), &cfg.m_dt_hit_chance_scout, 0.f, 75.f, "%.1f" );
        break;
    case 2:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_awp" ), &cfg.m_dt_hit_chance_awp, 0.f, 75.f, "%.1f" );
        break;
    case 3:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_heavy_pistol" ), &cfg.m_dt_hit_chance_heavy_pistol, 0.f, 75.f, "%.1f" );
        break;
    case 4:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_pistol" ), &cfg.m_dt_hit_chance_pistol, 0.f, 75.f, "%.1f" );
        break;
    case 5:
        gui::SliderFloat( xor_str( "double tap hit chance##rage_other" ), &cfg.m_dt_hit_chance_other, 0.f, 75.f, "%.1f" );
        break;
    default:
        break;
    }
}


void rage_dt_stop( ) { 

     auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) { 
    case 0:
        gui::Combo( xor_str( "double tap auto stop##rage_scar" ), &cfg.m_auto_stop_type_dt_scar, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 1:
        gui::Combo( xor_str( "double tap auto stop##rage_scout" ), &cfg.m_auto_stop_type_dt_scout, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 2:
        gui::Combo( xor_str( "double tap auto stop##rage_awp" ), &cfg.m_auto_stop_type_dt_awp, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 3:
        gui::Combo( xor_str( "double tap auto stop##rage_heavy_pistol" ), &cfg.m_auto_stop_type_dt_heavy_pistol, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 4:
        gui::Combo( xor_str( "double tap auto stop##rage_pistol" ), &cfg.m_auto_stop_type_dt_pistol, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    case 5:
        gui::Combo( xor_str( "double tap auto stop##rage_other" ), &cfg.m_auto_stop_type_dt_other, stop_type_dt, IM_ARRAYSIZE( stop_type_dt ), -1 );
        break;
    default:
        break;
    }
}

void draw_rage( ) { 
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    gui::Combo( xor_str( "current weapon##rage" ), &cur_weapon, wpns, IM_ARRAYSIZE( wpns ) );
    gui::Checkbox( xor_str( "master switch##rage" ), &cfg.m_rage_bot );
    gui::Checkbox( xor_str( "auto scope##rage" ), &cfg.m_auto_scope );
    gui::Checkbox( xor_str( "dynamic target limit##rage" ), &cfg.m_dynamic_limit );

    if( !cfg.m_dynamic_limit )
        gui::SliderInt( xor_str( "max targets per tick##rage" ), &cfg.m_max_targets, 2, 10 );

    gui::Combo( xor_str( "backtrack intensity##rage" ), &cfg.m_backtrack_intensity, scan_intensity, IM_ARRAYSIZE( scan_intensity ) );

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
    gui::Combo( "current visuals tab", &visual_sub_tab, visuals_tabs, IM_ARRAYSIZE( visuals_tabs ) );

    if( visual_sub_tab == 0 ) { 
        gui::Checkbox( xor_str( "shared esp" ), &cfg.m_shared_esp );
        gui::Checkbox( xor_str( "engine radar##esp" ), &cfg.m_engine_radar );
        gui::Checkbox( xor_str( "name##esp" ), &cfg.m_draw_name );
        gui::Checkbox( xor_str( "health bar" ), &cfg.m_draw_health );
        gui::Checkbox( xor_str( "bounding box" ), &cfg.m_draw_box );
        gui::Checkbox( xor_str( "flags" ), &cfg.m_draw_flags );
        
        if( gui::BeginCombo( xor_str( "show flag" ), "" ) ) { 
            static bool flag_vars[ IM_ARRAYSIZE( esp_flags ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( esp_flags ); ++i ) { 
                flag_vars[ i ] = cfg.m_player_flags & ( 1 << i );

                gui::Selectable(
                    esp_flags[ i ], &flag_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( flag_vars[ i ] )
                    cfg.m_player_flags |= ( 1 << i );
                else
                    cfg.m_player_flags &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
                
        if( gui::BeginCombo( xor_str( "weapon" ), "" ) ) { 
            static bool weapon_vars[ IM_ARRAYSIZE( weapon_selection ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( weapon_selection ); ++i ) { 
                weapon_vars[ i ] = cfg.m_weapon_selection & ( 1 << i );

                gui::Selectable(
                    weapon_selection[ i ], &weapon_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( weapon_vars[ i ] )
                    cfg.m_weapon_selection |= ( 1 << i );
                else
                    cfg.m_weapon_selection &= ~( 1 << i );
            }

            gui::EndCombo( );
        } gui::SameLine( );
        gui::ColorEdit4( xor_str( "##wpn_txt_clr" ), cfg.m_wpn_text_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##wpn_icon_clr" ), cfg.m_wpn_icon_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "ammo bar" ), &cfg.m_wpn_ammo ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##ammo_color" ), cfg.m_wpn_ammo_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::Checkbox( xor_str( "body update" ), &cfg.m_draw_lby ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##update_bar_color" ), cfg.m_lby_upd_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::Checkbox( xor_str( "glow" ), &cfg.m_glow ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##glow_color" ), cfg.m_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "footstep visualization" ), &cfg.m_foot_step_esp ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##foot_step_esp_clr" ), cfg.m_foot_step_esp_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "out of fov arrows" ), &cfg.m_oof_indicator ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##oof_color" ), cfg.m_oof_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SliderInt( xor_str( "out of fov radius" ), &cfg.m_oof_radius, 5, 850 );
        gui::SliderInt( xor_str( "out of fov size" ), &cfg.m_oof_size, 4, 25 );
    }
    else if( visual_sub_tab == 1 ) { 

        gui::Checkbox( xor_str( "ragdoll chams##chams" ), &chams_cfg.m_ragdoll_chams );
        gui::Checkbox( xor_str( "visible player##chams" ), &chams_cfg.m_enemy_chams );

        if( chams_cfg.m_enemy_chams ) { 
            gui::Combo( xor_str( "##player_chams_type" ), &chams_cfg.m_enemy_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##player_chams_color" ), chams_cfg.m_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
          
            gui::Checkbox( xor_str( "player behind walls" ), &chams_cfg.m_enemy_chams_invisible );

            if( chams_cfg.m_enemy_chams_invisible ){ 
                gui::Combo( xor_str( "##player_hidden_chams_type_" ), &chams_cfg.m_invisible_enemy_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
                gui::ColorEdit4( xor_str( "##player_hidden_chams_color" ), chams_cfg.m_invisible_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
            }

            gui::Checkbox( xor_str( "visible overlay##chams_overlay" ), &chams_cfg.m_enemy_chams_overlay );

            if( chams_cfg.m_enemy_chams_overlay ) {
                gui::Combo( xor_str( "##player_chams_type_overlay" ), &chams_cfg.m_enemy_chams_overlay_type, chams_overlay_type, IM_ARRAYSIZE( chams_overlay_type ) ); gui::SameLine( );
                gui::ColorEdit4( xor_str( "##player_chams_color_overlay" ), chams_cfg.m_enemy_clr_overlay, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
            }
          
            gui::Checkbox( xor_str( "overlay behind walls" ), &chams_cfg.m_enemy_chams_overlay_invisible );

            if( chams_cfg.m_enemy_chams_overlay_invisible ){ 
                gui::Combo( xor_str( "##player_hidden_chams_type__overlay" ), &chams_cfg.m_invisible_enemy_chams_overlay_type, chams_overlay_type, IM_ARRAYSIZE( chams_overlay_type ) ); gui::SameLine( );
                gui::ColorEdit4( xor_str( "##player_hidden_chams_color_overlay" ), chams_cfg.m_invisible_enemy_clr_overlay, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
            }
         }

        gui::Checkbox( xor_str( "viewmodel" ), &chams_cfg.m_arms_chams );

        if( chams_cfg.m_arms_chams ) { 
            gui::Combo( xor_str( "##viewmodel_chams_type" ), &chams_cfg.m_arms_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##viewmodel_chams_color" ), chams_cfg.m_arms_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        gui::Checkbox( xor_str( "weapon" ), &chams_cfg.m_wpn_chams );

        if( chams_cfg.m_wpn_chams ) { 
            gui::Combo( xor_str( "##weapon_chams_type" ), &chams_cfg.m_wpn_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##weapon_chams_color" ), chams_cfg.m_wpn_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        gui::Checkbox( xor_str( "shot record" ), &chams_cfg.m_shot_chams );

        if( chams_cfg.m_shot_chams ) { 
            gui::Combo( xor_str( "##shot_chams_type" ), &chams_cfg.m_shot_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##shot_chams_color" ), chams_cfg.m_shot_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        gui::Checkbox( xor_str( "history" ), &chams_cfg.m_history_chams );

        if( chams_cfg.m_history_chams ) { 
            gui::Combo( xor_str( "##history_chams_type" ), &chams_cfg.m_history_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##history_chams_color" ), chams_cfg.m_history_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        }

        gui::Checkbox( xor_str( "local chams" ), &chams_cfg.m_local_chams );

        if( chams_cfg.m_local_chams ) { 
            gui::Combo( xor_str( "##local_chams_type" ), &chams_cfg.m_local_chams_type, chams_type, IM_ARRAYSIZE( chams_type ) ); gui::SameLine( );
            gui::ColorEdit4( xor_str( "##local_chams_color" ), chams_cfg.m_local_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

            gui::Checkbox( xor_str( "local chams overlay" ), &chams_cfg.m_local_chams_overlay );

            if( chams_cfg.m_local_chams_overlay ){ 
                gui::Combo( xor_str( "##local_overlay_type" ), &chams_cfg.m_local_overlay_type, chams_overlay_type, IM_ARRAYSIZE( chams_overlay_type ) ); gui::SameLine( );
                gui::ColorEdit4( xor_str( "##local_overlay_clr" ), chams_cfg.m_local_overlay_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
            }
        }

        gui::Checkbox( xor_str( "local player transparency in scope" ), &cfg.m_blend_in_scope );

        if( cfg.m_blend_in_scope )
            gui::SliderInt( xor_str( "##transparency_amount" ), &cfg.m_blend_in_scope_val, 0, 100 );
    }
    else if( visual_sub_tab == 2 ) { 
         if( gui::BeginCombo( xor_str( "removals" ), "" ) ) { 
            static bool removal_vars[ IM_ARRAYSIZE( removals ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( removals ); ++i ) { 
                removal_vars[ i ] = cfg.m_removals & ( 1 << i );

                gui::Selectable(
                    removals[ i ], &removal_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( removal_vars[ i ] )
                    cfg.m_removals |= ( 1 << i );
                else
                    cfg.m_removals &= ~( 1 << i );
            }

            gui::EndCombo( );
        }

        gui::Combo( xor_str( "sky box" ), &cfg.m_skybox_type, skybox_list, IM_ARRAYSIZE( skybox_list ) ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##modulate_sky_color" ), cfg.m_sky_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "map effects modulation" ), &cfg.m_tone_map_modulation );

        if( cfg.m_tone_map_modulation ) { 
            gui::SliderInt( xor_str( "bloom amount" ), &cfg.m_bloom, 0, 750 );

            gui::SliderInt( xor_str( "exposure amount" ), &cfg.m_exposure, 0, 2000 );
        }

        gui::ColorEdit4( xor_str( "modulate world color" ), cfg.m_world_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::ColorEdit4( xor_str( "modulate props color" ), cfg.m_props_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "indicate manual anti-aim" ), &cfg.m_manuals_indication ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##indicate_manual_anti_aim" ), cfg.m_manuals_indication_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "show weapon in scope" ), &cfg.m_show_weapon_in_scope );

        if( gui::BeginCombo( xor_str( "bullet tracer options" ), "" ) ) { 
            static bool tracer_vars[ IM_ARRAYSIZE( bullet_tracers_options ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( bullet_tracers_options ); ++i ) { 
                tracer_vars[ i ] = cfg.m_bullet_tracer_selection & ( 1 << i );

                gui::Selectable(
                    bullet_tracers_options[ i ], &tracer_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( tracer_vars[ i ] )
                    cfg.m_bullet_tracer_selection |= ( 1 << i );
                else
                    cfg.m_bullet_tracer_selection &= ~( 1 << i );
            }

            gui::EndCombo( );
        }

        gui::ColorEdit4( xor_str( "local##bullet_tracers_clr" ), cfg.m_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "enemy##team_bullet_tracers_clr" ), cfg.m_enemy_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "friendly##friendly_grenade_proximity_warning_clr" ), cfg.m_team_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        if( gui::BeginCombo( xor_str( "nade trajectory options" ), "" ) ) { 
            static bool trajectory_vars[ IM_ARRAYSIZE( grenade_traj_options ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( grenade_traj_options ); ++i ) { 
                trajectory_vars[ i ] = cfg.m_grenade_trajectory_options & ( 1 << i );

                gui::Selectable(
                    grenade_traj_options[ i ], &trajectory_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( trajectory_vars[ i ] )
                    cfg.m_grenade_trajectory_options |= ( 1 << i );
                else
                    cfg.m_grenade_trajectory_options &= ~( 1 << i );
            }

            gui::EndCombo( );
        }

        gui::ColorEdit4( xor_str( "local##grenade_trajectory_color" ), cfg.m_grenade_trajectory_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "enemy##grenade_proximity_warning_color" ), cfg.m_grenade_proximity_warning_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "friendly##grenade_proximity_warning_color" ), cfg.m_friendly_grenade_proximity_warning_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        if( gui::BeginCombo( xor_str( "hit markers option" ), "" ) ) { 
            static bool trajectory_vars[ IM_ARRAYSIZE( hit_markers_option ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hit_markers_option ); ++i ) { 
                trajectory_vars[ i ] = cfg.m_hit_markers_selection & ( 1 << i );

                gui::Selectable(
                    hit_markers_option[ i ], &trajectory_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( trajectory_vars[ i ] )
                    cfg.m_hit_markers_selection |= ( 1 << i );
                else
                    cfg.m_hit_markers_selection &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        gui::ColorEdit4( xor_str( "screen ##screen_hit_markers_clr" ), cfg.m_screen_hit_markers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( ); 
        gui::ColorEdit4( xor_str( "world ##world_hitmarker_clr" ), cfg.m_hit_markers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( ); 
        gui::ColorEdit4( xor_str( "damage ##damage_markers_clr" ), cfg.m_damage_markers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "bullet impacts" ), &cfg.m_bullet_impacts ); gui::SameLine( ); 
        gui::ColorEdit4( xor_str( "server ##bullet_impacts_sv_clr" ), cfg.m_bullet_impacts_server_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "client ##bullet_impacts_cl_clr" ), cfg.m_bullet_impacts_client_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "molotov timer" ), &cfg.m_molotov_timer );
        gui::SameLine( );
        gui::ColorEdit4( xor_str( "##molotov_range" ), cfg.m_molotov_range, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "smoke timer" ), &cfg.m_smoke_timer );

        if( gui::BeginCombo( xor_str( "bomb options" ), "" ) ) { 

            static bool bomb_vars[ IM_ARRAYSIZE( grenade_selection ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( grenade_selection ); ++i ) { 
                    bomb_vars[ i ] = cfg.m_draw_bomb_options & ( 1 << i );

                    gui::Selectable(
                        grenade_selection[ i ], &bomb_vars[ i ],
                        ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                    );

                    if( bomb_vars[ i ] )
                        cfg.m_draw_bomb_options |= ( 1 << i );
                    else
                        cfg.m_draw_bomb_options &= ~( 1 << i );
                }

           gui::EndCombo( );
        }        
        gui::ColorEdit4( xor_str( "text ##draw_c4_clr" ), cfg.m_draw_c4_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "icon ##draw_c4_icon_clr" ), cfg.m_draw_c4_icon_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "glow ##draw_c4_glow_clr" ), cfg.m_draw_c4_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::ColorEdit4( xor_str( "p_text ##draw_planted_c4_clr" ), cfg.m_draw_planted_c4_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "p_icon ##draw_planted_c4_icon_clr" ), cfg.m_draw_planted_c4_icon_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "p_glow ##draw_planted_c4_glow_clr" ), cfg.m_draw_planted_c4_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        if( gui::BeginCombo( xor_str( "grenade options" ), "" ) ) { 
            static bool grenade_vars[ IM_ARRAYSIZE( grenade_selection ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( grenade_selection ); ++i ) { 
                grenade_vars[ i ] = cfg.m_grenade_selection & ( 1 << i );

                gui::Selectable(
                    grenade_selection[ i ], &grenade_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( grenade_vars[ i ] )
                    cfg.m_grenade_selection |= ( 1 << i );
                else
                    cfg.m_grenade_selection &= ~( 1 << i );
            }

            gui::EndCombo( );
        } 
        gui::ColorEdit4( xor_str( "text ##grenade_projectiles_clr" ), cfg.m_grenade_projectiles_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "icon ##grenade_projectiles_icon_clr" ), cfg.m_grenade_projectiles_icon_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );

        gui::ColorEdit4( xor_str( "glow ##draw_grenade_glow_clr" ), cfg.m_draw_grenade_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        if( gui::BeginCombo( xor_str( "dropped weapon options" ), "" ) ) { 
            static bool dropped_weapons_vars[ IM_ARRAYSIZE( grenade_selection ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( grenade_selection ); ++i ) { 
                dropped_weapons_vars[ i ] = cfg.m_dropped_weapon_selection & ( 1 << i );

                gui::Selectable(
                    grenade_selection[ i ], &dropped_weapons_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( dropped_weapons_vars[ i ] )
                    cfg.m_dropped_weapon_selection |= ( 1 << i );
                else
                    cfg.m_dropped_weapon_selection &= ~( 1 << i );
            }

            gui::EndCombo( );
        }
        gui::ColorEdit4( xor_str( "text ##proj_wpn_clr" ), cfg.m_proj_wpn_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "icon ##proj_icon_clr" ), cfg.m_proj_icon_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "glow ##draw_weapon_glow_clr" ), cfg.m_draw_weapon_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::Checkbox( xor_str( "modulate shadows direction" ), &cfg.m_shadows_modulation );

        gui::SliderFloat( "x dir", &cfg.m_x_dir, -100.f, 100.f, "%.1f" );
        gui::SliderFloat( "y dir", &cfg.m_y_dir, -100.f, 100.f, "%.1f" );
        gui::SliderFloat( "z dir", &cfg.m_z_dir, -100.f, 100.f, "%.1f" );

        gui::Checkbox( xor_str( "modulate world fog" ), &cfg.m_fog ); gui::SameLine( );
        gui::ColorEdit4( xor_str( "##fog_color" ), cfg.m_fog_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        gui::SliderInt( "fog start", &cfg.m_fog_start, 0, 1000 );
        gui::SliderInt( "fog end", &cfg.m_fog_end, 100, 1100 );
    }
}

void draw_movement( ) { 
    auto& move_cfg = csgo::hacks::g_move->cfg( );

    // movement
    gui::Checkbox( "bunny hop##misc", &move_cfg.m_bhop );
    gui::Checkbox( "automatic strafe##misc", &move_cfg.m_auto_strafe );
    gui::Checkbox( "standalone quick stop##misc", &move_cfg.m_fast_stop );
    gui::Checkbox( "infinite stamina##misc", &move_cfg.m_infinity_duck ); 
    g_key_binds->add_keybind( "slow motion##misc", &move_cfg.m_slow_walk, false, 140.f );
    g_key_binds->add_keybind( xor_str( "auto peek" ), &move_cfg.m_auto_peek_key, false, 138.f ); gui::SameLine( );
    gui::ColorEdit4( xor_str( "##auto_peek_clr" ), move_cfg.m_auto_peek_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
}

void draw_anti_aim( ) { 

    auto& cfg = csgo::hacks::g_anti_aim->cfg( );
    gui::Checkbox( xor_str( "master switch##antiaim" ), &cfg.m_master_switch );
    gui::Combo( xor_str( "pitch##antiaim" ), &cfg.m_pitch, pitch_type, IM_ARRAYSIZE( pitch_type ) );
    gui::SliderFloat( xor_str( "yaw##antiaim" ), &cfg.m_yaw, -180.f, 180.f, "%.1f" );
    gui::SliderFloat( xor_str( "yaw jitter##antiaim" ), &cfg.m_jitter_yaw, -180.f, 180.f, "%.1f" );
    gui::Checkbox( xor_str( "fake body##antiaim" ), &cfg.m_body_yaw );
    gui::Checkbox( xor_str( "crooked aa##antiaim" ), &cfg.m_crooked_aa );

    if( cfg.m_body_yaw ) { 
        gui::SliderFloat( xor_str( "##antiaim_body_yaw_angle" ), &cfg.m_body_yaw_angle, -180.f, 180.f, "%.1f" );
        gui::Checkbox( xor_str( "dynamic fake body##antiaim" ), &cfg.m_dynamic_body_yaw );
    }

    gui::Checkbox( xor_str( "manual anti-aim" ), &cfg.m_manual_antiaim );

    if( cfg.m_manual_antiaim ) { 
        g_key_binds->add_keybind( xor_str( "manual left" ), &cfg.m_left_manual_key, true, 95 );
        g_key_binds->add_keybind( xor_str( "manual right" ), &cfg.m_right_manual_key, true, 95 );
        g_key_binds->add_keybind( xor_str( "manual back" ), &cfg.m_back_manual_key, true, 95 );
        g_key_binds->add_keybind( xor_str( "manual forwards" ), &cfg.m_forward_manual_key, true, 105 );
        gui::Checkbox( xor_str( "ignore distortion on manual" ), &cfg.m_ignore_distortion_manual );
    }

    gui::Checkbox( xor_str( "distortion##antiaim" ), &cfg.m_should_distort );

    if( cfg.m_should_distort ) { 
        gui::SliderFloat( xor_str( "speed##antiaim" ), &cfg.m_distort_speed, 0.f, 100.f, "%.1f" );
        gui::SliderFloat( xor_str( "factor##antiaim" ), &cfg.m_distort_factor, 0.f, 100.f, "%.1f" );
        gui::Checkbox( xor_str( "force turn##antiaim" ), &cfg.m_force_turn );
        gui::Checkbox( xor_str( "shift##antiaim" ), &cfg.m_shift );

        if( cfg.m_shift ) { 
            gui::SliderInt( xor_str( "await##antiaim" ), &cfg.m_await_shift, 0, 14, "%ix" );
            gui::SliderFloat( xor_str( "factor##antiaim_shift" ), &cfg.m_shift_factor, 0.f, 100.f, "%.1f" );
        }
    }
   
    gui::Checkbox( "ignore distortion when freestanding##antiaim", &cfg.m_ignore_distortion_freestand );

    gui::Checkbox( xor_str( "fake lag##antiaim" ), &cfg.m_should_fake_lag );

    if( gui::BeginCombo( xor_str( "fake lag triggers" ), "" ) ) { 
        static bool lag_vars[ IM_ARRAYSIZE( fake_lag_triggers ) ]{ };

        for( std::size_t i{ }; i < IM_ARRAYSIZE( fake_lag_triggers ); ++i ) { 
            lag_vars[ i ] = cfg.m_lag_triggers & ( 1 << i );

            gui::Selectable(
                fake_lag_triggers[ i ], &lag_vars[ i ],
                ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            );

            if( lag_vars[ i ] )
                cfg.m_lag_triggers |= ( 1 << i );
            else
                cfg.m_lag_triggers &= ~( 1 << i );
        }

        gui::EndCombo( );
    }

    if( cfg.m_should_fake_lag )
        gui::SliderInt( xor_str( "##antiaim_ticks_to_choke" ), &cfg.m_ticks_to_choke, 2, 16 );

    g_key_binds->add_keybind( xor_str( "freestand" ), &cfg.m_freestand_key, false, 150 );
    g_key_binds->add_keybind( xor_str( "fake-flick" ), &cfg.m_fake_flick_key, false, 150 );
}

void draw_config( ) { 

    int& slot = csgo::g_ctx->cur_cfg_slot( );
    gui::Combo( xor_str( "cfg slot" ), &slot, cfg_slots, IM_ARRAYSIZE( cfg_slots ) );

    if( gui::Button( "load" ) )
        sdk::g_cfg->load( cfg_slots[ slot ] );

    if( gui::Button( "save" ) )
        sdk::g_cfg->save( cfg_slots[ slot ] );

    gui::Combo( xor_str( "knife changer" ), &csgo::hacks::g_skins->cfg( ).m_knife_type, knife_type, IM_ARRAYSIZE( knife_type ) );
    gui::Combo( xor_str( "weapon" ), &cur_wpn_for_skins, wpns_for_skins, IM_ARRAYSIZE( wpns_for_skins ) );

    /* skins via cur_wpn_for_skins idx */

    switch( cur_wpn_for_skins ) { 
    case 0:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_scar20, 1, 600 );
        break;
    case 1:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_g3sg1, 1, 600 );
        break;
    case 2:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_deagle, 1, 600 );
        break;
    case 3:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_elite, 1, 600 );
        break;
    case 4:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_five_seven, 1, 600 );
        break;
    case 5:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_glock, 1, 600 );
        break;
    case 6:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ak47, 1, 600 );
        break;
    case 7:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_aug, 1, 600 );
        break;
    case 8:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_awp, 1, 600 );
        break;
    case 9:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_famas, 1, 600 );
        break;
    case 10:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_galil, 1, 600 );
        break;
    case 11:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m249, 1, 600 );
        break;
    case 12:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4a4, 1, 600 );
        break;
    case 13:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mac10, 1, 600 );
        break;
    case 14:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p90, 1, 600 );
        break;
    case 15:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ump45, 1, 600 );
        break;
    case 16:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_xm1014, 1, 600 );
        break;
    case 17:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_bizon, 1, 600 );
        break;
    case 18:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mag7, 1, 600 );
        break;
    case 19:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_negev, 1, 600 );
        break;
    case 20:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sawedoff, 1, 600 );
        break;
    case 21:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_tec9, 1, 600 );
        break;
    case 22:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p2000, 1, 600 );
        break;
    case 23:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp7, 1, 600 );
        break;
    case 24:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp9, 1, 600 );
        break;
    case 25:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_nova, 1, 600 );
        break;
    case 26:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p250, 1, 600 );
        break;
    case 27:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sg553, 1, 600 );
        break;
    case 28:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ssg08, 1, 600 );
        break;
    case 29:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4_s, 1, 600 );
        break;
    case 30:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_usp_s, 1, 600 );
        break;
    case 31:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_cz75a, 1, 600 );
        break;
    case 32:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_revolver, 1, 600 );
        break;
    case 33:
        gui::SliderInt( xor_str( "skin id" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_knifes, 1, 600 );
        break;
    }
}

namespace csgo { 

    void c_menu::render( ) { 

        if( m_main.m_hidden )
            return;

        gui::PushFont( hacks::g_misc->m_fonts.m_verdana );  

        gui::Begin( "These Perkies, they don't stop, I can't feel enough, yeah", nullptr, ImGuiWindowFlags_NoCollapse );
        gui::StyleColorsClassic( );

        if( gui::Button( "aimbot" ) ) 
            m_main.m_current_tab = 0; gui::SameLine( );
        if( gui::Button( "anti-aim" ) )
            m_main.m_current_tab = 1; gui::SameLine( );
        if( gui::Button( "visuals" ) )
            m_main.m_current_tab = 2; gui::SameLine( );
        if( gui::Button( "misc" ) )
            m_main.m_current_tab = 4; gui::SameLine( );
        if( gui::Button( "movement" ) )
            m_main.m_current_tab = 3; gui::SameLine( );
        if( gui::Button( "config" ) )
            m_main.m_current_tab = 5;

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

        gui::PopFont( );

        gui::End( );
    }
}