#include "../../../csgo.hpp"

namespace csgo::hacks {
	std::optional < aim_target_t > c_aim_bot::extrapolate( const player_entry_t& entry ) const { 
		if( entry.m_lag_records.empty( ) )
			return std::nullopt;

		const auto& latest = entry.m_lag_records.front( );

		static int lag_max = crypt_int( 16 );
		static int lag_min = crypt_int( 0 );

		if( latest->m_choked_cmds > lag_max
			|| latest->m_dormant
			|| latest->m_choked_cmds <= lag_min )
			return std::nullopt;

		const c_ctx::net_info_t& net_info = g_ctx->net_info( );

		// uhh..
		// if( latest->valid( ) ) 
		//	return aim_target_t{ const_cast< player_entry_t* > ( &entry ), latest };

		const int receive_tick = std::abs( ( game::g_client_state.get( )->m_server_tick + ( game::to_ticks( net_info.m_latency.m_out ) ) ) - game::to_ticks( latest->m_sim_time ) );
		const float lag_delta = static_cast< float >( receive_tick / latest->m_choked_cmds );

		// too much lag to predict
		if( lag_delta >= 1.f )
			return std::nullopt;
		
		const float adjusted_arrive_tick = std::clamp( game::to_ticks( ( ( g_ctx->net_info( ).m_latency.m_out ) + game::g_global_vars.get( )->m_real_time )
			- entry.m_receive_time ), 0, 100 );

		if( ( adjusted_arrive_tick - latest->m_choked_cmds ) >= 0 ) { 
			return std::nullopt;
		}

		// no prediction needed
		if( lag_delta <= 0.f )
			return aim_target_t{ const_cast< player_entry_t* > ( &entry ), latest };

		const int delta_ticks = game::g_client_state.get( )->m_server_tick - latest->m_receive_tick;

		if( game::to_ticks( g_ctx->net_info( ).m_latency.m_out ) <= latest->m_choked_cmds - delta_ticks )
			return aim_target_t{ const_cast< player_entry_t* > ( &entry ), latest };

		extrapolation_data_t data{ entry.m_player, latest };

		float change = 0.f, dir = 0.f;

		// get the direction of the current velocity.
		if( data.m_velocity.y( ) != 0.f || data.m_velocity.x( ) != 0.f )
			dir = sdk::to_deg( std::atan2( data.m_velocity.y( ), data.m_velocity.x( ) ) );

		// we have more than one update
		// we can compute the direction.
		if( entry.m_lag_records.size( ) > 1 ) { 
			// get the delta time between the 2 most recent records.
			float dt = latest->m_sim_time - entry.m_lag_records.at( 1 )->m_sim_time;

			// init to 0.
			float prevdir = 0.f;

			// get the direction of the prevoius velocity.
			if( entry.m_lag_records.at( 1 )->m_anim_velocity.y( ) != 0.f || entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) != 0.f )
				prevdir = sdk::to_deg( std::atan2( entry.m_lag_records.at( 1 )->m_anim_velocity.y( ), entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) ) );

			if( std::abs( sdk::angle_diff( prevdir, dir ) ) > 35.f )
				return std::nullopt; // retracking is hard to properly predict, delay shot here

			// compute the direction change per tick.
			change = ( sdk::norm_yaw( dir - prevdir ) / dt ) * game::g_global_vars.get( )->m_interval_per_tick;
		}

		if( std::abs( change ) > 6.f )
			change = 0.f;

		data.m_change = change;
		data.m_dir = dir;

		for( int i { }; i < latest->m_choked_cmds; ++i ) { 
			data.m_dir = sdk::norm_yaw( data.m_dir + change );
			data.m_sim_time += game::g_global_vars.get( )->m_interval_per_tick;
			
			hacks::g_sim_ctx->handle_context( data );
		}	

		latest->m_extrapolated = true;

		aim_target_t ret{ const_cast< player_entry_t* > ( &entry ), std::make_shared< lag_record_t > ( ) };

		*ret.m_lag_record.value( ).get( ) = *latest.get( );

		ret.m_lag_record.value( )->m_sim_time = data.m_sim_time;
		ret.m_lag_record.value( )->m_flags = data.m_flags;

		ret.m_lag_record.value( )->m_origin = data.m_origin;
		ret.m_lag_record.value( )->m_anim_velocity = data.m_velocity;

		const auto origin_delta = data.m_origin - latest->m_origin;

		for( std::size_t i{ }; i < latest->m_bones_count; ++i ) { 
			sdk::mat3x4_t& bone = latest->m_bones.at( i );

			bone[ 0 ][ 3 ] += origin_delta.x( );
			bone[ 1 ][ 3 ] += origin_delta.y( );
			bone[ 2 ][ 3 ] += origin_delta.z( );
		}

		latest->m_extrapolated_bones = latest->m_bones;

		return ret;
	}

	void c_aim_bot::player_move( extrapolation_data_t& data ) const { 
		static auto sv_gravity = game::g_cvar->find_var( xor_str( "sv_gravity" ) );
		static auto sv_jump_impulse = game::g_cvar->find_var( xor_str( "sv_jump_impulse" ) );
		static auto sv_enable_bhop = game::g_cvar->find_var( xor_str( "sv_enablebunnyhopping" ) );
		if( data.m_flags & game::e_ent_flags::on_ground ) { 
			if( !sv_enable_bhop->get_int( ) ) { 
				const auto speed = data.m_velocity.length( );

				const auto max_speed = data.m_player->max_speed( ) * crypt_float( 1.1f );
				if( max_speed > 0.f
					&& speed > max_speed )
					data.m_velocity *= ( max_speed / speed );
			}

			data.m_velocity.z( ) = sv_jump_impulse->get_float( );
		}
		else
			data.m_velocity.z( ) -=
			sv_gravity->get_float( ) * game::g_global_vars.get( )->m_interval_per_tick;

		game::trace_t trace { };
		game::trace_filter_world_only_t trace_filter { };

		game::g_engine_trace->trace_ray( 
			{ 
			 data.m_origin,
			 data.m_origin + data.m_velocity * game::g_global_vars.get( )->m_interval_per_tick,
			 data.m_obb_min, data.m_obb_max
			},
			CONTENTS_SOLID, &trace_filter, &trace
		 );

		if( trace.m_frac != crypt_float( 1.f ) ) { 
			for( int i { }; i < 2; ++i ) { 
				data.m_velocity -= trace.m_plane.m_normal * data.m_velocity.dot( trace.m_plane.m_normal );

				const auto adjust = data.m_velocity.dot( trace.m_plane.m_normal );
				if( adjust < 0.f )
					data.m_velocity -= trace.m_plane.m_normal * adjust;

				game::g_engine_trace->trace_ray( 
					{ 
					 trace.m_end,
					 trace.m_end + ( data.m_velocity * ( game::g_global_vars.get( )->m_interval_per_tick * ( 1.f - trace.m_frac ) ) ),
					 data.m_obb_min, data.m_obb_max
					},
					CONTENTS_SOLID, &trace_filter, &trace
				 );

				if( trace.m_frac == 1.f )
					break;
			}
		}

		data.m_origin = trace.m_end;

		game::g_engine_trace->trace_ray( 
			{ 
			 trace.m_end,
			 { trace.m_end.x( ) , trace.m_end.y( ) , trace.m_end.z( ) - crypt_float( 2.f ) },
			 data.m_obb_min, data.m_obb_max
			},
			CONTENTS_SOLID, &trace_filter, &trace
		 );

		data.m_flags &= ~game::e_ent_flags::on_ground;

		if( trace.m_frac != crypt_float( 1.f )
			&& trace.m_plane.m_normal.z( ) > crypt_float( 0.7f ) )
			data.m_flags |= game::e_ent_flags::on_ground;
	}
}