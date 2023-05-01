#include "../../../csgo.hpp"

namespace csgo::hacks { 

	void c_eng_pred::prepare( ) { 
		if( m_last_frame_stage == game::e_frame_stage::net_update_end ) { 
			game::g_prediction->update( 
				game::g_client_state.get( )->m_delta_tick,
				game::g_client_state.get( )->m_delta_tick > 0,
				game::g_client_state.get( )->m_last_cmd_ack,
				game::g_client_state.get( )->m_last_cmd_out + game::g_client_state.get( )->m_choked_cmds
			 );
		}

		if( m_net_velocity_modifier < 1.f )
			game::g_prediction->m_prev_ack_had_errors = true;

		if( m_is_out_of_epsilon ) { 
			game::g_prediction->m_cmds_predicted = 0;
			game::g_prediction->m_prev_ack_had_errors = true;
		}

		if( game::g_client_state.get( )->m_delta_tick > 0 )
			m_is_out_of_epsilon = false;

		m_backup.m_cur_time = game::g_global_vars.get( )->m_cur_time;
		m_backup.m_frame_time = game::g_global_vars.get( )->m_frame_time;

		game::g_global_vars.get( )->m_cur_time = game::to_time( g_local_player->self( )->tick_base( ) );
		game::g_global_vars.get( )->m_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
	}

	void c_eng_pred::process( game::user_cmd_t* const user_cmd, bool init_data, bool force_repredict ) { 
		if( init_data ) { 
			m_local_data.at( user_cmd->m_number % 150 ).init( *user_cmd );
		}

		predict( user_cmd, init_data, force_repredict );
	}

	void c_eng_pred::predict( game::user_cmd_t* const user_cmd, bool init_data, bool force_repredict ) { 
		if( user_cmd->m_number == -1 )
			return;

		game::g_prediction->update( 
			game::g_client_state.get( )->m_delta_tick,
			game::g_client_state.get( )->m_delta_tick > 0,
			game::g_client_state.get( )->m_last_cmd_ack,
			game::g_client_state.get( )->m_last_cmd_out + game::g_client_state.get( )->m_choked_cmds
		 );

		g_ctx->addresses( ).m_pred_player = g_local_player->self( );
		*g_ctx->addresses( ).m_pred_seed = user_cmd->m_random_seed;

		g_local_player->self( )->cur_user_cmd( ) = user_cmd;
		g_local_player->self( )->last_user_cmd( ) = *user_cmd;

		const auto backup_in_prediction = game::g_prediction->m_in_prediction;
		const auto backup_first_time_predicted = game::g_prediction->m_first_time_predicted;

		game::g_prediction->m_in_prediction = true;
		game::g_prediction->m_first_time_predicted = false;

		game::g_move_helper->set_host( g_local_player->self( ) );

		user_cmd->m_buttons |= * ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x3310 );

		game::g_movement->start_track_pred_errors( g_local_player->self( ) );

		const auto backup_velocity_modifier = g_local_player->self( )->velocity_modifier( );
		const auto backup_tick_base = g_local_player->self( )->tick_base( );

		auto backup_buttons = user_cmd->m_buttons;

		auto v17 = backup_buttons ^ * ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31e8 );

		* ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31dc ) = * ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31e8 );
		* ( game::e_buttons* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31e8 ) = backup_buttons;
		* ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31e0 ) = backup_buttons & v17;
		* ( int* )( ( std::uintptr_t ) g_local_player->self( ) + 0x31e4 ) = v17 & ~backup_buttons;

		game::g_prediction->check_moving_on_ground( g_local_player->self( ), game::g_global_vars.get( )->m_interval_per_tick );
		game::g_prediction->setup_move( g_local_player->self( ), user_cmd, game::g_move_helper, &m_move_data );

		m_move_data.m_move = user_cmd->m_move;
		m_move_data.m_buttons = user_cmd->m_buttons;
		m_move_data.m_view_angles = user_cmd->m_view_angles;
		m_move_data.m_angles = user_cmd->m_view_angles;
		m_move_data.m_impulse_cmd = user_cmd->m_impulse;

		game::g_movement->process_movement( g_local_player->self( ), &m_move_data );
		g_local_player->self( )->tick_base( ) = backup_tick_base;
		game::g_prediction->finish_move( g_local_player->self( ), user_cmd, &m_move_data );
		g_local_player->self( )->set_abs_origin( g_local_player->self( )->origin( ) );
		game::g_movement->finish_track_pred_errors( g_local_player->self( ) );
		game::g_move_helper->set_host( nullptr );

		if( const auto weapon = g_local_player->self( )->weapon( ) ) { 
			weapon->update_inaccuracy( );

			m_inaccuracy = weapon->inaccuracy( );
			m_spread = weapon->spread( );

			const auto item_index = weapon->item_index( );
			const auto scope_able =
				item_index == game::e_item_index::awp || item_index == game::e_item_index::g3sg1
				|| item_index == game::e_item_index::scar20 || item_index == game::e_item_index::ssg08;
			const auto wpn_data = weapon->info( );

			if( g_local_player->self( )->flags( ) & game::e_ent_flags::ducking )
				m_min_inaccuracy = scope_able ? wpn_data->m_inaccuracy_crouch_alt : wpn_data->m_inaccuracy_crouch;
			else
				m_min_inaccuracy = scope_able ? wpn_data->m_inaccuracy_stand_alt : wpn_data->m_inaccuracy_stand;
		}
		else
			m_inaccuracy = m_spread = 0.f;

		m_buttons = user_cmd->m_buttons;
		m_move = user_cmd->m_move;

		g_local_player->self( )->velocity_modifier( ) = backup_velocity_modifier;

		g_local_sync->simulate( );

		game::g_prediction->m_in_prediction = backup_in_prediction;
		game::g_prediction->m_first_time_predicted = backup_first_time_predicted;
		update_shoot_pos( 0.5f );

		m_local_data.at( user_cmd->m_number % 150 ).m_repredicted = true;
	}

	void c_eng_pred::restore( ) { 
		g_ctx->addresses( ).m_pred_player = nullptr;
		*g_ctx->addresses( ).m_pred_seed = -1;

		game::g_global_vars.get( )->m_cur_time = m_backup.m_cur_time;
		game::g_global_vars.get( )->m_frame_time = m_backup.m_frame_time;
	}

	void c_eng_pred::update_shoot_pos( sdk::qang_t m_angle ) const { 
		const auto anim_state = g_local_player->self ( )->anim_state ( );
		if( !anim_state )
			return;

		// backup poseparams & animlayers
		game::pose_params_t backup_pose_params{ };
		game::anim_layers_t backup_anim_layers{ };
		sdk::vec3_t		     backup_abs_origin{ g_local_player->self( )->abs_origin( ) };
		std::memcpy( backup_pose_params.data( ), g_local_player->self( )->pose_params( ).data( ), sizeof( float_t ) * 24 );
		std::memcpy( backup_anim_layers.data( ), g_local_player->self( )->anim_layers( ).data( ), sizeof ( game::anim_layer_t ) * 13 );

		// set animations to last networked
		g_local_player->self( )->set_abs_origin( g_local_player->self( )->origin( ) );
		std::memcpy( g_local_player->self( )->pose_params( ).data( ), g_local_sync->m_pose_params.data( ), sizeof ( float_t ) * 24 );
		std::memcpy( g_local_player->self( )->anim_layers( ).data( ), g_local_sync->m_anim_layers.data( ), sizeof ( game::anim_layer_t ) * 13 );

		// force 0 pitch ( neutral pitch )
		g_local_player->self( )->pose_params( ).at( 12u ) = ( m_angle.x( ) + 90.f ) / 180.f;

		// setup bones with last sent data
		game::bones_t bones { };
		g_local_sync->setup_bones( bones, game::to_time( g_local_player->self( )->tick_base( ) ), -1 );
		g_ctx->shoot_pos( ) = g_local_player->self( )->get_shoot_pos( bones );

		// restore poseparams & animlayers
		g_local_player->self( )->set_abs_origin( backup_abs_origin );
		std::memcpy( g_local_player->self( )->pose_params( ).data( ), backup_pose_params.data( ), sizeof( float_t ) * 24 );
		std::memcpy( g_local_player->self( )->anim_layers( ).data( ), backup_anim_layers.data( ), sizeof( game::anim_layer_t ) * 13 );
	}

	void c_eng_pred::on_packet_update( std::uintptr_t cl_state ) { 

	}
}