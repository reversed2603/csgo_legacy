#include "../../../csgo.hpp"

namespace csgo::hacks {
	__forceinline void trace_hull( const sdk::vec3_t& src, const sdk::vec3_t& end, game::trace_t& trace, game::base_entity_t* entity, std::uint32_t mask, int col_group ) { 
		static const sdk::vec3_t hull[ 2 ] = { sdk::vec3_t( -2.0f, -2.0f, -2.0f ), sdk::vec3_t( 2.0f, 2.0f, 2.0f ) };

		game::trace_filter_simple_t filter{ entity, col_group };
		game::ray_t ray{ src, end, hull[ 0 ], hull[ 1 ] };

		game::g_engine_trace->trace_ray( ray, mask, reinterpret_cast< game::base_trace_filter_t* > ( &filter ), &trace );
	}

	__forceinline void trace_line( const sdk::vec3_t& src, const sdk::vec3_t& end, game::trace_t& trace, game::base_entity_t* entity, std::uint32_t mask, int col_group ) { 
		game::trace_filter_simple_t filter{ entity, col_group };

		game::ray_t ray{ src, end };

		game::g_engine_trace->trace_ray( ray, mask, reinterpret_cast< game::base_trace_filter_t* > ( &filter ), &trace );
	}

	std::vector < game::base_entity_t* > broken_entities{ };

	void clear_broken_entities( ) { broken_entities.clear( ); }

	void c_grenades::handle_warning_pred( game::base_entity_t* const entity, const game::e_class_id class_id ) { 
		if( !game::g_engine->in_game( ) 
			|| !g_local_player->self( ) ) { 
			return m_throwed_grenades.clear( );
		}

		if( class_id == game::e_class_id::molotov_projectile || class_id == game::e_class_id::base_cs_grenade_projectile ) { 

			bool can_do = true;

			if( class_id == game::e_class_id::base_cs_grenade_projectile ) { 
				const auto studio_model = entity->renderable( )->model( );
				if( !studio_model
					|| std::string_view( studio_model->m_path ).find( "fraggrenade" ) == std::string::npos )
					can_do = false;
			}

			if( !entity->networkable( )->dormant( ) && can_do )
			{ 
				const auto handle = ( static_cast < game::cs_player_t* > ( entity ) )->ref_handle( );

				if( entity->effects_via_offset( ) & 0x020 )
					m_throwed_grenades.erase( handle );
				else { 
					if( m_throwed_grenades.find( handle ) == m_throwed_grenades.end( ) ) { 
						m_throwed_grenades.emplace( 
							std::piecewise_construct,
							std::forward_as_tuple( handle ),
							std::forward_as_tuple( 
								static_cast< game::cs_player_t* > ( game::g_entity_list->get_entity( entity->thrower_handle( ) ) ),
								class_id == game::e_class_id::molotov_projectile ? game::e_item_index::molotov : game::e_item_index::he_grenade,
								entity->origin( ), entity->velocity( ),
								entity->get_ent_spawn_time( ), game::to_ticks( entity->sim_time( ) - entity->get_ent_spawn_time( ) )
							 )
						 );
					}

					if( !add_grenade_simulation( m_throwed_grenades.at( handle ), true ) )
						m_throwed_grenades.erase( handle );
				}
			}
		}
	}

	void c_grenades::grenade_simulation_t::predict( const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset ) { 
		m_origin = origin;
		m_velocity = velocity;
		m_collision_group = 13;
		m_bounces_count = 0;
		m_detonated = false;
		clear_broken_entities( );

		const auto tick = game::to_ticks( 1.f / 30.f );

		m_last_update_tick = -tick;
		static auto molotov_throw_detonate_time = game::g_cvar->find_var( xor_str( "molotov_throw_detonate_time" ) );
		switch( m_index ) { 
		case game::e_item_index::smoke_grenade: m_next_think_tick = game::to_ticks( 1.5f ); break;
		case game::e_item_index::decoy: m_next_think_tick = game::to_ticks( 2.f ); break;
		case game::e_item_index::flashbang:
		case game::e_item_index::he_grenade:
			m_detonate_time = 1.5f;
			m_next_think_tick = game::to_ticks( 0.02f );

			break;
		case game::e_item_index::molotov:
		case game::e_item_index::inc_grenade:
			m_detonate_time = molotov_throw_detonate_time->get_float( );
			m_next_think_tick = game::to_ticks( 0.02f );

			break;
		default: break;
		}

		m_source_time = throw_time;

		const auto max_sim_amt = game::to_ticks( 60.f );
		for( ; m_tick < max_sim_amt; ++m_tick ) { 
			if( m_next_think_tick < m_tick ) {
				think( );
			}

			if( m_tick < offset )
				continue;

			if( physics_simulate( ) )
				break;

			if( ( m_last_update_tick + tick ) > m_tick )
				continue;

			update_path( false );
		}

		if( ( m_last_update_tick + tick ) <= m_tick )
			update_path( false );

		m_expire_time = throw_time + game::to_time( m_tick );
	}

	bool c_grenades::grenade_simulation_t::physics_simulate( ) { 
		if( m_detonated )
			return true;

		static auto sv_gravity = game::g_cvar->find_var( xor_str( "sv_gravity" ) );
		const auto new_velocity_z = m_velocity.z( ) - ( sv_gravity->get_float( ) * 0.4f ) * game::g_global_vars.get( )->m_interval_per_tick;

		const auto move = sdk::vec3_t( 
			m_velocity.x( ) * game::g_global_vars.get( )->m_interval_per_tick,
			m_velocity.y( ) * game::g_global_vars.get( )->m_interval_per_tick,
			( m_velocity.z( ) + new_velocity_z ) / 2.f * game::g_global_vars.get( )->m_interval_per_tick
		 );

		m_velocity.z( ) = new_velocity_z;

		game::trace_t trace{ };

		physics_push_entity( move, trace );

		if( m_detonated )
			return true;

		if( trace.m_frac != 1.f ) { 
			update_path( true );

			perform_fly_collision_resolution( trace );
		}

		return false;
	}

	void c_grenades::grenade_simulation_t::physics_trace_entity( 
		const sdk::vec3_t& src, const sdk::vec3_t & dst,
		const std::uint32_t mask, game::trace_t& trace
	 ) { 
		trace_hull( src, dst, trace, g_local_player->self( ), 0x200400B, m_collision_group );

		if( trace.m_start_solid
			&& ( trace.m_contents & CONTENTS_CURRENT_90 ) ) { 
			trace.clear( );

			trace_hull( src, dst, trace, g_local_player->self( ), mask & ~CONTENTS_CURRENT_90, m_collision_group );
		}

		if( trace.m_frac < 1.f
			|| trace.m_all_solid 
			|| trace.m_start_solid )
		{
			if( trace.m_entity && trace.m_entity->is_player( ) )
			{
				trace.clear( );

				trace_line( src, dst, trace, g_local_player->self( ), mask, m_collision_group );
			}
		}
	}

	void c_grenades::grenade_simulation_t::physics_push_entity( const sdk::vec3_t& push, game::trace_t& trace ) { 
		int mask{ };
		if( m_collision_group == 1 )
			mask = ( MASK_SOLID | _CONTENTS_CURRENT_90 ) & ~CONTENTS_MONSTER;
		else
			mask = MASK_SOLID | CONTENTS_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE | _CONTENTS_CURRENT_90 | CONTENTS_HITBOX;

		physics_trace_entity( m_origin, m_origin + push, mask,
			trace
		 );

		if( trace.m_start_solid ) { 
			m_collision_group = 3;
			trace_line( m_origin - push, m_origin + push, trace, g_local_player->self( ), ( MASK_SOLID | CONTENTS_CURRENT_90 ) & ~CONTENTS_MONSTER, m_collision_group );
		}

		if( trace.m_frac != 0.f )
			m_origin = trace.m_end;

		static auto weapon_molotov_maxdetonateslope = game::g_cvar->find_var( xor_str( "weapon_molotov_maxdetonateslope" ) );

		if( !trace.m_entity )
			return;

		if( trace.m_entity->is_player( )
			|| ( m_index != game::e_item_index::molotov && m_index != game::e_item_index::inc_grenade )
			|| trace.m_plane.m_normal.z( ) < std::cos( sdk::to_rad( weapon_molotov_maxdetonateslope->get_float( ) ) ) )
			return;

		detonate( true );
	}

	void draw_beam_trail( sdk::vec3_t src, sdk::vec3_t end, sdk::col_t color )
	{ 
		game::beam_info_t info{ };

		info.m_start = src;
		info.m_end = end;
		info.m_model_index = game::g_model_info->model_index( xor_str( "sprites/white.vmt" ) );
		info.m_model_name = xor_str( "sprites/white.vmt" );
		info.m_life = 0.015f;
		info.m_width = 0.4f;
		info.m_end_width = 0.4f;
		info.m_fade_length = 1.f;
		info.m_amplitude = 2.3f;   // beam 'jitter'.
		info.m_brightness = color.a( );
		info.m_speed = 0.6f;
		info.m_segments = 2;
		info.m_renderable = true;
		info.m_flags = 0;
		info.m_red = color.r( );
		info.m_green = color.g( );
		info.m_blue = color.b( );

		const auto beam = game::g_beams->create_beam_points( info );
		if( !beam )
			return;

		game::g_beams->draw_beam( beam );
	}

	void c_grenades::add_trail( const grenade_simulation_t& sim, sdk::col_t clr, float lifetime, float thickness ) const { 
		auto prev = sim.m_path.front( ).first;

		// iterate and draw path.
		for( const auto& cur : sim.m_path ) { 
			sdk::vec3_t ang_orientation = ( prev - cur.first );
   
			sdk::vec3_t mins = sdk::vec3_t( 0.f, -thickness, -thickness );
			sdk::vec3_t maxs = sdk::vec3_t( ang_orientation.length( ), thickness, thickness );
 
			game::g_glow->add_glow_box( cur.first, ang_orientation.angles( ), mins, maxs, clr, lifetime );

			// store point for next iteration.
			prev = cur.first;
		}
	}

	bool c_grenades::add_grenade_simulation( const grenade_simulation_t& sim, const bool warning ) const { 
		if( sim.m_path.size( ) < 2u
			|| game::g_global_vars.get( )->m_cur_time >= sim.m_expire_time )
			return false;

		const auto mod = std::clamp( 
			( sim.m_expire_time - game::g_global_vars.get( )->m_cur_time )
			/ game::to_time( sim.m_tick ),
			0.f, 1.f
		 );

		auto cfg = g_visuals->cfg( );
		
		bool has_enemy_warn = ( cfg.m_grenade_trajectory_options & 4 && warning ) && !sim.m_owner->friendly( g_local_player->self( ) );
		bool has_team_warn = ( cfg.m_grenade_trajectory_options & 2 && warning ) && sim.m_owner->friendly( g_local_player->self( ) );

		if( sim.m_owner != g_local_player->self( ) 
			&& ( has_enemy_warn || has_team_warn ) ) { 
			const auto& explode_pos = sim.m_path.back( ).first;
			auto dist = ( g_local_player->self( )->origin( ) - explode_pos ).length( );

			sdk::col_t clr = sim.m_owner->friendly( g_local_player->self( ) ) ? 
				// friendly color
				sdk::col_t( cfg.m_friendly_grenade_proximity_warning_clr[ 0 ] * 255.f, cfg.m_friendly_grenade_proximity_warning_clr[ 1 ] * 255.f,
					cfg.m_friendly_grenade_proximity_warning_clr[ 2 ] * 255.f, cfg.m_friendly_grenade_proximity_warning_clr[ 3 ] * 255.f )
				: 
				// enemy color
				sdk::col_t( cfg.m_grenade_proximity_warning_clr[ 0 ] * 255.f, cfg.m_grenade_proximity_warning_clr[ 1 ] * 255.f, 
					cfg.m_grenade_proximity_warning_clr[ 2 ] * 255.f, cfg.m_grenade_proximity_warning_clr[ 3 ] * 255.f );

			if( dist < 1000.f ) { 
				add_trail( sim, clr.alpha( 145 * mod ), 0.75f * game::g_global_vars.get( )->m_frame_time, 1.f );

				sdk::vec3_t screen_pos{ };
				const auto on_screen = g_render->world_to_screen( explode_pos, screen_pos );
				if( !on_screen )
					return true;

				const auto unk = sdk::vec2_t( g_visuals->screen_x / 18.f, g_visuals->screen_y / 18.f );
				if( !on_screen
					|| screen_pos.x( ) < -unk.x( )
					|| screen_pos.x( ) > ( g_visuals->screen_x + unk.x( ) )
					|| screen_pos.y( ) < -unk.y( )
					|| screen_pos.y( ) > ( g_visuals->screen_y + unk.y( ) ) ) { 
					sdk::vec3_t dir{ };
					sdk::ang_vecs( game::g_view_render->m_setup.m_angles, &dir, nullptr, nullptr );

					dir.z( ) = 0.f;
					dir.normalize( );

					const auto radius = 210.f * ( g_visuals->screen_y / 480.f );

					auto delta = explode_pos - game::g_view_render->m_setup.m_origin;

					delta.normalize( );

					screen_pos.x( ) = radius * -delta.dot( sdk::vec3_t{ 0.f, 0.f, 1.f }.cross( dir ) );
					screen_pos.y( ) = radius * -delta.dot( dir );

					const auto radians = sdk::to_rad( 
						-sdk::to_deg( std::atan2( screen_pos.x( ), screen_pos.y( ) + sdk::pi ) )
					 );

					screen_pos.x( ) = static_cast< int > ( g_visuals->screen_x / 2.f + ( radius * std::sin( radians ) ) );
					screen_pos.y( ) = static_cast< int > ( g_visuals->screen_y / 2.f - ( radius * std::cos( radians ) ) );
				}

				g_render->m_draw_list->AddCircleFilled( ImVec2( screen_pos.x( ), screen_pos.y( ) ), 18.f, ImColor( 0.1f, 0.1f, 0.1f, 0.75f * mod ), 255.f );
				g_render->m_draw_list->AddCircle( ImVec2( screen_pos.x( ), screen_pos.y( ) ), 18.f, ImColor( 0.f, 0.f, 0.f, 0.75f * mod ), 255.f );
				std::string icon = "";
				switch( sim.m_index )
				{ 
					case game::e_item_index::he_grenade: icon = xor_str( "j" ); break;
					case game::e_item_index::smoke_grenade: icon = xor_str( "k" ); break;
					case game::e_item_index::flashbang: icon = xor_str( "i" ); break;
					case game::e_item_index::decoy: icon = xor_str( "m" ); break;
					case game::e_item_index::inc_grenade: icon = xor_str( "n" ); break;
					case game::e_item_index::molotov: icon = xor_str( "l" ); break;
					default: break;
				}

				g_render->text( icon, sdk::vec2_t( screen_pos.x( ) + 1, screen_pos.y( ) ), sdk::col_t( 255, 255, 255, 225 * mod ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
				return true;
			}
		}
		else if( sim.m_owner == g_local_player->self( ) ) {
			if( ( !warning && cfg.m_grenade_trajectory_options & 1 )
				|| ( warning && cfg.m_grenade_trajectory_options & 8 ) ) { 
				sdk::col_t clr = sdk::col_t( cfg.m_grenade_trajectory_clr[ 0 ] * 255.f, cfg.m_grenade_trajectory_clr[ 1 ] * 255.f,
					cfg.m_grenade_trajectory_clr[ 2 ] * 255.f, cfg.m_grenade_trajectory_clr[ 3 ] * 255.f );

				add_trail( sim, clr, 1.75f * game::g_global_vars.get( )->m_frame_time, 0.1f );
			}
		}

		return true;
	}

	sdk::vec3_t extrapolate_pos( sdk::vec3_t pos, sdk::vec3_t extension, float amount, float interval )
	{
		return pos + ( extension * ( interval * amount ) );
	}

	void c_grenades::on_create_move( const game::user_cmd_t& cmd ) { 
		m_grenade_trajectory = { };

		if( !g_local_player->weapon( )
			|| !g_local_player->weapon_info( ) )
			return;

		if( g_local_player->weapon_info( )->m_type != game::e_weapon_type::grenade
			|| ( !g_local_player->weapon( )->pin_pulled( ) && g_local_player->weapon( )->throw_time( ) == 0.f ) )
			return;

		m_grenade_trajectory.m_owner = g_local_player->self( );
		m_grenade_trajectory.m_index = g_local_player->weapon( )->item_index( );

		auto view_angles = cmd.m_view_angles;

		sdk::norm_yaw( view_angles.x( ) );

		view_angles.x( ) -= ( 90.f - std::fabsf( view_angles.x( ) ) ) * 10.f / 90.f;

		const auto throw_strength = std::clamp( g_local_player->weapon( )->throw_strength( ), 0.f, 1.f );

		float velocity = std::clamp( g_local_player->weapon_info( )->m_throw_velocity * 0.9f, 15.f, 750.f );
		velocity *= throw_strength * 0.7f + 0.3f;

		sdk::vec3_t forward{ };
		sdk::ang_vecs( view_angles, &forward, nullptr, nullptr );

		sdk::vec3_t src = extrapolate_pos( ( g_ctx->shoot_pos( ) ), g_local_player->self( )->velocity( ), 1.5f, game::g_global_vars.get( )->m_interval_per_tick );

		src.z( ) += throw_strength * 12.f - 12.f;

		game::trace_t trace{ };
		sdk::vec3_t mins{ -2.f, -2.f, -2.f };
		sdk::vec3_t maxs{ 2.f, 2.f, 2.f };
		game::trace_filter_simple_t trace_filter{ g_local_player->self( ), 0 };
		game::g_engine_trace->trace_ray( 
			{ src, src + forward * 22.f, mins, maxs },
			( MASK_SOLID | CONTENTS_CURRENT_90 ),
			reinterpret_cast< game::base_trace_filter_t* > ( &trace_filter ), &trace
		 );

		m_grenade_trajectory.predict( 
			trace.m_end - forward * 6.f,
			forward * velocity + g_local_player->self( )->velocity( ) * 1.25f,
			game::g_global_vars.get( )->m_cur_time, 0
		 );
	}
	 
	void c_grenades::grenade_simulation_t::physics_clip_velocity( const sdk::vec3_t& in, const sdk::vec3_t& normal, sdk::vec3_t& out, float overbounce ) // https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/shared/physics_main_shared.cpp#L1319
	{
		const auto stop_epsilon = 0.1f;
 
		auto backoff = in.dot( normal ) * overbounce;
		for( auto i = 0; i < 3; ++i )
		{
			auto change = normal.at( i ) * backoff;
			out.at( i ) = in.at( i ) - change;
			if( out.at( i ) > -stop_epsilon && out.at( i ) < stop_epsilon )
				out.at( i ) = 0.f;
		}
	}

	void c_grenades::grenade_simulation_t::perform_fly_collision_resolution( game::trace_t& trace ) { 
		auto surface_elasticity = 1.f;
 
		if( trace.m_entity )
		{
			if( g_auto_wall->is_breakable( trace.m_entity ) ) {
				m_velocity *= 0.4f;

				return;
			}
 
			if( trace.m_entity->is_player( ) )
				surface_elasticity = 0.3f;
 
			if( trace.m_entity != game::g_entity_list->get_entity( 0 ) ) // didn't hit world
			{
				if( m_last_hit_entity == trace.m_entity )
				{
					if( trace.m_entity->is_player( ) )
					{
						m_collision_group = 1;
 
						return;
					}
				}
 
				m_last_hit_entity = trace.m_entity;
			}
		}
 
		auto elasticity = 0.45f * surface_elasticity;
		elasticity = std::clamp( elasticity, 0.f, 0.9f );
 
		sdk::vec3_t velocity{ };
		physics_clip_velocity( m_velocity, trace.m_plane.m_normal, velocity, 2.f );
		velocity *= elasticity;
	
		if( trace.m_plane.m_normal.z( ) > 0.7f )
		{
			auto speed_sqr = velocity.dot( velocity );
			if( speed_sqr > 96000.f )
			{
				const auto l = velocity.normalized( ).dot( trace.m_plane.m_normal );
				if( l > 0.5f )
					velocity *= 1.f - l + 0.5f;
			}
 
			if( speed_sqr < 20.f * 20.f )
				m_velocity = { };
			else
			{
				m_velocity = velocity;
 
				physics_push_entity( velocity * ( ( 1.f - trace.m_frac ) * game::g_global_vars.get( )->m_interval_per_tick ), trace );
			}
		}
		else
		{
			m_velocity = velocity;
 
			physics_push_entity( velocity * ( ( 1.f - trace.m_frac ) * game::g_global_vars.get( )->m_interval_per_tick ), trace );
		}
 
		if( m_bounces_count > 20 )
			return detonate( false );
		else
			++m_bounces_count;
	}

	void c_grenades::grenade_simulation_t::think( ) { 
		switch( m_index ) { 
		case game::e_item_index::smoke_grenade:
			if( m_velocity.length_sqr( ) <= 0.01f )
				detonate( false );

			break;
		case game::e_item_index::decoy:
			if( m_velocity.length_sqr( ) <= 0.04f )
				detonate( false );

			break;
		case game::e_item_index::flashbang:
		case game::e_item_index::he_grenade:
		case game::e_item_index::molotov:
		case game::e_item_index::inc_grenade:
			if( game::to_time( m_tick ) > m_detonate_time )
				detonate( false );

			break;
		default: break;
		}

		m_next_think_tick = m_tick + game::to_ticks( 0.2f );
	}

	void c_grenades::grenade_simulation_t::detonate( const bool bounced ) { 
		m_detonated = true;

		update_path( bounced );
	}

	void c_grenades::grenade_simulation_t::update_path( const bool bounced ) { 
		m_last_update_tick = m_tick;

		if( m_path.size( ) < 200 ) { 
			m_path.emplace_back( m_origin, bounced );
		}
	}
}