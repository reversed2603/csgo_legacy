#include "../../csgo.hpp"

namespace csgo { 
      void mouse_fix( game::user_cmd_t* cmd ) {
	    static sdk::qang_t delta_viewangles{ };
	    sdk::qang_t delta = cmd->m_view_angles - delta_viewangles;

        static auto sensitivity = game::g_cvar->find_var( xor_str( "sensitivity" ) );

	    if( delta.x( ) != 0.f ) {
            static auto m_pitch = game::g_cvar->find_var( xor_str( "m_pitch" ) );

		    int final_dy = static_cast<int> ( ( delta.x( ) / m_pitch->get_float( ) ) / sensitivity->get_float( ) );
		    if( final_dy <= 32767 ) {
			    if( final_dy >= -32768 ) {
				    if( final_dy >= 1 || final_dy < 0 ) {
					    if( final_dy <= -1 || final_dy > 0 )
						    final_dy = final_dy;
					    else
						    final_dy = -1;
				    }
				    else {
					    final_dy = 1;
				    }
			    }
			    else {
				    final_dy = 32768;
			    }
		    }
		    else {
			    final_dy = 32767;
		    }

		    cmd->m_mouse_accum.y( ) = static_cast<short> ( final_dy );
	    }

	    if( delta.y( ) != 0.f ) {
            static auto m_yaw = game::g_cvar->find_var( xor_str( "m_yaw" ) );

		    int final_dx = static_cast<int> ( ( delta.y( ) / m_yaw->get_float( ) ) / sensitivity->get_float( ) );
		    if( final_dx <= 32767 ) {
			    if( final_dx >= -32768 ) {
				    if( final_dx >= 1 || final_dx < 0 ) {
					    if( final_dx <= -1 || final_dx > 0 )
						    final_dx = final_dx;
					    else
						    final_dx = -1;
				    }
				    else {
					    final_dx = 1;
				    }
			    }
			    else {
				    final_dx = 32768;
			    }
		    }
		    else {
			    final_dx = 32767;
		    }

		    cmd->m_mouse_accum.x( ) = static_cast<short> ( final_dx );
	    }

	    delta_viewangles = cmd->m_view_angles;
    }

    void c_local_player::create_move( bool& send_packet,
        game::user_cmd_t& cmd, game::vfyd_user_cmd_t& vfyd_cmd
    ) { 
        send_packet = true;
        game::g_cvar->find_var( xor_str( "r_jiggle_bones" ) )->set_int( 0 ); // fuck off bro
        hacks::g_grenades->on_create_move( cmd );

        static auto draw_spec_static_prop = game::g_cvar->find_var( xor_str( "r_DrawSpecificStaticProp" ) );

        if( draw_spec_static_prop->get_int( ) != 0 ) { 
            draw_spec_static_prop->set_int( 0 );
        }

        const auto net_channel_info = game::g_engine->net_channel_info( );
        if( !net_channel_info )
            return;
        { 
            auto& net_info = g_ctx->net_info( );

            net_info.m_lerp = std::max( 
                g_ctx->cvars( ).m_cl_interp->get_float( ),
                g_ctx->cvars( ).m_cl_interp_ratio->get_float( ) / g_ctx->cvars( ).m_cl_updaterate->get_float( )
            );
            net_info.m_latency = { net_channel_info->latency( game::e_net_flow::in ), net_channel_info->latency( game::e_net_flow::out ) };
        }

        g_ctx->left_create_move( ) = false;

        static float prev_spawn_time = g_local_player->self( )->spawn_time( );
        if( prev_spawn_time != g_local_player->self( )->spawn_time( ) ) { 
            hacks::g_eng_pred->reset_on_spawn( );
            g_ctx->anim_data( ).m_local_data.reset( );
            hacks::g_local_sync->m_anim_layers = { };
            hacks::g_local_sync->m_pose_params = { };
                
            prev_spawn_time = g_local_player->self( )->spawn_time( );
        }

        static auto crosshair_data = game::g_cvar->find_var( xor_str( "weapon_debug_spread_show" ) );
        if( g_local_player->cfg( ).m_force_crosshair && crosshair_data ) { 
            crosshair_data->set_int( !g_local_player->self( )->scoped( ) ? 3 : 0 );
        }
        else { 
            if( crosshair_data )
                crosshair_data->set_int( 0 );
        }

        g_ctx->ticks_data( ).m_tick_rate = crypt_float( 1.f ) / game::g_global_vars.get( )->m_interval_per_tick;

        const auto old_angles = cmd.m_view_angles;
        auto old_angles_ = cmd.m_view_angles;

        hacks::g_exploits->on_pre_predict( );
        hacks::g_eng_pred->prepare( );

        g_ctx->anim_data( ).m_local_data.m_shot = false;

        hacks::g_move->handle( cmd );

        if( cmd.m_move.length( ) < crypt_float( 23.f )
            && g_local_player->cfg( ).m_shitty_mrx_servers )
            cmd.m_move = { };

        hacks::g_eng_pred->process( &cmd, true );
        mouse_fix( &cmd );

        if( !g_ctx->can_shoot( )
            && m_weapon
            && !m_weapon->is_knife( )
            && m_weapon_info
            && m_weapon_info->m_type != game::e_weapon_type::grenade
            && m_weapon->item_index( ) != game::e_item_index::revolver )
            cmd.m_buttons &= ~game::e_buttons::in_attack;

        hacks::g_move->auto_peek( old_angles_, cmd );

        hacks::g_anti_aim->handle_fake_lag( cmd );

        if( ( m_weapon = self( )->weapon( ) ) && m_weapon != nullptr )
            m_weapon_info = m_weapon->info( );
        else
            m_weapon_info = nullptr;

        if( hacks::g_exploits->m_recharge && m_weapon
            && !m_weapon->is_knife( )
            && m_weapon_info
            && m_weapon_info->m_type != game::e_weapon_type::grenade
            && m_weapon->item_index( ) != game::e_item_index::revolver )
            cmd.m_buttons &= ~game::e_buttons::in_attack;

        if( !hacks::g_exploits->is_charged( ) ) {
            send_packet = !hacks::g_anti_aim->can_choke( );
        }
        else {
            if( !g_key_binds->get_keybind_state( &hacks::g_move->cfg( ).m_slow_walk ) 
                && !hacks::g_exploits->cl_move_data.m_shifting
                || hacks::g_exploits->cl_move_data.m_shifting
                && game::g_client_state.get( )->m_choked_cmds >= 14 )
                send_packet = true;
        }

        hacks::g_aim_bot->handle_ctx( cmd, send_packet );

        hacks::g_knife_bot->handle_knife_bot( cmd );

        if( ( cmd.m_buttons & game::e_buttons::in_attack )
            && !g_ctx->was_shooting( ) ) { 
            cmd.m_view_angles -= g_local_player->self( )->aim_punch( ) * game::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) )->get_float( );

            cmd.m_view_angles.x( ) = std::remainder( cmd.m_view_angles.x( ), 360.f );
            cmd.m_view_angles.y( ) = std::remainder( cmd.m_view_angles.y( ), 360.f );

            cmd.m_view_angles.x( ) = std::clamp( cmd.m_view_angles.x( ), -89.f, 89.f );
            cmd.m_view_angles.y( ) = std::clamp( cmd.m_view_angles.y( ), -180.f, 180.f );
            cmd.m_view_angles.z( ) = 0.f;
        }   
             
        hacks::g_anti_aim->handle_pitch( cmd );

        hacks::g_anti_aim->handle_ctx( cmd, send_packet, hacks::g_exploits->is_charged( ) || hacks::g_exploits->m_recharge );

        hacks::g_exploits->on_predict_start( &cmd );

        g_ctx->can_shoot( ) = hacks::g_aim_bot->can_shoot( false, hacks::g_exploits->m_allowed_ticks, false );

        if( g_ctx->can_shoot( )
            && will_shoot( m_weapon, cmd ) ) { 
            auto& anim_data = g_ctx->anim_data( ).m_local_data;

            anim_data.m_shot_cmd_number = cmd.m_number;

            if( m_weapon_info ) { 
                anim_data.m_shot_valid_wpn = true;
            }

            g_ctx->aim_shoot_pos( ) = g_ctx->shoot_pos( );

            if( !g_ctx->anim_data( ).m_local_data.m_shot ) { 
                hacks::g_shots->add( 
                    g_ctx->shoot_pos( ), nullptr,
                    hacks::g_exploits->m_shift_amount, cmd.m_number, game::g_global_vars.get( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
                );
            }
        }

        hacks::g_local_sync->handle_ctx( cmd, send_packet );

        if( m_weapon ) { 
            m_weapon->recoil_index( ) = hacks::g_eng_pred->recoil_index( );
            m_weapon->accuracy_penalty( ) = hacks::g_eng_pred->accuracy_penalty( );
        }

        g_ctx->send_packet( ) = send_packet;

        cmd.sanitize( );

        hacks::g_move->rotate( cmd, old_angles, self( )->flags( ), self( )->move_type( ) );

        if( !send_packet ) { 
            auto& net_channel = game::g_client_state.get( )->m_net_chan;

            const auto backup_choked_packets = net_channel->m_choked_packets;

            net_channel->m_choked_packets = 0;
            net_channel->send_datagram( );
            --net_channel->m_out_seq;

            net_channel->m_choked_packets = backup_choked_packets;
        }

        hacks::g_eng_pred->restore( );

        if( g_ctx->anim_data( ).m_local_data.m_shot
            || cmd.m_buttons & game::e_buttons::in_attack )
            g_ctx->anim_data( ).m_local_data.m_last_shot_time = game::g_global_vars.get( )->m_cur_time;

        bool has_exploits = hacks::g_exploits->is_charged( );

        if( cmd.m_tick != std::numeric_limits < float > ::max( )
            || has_exploits ) {
            auto& out = g_ctx->get_out_cmds( ).emplace_back( );

            out.m_is_outgoing = send_packet;
            out.m_command_nr = cmd.m_number;
            out.m_is_used = false;
            out.m_prev_command_nr = 0;
        }

        while( g_ctx->get_out_cmds( ).size( ) > int( 1.0f / game::g_global_vars.get( )->m_interval_per_tick ) ) { 
            g_ctx->get_out_cmds( ).pop_front( );
        }

        hacks::g_misc->buy_bot( );

        g_ctx->anim_data( ).m_local_data.m_old_shot = g_ctx->anim_data( ).m_local_data.m_shot;
        g_ctx->anim_data( ).m_local_data.m_old_packet = send_packet;

        hacks::g_eng_pred->local_data( ).at( cmd.m_number % crypt_int( 150 ) ).m_move = cmd.m_move;

        g_ctx->left_create_move( ) = true;

        vfyd_cmd.m_cmd = cmd;
        vfyd_cmd.m_checksum = cmd.checksum( );
    }

    bool c_local_player::will_shoot( game::cs_weapon_t* const weapon, const game::user_cmd_t& user_cmd ) const { 
        if( !weapon )
            return false;

        const auto item_index = weapon->item_index( );
        const auto wpn_data = weapon->info( );

        if( ( item_index < game::e_item_index::flashbang || item_index > game::e_item_index::inc_grenade )
            && !( user_cmd.m_buttons & game::e_buttons::in_attack )
            && ( !( user_cmd.m_buttons & game::e_buttons::in_attack2 )
                || weapon->next_secondary_attack( ) >= game::g_global_vars.get( )->m_cur_time
                || ( item_index != game::e_item_index::revolver && ( !wpn_data || wpn_data->m_type != game::e_weapon_type::knife ) )
                )
            )
            return false;

        return true;
    }
}