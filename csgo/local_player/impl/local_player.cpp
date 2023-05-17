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

    void run_create_move( bool& send_packet,
        game::user_cmd_t& cmd, sdk::qang_t old_angles ) {

        if( !g_local_player->self( )
            || !g_local_player->self( )->alive( ) )
            return;

        auto weapon = g_local_player->self( )->weapon( );
        if( !weapon )
            return;

        auto weapon_info = weapon->info( );

        if( !weapon_info )
            return;

        hacks::g_eng_pred->prepare( );

        g_ctx->anim_data( ).m_local_data.m_shot = false;

        hacks::g_move->handle( cmd );

        hacks::g_eng_pred->process( &cmd, true );

        hacks::g_move->auto_peek( old_angles, cmd );

        if( !weapon->is_knife( )
            && weapon_info
            && weapon_info->m_type != game::e_weapon_type::grenade
            && weapon->item_index( ) != game::e_item_index::revolver
            || !g_ctx->can_shoot( ) )
            cmd.m_buttons &= ~game::e_buttons::in_attack;

        hacks::g_aim_bot->handle_ctx( cmd, send_packet );

        hacks::g_knife_bot->handle_knife_bot( cmd );
             
        hacks::g_anti_aim->handle_pitch( cmd );

        hacks::g_anti_aim->handle_ctx( cmd, send_packet, hacks::g_exploits->is_charged( ) || hacks::g_exploits->m_in_charge );

        g_ctx->can_shoot( ) = hacks::g_aim_bot->can_shoot( false, hacks::g_exploits->m_allowed_ticks, false );

        if( g_ctx->can_shoot( ) ) { 
            auto& anim_data = g_ctx->anim_data( ).m_local_data;

            anim_data.m_shot_cmd_number = cmd.m_number;

            if( weapon_info ) { 
                anim_data.m_shot_valid_wpn = true;
            }

            if( !hacks::g_exploits->cl_move_data.m_can_shift
                && cmd.m_buttons & game::e_buttons::in_attack ) {
                send_packet = true;
            }

            g_ctx->was_shooting( ) = anim_data.m_shot_cmd_number == cmd.m_number;

            g_ctx->aim_shoot_pos( ) = g_ctx->shoot_pos( );

            if( !g_ctx->anim_data( ).m_local_data.m_shot ) { 
                hacks::g_shots->add( 
                    g_ctx->shoot_pos( ), nullptr,
                    hacks::g_exploits->m_shift_amount, cmd.m_number, game::g_global_vars.get( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
                );
            }   
        }

        hacks::g_local_sync->handle_ctx( cmd, send_packet );

        if( weapon ) { 
            weapon->recoil_index( ) = hacks::g_eng_pred->recoil_index( );
            weapon->accuracy_penalty( ) = hacks::g_eng_pred->accuracy_penalty( );
        }

        hacks::g_move->rotate( cmd, old_angles, g_local_player->self( )->flags( ), g_local_player->self( )->move_type( ) );

        cmd.sanitize( );

        hacks::g_eng_pred->restore( );
     }

    void c_local_player::create_move( bool& send_packet,
        game::user_cmd_t& cmd, game::vfyd_user_cmd_t& vfyd_cmd
    ) { 
        game::g_cvar->find_var( xor_str( "r_jiggle_bones" ) )->set_int( 0 ); // fuck off bro
        hacks::g_grenades->on_create_move( cmd );

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
        if( g_local_player->cfg( ).m_force_crosshair ) { 
            crosshair_data->set_int( !g_local_player->self( )->scoped( ) ? 3 : 0 );
        }
        else { 
            crosshair_data->set_int( 0 );
        }

        g_ctx->ticks_data( ).m_tick_rate = crypt_float( 1.f ) / game::g_global_vars.get( )->m_interval_per_tick;

        auto old_angles = cmd.m_view_angles;

        if( hacks::g_exploits->cl_move_data.m_shifting )
        {
            run_create_move( send_packet, cmd, old_angles ); // do not modify anything related to exploits/fakelag during shift.

            if( game::g_client_state.get( )->m_choked_cmds >= g_ctx->m_max_choke )
                g_ctx->send_packet( ) = send_packet = true;

            return;
        }

        hacks::g_exploits->on_pre_predict( );
        hacks::g_eng_pred->prepare( );

        g_ctx->anim_data( ).m_local_data.m_shot = false;

        hacks::g_move->handle( cmd );

        if( cmd.m_move.length( ) < crypt_float( 23.f )
            && g_local_player->cfg( ).m_shitty_mrx_servers )
            cmd.m_move = { };

        hacks::g_eng_pred->process( &cmd, true );
        {
            mouse_fix( &cmd );

            if( !g_ctx->can_shoot( )
                && m_weapon
                && !m_weapon->is_knife( )
                && m_weapon_info
                && m_weapon_info->m_type != game::e_weapon_type::grenade
                && m_weapon->item_index( ) != game::e_item_index::revolver )
                cmd.m_buttons &= ~game::e_buttons::in_attack;

            hacks::g_move->auto_peek( old_angles, cmd );

            hacks::g_anti_aim->handle_fake_lag( cmd );

            if( !hacks::g_exploits->is_charged( ) ) {
                send_packet = !hacks::g_anti_aim->can_choke( );
            }

            if( ( m_weapon = self( )->weapon( ) ) && m_weapon != nullptr )
                m_weapon_info = m_weapon->info( );
            else
                m_weapon_info = nullptr;

            if( hacks::g_exploits->m_in_charge && m_weapon
                && !m_weapon->is_knife( )
                && m_weapon_info
                && m_weapon_info->m_type != game::e_weapon_type::grenade
                && m_weapon->item_index( ) != game::e_item_index::revolver )
                cmd.m_buttons &= ~game::e_buttons::in_attack;

            hacks::g_aim_bot->handle_ctx( cmd, send_packet );

            hacks::g_knife_bot->handle_knife_bot( cmd );
             
            hacks::g_anti_aim->handle_pitch( cmd );

            hacks::g_anti_aim->handle_ctx( cmd, send_packet, hacks::g_exploits->is_charged( ) || hacks::g_exploits->m_in_charge );

            hacks::g_exploits->on_predict_start( &cmd );

            g_ctx->can_shoot( ) = hacks::g_aim_bot->can_shoot( false, hacks::g_exploits->m_allowed_ticks, false );

            if( g_ctx->can_shoot( ) ) { 
                auto& anim_data = g_ctx->anim_data( ).m_local_data;

                anim_data.m_shot_cmd_number = cmd.m_number;

                if( m_weapon_info ) { 
                    anim_data.m_shot_valid_wpn = true;
                }

                if( !hacks::g_exploits->cl_move_data.m_can_shift
                    && cmd.m_buttons & game::e_buttons::in_attack ) {
                    send_packet = true;
                }

                g_ctx->was_shooting( ) = anim_data.m_shot_cmd_number == cmd.m_number;

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

            cmd.sanitize( );

            hacks::g_move->rotate( cmd, old_angles, self( )->flags( ), self( )->move_type( ) );

            if( g_ctx->anim_data( ).m_local_data.m_shot
                || cmd.m_buttons & game::e_buttons::in_attack )
                g_ctx->anim_data( ).m_local_data.m_last_shot_time = game::g_global_vars.get( )->m_cur_time;
        }

        hacks::g_eng_pred->restore( );

        if( !hacks::g_exploits->m_in_charge ) {
            auto& out = g_ctx->get_out_cmds( ).emplace_back( );

            out.m_is_outgoing = send_packet;
            out.m_command_nr = cmd.m_number;
            out.m_is_used = false;
            out.m_prev_command_nr = 0;
        }

        while( g_ctx->get_out_cmds( ).size( ) > g_ctx->ticks_data( ).m_tick_rate ) { 
            g_ctx->get_out_cmds( ).pop_front( );
        }

        if( !send_packet ) { 
            auto& net_channel = game::g_client_state.get( )->m_net_chan;

            const auto backup_choked_packets = net_channel->m_choked_packets;

            net_channel->m_choked_packets = 0;
            net_channel->send_datagram( );
            --net_channel->m_out_seq;

            net_channel->m_choked_packets = backup_choked_packets;
        }
        
        if( game::g_client_state.get( )->m_choked_cmds >= g_ctx->m_max_choke )
            send_packet = true;

        g_ctx->send_packet( ) = send_packet;

        hacks::g_misc->buy_bot( );

        hacks::g_eng_pred->local_data( ).at( cmd.m_number % crypt_int( 150 ) ).m_move = cmd.m_move;

        g_ctx->left_create_move( ) = true;

        vfyd_cmd.m_cmd = cmd;
        vfyd_cmd.m_checksum = cmd.checksum( );
    }
}