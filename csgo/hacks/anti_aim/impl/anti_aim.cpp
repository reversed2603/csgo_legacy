#include "../../../csgo.hpp"

namespace csgo::hacks {

	void c_anti_aim::handle_ctx( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		m_last_anim_ang = user_cmd.m_view_angles;

		if( !g_local_player->weapon_info( ) )
			return;

		auto anim_state = g_local_player->self( )->anim_state( );

		if( g_local_player->weapon_info( )->m_type == valve::e_weapon_type::grenade
			&&( !g_local_player->weapon( )->pin_pulled( )
				|| user_cmd.m_buttons & valve::e_buttons::in_attack
				|| user_cmd.m_buttons & valve::e_buttons::in_attack2 )
			&& g_local_player->weapon( )->throw_time( ) > 0.f && g_local_player->weapon( )->throw_time( ) < valve::g_global_vars.get( )->m_cur_time ) {
			return;
		}

		if( user_cmd.m_buttons & valve::e_buttons::in_use )
			return;

		if( g_local_player->weapon_info( )->m_type == valve::e_weapon_type::knife
			&& g_ctx->can_shoot( )
			&&( user_cmd.m_buttons & valve::e_buttons::in_attack
				|| user_cmd.m_buttons & valve::e_buttons::in_attack2 ) )
			return;

		if( g_ctx->can_shoot( )
			&& user_cmd.m_buttons & valve::e_buttons::in_attack && g_local_player->weapon_info( )->m_type != valve::e_weapon_type::grenade )
			return;

		if( !should_disable( user_cmd ) )
			return;

		if( !m_cfg->m_anti_aim )
			return;

		float length_to_flick { m_cfg->m_flick_strength };

		length_to_flick = std::clamp( length_to_flick, 35.f, 179.f );

		const auto speed = ( ( anim_state->m_walk_to_run_transition * crypt_float( 20.f ) ) + crypt_float( 30.f ) ) * valve::g_global_vars.get( )->m_interval_per_tick;

		auto delta = -60.f;
		const auto upper_limit = crypt_float( 58.f ) + speed;
		if( delta > upper_limit )
			delta = upper_limit;
		else {
			const auto lower_limit = ( crypt_float( 58.f ) * -1.f ) - speed;
			if( lower_limit > delta )
				delta = lower_limit;
		}

		if( send_packet ) {
			user_cmd.m_view_angles.y( ) = handle_yaw( user_cmd ) + g_ctx->addresses( ).m_random_float( delta * 0.75f, -delta );
		}
		else {
			user_cmd.m_view_angles.y( ) = handle_yaw( user_cmd );
			handle_distortion( user_cmd );
			m_last_anim_ang = user_cmd.m_view_angles;
		}

		m_fake_moving = false;

		if( !valve::g_client_state.get( )->m_choked_cmds ) {
			if( valve::g_global_vars.get( )->m_cur_time >= g_ctx->anim_data( ).m_local_data.m_lby_upd
				&& g_ctx->anim_data( ).m_local_data.m_can_break ) {
				const auto& flick_angle = user_cmd.m_view_angles.y( ) - length_to_flick;

				if( m_cfg->m_lby_breaker 
					&& valve::g_client_state.get( )->m_last_cmd_out != hacks::g_exploits->m_recharge_cmd ) {
					if( !break_freestand( const_cast < float& >( flick_angle ) ) )
						user_cmd.m_view_angles.y( ) -= length_to_flick;
				}
				m_switch_dist_dir = !m_switch_dist_dir;

				m_lby_counter++;
			}

			if( valve::g_client_state.get( )->m_last_cmd_out != hacks::g_exploits->m_recharge_cmd )
				fake_flick( user_cmd, send_packet );
		}
		else {
		}
	}

	void c_anti_aim::fake_flick( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		if( !g_key_binds->get_keybind_state( &m_cfg->m_fake_flick ) )
			return;

		if( !( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) )
			return;

		if( user_cmd.m_buttons & valve::e_buttons::in_fwd
			|| user_cmd.m_buttons & valve::e_buttons::in_back
			|| user_cmd.m_buttons & valve::e_buttons::in_move_left
			|| user_cmd.m_buttons & valve::e_buttons::in_move_right )
			return;

		m_fake_moving = true;

		static int fake_flick_type = 0;

		if( fake_flick_type >= 4 )
			fake_flick_type = 0;

		if( fake_flick_type )
		{
			if( fake_flick_type == 1 )
			{
				fake_flick_type = 2;
				user_cmd.m_view_angles.y( ) = user_cmd.m_view_angles.y( ) - 30.0f;
				return;
			}
			if( fake_flick_type == 2 )
			{
				user_cmd.m_move.x( ) = 13.f;
				fake_flick_type = 3;
				user_cmd.m_view_angles.y( ) = user_cmd.m_view_angles.y( ) - 125.0f;
				return;
			}
		}
		else
		{
			user_cmd.m_move.x( ) = -13.f;
			user_cmd.m_view_angles.y( ) = user_cmd.m_view_angles.y( ) + 110.0f;
		}
		fake_flick_type = fake_flick_type + 1;

		/*static float fake_velo{g_ctx->addresses( ).m_random_float( 7.f, 13.f )};

		static bool switch_{ };

		if( fake_flick_type >= 4 ) {
			fake_velo = g_ctx->addresses( ).m_random_float( 7.f, 13.f );
			fake_flick_type = 0;
		}

		if( fake_flick_type ) {
			if( fake_flick_type == 1 ) {
				fake_flick_type = 2;
				if( g_visuals->m_cur_yaw_dir == 2 )
				user_cmd.m_view_angles.y( ) -= g_ctx->addresses( ).m_random_float( 30.f, 42.f );
				else {
					user_cmd.m_view_angles.y( ) += g_ctx->addresses( ).m_random_float( 30.f, 42.f );
				}
				return;
			}
			else if( fake_flick_type == 2 ) {
				user_cmd.m_move.x( ) = switch_ ? -fake_velo : fake_velo;
				switch_ = !switch_;
				fake_flick_type = 3;
				if( g_visuals->m_cur_yaw_dir == 2 )
					user_cmd.m_view_angles.y( ) += g_ctx->addresses( ).m_random_float( 105.f, 118.f );
				else {
					user_cmd.m_view_angles.y( ) -= g_ctx->addresses( ).m_random_float( 105.f, 118.f );
				}
				return;
			}
		}
		else {
			if( g_visuals->m_cur_yaw_dir == 2 )
				user_cmd.m_view_angles.y( ) -= g_ctx->addresses( ).m_random_float( crypt_float( 190.f ), crypt_float( 220.f ) );
			else {
				user_cmd.m_view_angles.y( ) += g_ctx->addresses( ).m_random_float( crypt_float( 190.f ), crypt_float( 220.f ) );
			}
		}

		fake_flick_type += 1;*/
	}

	void c_anti_aim::fake_move( valve::user_cmd_t& user_cmd ) {
	}

	void c_anti_aim::handle_distortion( valve::user_cmd_t& user_cmd ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if( !m_cfg->m_should_distort )
			return;

		if( g_key_binds->get_keybind_state( &hacks::g_move->cfg( ).m_slow_walk ) )
			return;

		if( g_key_binds->get_keybind_state( &m_cfg->m_freestand ) )
			return;

		if( get_manual_rotate( ) != std::numeric_limits < float >::max( ) )
			return;

		static auto interval_per_tick_sim{ valve::g_global_vars.get( )->m_interval_per_tick };
		bool change_dist_dir{ };
		static bool change_dir{ };

		interval_per_tick_sim += valve::g_global_vars.get( )->m_interval_per_tick;

		if( interval_per_tick_sim >= crypt_float( 1.f ) )
			interval_per_tick_sim = 0.f;

		static auto final_wish_ang = user_cmd.m_view_angles.y( );

		bool can_distort{ true };

		if( ( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) && g_local_player->self( )->velocity( ).length( 2u ) > crypt_float( 5.f ) )
			can_distort = false;

		if( can_distort ) {
			const auto distortion_speed = ( m_cfg->m_distort_speed * 3.f ) / crypt_float( 100.f );
			const auto distortion = std::sin( ( valve::g_global_vars.get( )->m_cur_time * distortion_speed ) * sdk::k_pi < float > );

			const auto dist_factor = ( m_cfg->m_distort_factor / crypt_float( 100.f ) ) * crypt_float( 120.f );
			auto total_distortion = dist_factor * distortion;

			if( m_cfg->m_shift ) {
				if( m_lby_counter_updated == m_lby_counter
					|| ( m_lby_counter_updated = m_lby_counter, ++m_lby_on_same_pos, m_lby_on_same_pos < m_cfg->m_await_shift ) ) {
					change_dist_dir = change_dir;
				}
				else {
					m_lby_on_same_pos = 0;
					change_dist_dir = change_dir == false;
					change_dir = !change_dir;
				}
					
				auto shift_factor_mult = m_cfg->m_shift_factor / crypt_float( 100.f );

				if( !change_dist_dir ) {
					shift_factor_mult *= -1.f;
				}
				total_distortion = ( shift_factor_mult + distortion ) * dist_factor;
			}

			if( m_cfg->m_force_turn ) {
				const auto distortion_speed_updated = std::exp( distortion_speed * interval_per_tick_sim );
				auto final_distortion_angle = distortion_speed_updated *( ( m_cfg->m_distort_factor * crypt_float( 120.f ) ) / crypt_float( 100.f ) );
				auto additional_delta{ 0.f };
				if( change_dir )
					additional_delta = crypt_float( 60.f );
				else {
					additional_delta = -60.f;
					final_distortion_angle *= -1.f;
				}

				final_wish_ang = ( additional_delta + user_cmd.m_view_angles.y( ) ) + final_distortion_angle;

				final_wish_ang = sdk::norm_yaw( final_wish_ang );
			}
			else {
				final_wish_ang = user_cmd.m_view_angles.y( ) + total_distortion;
				final_wish_ang = sdk::norm_yaw( final_wish_ang );
			}

			user_cmd.m_view_angles.y( ) = final_wish_ang;
		}
	}

	bool c_anti_aim::break_freestand( float& yaw ) {
		if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder )
			return false;

		if( !m_cfg->m_change_flick_dir ) // change_flick_dir is safe break......
			return false; 

		valve::cs_player_t* best_player{ };
		auto best_fov = std::numeric_limits< float >::max( );

		const auto view_angles = valve::g_engine->view_angles( );

		for( auto i = 1; i <= valve::g_global_vars.get( )->m_max_clients; ++i ) {
			const auto player = static_cast< valve::cs_player_t* >( 
				valve::g_entity_list->get_entity( i )
				 );

			if( !player
				|| player->networkable( )->dormant( )
				|| !player->alive( )
				|| player->friendly( g_local_player->self( ) ) )
				continue;

			const auto fov = sdk::calc_fov( view_angles, g_ctx->shoot_pos( ), player->world_space_center( ) );
			if( fov >= best_fov )
				continue;

			best_fov = fov;
			best_player = player;
		}

		if( !best_player )
			return false;

		struct angle_data_t {
			__forceinline constexpr angle_data_t( ) = default;

			__forceinline angle_data_t( const float yaw ) : m_yaw{ yaw } { }

			int		m_dmg{ };
			float	m_yaw{ }, m_dist{ };
			bool	m_can_do_dmg{ };
		};

		angle_data_t angle{ yaw };

		bool valid{ };
		constexpr auto k_range{ 32.f };
		constexpr auto k_step{ 4.f };

		const auto start = best_player->wpn_shoot_pos( );

		auto& cur_angle = angle;

		sdk::vec3_t dst{ g_ctx->shoot_pos( ).x( ) + std::cos( sdk::to_rad( cur_angle.m_yaw ) ) * k_range,
				g_ctx->shoot_pos( ).y( ) + std::sin( sdk::to_rad( cur_angle.m_yaw ) ) * k_range,
				g_ctx->shoot_pos( ).z( ) };

		auto dir = dst - start;
		float len = dir.normalize( );

		// should never happen.
		if( len <= 0.f )
			return false;

		for( float i{ }; i < len; i += k_step ) {
			const auto point = start +( dir * i );

			const auto contents = valve::g_engine_trace->get_point_contents( point, valve::e_mask::shot_hull );

			if( !( contents & valve::e_mask::shot_hull ) )
				continue;

			float mult = 1.f;

			if( i >( len * 0.5f ) )
				mult = crypt_float( 1.25f );

			if( i >( len * crypt_float( 0.75f ) ) )
				mult = crypt_float( 1.25f );

			if( i >( len * crypt_float( 0.9f ) ) )
				mult = crypt_float( 2.f );

			cur_angle.m_dist += ( k_step * mult );

			valid = true;
		}

		if( !valid 
			|| cur_angle.m_dist <= 60 ) {
			return true;
		}

		return false;
	}

	bool c_anti_aim::freestanding( float& yaw ) {
		if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder )
			return false;

		if( !g_key_binds->get_keybind_state( &m_cfg->m_freestand ) )
			return false;

		valve::cs_player_t* best_player{ };
		auto best_fov = std::numeric_limits< float >::max( );

		const auto view_angles = valve::g_engine->view_angles( );

		for( auto i = 1; i <= valve::g_global_vars.get( )->m_max_clients; ++i ) {
			const auto player = static_cast< valve::cs_player_t* >( 
				valve::g_entity_list->get_entity( i )
				 );

			if( !player
				|| player->networkable( )->dormant( )
				|| !player->alive( )
				|| player->friendly( g_local_player->self( ) ) )
				continue;

			const auto fov = sdk::calc_fov( view_angles, g_ctx->shoot_pos( ), player->world_space_center( ) );
			if( fov >= best_fov )
				continue;

			best_fov = fov;
			best_player = player;
		}

		if( !best_player )
			return false;

		auto& best_origin = best_player->origin( );

		const auto angle_yaw = sdk::calc_ang( g_local_player->self( )->origin( ), best_origin );

		sdk::vec3_t forward{ }, right{ }, up{ };
		sdk::ang_vecs( sdk::qang_t( angle_yaw.x( ), angle_yaw.y( ), angle_yaw.z( ) ), &forward, &right, &up );

		sdk::vec3_t start = g_ctx->shoot_pos( );
		sdk::vec3_t end = start + forward * crypt_float( 100.0f );

		valve::ray_t right_ray( start + right * crypt_float( 35.0f ), end + right * crypt_float( 35.0f ) );
		valve::ray_t left_ray( start - right * crypt_float( 35.0f ), end - right * crypt_float( 35.0f ) );

		valve::trace_filter_t filter{ };
		filter.m_ignore_entity = g_local_player->self( );

		valve::trace_t trace{ };

		valve::g_engine_trace->trace_ray( right_ray, valve::e_mask::solid, &filter, &trace );
		float right_length = ( trace.m_end - trace.m_start ).length( 3u );

		valve::g_engine_trace->trace_ray( left_ray, valve::e_mask::solid, &filter, &trace );
		float left_length = ( trace.m_end - trace.m_start ).length( 3u );

		static auto left_ticks = 0;
		static auto right_ticks = 0;
		static auto back_ticks = 0;

		if( right_length - left_length > 17.f )
			left_ticks++;
		else
			left_ticks = 0;

		if( left_length - right_length > 17.f )
			right_ticks++;
		else
			right_ticks = 0;

		if( fabs( right_length - left_length ) <= 17.f )
			back_ticks++;
		else
			back_ticks = 0;

		if( right_ticks > crypt_int( 10 ) ) {
			m_auto_dir_side = crypt_int( 1 );
		}
		else {
			if( left_ticks > crypt_int( 10 ) ) {
				m_auto_dir_side = crypt_int( 2 );
			}
			else {
				if( back_ticks > crypt_int( 10 ) )
					return false;
			}
		}

		return true;
	}

	void c_anti_aim::handle_fake_lag( valve::user_cmd_t& user_cmd ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) ) {
			m_can_choke = false;
			return;
		}

		if( g_exploits->m_charged
			|| valve::g_client_state.get( )->m_choked_cmds > 14
			|| g_local_player->self( )->flags( ) & valve::e_ent_flags::frozen
			|| g_ctx->in_charge( ) 
			|| !g_exploits->m_allow_choke ) {
			m_can_choke = false;
			return;
		}

		if( m_fake_moving ) {
			if( valve::g_client_state.get( )->m_net_chan->m_choked_packets >= crypt_int( 1 ) )
				m_can_choke = false;
			else
				m_can_choke = true;

			return;

		}

		if( !m_cfg->m_should_fake_lag ) {
			m_can_choke = false;
			return;
		}

		if( m_cfg->m_disable_lag_on_stand
			&&( g_ctx->anim_data( ).m_local_data.m_speed_2d <= 3.f || m_fake_moving ) ) {
			if( valve::g_client_state.get( )->m_choked_cmds >= 1 )
				m_can_choke = false;
			else
				m_can_choke = true;

			return;
		}

		if( g_key_binds->get_keybind_state( &hacks::g_move->cfg( ).m_slow_walk ) ) {
			m_can_choke = true;
		}

		if( valve::g_client_state.get( )->m_choked_cmds >= m_cfg->m_ticks_to_choke )
			m_can_choke = false;
		else
			m_can_choke = true;
	} 


	float c_anti_aim::handle_yaw( valve::user_cmd_t& user_cmd ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		if( freestanding( user_cmd.m_view_angles.y( ) ) ) {
			if( m_auto_dir_side ) {
				if( m_auto_dir_side == 2 ) {
					g_visuals->m_cur_yaw_dir = 2; // right
					return user_cmd.m_view_angles.y( ) += crypt_float( 90.f );
				}
				else {
					g_visuals->m_cur_yaw_dir = 1; // left
					return user_cmd.m_view_angles.y( ) -= crypt_float( 90.f );
				}
			}
		}
		else
			g_visuals->m_cur_yaw_dir = 0;

		if( get_manual_rotate( ) != std::numeric_limits < float > ::max( ) ) {
			return user_cmd.m_view_angles.y( ) + get_manual_rotate( );
		}
		else
		return user_cmd.m_view_angles.y( ) + m_cfg->m_real_yaw + g_ctx->addresses( ).m_random_float( -m_cfg->m_jitter_yaw, m_cfg->m_jitter_yaw );
	}

	void c_anti_aim::handle_pitch( valve::user_cmd_t& user_cmd ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		auto anim_state = g_local_player->self( )->anim_state( );

		if( !g_local_player->weapon_info( ) )
			return;

		if( g_local_player->weapon_info( )->m_type == valve::e_weapon_type::grenade
			&&( !g_local_player->weapon( )->pin_pulled( )
				|| user_cmd.m_buttons & valve::e_buttons::in_attack
				|| user_cmd.m_buttons & valve::e_buttons::in_attack2 )
			&& g_local_player->weapon( )->throw_time( ) > 0.f && g_local_player->weapon( )->throw_time( ) < valve::g_global_vars.get( )->m_cur_time )
			return;

		if( g_local_player->weapon_info( )->m_type == valve::e_weapon_type::knife
			&& g_ctx->can_shoot( )
			&&( user_cmd.m_buttons & valve::e_buttons::in_attack
				|| user_cmd.m_buttons & valve::e_buttons::in_attack2 ) )
			return;

		if( g_ctx->can_shoot( )
			&& user_cmd.m_buttons & valve::e_buttons::in_attack
			&& g_local_player->weapon_info( )->m_type != valve::e_weapon_type::grenade )
			return;

		if( user_cmd.m_buttons & valve::e_buttons::in_use )
			return;

		if( !anim_state
			|| !should_disable( user_cmd ) )
			return;

		switch( m_cfg->m_pitch_type ) {
		case 1:
			user_cmd.m_view_angles.x( ) = 90;
			break;
		case 2:
			user_cmd.m_view_angles.x( ) = -90;
			break;
		}
	}
}