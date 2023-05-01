#include "../../../csgo.hpp"

namespace csgo::hacks { 
	void c_sim_context::handle_context( extrapolation_data_t& data ) { 
		if( !data.m_was_in_air ) { 
			friction( data );
			data.m_velocity.z( ) = 0.f;
		}

		check_velocity( data );

		if( !data.m_was_in_air )
			walk_move( data );
		else
			air_move( data );

		data.m_was_in_air = !categorize_pos( data );

		check_velocity( data );

		if( !data.m_was_in_air )
			data.m_velocity.z( ) = 0.f;
	}

	void c_sim_context::air_move( extrapolation_data_t& data ) { 
		sdk::vec3_t right{ };
		sdk::vec3_t fwd{ };
		sdk::vec3_t wish_vel{ };
		sdk::ang_vecs( sdk::qang_t( 0.f, data.m_dir, 0.f ), &fwd, &right, nullptr );

		right.z( ) = 0.f;
		fwd.z( ) = 0.f;

		right.normalize( );
		fwd.normalize( );

		for( int i{ }; i < 2; ++i )
			wish_vel.at( i ) = ( fwd.at( i ) * data.m_velocity.x( ) ) + ( right.at( i ) * data.m_velocity.y( ) );
			

		//wish_vel = data.m_velocity;
		wish_vel.z( ) = 0.f;

		auto wish_dir = data.m_velocity;
		auto wish_spd = wish_dir.normalize( );

		air_accelerate( data, wish_dir, wish_spd );

		try_player_move( data );
	}

	/* that's a pasta from moneybot and i really hope nave didn't fail anything here like in air_move o__o */
	void c_sim_context::try_touch_ground_in_quad( extrapolation_data_t& data, const sdk::vec3_t& start, const sdk::vec3_t& end, game::trace_t* trace ) { 
		sdk::vec3_t mins{ }, maxs{ };

		sdk::vec3_t mins_src = data.m_obb_min;
		sdk::vec3_t maxs_src = data.m_obb_max;

		float fraction = trace->m_frac;
		sdk::vec3_t end_pos = trace->m_end;

		mins = mins_src;
		maxs = sdk::vec3_t( std::min( 0.f, maxs_src.x( ) ), std::min( 0.f, maxs_src.y( ) ), maxs_src.z( ) );

		try_touch_ground( start, end, mins, maxs, trace );
		if( trace->m_entity && trace->m_plane.m_normal.z( ) >= 0.7f ) { 
			trace->m_frac = fraction;
			trace->m_end = end_pos;
			return;
		}

		mins = sdk::vec3_t( std::max( 0.f, mins_src.x( ) ), std::max( 0.f, mins_src.y( ) ), mins_src.z( ) );
		maxs = maxs_src;

		try_touch_ground( start, end, mins, maxs, trace );
		if( trace->m_entity && trace->m_plane.m_normal.z( ) >= 0.7f ) { 
			trace->m_frac = fraction;
			trace->m_end = end_pos;
			return;
		}

		mins = sdk::vec3_t( mins_src.x( ), std::max( 0.f, mins_src.y( ) ), mins_src.z( ) );
		maxs = sdk::vec3_t( std::min( 0.f, maxs_src.x( ) ), maxs_src.y( ), maxs_src.z( ) );

		try_touch_ground( start, end, mins, maxs, trace );
		if( trace->m_entity && trace->m_plane.m_normal.z( ) >= 0.7f ) { 
			trace->m_frac = fraction;
			trace->m_end = end_pos;
			return;
		}

		mins = sdk::vec3_t( std::max( 0.f, mins_src.x( ) ), mins_src.y( ), mins_src.z( ) );
		maxs = sdk::vec3_t( maxs_src.x( ), std::min( 0.f, maxs_src.y( ) ), maxs_src.z( ) );

		try_touch_ground( start, end, mins, maxs, trace );
		if( trace->m_entity && trace->m_plane.m_normal.z( ) >= 0.7f ) { 
			trace->m_frac = fraction;
			trace->m_end = end_pos;
			return;
		}

		trace->m_frac = fraction;
		trace->m_end = end_pos;
	}
}