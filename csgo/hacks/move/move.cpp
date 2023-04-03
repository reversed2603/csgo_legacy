#include "../../csgo.hpp"

#define check_if_non_valid_number( x )( fpclassify( x ) == FP_INFINITE || fpclassify( x ) == FP_NAN || fpclassify( x ) == FP_SUBNORMAL )

namespace csgo::hacks {
    void c_move::handle( valve::user_cmd_t& cmd, sdk::qang_t nigga_who ) {
       
        /* @note - l1ney.
           - all movement features must be called here... */

        bunny_hop( cmd );

	    auto_strafe( cmd );

	    fast_stop( cmd );

	    slow_walk( cmd );

		unlock_crouch_cooldown( cmd );

		if( g_local_player->self( ) &&
			g_local_player->self( )->weapon( )
			&& g_local_player->self( )->weapon( )->info( ) ) {
			m_max_weapon_speed = g_local_player->self( )->scoped( ) ?
				g_local_player->self( )->weapon( )->info( )->m_max_speed_alt :
				g_local_player->self( )->weapon( )->info( )->m_max_speed;
		}
		else {
			m_max_weapon_speed = 260.f;
		}

		m_max_player_speed = g_local_player->self( )->max_speed( );

		auto target_speed{ 47.f };

		if( hacks::g_aim_bot->stop_type( ) == 2 )
			target_speed = 0.f;

		auto_stop( cmd, nigga_who, target_speed );

    }
	__forceinline bool is_zero( sdk::vec3_t vec )
	{
		return ( vec.x( ) > -0.01f && vec.x( ) < 0.01f &&
			vec.y( ) > -0.01f && vec.y( ) < 0.01f &&
			vec.z( ) > -0.01f && vec.z( ) < 0.01f );
	}

	void c_move::auto_peek( sdk::qang_t& wish_ang, valve::user_cmd_t& user_cmd )
	{
		if( !g_local_player->self( ) )
			return;

		if( !g_local_player->self( )->weapon( ) )
			return;

		if( g_local_player->self( )->weapon( )->is_knife( ) )
			return;

		if( g_local_player->self( )->weapon( )->item_index( ) == valve::e_item_index::taser )
			return;

		if( !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == static_cast < valve::e_weapon_type >( 9 ) )
			return;

		if( g_key_binds->get_keybind_state( &m_cfg->m_auto_peek_key ) )
		{
			if( is_zero( g_ctx->get_auto_peek_info( ).m_start_pos ) )
			{
				g_ctx->get_auto_peek_info( ).m_start_pos = g_local_player->self( )->abs_origin( );

				if( !( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) )
				{
					valve::ray_t* ray = new valve::ray_t( g_ctx->get_auto_peek_info( ).m_start_pos, g_ctx->get_auto_peek_info( ).m_start_pos - sdk::vec3_t( 0.0f, 0.0f, 1000.0f ) );
					valve::trace_filter_world_only_t filter;
					valve::trace_t trace;

					valve::g_engine_trace->trace_ray( *ray, MASK_SOLID, &filter, &trace );

					if( trace.m_frac < 1.f )
					{
						g_ctx->get_auto_peek_info( ).m_start_pos = trace.m_end + sdk::vec3_t( 0.f, 0.f, 2.f );
					}
				}
			}
			else
			{
				if( user_cmd.m_buttons & valve::e_buttons::in_attack && g_local_player->self( )->weapon( ) )
				{
					if( g_local_player->self( )->weapon( )->item_index( ) != valve::e_item_index::revolver )
						g_ctx->get_auto_peek_info( ).m_is_firing = true;
				}

				if( g_ctx->get_auto_peek_info( ).m_is_firing )
				{
					auto current_position = g_local_player->self( )->abs_origin( );
					auto difference = current_position - g_ctx->get_auto_peek_info( ).m_start_pos;

					if( difference.length( 2u ) > 1.7f )
					{
						user_cmd.m_buttons &= ~valve::e_buttons::in_jump;
						const auto chocked_ticks = ( user_cmd.m_number % 2 ) != 1 ?( 14 - valve::g_client_state.get( )->m_choked_cmds ) : valve::g_client_state.get( )->m_choked_cmds;
						const auto& wish_ang_ = sdk::calc_ang( g_local_player->self( )->abs_origin( ), g_ctx->get_auto_peek_info( ).m_start_pos );

						wish_ang.x( ) = wish_ang_.x( );
						wish_ang.y( ) = wish_ang_.y( );
						wish_ang.z( ) = wish_ang_.z( );

						static auto cl_forwardspeed = valve::g_cvar->find_var( xor_str( "cl_forwardspeed" ) );
						user_cmd.m_move.x( ) = cl_forwardspeed->get_float( ) -( 1.2f * chocked_ticks );
						user_cmd.m_move.y( ) = 0.0f;
					}
					else
					{
						g_ctx->get_auto_peek_info( ).m_is_firing = false;
						g_ctx->get_auto_peek_info( ).m_start_pos = sdk::vec3_t( );
					}

					rotate( 
						user_cmd, wish_ang,
						g_local_player->self( )->flags( ),
						g_local_player->self( )->move_type( )
					 );
				}
			}
		}
		else
		{
			g_ctx->get_auto_peek_info( ).m_is_firing = false;
			g_ctx->get_auto_peek_info( ).m_start_pos = sdk::vec3_t( );
		}
	}

	void c_move::accelerate( 
		const valve::user_cmd_t& user_cmd, const sdk::vec3_t& wishdir,
		const float wishspeed, sdk::vec3_t& velocity, float acceleration
	 ) const {
		const auto cur_speed = velocity.dot( wishdir );

		static auto sv_accelerate_use_weapon_speed = valve::g_cvar->find_var( xor_str( "sv_accelerate_use_weapon_speed" ) );

		const auto add_speed = wishspeed - cur_speed;
		if( add_speed <= 0.f )
			return;

		const auto v57 = std::max( cur_speed, 0.f );

		const auto ducking =
			user_cmd.m_buttons & valve::e_buttons::in_duck
			|| g_local_player->self( )->flags( ) & valve::e_ent_flags::ducking;

		auto v20 = true;
		if( ducking
			|| !( user_cmd.m_buttons & valve::e_buttons::in_speed ) )
			v20 = false;

		auto finalwishspeed = std::max( wishspeed, 250.f );
		auto abs_finalwishspeed = finalwishspeed;

		const auto weapon = g_local_player->self( )->weapon( );

		bool slow_down_to_fast_nigga { };

		if( weapon
			&& sv_accelerate_use_weapon_speed->get_int( ) ) {
			const auto item_index = static_cast< std::uint16_t >( weapon->item_index( ) );
			if( weapon->zoom_lvl( ) > 0
				&&( item_index == crypt_int( 11 ) || item_index == crypt_int( 38 ) || item_index == crypt_int( 9 ) || item_index == crypt_int( 8 ) || item_index == crypt_int( 39 ) || item_index == crypt_int( 40 ) ) )
				slow_down_to_fast_nigga = ( m_max_weapon_speed * crypt_float( 0.52f ) ) < crypt_float( 110.f );

			const auto modifier = std::min( 1.f, m_max_weapon_speed / 250.f );

			abs_finalwishspeed *= modifier;

			if( ( !ducking && !v20 )
				|| slow_down_to_fast_nigga )
				finalwishspeed *= modifier;
		}

		if( ducking ) {
			if( !slow_down_to_fast_nigga )
				finalwishspeed *= 0.34f;

			abs_finalwishspeed *= 0.34f;
		}

		if( v20 ) {
			if( !slow_down_to_fast_nigga )
				finalwishspeed *= 0.52f;

			abs_finalwishspeed *= 0.52f;

			const auto abs_finalwishspeed_minus5 = abs_finalwishspeed - 5.f;
			if( v57 < abs_finalwishspeed_minus5 ) {
				const auto v30 =
					std::max( v57 - abs_finalwishspeed_minus5, 0.f )
					/ std::max( abs_finalwishspeed - abs_finalwishspeed_minus5, 0.f );

				const auto v27 = 1.f - v30;
				if( v27 >= 0.f )
					acceleration = std::min( v27, 1.f ) * acceleration;
				else
					acceleration = 0.f;
			}
		}

		const auto v33 = std::min( 
			add_speed,
			( ( valve::g_global_vars.get( )->m_interval_per_tick * acceleration ) * finalwishspeed )
			* g_local_player->self( )->surface_friction( )
		 );

		velocity += wishdir * v33;

		const auto len = velocity.length( );
		if( len
			&& len > m_max_weapon_speed )
			velocity *= m_max_weapon_speed / len;

	}

	void c_move::full_walk_move( 	
		const valve::user_cmd_t& user_cmd, sdk::vec3_t& move,
		sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
	 ) const {
		static auto sv_maxvelocity = valve::g_cvar->find_var( xor_str( "sv_maxvelocity" ) );
		static auto sv_friction = valve::g_cvar->find_var( xor_str( "sv_friction" ) );

		if( static_cast < sdk::ulong_t >( g_local_player->self( )->ground_entity_handle( ) ) ) {
			velocity.z( ) = 0.f;

			const auto speed = velocity.length( );
			if( speed >= 0.1f ) {
				const auto friction = sv_friction->get_float( ) * g_local_player->self( )->surface_friction( );
				const auto sv_stopspeed = sv_friction->get_float( );
				const auto control = speed < sv_stopspeed ? sv_stopspeed : speed;

				const auto new_speed = std::max( 0.f, speed -( ( control * friction ) * valve::g_global_vars.get( )->m_interval_per_tick ) );
				if( speed != new_speed )
					velocity *= new_speed / speed;
			}

			walk_move( user_cmd, move, fwd, right, velocity );

			velocity.z( ) = 0.f;
		}

		const auto sv_maxvelocity_ = sv_maxvelocity->get_float( );
		for( std::size_t i { }; i < 3u; ++i ) {
			auto& element = velocity.at( i );

			if( element > sv_maxvelocity_ )
				element = sv_maxvelocity_;
			else if( element < -sv_maxvelocity_ )
				element = -sv_maxvelocity_;
		}
	}

	void c_move::modify_move( valve::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const {
		sdk::vec3_t fwd { }, right { };

		sdk::ang_vecs( user_cmd.m_view_angles, &fwd, &right, nullptr );

		const auto speed_sqr = user_cmd.m_move.length_sqr( );
		if( speed_sqr >( m_max_player_speed * m_max_player_speed ) )
			user_cmd.m_move *= m_max_player_speed / std::sqrt( speed_sqr );

		full_walk_move( user_cmd, user_cmd.m_move, fwd, right, velocity );
	}

	void c_move::predict_move( const valve::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const {
		sdk::vec3_t fwd { }, right { };

		sdk::ang_vecs( user_cmd.m_view_angles, &fwd, &right, nullptr );

		auto move = user_cmd.m_move;

		const auto speed_sqr = user_cmd.m_move.length_sqr( );
		if( speed_sqr >( m_max_player_speed * m_max_player_speed ) )
			move *= m_max_player_speed / std::sqrt( speed_sqr );

		full_walk_move( user_cmd, move, fwd, right, velocity );
	}


	void c_move::walk_move( 
		const valve::user_cmd_t& user_cmd, sdk::vec3_t& move,
		sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
	 ) const {
		if( fwd.z( ) != 0.f )
			fwd.normalize( );

		if( right.z( ) != 0.f )
			right.normalize( );

		sdk::vec3_t wishvel {
			fwd.x( )* move.x( ) + right.x( ) * move.y( ),
			fwd.y( )* move.x( ) + right.y( ) * move.y( ),
			0.f
		};
		static auto sv_accelerate = valve::g_cvar->find_var( xor_str( "sv_accelerate" ) );
		auto wishdir = wishvel;

		auto wishspeed = wishdir.normalize( );
		if( wishspeed
			&& wishspeed > m_max_player_speed ) {
			wishvel *= m_max_player_speed / wishspeed;

			wishspeed = m_max_player_speed;
		}

		velocity.z( ) = 0.f;
		accelerate( user_cmd, wishdir, wishspeed, velocity, sv_accelerate->get_float( ) );
		velocity.z( ) = 0.f;

		const auto speed_sqr = velocity.length_sqr( );
		if( speed_sqr >( m_max_player_speed * m_max_player_speed ) )
			velocity *= m_max_player_speed / std::sqrt( speed_sqr );

		if( velocity.length( ) < 1.f )
			velocity = { };
	}


	void c_move::slow_walk( valve::user_cmd_t& user_cmd ) const {
		
		sdk::vec3_t velocity { g_local_player->self( )->velocity( ) };
		int    ticks { }, max { 14 };

		if( !g_key_binds->get_keybind_state( &m_cfg->m_slow_walk ) )
			return;

		if( !( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) )
			return;

		static auto sv_friction = valve::g_cvar->find_var( xor_str( "sv_friction" ) );
		static auto sv_stopspeed = valve::g_cvar->find_var( xor_str( "sv_stopspeed" ) );
		float friction = sv_friction->get_float( ) * g_local_player->self( )->surface_friction( );

		for( ; ticks < 15; ++ticks ) {
			float speed = velocity.length( );

			if( speed <= 0.1f )
				break;

			float control = std::max( speed, sv_stopspeed->get_float( ) );
			float drop = control * friction * valve::g_global_vars.get( )->m_interval_per_tick;
			float newspeed = std::max( 0.f, speed - drop );

			if( newspeed != speed ) {
				newspeed /= speed;
				velocity *= newspeed;
			}
		}

		if( ticks >( ( max ) - valve::g_client_state.get( )->m_choked_cmds ) 
			|| !valve::g_client_state.get( )->m_choked_cmds ) {
			user_cmd.m_move = sdk::vec3_t{ 0.f, 0.f, 0.f };
		}
	}

	void c_move::auto_stop( valve::user_cmd_t& user_cmd, sdk::qang_t& who, float target_spd ) {

		static sdk::qang_t wish_ang{ };
		bool predict_available{ };

		if( const auto weapon = g_local_player->self( )->weapon( ) ) {
			if( weapon->info( ) ) {
				if( weapon->info( )->m_type == valve::e_weapon_type::grenade
					|| weapon->info( )->m_type == valve::e_weapon_type::c4
					|| weapon->info( )->m_type == valve::e_weapon_type::knife )
					return;
			}
		}

		if( g_ctx->can_shoot( )
			&& hacks::g_aim_bot->cfg( ).m_stop_modifiers & 1
			&& hacks::g_move->allow_early_stop( ) 
			&& hacks::g_exploits->is_peeking( wish_ang, 2.f ) )
			predict_available = true;

		if( !predict_available )
		if( !hacks::g_aim_bot->stop_type( ) )
			return;

		auto stop_type = hacks::g_aim_bot->stop_type( );

		hacks::g_aim_bot->stop_type( ) = 0;

		if( predict_available ) {
			stop_type = 1;
		}

		if( !( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) 
			|| user_cmd.m_buttons & valve::e_buttons::in_jump )
			return;

		static auto sv_accelerate_use_weapon_speed = valve::g_cvar->find_var( xor_str( "sv_accelerate_use_weapon_speed" ) );

		static auto sv_accelerate = valve::g_cvar->find_var( xor_str( "sv_accelerate" ) );

		const auto weapon = g_local_player->self( )->weapon( );
		if( !weapon )
			return;

		const auto wpn_data = weapon->info( );
		if( !wpn_data )
			return;
		 
		auto max_speed{ 260.f };

		if( g_local_player->self( )->weapon( )
			&& g_local_player->self( )->weapon( )->info( ) ) {
			max_speed = g_local_player->self( )->scoped( ) ?
				g_local_player->self( )->weapon( )->info( )->m_max_speed_alt :
				g_local_player->self( )->weapon( )->info( )->m_max_speed;
		}

		float target_speed{ };

		if( stop_type == 1 )
			target_speed = 36.5f;

		sdk::vec3_t cur_velocity{ g_local_player->self( )->velocity( ) };

		if( predict_available )
			predict_move( user_cmd, cur_velocity );

		const auto speed_2d = cur_velocity.length( 2u );

		if( stop_type == 2 ) {
			if( speed_2d < 1.f ) {
				user_cmd.m_move.x( ) = user_cmd.m_move.y( ) = 0.f;

				return;
			}
		}
		else if( target_speed >= speed_2d ) {
			m_max_player_speed = m_max_weapon_speed = target_speed;

			modify_move( user_cmd, cur_velocity );

			return;
		}

		user_cmd.m_buttons &= ~valve::e_buttons::in_speed;

		auto finalwishspeed = std::min( max_speed, 250.f );

		const auto ducking =
			user_cmd.m_buttons & valve::e_buttons::in_duck	
			|| g_local_player->self( )->flags( ) & valve::e_ent_flags::ducking;

		bool slow_down_to_fast_nigga{ };

		if( sv_accelerate_use_weapon_speed->get_int( ) ) {
			const auto item_index = static_cast< std::uint16_t >( weapon->item_index( ) );
			if( weapon->zoom_lvl( ) > 0
				&&( item_index == crypt_int( 11 ) || item_index == crypt_int( 38 ) || item_index == crypt_int( 9 ) || item_index == crypt_int( 8 ) || item_index == crypt_int( 39 ) || item_index == crypt_int( 40 ) ) )
				slow_down_to_fast_nigga = ( max_speed * crypt_float( 0.52f ) ) < crypt_float( 110.f );

			if( !ducking
				|| slow_down_to_fast_nigga )
				finalwishspeed *= std::min( 1.f, max_speed / 250.f );
		}

		if( ducking
			&& !slow_down_to_fast_nigga )
			finalwishspeed *= crypt_float( 0.33f );

		finalwishspeed =
			( ( valve::g_global_vars.get( )->m_interval_per_tick * sv_accelerate->get_float( ) ) * finalwishspeed )
			* g_local_player->self( )->surface_friction( );

		if( stop_type == 1 ) {
			if( max_speed * 0.3f <= speed_2d ) {
				sdk::qang_t dir{ };
				sdk::vec_angs( cur_velocity *= -1.f, dir );

				dir.y( ) = user_cmd.m_view_angles.y( ) - dir.y( );

				sdk::vec3_t dir_ang_handler{ };

				sdk::ang_vecs( dir, &dir_ang_handler, nullptr, nullptr );

				user_cmd.m_move.x( ) = dir_ang_handler.x( ) * finalwishspeed;
				user_cmd.m_move.y( ) = dir_ang_handler.y( ) * finalwishspeed;
			}
			else {
				m_max_player_speed = m_max_weapon_speed = target_speed;

				modify_move( user_cmd, cur_velocity );
			}
		}
		else {
			sdk::qang_t dir{ };
			sdk::vec_angs( cur_velocity *= -1.f, dir );

			dir.y( ) = user_cmd.m_view_angles.y( ) - dir.y( );

			sdk::vec3_t dir_ang_handler{ };

			sdk::ang_vecs( dir, &dir_ang_handler, nullptr, nullptr );

			user_cmd.m_move.x( ) = dir_ang_handler.x( ) * finalwishspeed;
			user_cmd.m_move.y( ) = dir_ang_handler.y( ) * finalwishspeed;
		}
	}

    void c_move::bunny_hop( valve::user_cmd_t& cmd ) const { 

        if( !m_cfg->m_bhop )
            return;

        if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder )
            return;

        static bool last_jmp = false;
        static bool should_jmp = false;

        if( !last_jmp && should_jmp )
        {
            should_jmp = false;
            cmd.m_buttons |= valve::e_buttons::in_jump;
        }
        else if( cmd.m_buttons & valve::e_buttons::in_jump )
        {
            if( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground || !g_local_player->self( )->velocity( ).z( ) )
            {
                last_jmp = true;
                should_jmp = true;
            }
            else
            {
                cmd.m_buttons &= ~valve::e_buttons::in_jump;
                last_jmp = false;
            }
        }
        else
        {
            last_jmp = false;
            should_jmp = false;
        }
    }

	void c_move::unlock_crouch_cooldown( valve::user_cmd_t& user_cmd ) const {
		
		if( !m_cfg->m_infinity_duck )
			return;

		user_cmd.m_buttons |= valve::e_buttons::in_bullrush;
	}

	void c_move::fast_stop( valve::user_cmd_t& cmd ) const { 

		if( !g_local_player->self( )->alive( ) )
			return;

		if( !m_cfg->m_fast_stop )
			return;

		if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder )
			return;

		if( cmd.m_buttons &( ( 1 << 1 ) |( 1 << 9 ) |( 1 << 10 ) |( 1 << 3 ) |( 1 << 4 ) ) )
			return;

		if( !( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) )
			return;

		if( g_local_player->self( )->velocity( ).length( 2 ) <= 260 * 0.34f )
		{
			cmd.m_move.x( ) = cmd.m_move.y( ) = 0.f;
			return;
		}

		sdk::qang_t resistance_ang = sdk::qang_t( );

		sdk::vec_angs( g_local_player->self( )->velocity( ) * -1.f, resistance_ang );

		resistance_ang.y( ) = cmd.m_view_angles.y( ) - resistance_ang.y( );
		resistance_ang.x( ) = cmd.m_view_angles.x( ) - resistance_ang.x( );

		sdk::vec3_t resistance_vec = sdk::vec3_t( );

		sdk::ang_vecs( resistance_ang, &resistance_vec, nullptr, nullptr );

		cmd.m_move.x( ) = std::clamp( resistance_vec.x( ), -450.f, 450.0f );
		cmd.m_move.y( ) = std::clamp( resistance_vec.y( ), -450.f, 450.0f );
	}

    void c_move::auto_strafe( valve::user_cmd_t& cmd ) const {

		if( !g_local_player->self( )->alive( ) )
			return;

		if( !m_cfg->m_auto_strafe )
			return;

		if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder )
			return;

		if( g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground )
			return;

		static auto cl_sidespeed = valve::g_cvar->find_var( "cl_sidespeed" );
		auto side_speed = cl_sidespeed->get_float( );

		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [ ]( float velocity )
		{
			auto tmp = sdk::to_deg( atan( 30.0f / velocity ) );

			if( check_if_non_valid_number( tmp ) || tmp > 90.0f )
				return 90.0f;

			else if( tmp < 0.0f )
				return 0.0f;
			else
				return tmp;
		};

		auto velocity = g_local_player->self( )->velocity( );
		velocity.z( ) = 0.0f;

		auto forwardmove = cmd.m_move.x( );
		auto sidemove = cmd.m_move.y( );

		if( velocity.length( 2 ) < 5.0f && !forwardmove && !sidemove )
			return;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = cmd.m_view_angles;

		if( forwardmove || sidemove )
		{
			cmd.m_move.x( ) = 0.0f;
			cmd.m_move.y( ) = 0.0f;

			auto turn_angle = atan2( -sidemove, forwardmove );
			viewangles.y( ) += turn_angle * 57.295779513082f;
		}
		else if( forwardmove )
			cmd.m_move.x( ) = 0.0f;

		auto strafe_angle = sdk::to_deg( atan( 15.0f / velocity.length( 2 ) ) );

		if( strafe_angle > 90.0f )
			strafe_angle = 90.0f;
		else if( strafe_angle < 0.0f )
			strafe_angle = 0.0f;

		auto temp = sdk::vec3_t( 0.0f, viewangles.y( ) - old_yaw, 0.0f );
		temp.y( ) = sdk::norm_yaw( temp.y( ) );

		auto yaw_delta = temp.y( );
		old_yaw = viewangles.y( );

		auto abs_yaw_delta = fabs( yaw_delta );

		if( abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f )
		{
			sdk::qang_t velocity_angles;
			sdk::vec_angs( velocity, velocity_angles );

			temp = sdk::vec3_t( 0.0f, viewangles.y( ) - velocity_angles.y( ), 0.0f );
			temp.y( ) = sdk::norm_yaw( temp.y( ) );

			auto velocityangle_yawdelta = temp.y( );
			auto velocity_degree = get_velocity_degree( velocity.length( 2 ) );

			if( velocityangle_yawdelta <= velocity_degree || velocity.length( 2 ) <= 15.0f )
			{
				if( -velocity_degree <= velocityangle_yawdelta || velocity.length( 2 ) <= 15.0f )
				{
					viewangles.y( ) += strafe_angle * turn_direction_modifier;
					cmd.m_move.y( ) = side_speed * turn_direction_modifier;
				}
				else
				{
					viewangles.y( ) = velocity_angles.y( ) - velocity_degree;
					cmd.m_move.y( ) = side_speed;
				}
			}
			else
			{
				viewangles.y( ) = velocity_angles.y( ) + velocity_degree;
				cmd.m_move.y( ) = -side_speed;
			}
		}
		else if( yaw_delta > 0.0f )
			cmd.m_move.y( ) = -side_speed;

		else if( yaw_delta < 0.0f )
			cmd.m_move.y( ) = side_speed;

		auto move = sdk::vec3_t( cmd.m_move.x( ), cmd.m_move.y( ), 0.0f );
		auto speed = move.length( );

		sdk::qang_t angles_move;
		sdk::vec_angs( move, angles_move );

		auto normalized_x = fmod( cmd.m_view_angles.x( ) + 180.0f, 360.0f ) - 180.0f;
		auto normalized_y = fmod( cmd.m_view_angles.y( ) + 180.0f, 360.0f ) - 180.0f;

		auto yaw = sdk::to_rad( normalized_y - viewangles.y( ) + angles_move.y( ) );

		if( normalized_x >= 90.0f 
			|| normalized_x <= -90.0f 
			|| ( cmd.m_view_angles.x( ) >= 90.0f && cmd.m_view_angles.x( ) <= 200.0f ) 
			|| ( cmd.m_view_angles.x( ) <= -90.0f && cmd.m_view_angles.x( ) <= 200.0f ) )
			cmd.m_move.x( ) = -cos( yaw ) * speed;
		else
			cmd.m_move.x( ) = cos( yaw ) * speed;

		cmd.m_move.y( ) = sin( yaw ) * speed;
    }

    void c_move::rotate( valve::user_cmd_t& user_cmd, const sdk::qang_t& wish_angles, const valve::e_ent_flags flags, const valve::e_move_type move_type ) const {
		sdk::vec3_t  move, dir;
		float   delta, len;
		sdk::qang_t   move_angle;

		move = { user_cmd.m_move.x( ),user_cmd.m_move.y( ), 0.f};

		len = move.normalize( );
		if( !len )
			return;

		sdk::vec_angs( move, move_angle );

		delta = ( user_cmd.m_view_angles.y( ) - wish_angles.y( ) );
		move_angle.y( ) += delta;

		sdk::ang_vecs( move_angle, &dir, nullptr, nullptr );

		dir *= len;

		user_cmd.m_buttons &= ~( static_cast < int >( valve::e_buttons::in_fwd ) 
			| static_cast < int >( valve::e_buttons::in_back ) 
			| static_cast < int >( valve::e_buttons::in_move_left ) 
			| static_cast < int >( valve::e_buttons::in_move_right ) );

		if( g_local_player->self( )->move_type( ) == valve::e_move_type::ladder ) {
			if( user_cmd.m_view_angles.x( ) >= 45.f && wish_angles.x( ) < 45.f && std::abs( delta ) <= 65.f )
				dir.x( ) = -dir.x( );

			user_cmd.m_move.x( ) = dir.x( );
			user_cmd.m_move.y( ) = dir.y( );

			if( user_cmd.m_move.x( ) > 200.f )
				user_cmd.m_buttons |= valve::e_buttons::in_fwd;

			else if( user_cmd.m_move.x( ) < -200.f )
				user_cmd.m_buttons |= valve::e_buttons::in_back;

			if( user_cmd.m_move.y( ) > 200.f )
				user_cmd.m_buttons |= valve::e_buttons::in_move_right;

			else if( user_cmd.m_move.y( ) < -200.f )
				user_cmd.m_buttons |= valve::e_buttons::in_move_left;
		}

		else {
			if( user_cmd.m_view_angles.x( ) < -90.f || user_cmd.m_view_angles.x( ) > 90.f )
				dir.x( ) = -dir.x( );

			user_cmd.m_move.x( ) = dir.x( );
			user_cmd.m_move.y( ) = dir.y( );

			if( user_cmd.m_move.x( ) > 0.f )
				user_cmd.m_buttons |= valve::e_buttons::in_fwd;

			else if( user_cmd.m_move.x( ) < 0.f )
				user_cmd.m_buttons |= valve::e_buttons::in_back;

			if( user_cmd.m_move.y( ) > 0.f )
				user_cmd.m_buttons |= valve::e_buttons::in_move_right;

			else if( user_cmd.m_move.y( ) < 0.f )
				user_cmd.m_buttons |= valve::e_buttons::in_move_left;
		}
	}
}