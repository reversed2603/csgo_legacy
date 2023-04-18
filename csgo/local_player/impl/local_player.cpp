#include "../../csgo.hpp"

namespace csgo {
    void c_local_player::update_prediction( ) const {
        if( valve::g_client_state.get( )->m_delta_tick <= 0 )
            return;

        valve::g_prediction->update( valve::g_client_state.get( )->m_delta_tick, true,
            valve::g_client_state.get( )->m_last_cmd_ack, valve::g_client_state.get( )->m_last_cmd_out + valve::g_client_state.get( )->m_choked_cmds
        );
    }

    void c_local_player::create_move( bool& send_packet,
        valve::user_cmd_t& cmd, valve::vfyd_user_cmd_t& vfyd_cmd
    ) {

  //      valve::g_cvar->find_var( ( "con_enable" ) )->set_int( 1 );
		//valve::g_cvar->find_var( ( "con_filter_enable" ) )->set_int( 1 );
		//valve::g_cvar->find_var( ( "con_filter_text" ) )->set_str( "[csgo_project]" );
        valve::g_cvar->find_var( xor_str( "r_jiggle_bones" ) )->set_int( 0 ); // fuck off bro

        send_packet = true;
        g_ctx->allow_defensive( ) = true;
        static auto draw_spec_static_prop = valve::g_cvar->find_var( xor_str( "r_DrawSpecificStaticProp" ) );

        if( draw_spec_static_prop->get_int( ) != 0 ) {
            draw_spec_static_prop->set_int( 0 );
        }

        const auto net_channel_info = valve::g_engine->net_channel_info( );
        if( !net_channel_info )
            return;
        {
            auto& net_info = g_ctx->net_info( );

            net_info.m_lerp = std::max( 
                g_ctx->cvars( ).m_cl_interp->get_float( ),
                g_ctx->cvars( ).m_cl_interp_ratio->get_float( ) / g_ctx->cvars( ).m_cl_updaterate->get_float( )
            );
            net_info.m_latency = { net_channel_info->latency( 1 ), net_channel_info->latency( 0 ) };
        }

        g_ctx->left_create_move( ) = false;

        static float prev_spawn_time = g_local_player->self( )->spawn_time( );
        if( prev_spawn_time != g_local_player->self( )->spawn_time( ) ) {
            hacks::g_eng_pred->reset_on_spawn( );

            hacks::g_exploits->m_ticks_allowed = 0;

            g_ctx->anim_data( ).m_local_data.reset( );
            hacks::g_local_sync->m_anim_layers = { };
            hacks::g_local_sync->m_pose_params = { };
                
            prev_spawn_time = g_local_player->self( )->spawn_time( );
        }

        static auto crosshair_data = valve::g_cvar->find_var( xor_str( "weapon_debug_spread_show" ) );
        if( g_local_player->cfg( ).m_force_crosshair && crosshair_data ) {
            crosshair_data->set_int( !g_local_player->self( )->scoped( ) ? 3 : 0 );
        }
        else {
            if( crosshair_data )
                crosshair_data->set_int( 0 );
        }

        g_ctx->ticks_data( ).m_tick_rate = crypt_float( 1.f ) / valve::g_global_vars.get( )->m_interval_per_tick;

        const auto old_angles = cmd.m_view_angles;
        auto old_angles_ = cmd.m_view_angles;
        sdk::qang_t wish_ang = cmd.m_view_angles;
        bool break_lc{ };
        bool can_send{ true };
        hacks::g_exploits->m_cl_move_manipulation = false;

        if( !hacks::g_exploits->try_to_recharge( send_packet, cmd ) ) {
            hacks::g_eng_pred->prepare( );

            g_ctx->anim_data( ).m_local_data.m_shot = false;

            hacks::g_move->handle( cmd, cmd.m_view_angles );

            hacks::g_anti_aim->fake_move( cmd );

            if( cmd.m_move.length( ) <= 22.f
                && g_local_player->cfg( ).m_shitty_mrx_servers )
                cmd.m_move = { };

            hacks::g_eng_pred->process( &cmd, true );

            hacks::g_exploits->handle_context( cmd );

            if( !g_ctx->can_shoot( )
                && m_weapon
                && !m_weapon->is_knife( )
                && m_weapon_info
                && m_weapon_info->m_type != valve::e_weapon_type::grenade
                && m_weapon->item_index( ) != valve::e_item_index::revolver )
                cmd.m_buttons &= ~valve::e_buttons::in_attack;

            for( std::ptrdiff_t i{ 1 }; i <= valve::g_global_vars.get( )->m_max_clients; ++i ) {
                auto& entry = hacks::g_lag_comp->entry( i - 1 );
                hacks::g_resolver->anti_freestand( entry );
            }

            hacks::g_visuals->on_create_move( cmd );

            hacks::g_move->auto_peek( old_angles_, cmd );

            hacks::g_anti_aim->handle_fake_lag( cmd );

            send_packet = !hacks::g_anti_aim->can_choke( );

            {
                if( ( m_weapon = self( )->weapon( ) ) )
                    m_weapon_info = m_weapon->info( );
                else
                    m_weapon_info = nullptr;
            }

            if( g_ctx->in_charge( ) && m_weapon
                && !m_weapon->is_knife( )
                && m_weapon_info
                && m_weapon_info->m_type != valve::e_weapon_type::grenade
                && m_weapon->item_index( ) != valve::e_item_index::revolver )
                cmd.m_buttons &= ~valve::e_buttons::in_attack;

            hacks::g_aim_bot->handle_ctx( cmd, send_packet );

            hacks::g_knife_bot->handle_knife_bot( cmd );

            static auto weapon_recoil_scale = valve::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) );
            if( ( cmd.m_buttons & valve::e_buttons::in_attack )
                && !g_ctx->was_shooting( ) ) {
                cmd.m_view_angles -= g_local_player->self( )->aim_punch( ) * weapon_recoil_scale->get_float( );

                cmd.m_view_angles.x( ) = std::remainder( cmd.m_view_angles.x( ), 360.f );
                cmd.m_view_angles.y( ) = std::remainder( cmd.m_view_angles.y( ), 360.f );

                cmd.m_view_angles.x( ) = std::clamp( cmd.m_view_angles.x( ), -89.f, 89.f );
                cmd.m_view_angles.y( ) = std::clamp( cmd.m_view_angles.y( ), -180.f, 180.f );
                cmd.m_view_angles.z( ) = 0.f;
            }   
             
            hacks::g_anti_aim->handle_pitch( cmd );

            hacks::g_anti_aim->handle_ctx( cmd, send_packet );
            
            // we're charged, not shooting, break lc is on and we are dting
            if( hacks::g_exploits->m_ticks_allowed >= 14 
                && !( cmd.m_buttons & valve::e_buttons::in_attack )
                && g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key ) ) {
                
                if( ( valve::g_client_state.get( )->m_last_cmd_out != hacks::g_exploits->m_recharge_cmd
                        && ( hacks::g_exploits->m_type == 2 || hacks::g_exploits->m_type == 3 )
                        && !hacks::g_exploits->m_shift_cycle
                        && hacks::g_exploits->is_peeking( wish_ang, 2.f, false ) 
                        && hacks::g_exploits->m_type != 4
                        && g_ctx->allow_defensive( )
                        && std::abs( valve::g_global_vars.get( )->m_tick_count - hacks::g_exploits->m_last_defensive_tick ) >= 14 ) ) { // dont unshift twice if interval is too small
                    
                    // unshift on trigger to break lc
                    hacks::g_exploits->m_last_defensive_tick = valve::g_global_vars.get( )->m_tick_count;
                    hacks::g_exploits->m_cur_shift_amount = 0;
                }
                else {
                    auto& local_data = hacks::g_eng_pred->local_data( ).at( cmd.m_number % crypt_int( 150 ) );

                    // NOTE: doesnt seem to be needed?
                    local_data.m_override_tick_base = local_data.m_restore_tick_base = true;
                    local_data.m_adjusted_tick_base = local_data.m_tick_base - hacks::g_exploits->m_ticks_allowed;
                    break_lc = send_packet = true;
                }
            }

            if( g_ctx->can_shoot( )
                && will_shoot( m_weapon, cmd ) ) {
                auto& anim_data = g_ctx->anim_data( ).m_local_data;

                anim_data.m_shot_cmd_number = cmd.m_number;

                if( m_weapon_info ) {
                    anim_data.m_shot_valid_wpn = true;
                }
                if( hacks::g_exploits->m_next_shift_amount
                    && cmd.m_buttons & valve::e_buttons::in_attack ) {
                    send_packet = g_ctx->send_packet( ) = true;
                }

                g_ctx->aim_shoot_pos( ) = g_ctx->shoot_pos( );

                if( g_ctx->anim_data( ).m_local_data.m_shot ) {
                }
                else {
                    hacks::g_shots->add( 
                        g_ctx->shoot_pos( ), nullptr,
                        hacks::g_exploits->m_next_shift_amount, cmd.m_number, valve::g_global_vars.get( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
                    );
                }

               hacks::g_exploits->m_cur_shift_amount = hacks::g_exploits->m_next_shift_amount;
            }

            hacks::g_local_sync->handle_ctx( cmd, send_packet );

            if( m_weapon ) {
                m_weapon->recoil_index( ) = hacks::g_eng_pred->recoil_index( );
                m_weapon->accuracy_penalty( ) = hacks::g_eng_pred->accuracy_penalty( );
            }
        }
        else {
            send_packet = false;

            hacks::g_exploits->handle_context( cmd );
            hacks::g_anti_aim->handle_pitch( cmd );
            hacks::g_anti_aim->handle_ctx( cmd, send_packet );

            cmd.m_tick = std::numeric_limits< int >::max( );

            auto& local_data = hacks::g_eng_pred->local_data( ).at( cmd.m_number % 150 );

            local_data.init( cmd );
        }

        hacks::g_local_sync->handle_ctx( cmd, send_packet );

        g_ctx->send_packet( ) = send_packet;

        cmd.sanitize( );

        hacks::g_move->rotate( cmd, old_angles, self( )->flags( ), self( )->move_type( ) );

        if( valve::g_client_state.get( )->m_choked_cmds >= 14 )
            send_packet = true;

        if( !send_packet 
            && can_send ) {
            auto& net_channel = valve::g_client_state.get( )->m_net_chan;

            const auto backup_choked_packets = net_channel->m_choked_packets;

            net_channel->m_choked_packets = 0;
            net_channel->send_datagram( );
            --net_channel->m_out_seq;

            net_channel->m_choked_packets = backup_choked_packets;
        }
        else {
            if( valve::g_client_state.get( )->m_last_cmd_out == hacks::g_exploits->m_recharge_cmd ) {
                auto& local_data = hacks::g_eng_pred->local_data( ).at( cmd.m_number % ( 150 ) );

                local_data.m_override_tick_base = true;
                local_data.m_adjusted_tick_base = hacks::g_exploits->adjust_tick_base( 
                    valve::g_client_state.get( )->m_choked_cmds + 1, 1, -valve::g_client_state.get( )->m_choked_cmds
                );
            }
            else if( break_lc ) {
                hacks::g_exploits->m_type = hacks::c_exploits::type_defensive;
                hacks::g_exploits->m_cur_shift_amount = hacks::g_exploits->m_ticks_allowed;
            }
        }

        hacks::g_eng_pred->restore( );

        hacks::g_exploits->m_charged = false;

        if( g_ctx->anim_data( ).m_local_data.m_shot
            || cmd.m_buttons & valve::e_buttons::in_attack )
            g_ctx->anim_data( ).m_local_data.m_last_shot_time = valve::g_global_vars.get( )->m_cur_time;

        bool has_exploits = hacks::g_exploits->m_type == hacks::c_exploits::type_doubletap 
            || hacks::g_exploits->m_type == hacks::c_exploits::type_ready 
            || break_lc;

        bool can_send_cmd_with_exploits = false;

        if( has_exploits && !hacks::g_exploits->m_cur_shift_amount )
            can_send_cmd_with_exploits = true;

        if( cmd.m_tick != std::numeric_limits < float > ::max( )
            || ( has_exploits && can_send_cmd_with_exploits ) ) {
            auto& out = g_ctx->get_out_cmds( ).emplace_back( );

            out.m_is_outgoing = send_packet;
            out.m_command_nr = cmd.m_number;
            out.m_is_used = false;
            out.m_prev_command_nr = 0;
        }

        while( int( g_ctx->get_out_cmds( ).size( ) ) > int( 1.0f / valve::g_global_vars.get( )->m_interval_per_tick ) ) {
            g_ctx->get_out_cmds( ).pop_front( );
        }

        hacks::g_exploits->m_should_send = false;

        hacks::g_misc->buy_bot( );

        g_ctx->anim_data( ).m_local_data.m_old_shot = g_ctx->anim_data( ).m_local_data.m_shot;
        g_ctx->anim_data( ).m_local_data.m_old_packet = send_packet;

        hacks::g_eng_pred->local_data( ).at( cmd.m_number % 150 ).m_move = cmd.m_move;

        g_ctx->left_create_move( ) = true;

        vfyd_cmd.m_cmd = cmd;
        vfyd_cmd.m_checksum = cmd.checksum( );
    }

    bool c_local_player::will_shoot( valve::cs_weapon_t* const weapon, const valve::user_cmd_t& user_cmd ) const {
        if( !weapon )
            return false;

        const auto item_index = weapon->item_index( );
        const auto wpn_data = weapon->info( );

        if( ( item_index < valve::e_item_index::flashbang || item_index > valve::e_item_index::inc_grenade )
            && !( user_cmd.m_buttons & valve::e_buttons::in_attack )
            &&( !( user_cmd.m_buttons & valve::e_buttons::in_attack2 )
                || weapon->next_secondary_attack( ) >= valve::g_global_vars.get( )->m_cur_time
                || ( item_index != valve::e_item_index::revolver &&( !wpn_data || wpn_data->m_type != valve::e_weapon_type::knife ) )
                )
            )
            return false;

        return true;
    }
}