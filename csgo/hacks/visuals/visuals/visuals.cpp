#include "../../../csgo.hpp"

void* dm_ecx = 0;
std::map < int, uintptr_t > dm_ctx = { };
std::map < int, const csgo::valve::draw_model_state_t* > dm_state = { };
std::map < int, const csgo::valve::model_render_info_t* > dm_info = { };

namespace csgo::hacks { 

	RECT get_bbox( valve::cs_player_t* ent ) {

		if( ( ( valve::cs_player_t* )ent )->is_player( ) ) {
			float x, y, w, h;
			int screen_x{ }, screen_y{ };
			valve::g_engine->get_screen_size( screen_x, screen_y );

			sdk::vec3_t pos = ent->abs_origin( );

			sdk::vec3_t top = pos + sdk::vec3_t( 0, 0, ent->obb_max( ).z( ) );

			sdk::vec3_t pos_screen, top_screen;

			if( !g_render->world_to_screen( pos, pos_screen ) ||
				!g_render->world_to_screen( top, top_screen ) )
				return RECT{ };

			x = int( top_screen.x( ) - ( ( pos_screen.y( ) - top_screen.y( ) ) / 2 ) / 2 );
			y = int( top_screen.y( ) );

			w = int( ( ( pos_screen.y( ) - top_screen.y( ) ) ) / 2 );
			h = int( ( pos_screen.y( ) - top_screen.y( ) ) );

			const bool out_of_fov = pos_screen.x( ) + w + 20 < 0 || pos_screen.x( ) - w - 20 > screen_x || pos_screen.y( ) + 20 < 0 || pos_screen.y( ) - h - 20 > screen_y;

			if( out_of_fov ) {
				return RECT{ };
			}

			return RECT{ long( x ), long( y ), long( x + w ), long( y + h ) };
		}
	}

	__forceinline bool is_zero_vec3_t( sdk::vec3_t vec )
	{
		return ( vec.x( ) > -0.01f && vec.x( ) < 0.01f &&
			vec.y( ) > -0.01f && vec.y( ) < 0.01f &&
			vec.z( ) > -0.01f && vec.z( ) < 0.01f );
	}

	void c_visuals::manuals_indicators( ) {

		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) || !m_cfg->m_manuals_indication )
			return;

		static int left_side_alpha{ 255 };
		static int right_side_alpha{ 255 };
		if( g_visuals->m_cur_yaw_dir == 0 ) {
			left_side_alpha -= 20;
			right_side_alpha -= 20;
		}
		int w, h;

		valve::g_engine->get_screen_size( w, h );

		auto center = sdk::vec2_t( w / 2.f, h / 2.f );

		if( g_visuals->m_cur_yaw_dir == 1 ) {
			left_side_alpha = std::lerp( left_side_alpha, 255, 20.f * valve::g_global_vars.get( )->m_frame_time );
			right_side_alpha = std::lerp( right_side_alpha, 0, 20.f * valve::g_global_vars.get( )->m_frame_time );
		}
		else if( g_visuals->m_cur_yaw_dir == 2 ) {
			right_side_alpha = std::lerp( right_side_alpha, 255, 20.f * valve::g_global_vars.get( )->m_frame_time );
			left_side_alpha = std::lerp( left_side_alpha, 0, 20.f * valve::g_global_vars.get( )->m_frame_time );
		}

		right_side_alpha = std::clamp( right_side_alpha, 0, 255 );
		left_side_alpha = std::clamp( left_side_alpha, 0, 255 );

		if( right_side_alpha )
			g_render->text( ">", sdk::vec2_t( center.x( ) + 25, center.y( ) - 9 ), sdk::col_t( 255, 255, 255, right_side_alpha ), g_misc->m_fonts.m_esp.m_verdana, false, false, false, false, true );

		if( left_side_alpha )
			g_render->text( "<", sdk::vec2_t( center.x( ) - 30, center.y( ) - 9 ), sdk::col_t( 255, 255, 255, left_side_alpha ), g_misc->m_fonts.m_esp.m_verdana, false, false, false, false, true );
	}

	void c_visuals::oof_indicators( valve::cs_player_t* player ) {
		if( !player->weapon( ) )
			return;

		int screen_x{ }, screen_y{ };
		valve::g_engine->get_screen_size( screen_x, screen_y );

		sdk::qang_t viewangles = valve::g_engine->view_angles( );

		auto rot = sdk::to_rad( viewangles.y( ) - sdk::calc_ang( g_ctx->shoot_pos( ), player->abs_origin( ) ).y( ) - 90.f );
		auto radius = 125.f;
		auto size = 18.f;
		sdk::vec2_t center = sdk::vec2_t( screen_x, screen_y ) / 2.f;

		sdk::vec2_t pos = sdk::vec2_t( center.x( ) + radius * cosf( rot ) * ( 2 * ( 0.5f + 10 * 0.5f * 0.01f ) ), center.y( ) + radius * sinf( rot ) );
		auto line = pos - center;

		auto arrow_base = pos - ( line * ( size /( 2 * ( tanf( sdk::pi / 4 ) / 2 ) * line.length( ) ) ) );
		sdk::vec2_t normal = sdk::vec2_t( -line.y( ), line.x( ) );
		auto left = arrow_base + normal * ( size /( 2 * line.length( ) ) );
		auto right = arrow_base + normal * ( -size /( 2 * line.length( ) ) );

		auto clr = sdk::col_t( m_cfg->m_oof_clr[ 0 ] * 255.f, m_cfg->m_oof_clr[ 1 ] * 255.f, m_cfg->m_oof_clr[ 2 ] * 255.f, m_cfg->m_oof_clr[ 3 ] * m_dormant_data[ player->networkable( )->index( ) ].m_alpha );

		g_render->m_draw_list->AddTriangleFilled( ImVec2( left.x( ), left.y( ) ), ImVec2( right.x( ), right.y( ) ), 
			ImVec2( pos.x( ), pos.y( ) ),
			clr.hex( ) );
	}

	struct pred_trace_filter_t : public valve::base_trace_filter_t {
		pred_trace_filter_t( ) = default;

		bool should_hit_entity( valve::base_entity_t* entity, int ) const {
			if( !entity
				|| m_ignored_entities.empty( ) )
				return false;

			auto it = std::find( m_ignored_entities.begin( ), m_ignored_entities.end( ), entity );
			if( it != m_ignored_entities.end( ) )
				return false;

			auto client_class = entity->networkable( )->client_class( );
			if( client_class && strcmp( m_ignore_class, "" ) ) {
				if( client_class->m_network_name == m_ignore_class )
					return false;
			}

			return true;
		}

		virtual int type( ) const { return 0; }

		inline void set_class_to_ignore( const char* class_ ) { m_ignore_class = class_; }

		std::vector < valve::base_entity_t* > m_ignored_entities{ };
		const char* m_ignore_class{ };
	};

	__forceinline void trace_hull( const sdk::vec3_t& src, const sdk::vec3_t& end, valve::trace_t& trace, valve::base_entity_t* entity, std::uint32_t mask, int col_group ) {
		static const sdk::vec3_t hull[ 2 ] = { sdk::vec3_t( -2.0f, -2.0f, -2.0f ), sdk::vec3_t( 2.0f, 2.0f, 2.0f ) };

		valve::trace_filter_simple_t filter{ entity, col_group };
		valve::ray_t ray{ src, end, hull[ 0 ], hull[ 1 ] };

		valve::g_engine_trace->trace_ray( ray, mask, reinterpret_cast< valve::base_trace_filter_t* >( &filter ), &trace );
	}

	__forceinline void trace_line( const sdk::vec3_t& src, const sdk::vec3_t& end, valve::trace_t& trace, valve::base_entity_t* entity, std::uint32_t mask, int col_group ) {
		valve::trace_filter_simple_t filter{ entity, col_group };

		valve::ray_t ray{ src, end };

		valve::g_engine_trace->trace_ray( ray, mask, reinterpret_cast< valve::base_trace_filter_t* >( &filter ), &trace );
	}

	void c_visuals::draw_key_binds( ) {
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return;

		int x, y;
		valve::g_engine->get_screen_size( x, y );
		int  padding{ 0 };

		struct ind_t 
		{ 
			sdk::col_t clr { };
			std::string_view text { };
			bool has_progression_bar{ };
			float fill_bar{ };
		}; std::vector< ind_t > indicators { };

		if( g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_baim_key ) ) {
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;
			ind.text = "BAIM";

			indicators.push_back( ind );
		}

		if( g_key_binds->get_keybind_state( &g_exploits->cfg( ).m_dt_key ) ) {
			ind_t ind{ };
			ind.clr = ind.clr = sdk::col_t::lerp( sdk::col_t( 255, 0, 0 ),
				sdk::col_t( 255, 255, 255 ), g_exploits->m_ticks_allowed ).alpha( 200 );
			ind.has_progression_bar = false;

			ind.text = "DT";

			if( ind.has_progression_bar ) {
				ind.fill_bar = std::clamp( g_exploits->m_ticks_allowed, 0, 1 );

				padding += 2;
			}

			indicators.push_back( ind );
		}

		if( g_aim_bot->get_min_dmg_override_state( ) ) {
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "DMG";

			indicators.push_back( ind );
		}

		if( g_key_binds->get_keybind_state( &g_anti_aim->cfg( ).m_freestand ) ) {
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "FREESTAND";

			indicators.push_back( ind );
		}

		if( valve::g_engine->net_channel_info( ) ) {
			auto incoming_latency = valve::g_engine->net_channel_info( )->latency( 1 );
			float percent = std::clamp( ( incoming_latency * 1000.f ) / g_ping_spike->cfg( ).m_to_spike, 0.f, 1.f );

			if( g_key_binds->get_keybind_state( &g_ping_spike->cfg( ).m_ping_spike_key ) ) {
				ind_t ind{ };
				ind.clr = sdk::col_t::lerp( sdk::col_t( 255, 0, 0, 255 ), 
					sdk::col_t( 150, 200, 60, 255 ), percent );

				ind.text = "PING";
				ind.has_progression_bar = false;

				if( ind.has_progression_bar ) {
					ind.fill_bar = percent;

					padding += 2;
				}

				indicators.push_back( ind );
			}
		}

		if( g_key_binds->get_keybind_state( &g_anti_aim->cfg( ).m_fake_flick ) ) {
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "FFLICK";

			indicators.push_back( ind );		
		}	

		if( indicators.empty( ) )
			return;

		// iterate and draw indicators.
		for( int i{ }; i < indicators.size( ); ++i ) {
			auto& indicator = indicators[ i ];
			
			auto size = g_misc->m_fonts.m_esp.m_verdana->CalcTextSizeA( 18.f, FLT_MAX, NULL, indicator.text.data( ) );
			int add { };
			add = 50 + padding + ( size.y * i );

			//if( indicator.has_progression_bar ) {
			//	g_render->draw_rect_filled( 25, ( y / 2 + add + 3 ),
			//		( size.x + 2 ), 2, sdk::col_t( 0, 0, 0, 175 ), 0 );

			//	/* background above, below is the bar fill-up */

			//	g_render->draw_rect_filled( 25, ( y / 2 + add + 3 ),
			//		( ( size.x + 2 ) * indicator.fill_bar ), 2, indicator.clr, 0 );
			//}
			
			g_render->text( indicator.text, sdk::vec2_t( 26, y / 2 + add ),
				indicator.clr, g_misc->m_fonts.m_esp.m_verdana, false, false, false, false, true );
		}
	}

	std::vector < valve::base_entity_t* > broken_entities{ };

	void clear_broken_entities( ) { broken_entities.clear( ); }

	void mark_entity_as_broken( valve::base_entity_t* entity ) { broken_entities.emplace_back( entity ); }

	bool c_visuals::is_entity_broken( valve::base_entity_t* entity ) {
		return std::find( broken_entities.begin( ), broken_entities.end( ), entity ) != broken_entities.end( );
	}

	void c_visuals::handle_warning_pred( valve::base_entity_t* const entity, const valve::e_class_id class_id ) {
		if( !valve::g_engine->in_game( ) 
			|| !g_local_player->self( ) ) {
			return m_throwed_grenades.clear( );
		}
		if( class_id == valve::e_class_id::molotov_projectile || class_id == valve::e_class_id::base_cs_grenade_projectile ) {

			bool can_do = true;

			if( class_id == valve::e_class_id::base_cs_grenade_projectile ) {
				const auto studio_model = entity->renderable( )->model( );
				if( !studio_model
					|| std::string_view( studio_model->m_path ).find( "fraggrenade" ) == std::string::npos )
					can_do = false;
			}

			if( !entity->networkable( )->dormant( ) && can_do )
			{
				const auto handle = ( static_cast < valve::cs_player_t* >( entity ) )->ref_handle( );

				if( entity->effects_via_offset( ) & 0x020 )
					m_throwed_grenades.erase( handle );
				else {
					if( m_throwed_grenades.find( handle ) == m_throwed_grenades.end( ) ) {
						m_throwed_grenades.emplace( 
							std::piecewise_construct,
							std::forward_as_tuple( handle ),
							std::forward_as_tuple( 
								static_cast< valve::cs_player_t* >( valve::g_entity_list->get_entity( entity->thrower_handle( ) ) ),
								class_id == valve::e_class_id::molotov_projectile ? valve::e_item_index::molotov : valve::e_item_index::he_grenade,
								entity->origin( ), entity->velocity( ),
								entity->get_ent_spawn_time( ), valve::to_ticks( entity->sim_time( ) - entity->get_ent_spawn_time( ) )
							 )
						 );
					}

					if( !add_grenade_simulation( m_throwed_grenades.at( handle ), true ) )
						m_throwed_grenades.erase( handle );
				}
			}
		}
	}

	void c_visuals::grenade_simulation_t::predict( const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset ) {
		m_origin = origin;
		m_velocity = velocity;
		m_collision_group = 13;
		clear_broken_entities( );

		const auto tick = valve::to_ticks( 1.f / 30.f );

		m_last_update_tick = -tick;
		static auto molotov_throw_detonate_time = valve::g_cvar->find_var( xor_str( "molotov_throw_detonate_time" ) );
		switch( m_index ) {
		case valve::e_item_index::smoke_grenade: m_next_think_tick = valve::to_ticks( 1.5f ); break;
		case valve::e_item_index::decoy: m_next_think_tick = valve::to_ticks( 2.f ); break;
		case valve::e_item_index::flashbang:
		case valve::e_item_index::he_grenade:
			m_detonate_time = 1.5f;
			m_next_think_tick = valve::to_ticks( 0.02f );

			break;
		case valve::e_item_index::molotov:
		case valve::e_item_index::inc_grenade:
			m_detonate_time = molotov_throw_detonate_time->get_float( );
			m_next_think_tick = valve::to_ticks( 0.02f );

			break;
		default: break;
		}

		m_source_time = throw_time;

		const auto max_sim_amt = valve::to_ticks( 60.f );
		for( ; m_tick < max_sim_amt; ++m_tick ) {
			if( m_next_think_tick <= m_tick )
				think( );

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

		m_expire_time = throw_time + valve::to_time( m_tick );
	}

	bool c_visuals::grenade_simulation_t::physics_simulate( ) {
		if( m_detonated )
			return true;
		static auto sv_gravity = valve::g_cvar->find_var( xor_str( "sv_gravity" ) );
		const auto new_velocity_z = m_velocity.z( ) - ( sv_gravity->get_float( ) * 0.4f ) * valve::g_global_vars.get( )->m_interval_per_tick;

		const auto move = sdk::vec3_t( 
			m_velocity.x( ) * valve::g_global_vars.get( )->m_interval_per_tick,
			m_velocity.y( ) * valve::g_global_vars.get( )->m_interval_per_tick,
			( m_velocity.z( ) + new_velocity_z ) / 2.f * valve::g_global_vars.get( )->m_interval_per_tick
		 );

		m_velocity.z( ) = new_velocity_z;

		valve::trace_t trace{ };

		physics_push_entity( move, trace );

		if( m_detonated )
			return true;

		if( trace.m_frac != 1.f ) {
			update_path( true );

			perform_fly_collision_resolution( trace );
		}

		return false;
	}

	void c_visuals::grenade_simulation_t::physics_trace_entity( 
		const sdk::vec3_t& src, const sdk::vec3_t & dst,
		const std::uint32_t mask, valve::trace_t& trace
	 ) {
		valve::trace_filter_skip_two_entities_t trace_filter{ m_owner, m_last_breakable, m_collision_group };

		trace_hull( src, dst, trace, g_local_player->self( ), 0x200400B, m_collision_group );

		if( trace.m_start_solid
			&&( trace.m_contents & CONTENTS_CURRENT_90 ) ) {
			trace.clear( );

			trace_hull( src, dst, trace, g_local_player->self( ), mask & ~CONTENTS_CURRENT_90, m_collision_group );
		}

		if( !trace.hit( )
			|| !trace.m_entity
			|| !trace.m_entity->is_player( ) )
			return;

		trace.clear( );

		trace_line( src, dst, trace, g_local_player->self( ), mask, m_collision_group );
	}

	void c_visuals::grenade_simulation_t::physics_push_entity( const sdk::vec3_t& push, valve::trace_t& trace ) {
		physics_trace_entity( m_origin, m_origin + push,
			m_collision_group == 1
			?( _MASK_SOLID | _CONTENTS_CURRENT_90 ) & ~_CONTENTS_MONSTER
			: _MASK_SOLID | _CONTENTS_OPAQUE | _CONTENTS_IGNORE_NODRAW_OPAQUE | _CONTENTS_CURRENT_90 | _CONTENTS_HITBOX,
			trace
		 );

		valve::trace_filter_skip_two_entities_t trace_filter{ m_owner, m_last_breakable, m_collision_group };

		if( trace.m_start_solid ) {
			m_collision_group = 3;
			trace_line( m_origin - push, m_origin + push, trace, g_local_player->self( ),( MASK_SOLID | CONTENTS_CURRENT_90 ) & ~CONTENTS_MONSTER, m_collision_group );
		}

		if( trace.m_frac != 0.f )
			m_origin = trace.m_end;

		static auto weapon_molotov_maxdetonateslope = valve::g_cvar->find_var( xor_str( "weapon_molotov_maxdetonateslope" ) );

		if( !trace.m_entity )
			return;

		if( trace.m_entity->is_player( )
			|| ( m_index != valve::e_item_index::molotov && m_index != valve::e_item_index::inc_grenade )
			|| trace.m_plane.m_normal.z( ) < std::cos( sdk::to_rad( weapon_molotov_maxdetonateslope->get_float( ) ) ) )
			return;

		detonate( true );
	}

	bool c_visuals::add_grenade_simulation( const grenade_simulation_t& sim, const bool warning ) const {

		const auto points_count = sim.m_path.size( );
		if( points_count < 2u
			|| valve::g_global_vars.get( )->m_cur_time >= sim.m_expire_time )
			return false;

		const auto mod = std::clamp( 
			( sim.m_expire_time - valve::g_global_vars.get( )->m_cur_time )
			/ valve::to_time( sim.m_tick ),
			0.f, 1.f
		 );

		const auto& screen_size = ImGui::GetIO( ).DisplaySize;
		if( warning ) {
			const auto& explode_pos = sim.m_path.back( ).first;
			auto dist = ( g_local_player->self( )->origin( ) - explode_pos ).length( );

			if( dist < 1000.f ) {
				sdk::vec3_t prev_screen_pos{ };
				auto prev_on_screen = g_render->world_to_screen( sim.m_path.front( ).first, prev_screen_pos
				 );

				for( auto i = 1u; i < points_count; ++i ) {
					sdk::vec3_t cur_screen_pos{ };
					const auto cur_on_screen = g_render->world_to_screen( sim.m_path.at( i ).first, cur_screen_pos
					 );
					if( prev_on_screen
						&& cur_on_screen ) {
						g_render->line( sdk::vec2_t( prev_screen_pos.x( ), prev_screen_pos.y( ) ), sdk::vec2_t( cur_screen_pos.x( ), cur_screen_pos.y( ) ), sdk::col_t( 255, 255, 255, 255 * mod ) );

					}
					prev_screen_pos = cur_screen_pos;
					prev_on_screen = cur_on_screen;
				}

				sdk::vec3_t screen_pos{ };
				const auto on_screen = g_render->world_to_screen( explode_pos, screen_pos );
				if( !on_screen )
					return true;

				const auto unk = sdk::vec2_t( screen_size.x / 18.f, screen_size.y / 18.f );
				if( !on_screen
					|| screen_pos.x( ) < -unk.x( )
					|| screen_pos.x( ) >( screen_size.x + unk.x( ) )
					|| screen_pos.y( ) < -unk.y( )
					|| screen_pos.y( ) >( screen_size.y + unk.y( ) ) ) {
					sdk::vec3_t dir{ };
					sdk::ang_vecs( valve::g_view_render->m_setup.m_angles, &dir, nullptr, nullptr );

					dir.z( ) = 0.f;
					dir.normalize( );

					const auto radius = 210.f * ( screen_size.y / 480.f );

					auto delta = explode_pos - valve::g_view_render->m_setup.m_origin;

					delta.normalize( );

					screen_pos.x( ) = radius * -delta.dot( sdk::vec3_t{0.f, 0.f, 1.f}.cross( dir ) );
					screen_pos.y( ) = radius * -delta.dot( dir );

					const auto radians = sdk::to_rad( 
						-sdk::to_deg( std::atan2( screen_pos.x( ), screen_pos.y( ) + 3.141592653589793 ) )
					 );

					screen_pos.x( ) = static_cast< int >( screen_size.x / 2.f + radius * std::sin( radians ) );
					screen_pos.y( ) = static_cast< int >( screen_size.y / 2.f - radius * std::cos( radians ) );
				}

				g_render->m_draw_list->AddCircleFilled( ImVec2( screen_pos.x( ), screen_pos.y( ) ), 23.f, ImColor( 0.03f, 0.03f, 0.03f, 1.f * mod ), 255.f );
				g_render->m_draw_list->PathArcTo( ImVec2( screen_pos.x( ), screen_pos.y( ) ), 20.f, 0.f, mod * 2.f * sdk::pi, 32 );
				g_render->m_draw_list->PathStroke( ImColor( 1.f, 1.f, 1.f, 1.f * mod ), false, 3.f );

				std::string icon = "";
				switch( sim.m_index )
				{
				case valve::e_item_index::he_grenade: icon = xor_str( "j" ); break;
				case valve::e_item_index::smoke_grenade: icon = xor_str( "k" ); break;
				case valve::e_item_index::flashbang: icon = xor_str( "i" ); break;
				case valve::e_item_index::decoy: icon = xor_str( "m" ); break;
				case valve::e_item_index::inc_grenade: icon = xor_str( "n" ); break;
				case valve::e_item_index::molotov: icon = xor_str( "l" ); break;
				default: break;
				}

				g_render->text( icon, sdk::vec2_t( screen_pos.x( ) + 1, screen_pos.y( ) ), sdk::col_t( 255, 255, 255, 255 * mod ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
				return true;
			}
		}

		sdk::vec3_t prev_screen_pos{ };
		auto prev_on_screen = g_render->world_to_screen( sim.m_path.front( ).first, prev_screen_pos
		 );

		for( auto i = 1u; i < points_count; ++i ) {
			sdk::vec3_t cur_screen_pos{ };
			const auto cur_on_screen = g_render->world_to_screen( sim.m_path.at( i ).first, cur_screen_pos
			 );

			if( prev_on_screen
				&& cur_on_screen ) {
				g_render->line( sdk::vec2_t( prev_screen_pos.x( ), prev_screen_pos.y( ) ), sdk::vec2_t( cur_screen_pos.x( ), cur_screen_pos.y( ) ), sdk::col_t( 255, 255, 255, 255 ) );
			}

			prev_screen_pos = cur_screen_pos;
			prev_on_screen = cur_on_screen;
		}

		return true;
	}

	void c_visuals::on_create_move( const valve::user_cmd_t& cmd ) {
		m_grenade_trajectory = { };

		if( !g_local_player->weapon( )
			|| !g_local_player->weapon_info( ) )
			return;

		sdk::vec3_t dir{ };

		if( 0
			&& g_local_player->weapon_info( )->m_type != static_cast < valve::e_weapon_type >( 0 )
			&& g_local_player->weapon_info( )->m_type < static_cast < valve::e_weapon_type >( 7 ) ) {
			sdk::ang_vecs( cmd.m_view_angles, &dir, nullptr, nullptr );

		}

		if( g_local_player->weapon_info( )->m_type != static_cast < valve::e_weapon_type >( 9 )
			|| ( !g_local_player->weapon( )->pin_pulled( ) && g_local_player->weapon( )->throw_time( ) == 0.f ) )
			return;

		m_grenade_trajectory.m_owner = g_local_player->self( );
		m_grenade_trajectory.m_index = g_local_player->weapon( )->item_index( );

		auto view_angles = cmd.m_view_angles;

		if( view_angles.x( ) < -90.f )
			view_angles.x( ) += 360.f;
		else if( view_angles.x( ) > 90.f )
			view_angles.x( ) -= 360.f;

		view_angles.x( ) -= ( 90.f - std::fabsf( view_angles.x( ) ) ) * 10.f / 90.f;

		sdk::ang_vecs( view_angles, &dir, nullptr, nullptr );

		const auto throw_strength = std::clamp( g_local_player->weapon( )->throw_strength( ), 0.f, 1.f );

		auto src = g_ctx->shoot_pos( );

		src.z( ) += throw_strength * 12.f - 12.f;

		valve::trace_t trace{ };
		valve::trace_filter_simple_t trace_filter{ g_local_player->self( ), 0 };
		valve::g_engine_trace->trace_ray( 
			{ src, src + dir * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f } },
			( MASK_SOLID | CONTENTS_CURRENT_90 ),
			reinterpret_cast< valve::base_trace_filter_t* >( &trace_filter ), &trace
		 );

		const auto velocity = std::clamp( 
			g_local_player->weapon_info( )->m_throw_velocity * 0.9f, 15.f, 750.f
		 ) * ( throw_strength * 0.7f + 0.3f );

		m_grenade_trajectory.predict( 
			trace.m_end - dir * 6.f,
			dir * velocity + g_local_player->self( )->velocity( ) * 1.25f,
			valve::g_global_vars.get( )->m_cur_time, 0
		 );
	}

	void c_visuals::grenade_simulation_t::perform_fly_collision_resolution( valve::trace_t& trace ) {
		auto surface_elasticity = 1.f;

		if( trace.m_entity ) {
			if( g_auto_wall->is_breakable( trace.m_entity ) ) {
				m_last_breakable = trace.m_entity;

				mark_entity_as_broken( trace.m_entity );

				m_velocity *= 0.4f;

				return;
			}

			const auto is_player = trace.m_entity->is_player( );
			if( is_player )
				surface_elasticity = 0.3f;

			if( trace.m_entity->networkable( )->index( ) ) {
				if( is_player
					&& m_last_hit_entity == trace.m_entity ) {
					m_collision_group = 1;

					return;
				}

				m_last_hit_entity = trace.m_entity;
			}
		}

		sdk::vec3_t velocity{ };

		const auto back_off = m_velocity.dot( trace.m_plane.m_normal ) * 2.f;

		for( std::size_t i{ }; i < 3u; ++i ) {
			const auto change = trace.m_plane.m_normal.at( i ) * back_off;

			velocity.at( i ) = m_velocity.at( i ) - change;

			if( std::fabsf( velocity.at( i ) ) >= 1.f )
				continue;

			velocity.at( i ) = 0.f;
		}

		velocity *= std::clamp( surface_elasticity * 0.45f, 0.f, 0.9f );

		if( trace.m_plane.m_normal.z( ) > 0.7f ) {
			const auto speed_sqr = velocity.length_sqr( 3u );
			if( speed_sqr > 96000.f ) {
				const auto l = velocity.normalized( ).dot( trace.m_plane.m_normal );
				if( l > 0.5f )
					velocity *= 1.f - l + 0.5f;
			}

			if( speed_sqr < 400.f )
				m_velocity = { };
			else {
				m_velocity = velocity;

				physics_push_entity( velocity * ( ( 1.f - trace.m_frac ) * valve::g_global_vars.get( )->m_interval_per_tick ), trace );
			}
		}
		else {
			m_velocity = velocity;

			physics_push_entity( velocity * ( ( 1.f - trace.m_frac ) * valve::g_global_vars.get( )->m_interval_per_tick ), trace );
		}

		if( m_bounces_count > 20 )
			return detonate( false );

		++m_bounces_count;
	}

	void c_visuals::grenade_simulation_t::think( ) {
		switch( m_index ) {
		case valve::e_item_index::smoke_grenade:
			if( m_velocity.length_sqr( ) <= 0.01f )
				detonate( false );

			break;
		case valve::e_item_index::decoy:
			if( m_velocity.length_sqr( ) <= 0.04f )
				detonate( false );

			break;
		case valve::e_item_index::flashbang:
		case valve::e_item_index::he_grenade:
		case valve::e_item_index::molotov:
		case valve::e_item_index::inc_grenade:
			if( valve::to_time( m_tick ) > m_detonate_time )
				detonate( false );

			break;
		default: break;
		}

		m_next_think_tick = m_tick + valve::to_ticks( 0.2f );
	}

	void c_visuals::grenade_simulation_t::detonate( const bool bounced ) {
		m_detonated = true;

		update_path( bounced );
	}

	void c_visuals::grenade_simulation_t::update_path( const bool bounced ) {
		m_last_update_tick = m_tick;

		if( m_path.size( ) < 200 ) {
			m_path.emplace_back( m_origin, bounced );
		}
	}

	void c_visuals::draw_auto_peek( float alpha )
	{
		if( !g_local_player->self( ) ||
			!g_local_player->self( )->alive( ) )
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

		if( alpha < 0.01f )
			return;

		auto pos = sdk::vec3_t( );

		if( !is_zero_vec3_t( g_ctx->get_auto_peek_info( ).m_start_pos ) )
			pos = g_ctx->get_auto_peek_info( ).m_start_pos;

		if( is_zero_vec3_t( pos ) )
			return;

		float step = sdk::pi * 2.0f / 60;
		std::vector<ImVec2> points;
		for( float lat = 0.f; lat <= sdk::pi * 2.0f; lat += step )
		{
			const auto& point3d = sdk::vec3_t( sin( lat ), cos( lat ), 0.f ) * 15.f;
			sdk::vec3_t point2d;
			if( g_render->world_to_screen( pos + point3d, point2d ) )
				points.push_back( ImVec2( point2d.x( ), point2d.y( ) ) );
		}
		auto flags_backup = g_render->m_draw_list->Flags;

		g_render->m_draw_list->Flags |= ImDrawListFlags_AntiAliasedFill;
		g_render->m_draw_list->AddConvexPolyFilled( points.data( ), points.size( ), ImColor( 0.5f, 0.5f, 0.5f, 0.5f * alpha ) );
		g_render->m_draw_list->AddPolyline( points.data( ), points.size( ), ImColor( 0.5f, 0.5f, 0.5f, 0.9f * alpha ), true, 2.f );
		g_render->m_draw_list->Flags = flags_backup;
	}

	void c_dormant_esp::start( )
	{
		m_utlCurSoundList.remove_all( );
		valve::g_engine_sound->get_act_sounds( m_utlCurSoundList );

		if( !m_utlCurSoundList.m_size )
			return;

		for( auto i = 0; i < m_utlCurSoundList.m_size; i++ )
		{
			auto& sound = m_utlCurSoundList.at( i );

			if( sound.m_nSoundSource < 1 || sound.m_nSoundSource > 64 )
				continue;

			if( sound.m_pOrigin->x( ) == 0.f && sound.m_pOrigin->y( ) == 0.f && sound.m_pOrigin->z( ) == 0.f )
				continue;

			if( !valid_sound( sound ) )
				continue;

			auto player = static_cast< valve::cs_player_t* >( valve::g_entity_list->get_entity( sound.m_nSoundSource ) );


			if( !player || !player->alive( ) || player->friendly( g_local_player->self( ) ) ||
				player == g_local_player->self( ) )
				continue;

			setup_adjust( player, sound );
			m_sound_players[ sound.m_nSoundSource ].Override( sound );
		}

		m_utlvecSoundBuffer = m_utlCurSoundList;
	}

	void c_dormant_esp::setup_adjust( valve::cs_player_t* player, valve::snd_info_t& sound )
	{
		sdk::vec3_t src3D, dst3D;
		valve::trace_t tr;
		valve::trace_filter_simple_t filter;

		src3D = *sound.m_pOrigin + sdk::vec3_t( 0.0f, 0.0f, 1.0f );
		dst3D = src3D - sdk::vec3_t( 0.0f, 0.0f, 100.0f );

		filter.m_ignore_entity = player;
		valve::ray_t ray = { src3D, dst3D };

		valve::g_engine_trace->trace_ray( ray, MASK_PLAYERSOLID, reinterpret_cast < valve::base_trace_filter_t* >( &filter ), &tr );

		if( tr.m_all_solid )
			m_sound_players[ sound.m_nSoundSource ].m_iReceiveTime = -1;

		*sound.m_pOrigin = tr.m_frac <= 0.97f ? tr.m_end : *sound.m_pOrigin;
		m_sound_players[ sound.m_nSoundSource ].m_nFlags = static_cast < int >( player->flags( ) );
		m_sound_players[ sound.m_nSoundSource ].m_nFlags |= ( tr.m_frac < 0.50f ? valve::e_ent_flags::ducking : static_cast < valve::e_ent_flags >( 0 ) ) |( tr.m_frac < 1.0f ? valve::e_ent_flags::on_ground : static_cast < valve::e_ent_flags >( 0 ) );
		m_sound_players[ sound.m_nSoundSource ].m_nFlags &= ( tr.m_frac >= 0.50f ? ~valve::e_ent_flags::ducking : 0 ) |( tr.m_frac >= 1.0f ? ~valve::e_ent_flags::on_ground : 0 );
	}

	bool c_dormant_esp::adjust_sound( valve::cs_player_t* entity )
	{
		auto i = entity->networkable( )->index( );
		auto sound_player = m_sound_players[ i ];

		auto expired = false;

		if( fabs( valve::g_global_vars.get( )->m_real_time - sound_player.m_iReceiveTime ) > 10.0f )
			expired = true;

		//entity->spotted( ) = true;
		entity->flags( ) = ( valve::e_ent_flags ) sound_player.m_nFlags;
		entity->set_abs_origin( sound_player.m_vecOrigin );

		return !expired;
	}

	bool c_dormant_esp::valid_sound( valve::snd_info_t& sound )
	{
		for( auto i = 0; i < m_utlvecSoundBuffer.m_size; i++ )
			if( m_utlvecSoundBuffer.at( i ).m_nGuid == sound.m_nGuid )
				return false;

		return true;
	}

	void c_visuals::shared_t::send_net_data( valve::cs_player_t* const player ) {
		valve::player_info_t info{ };

		const bool exists = valve::g_engine->get_player_info( player->networkable( )->index( ), &info );

		if( !exists )
			return;

		if( info.m_fake_player )
			return;

		valve::cclc_msg_data_legacy_t client_msg{ };

		memset( &client_msg, 0, sizeof( client_msg ) );

		const auto func = ( std::uint32_t( __fastcall* )( void*, void* ) )g_ctx->addresses( ).m_voice_msg_ctor;

		func( ( void* ) &client_msg, nullptr );

		cheat_data_t* ptr = ( cheat_data_t* ) &client_msg.m_xuid_low;

		ptr->m_unique_key = 228;
		ptr->m_player_idx = static_cast < std::uint8_t >( player->networkable( )->index( ) );
		ptr->m_x = static_cast < std::int16_t >( player->origin( ).x( ) );
		ptr->m_y = static_cast < std::int16_t >( player->origin( ).y( ) );
		ptr->m_z = static_cast < std::int16_t >( player->origin( ).z( ) );

		client_msg.m_flags = 63;
		client_msg.m_format = 0;

		valve::g_client_state.get( )->m_net_chan->send_net_msg( &client_msg );
	}

	void c_visuals::handle_player_drawings( ) {
		g_dormant_esp->start( );
		for( int i = 1; i < valve::g_global_vars.get( )->m_max_clients; ++i )
		{
			auto player = ( valve::cs_player_t* )valve::g_entity_list->get_entity( i );

			if( !g_local_player || !g_local_player->self( )
				|| !player
				|| player == g_local_player->self( )
				|| player->team( ) == g_local_player->self( )->team( ) )
				continue;

			bool alive_check{ false };

			if( !player->alive( ) ) {
				m_dormant_data[ player->networkable( )->index( ) ].m_alpha = std::lerp( m_dormant_data[ player->networkable( )->index( ) ].m_alpha, 0.f, 8.f * valve::g_global_vars.get( )->m_frame_time );
				alive_check = true;
			}

			m_dormant_data[ player->networkable( )->index( ) ].m_alpha = std::clamp( m_dormant_data[ player->networkable( )->index( ) ].m_alpha, 0.f, 255.f );

			if( !m_dormant_data[ player->networkable( )->index( ) ].m_alpha
				&& alive_check )
				continue;

			if( !alive_check ) {
				if( player->networkable( )->dormant( ) ) {
					if( !m_dormant_data.at( player->networkable( )->index( ) ).m_use_shared 
						&& valve::g_global_vars.get( )->m_real_time - m_dormant_data.at( player->networkable( )->index( ) ).m_last_shared_time > 4.f )
						g_dormant_esp->adjust_sound( player );

					if( !m_dormant_data.at( player->networkable( )->index( ) ).m_use_shared 
						&& valve::g_global_vars.get( )->m_real_time - m_dormant_data.at( player->networkable( )->index( ) ).m_last_shared_time > 4.f ) {
						if( m_dormant_data[ player->networkable( )->index( ) ].m_alpha < 160.f )
						{
							m_dormant_data[ player->networkable( )->index( ) ].m_alpha -= 255.f / 4.f * valve::g_global_vars.get( )->m_frame_time;
							m_dormant_data[ player->networkable( )->index( ) ].m_alpha = std::clamp( m_dormant_data[ player->networkable( )->index( ) ].m_alpha, 0.f, 160.f );
						}
						else
						{
							m_dormant_data[ player->networkable( )->index( ) ].m_alpha -= 255.f / 1.f * valve::g_global_vars.get( )->m_frame_time;
						}
					}
					else {
						m_dormant_data[ player->networkable( )->index( ) ].m_alpha = 190.f;
					}

					if( player->weapon( ) ) {
						if( m_dormant_data.at( i ).m_weapon_id > 0 )
							player->weapon( )->item_index( ) = static_cast < valve::e_item_index >( m_dormant_data.at( i ).m_weapon_id );

						if( player->weapon( )->info( ) )
							if( m_dormant_data.at( i ).m_weapon_type > -1 )
								player->weapon( )->info( )->m_type = static_cast < valve::e_weapon_type >( m_dormant_data.at( i ).m_weapon_type );
					}
				}
				else {
					g_dormant_esp->m_sound_players[ i ].reset( true, player->abs_origin( ), static_cast < int >( player->flags( ) ) );
					m_dormant_data[ i ].m_origin = sdk::vec3_t( );
					m_dormant_data[ i ].m_receive_time = 0.f;
					m_dormant_data[ i ].m_alpha += 255.f / 0.68f * valve::g_global_vars.get( )->m_frame_time;
					m_dormant_data[ i ].m_alpha = std::clamp( m_dormant_data[ i ].m_alpha, 0.f, 255.f );
					m_dormant_data[ i ].m_weapon_id = 0;
					m_dormant_data[ i ].m_weapon_type = -1;
					m_dormant_data.at( i ).m_use_shared = false;
					m_dormant_data.at( player->networkable( )->index( ) ).m_last_shared_time = 0.f;
				}
			}
			int screen_x, screen_y;

			valve::g_engine->get_screen_size( screen_x, screen_y );

			sdk::vec3_t screen = sdk::vec3_t( );

			if( m_cfg->m_oof_indicator ) {
				if( !csgo::g_render->world_to_screen( player->abs_origin( ), screen ) )
				{
					oof_indicators( player );
					continue;
				}

				if( screen.x( ) < 0 || screen.x( ) > screen_x || screen.y( ) < 0 || screen.y( ) > screen_y )
				{
					oof_indicators( player );
					continue;
				}
			}

			RECT rect = get_bbox( player );

			draw_name( player, rect );
			draw_health( player, rect );
			draw_box( player, rect );
			draw_wpn( player, rect );
			draw_ammo( player, rect );
			draw_lby_upd( player, rect );
			draw_flags( player, rect );
		}
	}

	void c_visuals::draw_wpn( valve::cs_player_t* player, RECT& rect ) {
		if( !m_cfg->m_wpn_icon && !m_cfg->m_wpn_text )
			return;

		if( !player->weapon( ) || !player->weapon( )->info( ) )
			return;

		int offset { -2 };

		bool has_something{ };

		if( m_cfg->m_wpn_ammo 
			&& m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) )
			offset += 6;

		if( m_cfg->m_draw_lby 
			&& m_change_offset_due_to_lby.at( player->networkable( )->index( ) ) )
			offset += 6;

		has_something = offset != -2;

		if( !has_something )
			offset = -1;

		if( m_cfg->m_wpn_text ) {
			g_render->text( get_weapon_name( player->weapon( ) ), sdk::vec2_t( rect.left + ( abs( rect.right - rect.left ) * 0.5f ), rect.bottom + offset + 3 ), sdk::col_t( 255, 255, 255,( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha ), g_misc->m_fonts.m_esp.m_04b, true, true, false, true, false );

			if( has_something )
				offset += 10;
			else
				offset += 7;
		}

		if( m_cfg->m_wpn_icon )
			g_render->text( get_weapon_icon( player->weapon( ) ), sdk::vec2_t( rect.left + ( abs( rect.right - rect.left ) * 0.5f ), rect.bottom + offset + 3 ), sdk::col_t( 255, 255, 255,( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha ), g_misc->m_fonts.m_icon_font, false, true, false, false, true );
	}

	void c_visuals::handle_world_drawings( ) {

		int x, y;
		valve::g_engine->get_screen_size( x, y );
		const sdk::vec2_t center{ x / 2.f, y / 2.f };

		const std::ptrdiff_t ents = valve::g_entity_list->highest_ent_index( );

		for( std::ptrdiff_t i { }; i <= ents; ++i ) {
			const auto entity = valve::g_entity_list->get_entity( i );

			if( !entity
				|| entity->networkable( )->dormant( )
				|| entity->is_player( )
				|| !entity->networkable( )->client_class( ) )
				continue;

			if( entity->is_weapon( ) ) {
				auto model = entity->renderable( )->model( );
				if( model ) {
					auto weapon = ( valve::cs_weapon_t* ) entity;
					sdk::vec3_t screen{ };

					const auto& origin = weapon->origin( );
					if( origin.x( ) == 0.f
						&& origin.y( ) == 0.f
						&& origin.z( ) == 0.f )
						continue;

					auto on_screen = g_render->world_to_screen( origin, screen );

					if( !on_screen )
						continue;

					auto offset = 0;

					if( m_cfg->m_proj_icon ) {
						g_render->text( get_weapon_icon( weapon ), sdk::vec2_t( screen.x( ), screen.y( ) ),
							sdk::col_t( 255, 255, 255, 255 ), g_misc->m_fonts.m_icon_font, false, true, false, false, true );
						offset += 20;
					}

					if( m_cfg->m_proj_wpn )
						g_render->text( get_weapon_name( weapon ), sdk::vec2_t( screen.x( ), screen.y( ) + offset ),
							sdk::col_t( 255, 255, 255, 255 ), g_misc->m_fonts.m_esp.m_04b, true, true, false, true, false );
				}
			}

			switch( entity->networkable( )->client_class( )->m_class_id ) {
			case valve::e_class_id::inferno:
				molotov_timer( entity );
				break;

			case valve::e_class_id::smoke_grenade_projectile:
				smoke_timer( entity );
				break;
			case valve::e_class_id::cascade_light:
				change_shadows( entity );
				break;

			case valve::e_class_id::tone_map_controller:
				tone_map_modulation( entity );
				break;

			default:
				grenade_projectiles( entity );
				break;
			}
		}
	}

	void c_visuals::molotov_timer( valve::base_entity_t* entity ) {
		if( !m_cfg->m_molotov_timer )
			return;

		auto inferno = reinterpret_cast< valve::inferno_t* >( entity );
		auto origin = inferno->abs_origin( );

		sdk::vec3_t screen_origin;

		if( !g_render->world_to_screen( origin, screen_origin ) )
			return;

		if( ( inferno->origin( ) - g_local_player->self( )->origin( ) ).length( ) > 450.f )
			return;

		auto spawn_time = inferno->get_spawn_time( );
		auto factor = ( spawn_time + valve::inferno_t::get_expiry_time( ) - valve::g_global_vars.get( )->m_cur_time ) / valve::inferno_t::get_expiry_time( );

		const auto mod = std::clamp( 
			factor,
			0.f, 1.f
		 );

		g_render->m_draw_list->AddCircleFilled( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 23.f, ImColor( 0.05f, 0.05f, 0.05f, 1.f * mod ), 255.f );
		g_render->m_draw_list->PathArcTo( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 20.f, 0.f, factor * 2.f * sdk::pi, 32 );
		g_render->m_draw_list->PathStroke( ImColor( 1.f, 0.f, 0.f, 1.f * mod ), false, 3.f );

		g_render->text( xor_str( "l" ), sdk::vec2_t( screen_origin.x( ) + 1, screen_origin.y( ) ),
			sdk::col_t( 255, 255, 255, 255 * mod ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
	}

	void c_visuals::smoke_timer( valve::base_entity_t* entity ) {
		if( !m_cfg->m_smoke_timer )
			return;

		auto smoke = reinterpret_cast< valve::smoke_t* >( entity );

		if( !smoke->smoke_effect_tick_begin( ) || !smoke->did_smoke_effect( ) )
			return;

		auto origin = smoke->abs_origin( );

		if( ( smoke->origin( ) - g_local_player->self( )->origin( ) ).length( ) > 450.f )
			return;

		sdk::vec3_t screen_origin;

		if( !g_render->world_to_screen( origin, screen_origin ) )
			return;

		auto spawn_time = valve::to_time( smoke->smoke_effect_tick_begin( ) );
		auto factor = ( spawn_time + valve::smoke_t::get_expiry_time( ) - valve::g_global_vars.get( )->m_cur_time ) / 
			valve::smoke_t::get_expiry_time( );

		const auto mod = std::clamp( 
			factor,
			0.f, 1.f
		 );

		g_render->m_draw_list->AddCircleFilled( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 23.f, ImColor( 0.05f, 0.05f, 0.05f, 1.f * mod ), 255.f );
		g_render->m_draw_list->PathArcTo( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 20.f, 0.f, factor * 2.f * sdk::pi, 32 );
		g_render->m_draw_list->PathStroke( ImColor( 0.41f, 0.54f, 1.f, 1.f * mod ), false, 3.f );

		g_render->text( xor_str( "k" ), sdk::vec2_t( screen_origin.x( ) + 1, screen_origin.y( ) ),
			sdk::col_t( 255, 255, 255, 255 * mod ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
	}

	void c_visuals::grenade_projectiles( valve::base_entity_t* entity ) {
		if( !m_cfg->m_grenade_projectiles )
		return;

		auto client_class = entity->networkable( )->client_class( );

		auto model = entity->renderable( )->model( );

		if( !model )
			return;

		if( client_class->m_class_id == valve::e_class_id::base_cs_grenade_projectile 
			|| client_class->m_class_id == valve::e_class_id::molotov_projectile )
		{
			auto name = ( std::string_view ) model->m_path;
			auto grenade_origin = entity->abs_origin( );
			auto grenade_position = sdk::vec3_t( );

			if( !g_render->world_to_screen( grenade_origin, grenade_position ) )
				return;

			std::string grenade_name, grenade_icon;

			if( name.find( "flashbang" ) != std::string::npos )
			{
				grenade_name = "FLASHBANG";
				grenade_icon = "i";
			}
			else if( name.find( "smokegrenade" ) != std::string::npos )
			{
				grenade_name = "SMOKE";
				grenade_icon = "k";
			}
			else if( name.find( "incendiarygrenade" ) != std::string::npos )
			{
				grenade_name = "INCENDIARY";
				grenade_icon = "n";
			}
			else if( name.find( "molotov" ) != std::string::npos )
			{
				grenade_name = "MOLOTOV";
				grenade_icon = "l";
			}
			else if( name.find( "fraggrenade" ) != std::string::npos )
			{
				grenade_name = "HE GRENADE";
				grenade_icon = "j";
			}
			else
				return;

			g_render->text( grenade_name.c_str( ), sdk::vec2_t( grenade_position.x( ), grenade_position.y( ) ),
				sdk::col_t( 255, 255, 255, 255 ), g_misc->m_fonts.m_esp.m_04b, true, true, false );
		}
	}

	void c_visuals::draw_ammo( valve::cs_player_t* player, RECT& rect ) {
		auto wpn = player->weapon( );

		if( !wpn )
			return;

		auto wpn_data = wpn->info( );

		if( !wpn_data )
			return;

		m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) = false;

		if( wpn->clip1( ) <= -1 )
			wpn->clip1( ) = 0;

		if( m_cfg->m_wpn_ammo
			&& wpn_data
			&& wpn_data->m_type != valve::e_weapon_type::knife
			&& wpn_data->m_type < valve::e_weapon_type::c4
			&& wpn_data->m_max_clip1 != -1 ) {

			m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) = true;

			float box_width = std::abs( rect.right - rect.left );
			float current_box_width = ( box_width * wpn->clip1( ) / wpn_data->m_max_clip1 );

			if( player->lookup_seq_act( player->anim_layers( ).at( 1 ).m_seq ) == 967 )
				current_box_width = box_width * player->anim_layers( ).at( 1 ).m_cycle;

			auto clr = sdk::col_t( m_cfg->m_wpn_ammo_clr[ 0 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 1 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 2 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 3 ] * m_dormant_data [ player->networkable( )->index( ) ].m_alpha );

			auto magic_clr = player->networkable( )->dormant( ) ? sdk::col_t( 0.f, 0.f, 0.f, m_dormant_data.at( player->networkable( )->index( ) ).m_alpha ) : sdk::col_t( 0.0f, 0.0f, 0.0f, 100.0f );

			g_render->rect_filled( sdk::vec2_t( rect.right + 1, rect.bottom + 2 ), sdk::vec2_t( rect.left - 1, rect.bottom + 6 ), magic_clr );
			g_render->rect_filled( sdk::vec2_t( rect.left, rect.bottom + 3 ), sdk::vec2_t( rect.left + current_box_width, rect.bottom + 5 ), clr );

			// less than 90% ammo
			if( wpn->clip1( ) <( wpn_data->m_max_clip1 * 0.9 ) )
				g_render->text( std::to_string( wpn->clip1( ) ), sdk::vec2_t( rect.left + current_box_width, rect.bottom ), sdk::col_t( 255, 255, 255, m_dormant_data.at( player->networkable( )->index( ) ).m_alpha ), g_misc->m_fonts.m_esp.m_04b, true, false, false );
		}
	}

	void c_visuals::draw_lby_upd( valve::cs_player_t* player, RECT& rect ) {
		m_change_offset_due_to_lby.at( player->networkable( )->index( ) ) = false;

		if( !m_cfg->m_draw_lby )
			return;

		const auto& entry = g_lag_comp->entry( player->networkable( )->index( ) - 1 );
		if( entry.m_lag_records.empty( ) )
			return;

		const auto& lag_record = entry.m_lag_records.front( );

		if( entry.m_lby_misses >= crypt_int( 2 ) )
			return;

		if( lag_record->m_fake_walking )
			return;

		if( lag_record->m_anim_velocity.length( 2u ) >= 0.1f )
			return;

		float cycle = std::clamp<float>( entry.m_lby_upd - lag_record->m_anim_time, 0.f, 1.1f );

		float scale = ( cycle ) / 1.1f;

		m_change_offset_due_to_lby.at( player->networkable( )->index( ) ) = true;

		const float box_width = std::abs( rect.right - rect.left );
		int offset{ };
		if( m_cfg->m_wpn_ammo &&
			m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) )
			offset += 6;

		auto clr = sdk::col_t( m_cfg->m_lby_upd_clr[ 0 ] * 255.f, m_cfg->m_lby_upd_clr[ 1 ] * 255.f, m_cfg->m_lby_upd_clr[ 2 ] * 255.f, m_cfg->m_lby_upd_clr[ 3 ] * m_dormant_data[ player->networkable( )->index( ) ].m_alpha );

		auto magic_clr = player->networkable( )->dormant( ) ? sdk::col_t( 0.f, 0.f, 0.f, m_dormant_data.at( player->networkable( )->index( ) ).m_alpha ) : sdk::col_t( 0.0f, 0.0f, 0.0f, 100.0f );
		g_render->rect_filled( sdk::vec2_t( rect.right + 1, rect.bottom + 2 + offset ), sdk::vec2_t( rect.left - 1, rect.bottom + 6 + offset ), magic_clr );
		g_render->rect_filled( sdk::vec2_t( rect.left, rect.bottom + 3 + offset ), sdk::vec2_t( rect.left + box_width * scale, rect.bottom + 5 + offset ), clr );
	}

	void c_visuals::tone_map_modulation( valve::base_entity_t* ent ) {
		if( !m_cfg->m_tone_map_modulation )
			return;

		valve::tone_map_t* const entity = static_cast < valve::tone_map_t* >( ent );

		entity->use_custom_bloom_scale( ) = true;
		entity->custom_bloom_scale( ) = m_cfg->m_bloom * 0.01f;

		entity->user_custom_auto_exposure_min( ) = true;
		entity->auto_custom_exposure_min( ) = m_cfg->m_exposure * 0.001f;

		entity->user_custom_auto_exposure_max( ) = true;
		entity->auto_custom_exposure_max( ) = m_cfg->m_exposure * 0.001f;
	}

	void c_visuals::draw_flags( valve::cs_player_t* player, RECT& rect ) {
		if( !player->weapon( ) )
			return;

		if( !m_cfg->m_draw_flags )
			return;

		std::vector < flags_data_t > flags_data { };

		std::string money_str{  };

		if( m_cfg->m_draw_money )
			flags_data.push_back( { ( "$" + std::to_string( player->money( ) ) ), sdk::col_t( 155, 210, 100 ) } );

		const auto& entry = g_lag_comp->entry( player->networkable( )->index( ) - 1 );

		// std::string_view solve_method{ "unk" };
		//  kevlar
		{
			auto kevlar = player->armor_val( ) > 0;
			auto helmet = player->has_helmet( );

			std::string text;

			if( helmet && kevlar ) {
				text = xor_str( "HK" );
			}
			else if( kevlar ) {
				text = xor_str( "K" );
			}
			else {
				text = xor_str( "VULN" );
			}

			if( g_visuals->cfg( ).m_player_flags & 4 )
				flags_data.push_back( { text, sdk::col_t( 240, 240, 240 ) } );
		}

		// scoped
		{
			if( g_visuals->cfg( ).m_player_flags & 8 && player->scoped( ) )
				flags_data.push_back( { xor_str( "ZOOM" ), sdk::col_t( 0, 175, 255, 255 ) } );
		}

		if( player->ping( ) > 70 )
			flags_data.push_back( { std::to_string( player->ping( ) ) + "MS", player->ping( ) > 250 ? sdk::col_t( 163, 56, 56 ) : sdk::col_t( 219, 159, 37 ) } );

		if( !entry.m_lag_records.empty( ) ) {
			//for( auto record = entry.m_lag_records.rbegin ( ); record != entry.m_lag_records.rend ( ); ++record )
			//{
			//	if( record->get( )->valid( ) && record->get( ) ) {
			//		flags_data.push_back( { std::to_string( record->get( )->m_anim_layers.at( 6u ).m_playback_rate ), sdk::col_t( 99, 175, 201 ) } );
			//		flags_data.push_back( { std::to_string( record->get( )->m_anim_layers.at( 12u ).m_playback_rate ), sdk::col_t( 99, 175, 201 ) } );

			//		flags_data.push_back( { std::to_string( record->get( )->m_lby ), sdk::col_t( 255, 175, 201 ) } );
			//		flags_data.push_back( { std::to_string( record->get( )->m_old_lby ), sdk::col_t( 255, 175, 201 ) } );
			//	}
			//}
			auto lag_record = entry.m_lag_records.front( ).get( );

			if( lag_record && !lag_record->m_dormant ) {
				if( lag_record->m_broke_lc ) {
					flags_data.push_back( { xor_str( "LC" ), 
						sdk::col_t( 255, 16, 16 ) } );
				}

				std::string_view solve_method{ "unk" };

				switch( lag_record->m_resolver_method ) {
					case e_solve_methods::no_fake:
						solve_method = "no fake";
						break;
					case e_solve_methods::lby_delta:
						solve_method = "lby delta";
						break;
					case e_solve_methods::fake_walk:
						solve_method = "fake walk";
					break;
					case e_solve_methods::last_move_lby:
						solve_method = "last move";
						break;
					case e_solve_methods::body_flick:
						solve_method = "flick";
						break;	
					case e_solve_methods::backwards:
						solve_method = "backwards";
						break;
					case e_solve_methods::forwards:
						solve_method = "forwards";
						break;
					case e_solve_methods::freestand_l:
						solve_method = "anti-fs logic";
						break;
					case e_solve_methods::brute:
						solve_method = "brute";
						break;
					case e_solve_methods::brute_not_moved:
						solve_method = "brute( n )";
						break;
					case e_solve_methods::just_stopped:
						solve_method = "anim lby";
						break;
					case e_solve_methods::body_flick_res:
						solve_method = "body flick";
					break;
					case e_solve_methods::air:
						solve_method = "in air";
						break;
					case e_solve_methods::move:
						solve_method = "move";
						break;
					default:
						solve_method = "unk";
						break;
				}

				//flags_data.push_back( { solve_method.data( ), sdk::col_t( 99, 175, 201 ) } );
			}
		}

		// iterate flags.
		for( int i{ }; i < flags_data.size( ); ++i ) {
			// get flag job ( pair ).
			const auto& f = flags_data[ i ];

			int offset = i * 9;

			// draw flag.
			sdk::col_t clr = player->networkable( )->dormant( ) ? sdk::col_t( f.m_clr.r( ), f.m_clr.g( ), f.m_clr.b( ), m_dormant_data.at( player->networkable( )->index( ) ).m_alpha ) : f.m_clr;

			g_render->text( f.m_name, sdk::vec2_t( rect.right + 5, rect.top + offset - 1 ), clr, g_misc->m_fonts.m_esp.m_04b, true, false, false, true, false );
		}
	}

	void c_visuals::draw_box( valve::cs_player_t* player, RECT& rect ) {
		if( !m_cfg->m_draw_box )
			return;

		auto bg_alpha = std::clamp( ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha, 0, 180 );

		auto color = sdk::col_t( 255, 255, 255,( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha );
		g_render->rect( sdk::vec2_t( rect.left + 1, rect.top + 1 ), sdk::vec2_t( rect.right - 1, rect.bottom - 1 ), sdk::col_t( 10, 10, 10, bg_alpha ) );
		g_render->rect( sdk::vec2_t( rect.left - 1, rect.top - 1 ), sdk::vec2_t( rect.right + 1, rect.bottom + 1 ), sdk::col_t( 10, 10, 10, bg_alpha ) );
		g_render->rect( sdk::vec2_t( rect.left, rect.top ), sdk::vec2_t( rect.right, rect.bottom ), color );
	}

	void c_visuals::draw_glow( ) {
		if( !m_cfg->m_glow
			|| !g_local_player->self( ) )
			return;

		if( !valve::g_glow->m_object_definitions.size( ) )
			return;

		for( int i{ }; i < valve::g_glow->m_object_definitions.size( ); ++i ) {
			auto obj = &valve::g_glow->m_object_definitions.at( i );

			if( !obj->m_entity || !obj->m_entity->is_player( ) )
				continue;

			auto player = static_cast < valve::cs_player_t* >( obj->m_entity );

			if( player->friendly( g_local_player->self( ) ) )
				continue;

			obj->m_render_occluded = true;
			obj->m_render_unoccluded = false;
			obj->m_render_full_bloom = false;
			obj->m_color = { m_cfg->m_glow_clr[ 0 ], m_cfg->m_glow_clr[ 1 ], m_cfg->m_glow_clr[ 2 ] };
			obj->m_alpha = m_cfg->m_glow_clr[ 3 ];
		}
	}

	void c_visuals::change_shadows( valve::base_entity_t* entity ) {
		if( !m_cfg->m_shadows_modulation )
			return;

		auto ent = reinterpret_cast < valve::cascade_light_t* >( entity );

		ent->shadow_dir( ) = sdk::vec3_t( m_cfg->m_x_dir, m_cfg->m_y_dir, m_cfg->m_z_dir );
	}

	void c_visuals::draw_hitmarkers( ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if( !m_cfg->m_hit_markers ) {
			return m_hit_markers.clear( );
		}

		if( !m_hit_markers.empty( ) ) {
			auto it = m_hit_markers.begin( );

			while( it != m_hit_markers.end( ) ) {
				auto& cur_it = *it;
				const auto life_time = valve::g_global_vars.get( )->m_cur_time - cur_it.m_spawn_time;

				if( cur_it.m_alpha > 0.f && life_time > 0.3f ) {
					cur_it.m_alpha -= 3;
				}

				std::clamp( cur_it.m_alpha, 0.f, 255.f );

				auto col = sdk::col_t( 255, 255, 255, cur_it.m_alpha );

				sdk::vec3_t on_screen{ };
				if( g_render->world_to_screen( cur_it.m_pos, on_screen ) ) {
					constexpr auto k_size = 6;

					g_render->line( 
						{ on_screen.x( ) - k_size, on_screen.y( ) - k_size },
						{ on_screen.x( ) - ( k_size / 2 ), on_screen.y( ) - ( k_size / 2 ) }, col
					 );
					g_render->line( 
						{ on_screen.x( ) - k_size, on_screen.y( ) + k_size },
						{ on_screen.x( ) - ( k_size / 2 ), on_screen.y( ) + ( k_size / 2 ) }, col
					 );
					g_render->line( 
						{ on_screen.x( ) + k_size, on_screen.y( ) + k_size} ,
						{ on_screen.x( ) + ( k_size / 2 ), on_screen.y( ) + ( k_size / 2 ) }, col
					 );
					g_render->line( 
						{ on_screen.x( ) + k_size, on_screen.y( ) - k_size },
						{ on_screen.x( ) + ( k_size / 2 ), on_screen.y( ) - ( k_size / 2 ) }, col
					 );
				}

				if( cur_it.m_alpha < 4.f )
					it = m_hit_markers.erase( it );
				else
					it++;
			}
		}
	}

	void c_visuals::draw_enemy_bullet_tracers( ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) ) {
			return m_enemy_bullet_tracers.clear( );
		}

		if( !m_cfg->m_enemy_bullet_tracers ) {
			return m_enemy_bullet_tracers.clear( );
		}

		auto it = m_enemy_bullet_tracers.begin( );

		while( it != m_enemy_bullet_tracers.end( ) ) {
			auto& cur_it = *it;
			auto next_it = ( it + 1 );
			const auto life_time = valve::g_global_vars.get( )->m_cur_time - cur_it.m_spawn_time;

			if( next_it != m_enemy_bullet_tracers.end( )
				&& cur_it.m_spawn_time == ( *next_it ).m_spawn_time ) {
				it = m_enemy_bullet_tracers.erase( it );
				continue;
			}

			if( std::abs( life_time ) > 1.f )
				it = m_enemy_bullet_tracers.erase( it );
			else
				it++;

			valve::beam_info_t info{ };

			info.m_start = cur_it.m_start_pos;
			info.m_end = cur_it.m_end_pos;
			info.m_model_index = valve::g_model_info->model_index( xor_str( "sprites/purplelaser1.vmt" ) );
			info.m_model_name = xor_str( "sprites/purplelaser1.vmt" );
			info.m_life = 0.5f;
			info.m_width = 0.6f;
			info.m_end_width = 0.6f;
			info.m_fade_length = 0.f;
			info.m_amplitude = 0.f;   // beam 'jitter'.
			info.m_brightness = m_cfg->m_enemy_bullet_tracers_clr[ 3 ] * 255.f;
			info.m_speed = 0.5f;
			info.m_segments = 2;
			info.m_renderable = true;
			info.m_flags = 0;
			info.m_red = m_cfg->m_enemy_bullet_tracers_clr[ 0 ] * 255.f;
			info.m_green = m_cfg->m_enemy_bullet_tracers_clr[ 1 ] * 255.f;
			info.m_blue = m_cfg->m_enemy_bullet_tracers_clr[ 2 ] * 255.f;

			const auto beam = valve::g_beams->create_beam_points( info );
			if( !beam )
				return;

			valve::g_beams->draw_beam( beam );
		}
	}

	void c_visuals::draw_bullet_tracers( ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) ) {
			return m_bullet_tracers.clear( );
		}

		if( !m_cfg->m_bullet_tracers ) {
			return m_bullet_tracers.clear( );
		}

		if( !m_bullet_tracers.empty( ) ) {
			if( m_bullet_tracers.size( ) > 4 )
				m_bullet_tracers.pop_front( );

			auto it = m_bullet_tracers.begin( );

			while( it != m_bullet_tracers.end( ) ) {
				auto& cur_it = *it;
				auto next_it = ( it + 1 );
				const auto life_time = valve::g_global_vars.get( )->m_cur_time - cur_it.m_spawn_time;

				if( next_it != m_bullet_tracers.end( )
					&& cur_it.m_spawn_time == ( *next_it ).m_spawn_time ) {
					it = m_bullet_tracers.erase( it );
					continue;
				}

				if( std::abs( life_time ) > 1.f )
					it = m_bullet_tracers.erase( it );
				else
					it++;

				auto start_pos = cur_it.m_start_pos;

				start_pos.z( ) -= 2;

				valve::beam_info_t info{ };

				info.m_start = start_pos;
				info.m_end = cur_it.m_end_pos;
				info.m_model_index = valve::g_model_info->model_index( xor_str( "sprites/purplelaser1.vmt" ) );
				info.m_model_name = xor_str( "sprites/purplelaser1.vmt" );
				info.m_life = 0.5f;
				info.m_width = 0.6f;
				info.m_end_width = 0.6f;
				info.m_fade_length = 0.f;
				info.m_amplitude = 0.f;   // beam 'jitter'.
				info.m_brightness = m_cfg->m_bullet_tracers_clr[ 3 ] * 255.f;
				info.m_speed = 0.5f;
				info.m_segments = 2;
				info.m_renderable = true;
				info.m_flags = 0;
				info.m_red = m_cfg->m_bullet_tracers_clr[ 0 ] * 255.f;
				info.m_green = m_cfg->m_bullet_tracers_clr[ 1 ] * 255.f;
				info.m_blue = m_cfg->m_bullet_tracers_clr[ 2 ] * 255.f;

				const auto beam = valve::g_beams->create_beam_points( info );
				if( !beam )
					return;

				valve::g_beams->draw_beam( beam );
			}
		}
	}

	void c_visuals::draw_shot_mdl( ) {
		if( !valve::g_engine->in_game( ) )
			return m_shot_mdls.clear( );

		if( !g_chams->cfg( ).m_shot_chams
			|| m_shot_mdls.empty( ) )
			return;

		const auto context = valve::g_mat_sys->render_context( );
		if( !context )
			return;

		auto& cfg = g_chams->cfg( );

		for( auto i = m_shot_mdls.begin( ); i != m_shot_mdls.end( ); ) {

			const float max_time = i->m_is_death ? 0.4f : 1.f;
			const float delta = ( i->m_time + max_time ) - valve::g_global_vars.get( )->m_real_time;

			if( delta <= 0.f ) {
				i = m_shot_mdls.erase( i );
				continue;
			}

			if( !i->m_bones.data( ) )
				continue;

			sdk::col_t clr = sdk::col_t( cfg.m_shot_clr[ 0 ] * 255.f, 
				cfg.m_shot_clr[ 1 ] * 255.f,
				cfg.m_shot_clr[ 2 ] * 255.f, 
				cfg.m_shot_clr[ 3 ] * ( 255.f * delta ) );

			if( i->m_is_death )
				clr = sdk::col_t( cfg.m_enemy_clr[ 0 ] * 255.f,
					cfg.m_enemy_clr[ 1 ] * 255.f,
					cfg.m_enemy_clr[ 2 ] * 255.f, 
					cfg.m_enemy_clr[ 3 ] * ( 255.f * delta ) );
		
			if( i->m_is_death && cfg.m_enemy_chams_invisible ) {

				sdk::col_t clr2 = sdk::col_t( cfg.m_invisible_enemy_clr[ 0 ] * 255.f,
					cfg.m_invisible_enemy_clr[ 1 ] * 255.f,
					cfg.m_invisible_enemy_clr[ 2 ] * 255.f,
					cfg.m_invisible_enemy_clr[ 3 ] * ( 255.f * delta ) );

				g_chams->override_mat( cfg.m_enemy_chams_invisible, clr2, true );
				hooks::orig_draw_mdl_exec( valve::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
				valve::g_studio_render->forced_mat_override( nullptr );
			}

			g_chams->override_mat( i->m_is_death ? cfg.m_enemy_chams_type : cfg.m_shot_chams_type, clr, !i->m_is_death );
			hooks::orig_draw_mdl_exec( valve::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
			valve::g_studio_render->forced_mat_override( nullptr );

			i = std::next( i );
		}
	}

	unsigned int find_in_datamap( valve::data_map_t* map, const char* name )
	{
		while( map )
		{
			for( auto i = 0; i < map->m_size; ++i )
			{
				if( !map->m_descriptions [ i ].m_name )
					continue;

				if( !strcmp( name, map->m_descriptions [ i ].m_name ) )
					return map->m_descriptions [ i ].m_flat_offset [ 0 ];

				if( map->m_descriptions [ i ].m_type == 10 )
				{
					if( map->m_descriptions [ i ].m_data_map )
					{
						unsigned int offset;

						if( offset = find_in_datamap( map->m_descriptions [ i ].m_data_map, name ) )
							return offset;
					}
				}
			}

			map = map->m_base_map;
		}

		return 0;
	}

	void c_visuals::add_shot_mdl( valve::cs_player_t* player, const sdk::mat3x4_t* bones, bool is_death ) {
		const auto model = player->renderable( )->model( );
		if( !model )
			return;

		if( !bones )
			return;

		if( !player )
			return;

		const auto mdl_data = * ( valve::studio_hdr_t** ) player->studio_hdr( );
		if( !mdl_data )
			return;

		auto& shot_mdl = m_shot_mdls.emplace_back( );

		static int skin = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nSkin" ) );
		static int body = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nBody" ) );

		shot_mdl.m_player_index = player->networkable( )->index ( );
		shot_mdl.m_time = valve::g_global_vars.get( )->m_real_time;
		shot_mdl.m_is_death = is_death;
		shot_mdl.m_state.m_studio_hdr = mdl_data;
		shot_mdl.m_state.m_studio_hw_data = valve::g_mdl_cache->lookup_hw_data( model->m_studio );
		shot_mdl.m_state.m_cl_renderable = player->renderable( );
		shot_mdl.m_state.m_draw_flags = 0;
		shot_mdl.m_info.m_renderable = player->renderable( );
		shot_mdl.m_info.m_model = model;
		shot_mdl.m_info.m_hitboxset = player->hitbox_set_index( );
		shot_mdl.m_info.m_skin = * ( int* ) ( uintptr_t ( player ) + skin );
		shot_mdl.m_info.m_body = * ( int* ) ( uintptr_t ( player ) + body );
		shot_mdl.m_info.m_index = player->networkable ( )->index ( );
		shot_mdl.m_info.m_origin = player->origin ( );
		shot_mdl.m_info.m_angles.y( ) = player->anim_state( )->m_foot_yaw;

		shot_mdl.m_info.m_instance = player->renderable( )->mdl_instance ( );
		shot_mdl.m_info.m_flags = 1;		

		std::memcpy( shot_mdl.m_bones.data( ), bones, sizeof( sdk::mat3x4_t ) * player->bone_cache( ).m_size );

		g_ctx->addresses( ).m_angle_matrix( shot_mdl.m_info.m_angles, shot_mdl.m_world_matrix );

		shot_mdl.m_world_matrix [ 0 ][ 3 ] = player->origin ( ).x ( );
		shot_mdl.m_world_matrix [ 1 ][ 3 ] = player->origin ( ).y ( );
		shot_mdl.m_world_matrix [ 2 ][ 3 ] = player->origin ( ).z ( );

		shot_mdl.m_info.m_model_to_world = shot_mdl.m_state.m_bones = &shot_mdl.m_world_matrix;
	}

	void c_visuals::skybox_changer( ) {
		static auto sv_skyname = valve::g_cvar->find_var( xor_str( "sv_skyname" ) );
		std::string skybox = sv_skyname->get_str( );

		using sky_box_fn = void( __fastcall* )( const char* );

		static auto fn = reinterpret_cast < sky_box_fn >( g_ctx->addresses( ).m_sky_box );

		switch( m_cfg->m_skybox_type )
		{
		case 1:
			skybox = ( "cs_tibet" );
			break;
		case 2:
			skybox = ( "cs_baggageskybox" );
			break;
		case 3:
			skybox = ( "italy" );
			break;
		case 4:
			skybox = ( "jungle" );
			break;
		case 5:
			skybox = ( "office" );
			break;
		case 6:
			skybox = ( "sky_cs15_daylight01_hdr" );
			break;
		case 7:
			skybox = ( "sky_cs15_daylight02_hdr" );
			break;
		case 8:
			skybox = ( "vertigoblue_hdr" );
			break;
		case 9:
			skybox = ( "vertigo" );
			break;
		case 10:
			skybox = ( "sky_day02_05_hdr" );
			break;
		case 11:
			skybox = ( "nukeblank" );
			break;
		case 12:
			skybox = ( "sky_venice" );
			break;
		case 13:
			skybox = ( "sky_cs15_daylight03_hdr" );
			break;
		case 14:
			skybox = ( "sky_cs15_daylight04_hdr" );
			break;
		case 15:
			skybox = ( "sky_csgo_cloudy01" );
			break;
		case 16:
			skybox = ( "sky_csgo_night02" );
			break;
		case 17:
			skybox = ( "sky_csgo_night02b" );
			break;
		case 18:
			skybox = ( "sky_csgo_night_flat" );
			break;
		case 19:
			skybox = ( "sky_dust" );
			break;
		case 20:
			skybox = ( "vietnam" );
			break;
		}

		fn( skybox.c_str( ) );
	}

	void c_visuals::draw_health( valve::cs_player_t* player, RECT& rect ) {
		if( !m_cfg->m_draw_health )
			return;

		static float hp_array[64]{ };

		auto plr_idx = player->networkable( )->index( );

		if( hp_array[plr_idx] > player->health( ) )
			hp_array[plr_idx] = std::lerp( hp_array[plr_idx], player->health( ), valve::g_global_vars.get( )->m_frame_time * 16.f );
        else
            hp_array[plr_idx] = player->health( );

		float box_height = static_cast< float >( rect.bottom - rect.top );

		auto player_idx = player->networkable( )->index( );

		float health_multiplier = 12.f / 360.f;
		health_multiplier *= std::ceil( player->health( ) / 10.f ) - 1;

		sdk::col_t color = sdk::col_t::from_hsb( health_multiplier, 1, 1 ).alpha( ( int ) m_dormant_data [ player_idx ].m_alpha );

		auto bg_alpha = std::clamp( ( int ) m_dormant_data [ player_idx ].m_alpha, 0, 140 );

		float colored_bar_height = ( ( box_height * std::fmin( hp_array[plr_idx], 100.f ) ) / 100.0f );
		float colored_max_bar_height = ( ( box_height * 100.0f ) / 100.0f );

		g_render->rect_filled( sdk::vec2_t( rect.left - 6.0f, rect.top - 1 ), sdk::vec2_t( rect.left - 2.0f, rect.top + colored_max_bar_height + 1 ), sdk::col_t( 0.0f, 0.0f, 0.0f,( float ) bg_alpha ) );
		g_render->rect_filled( sdk::vec2_t( rect.left - 5.0f, rect.top + ( colored_max_bar_height - colored_bar_height ) ), sdk::vec2_t( rect.left - 3.0f, rect.top + colored_max_bar_height ), color );

		if( player->health( ) <= 92 || player->health( ) > 100 )
		{
			g_render->text( std::to_string( player->health( ) ), sdk::vec2_t( rect.left - 5.f,
				( rect.top + ( colored_max_bar_height - colored_bar_height ) - 1 ) ), sdk::col_t( 255, 255, 255,( int ) m_dormant_data [ player_idx ].m_alpha ), g_misc->m_fonts.m_esp.m_04b, true, true, false, false, false );
		}
	}

	void c_visuals::draw_name( valve::cs_player_t* player, RECT& rect ) {
		if( !m_cfg->m_draw_name )
			return;

		valve::player_info_t info;

		valve::g_engine->get_player_info( player->networkable( )->index( ), &info );

		std::string name = info.m_name;

		if( name.length( ) > 36 )
		{
			name.erase( 36, name.length( ) - 36 );
			name.append( xor_str( "..." ) );
		}

		auto width = abs( rect.right - rect.left );

		auto size = g_misc->m_fonts.m_font_for_fkin_name->CalcTextSizeA( 14.f, FLT_MAX, NULL, name.c_str( ) );

		g_render->text( name, sdk::vec2_t( rect.left + width * 0.5f, rect.top - size.y - 2 ), sdk::col_t( 255, 255, 255, ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha ), g_misc->m_fonts.m_font_for_fkin_name, false, true, false, false, true );
	}

	void c_chams::init_chams( ) {
		std::ofstream( xor_str( "csgo/materials/desync_glow.vmt" ) ) << xor_str( R"#( "VertexLitGeneric" {
			"$additive" "1"
			"$envmap" "models/effects/cube_white"
			"$envmaptint" "[1 1 1]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[0 1 2]"
			"$alpha" "1"
		} )#" );

		std::ofstream( "csgo/materials/metallic_chams.vmt" ) << R"#( "VertexLitGeneric"	{
			"$basetexture"				    "vgui/white"
			"$envmap"						"env_cubemap"
			"$envmaptint"                   "[ .10 .10 .10 ]"
			"$pearlescent"					"0"
			"$phong"						"1"
			"$phongexponent"				"10"
			"$phongboost"					"1.0"
			"$rimlight"					    "1"
			"$rimlightexponent"		        "1"
			"$rimlightboost"		        "1"
			"$model"						"1"
			"$nocull"						"0"
			"$halflambert"				    "1"
			"$lightwarptexture"             "metalic"
			}
		 )#";

		m_reg_mat = valve::g_mat_sys->find_mat( xor_str( "debug/debugambientcube" ), xor_str( "Model textures" ) );
		m_reg_mat->increment_ref_count( );
		m_flat_mat = valve::g_mat_sys->find_mat( xor_str( "debug/debugdrawflat" ), xor_str( "Model textures" ) );
		m_flat_mat->increment_ref_count( );
		m_glow_mat = valve::g_mat_sys->find_mat( xor_str( "desync_glow" ), xor_str( "Model textures" ) );
		m_glow_mat->increment_ref_count( );
		m_glow_overlay_mat = valve::g_mat_sys->find_mat( xor_str( "dev/glow_armsrace" ), nullptr );
		m_glow_overlay_mat->increment_ref_count( );
		m_metallic_mat = valve::g_mat_sys->find_mat( xor_str( "metallic_chams" ), xor_str( "Model textures" ) );
		m_metallic_mat->increment_ref_count( );
	}

	void c_chams::override_mat( int mat_type, sdk::col_t col, bool ignore_z ) {
		valve::c_material* mat { };
		
		switch( mat_type ) {
		case 0:
			mat = m_reg_mat;
			break;
		case 1:
			mat = m_flat_mat;
			break;
		case 2:
			mat = m_glow_mat;
			break;
		case 3:
			mat = m_glow_overlay_mat;
			break;
		case 4:
			mat = m_metallic_mat;
			break;
		}

		if( !mat )
			return;

		if( mat_type == 2 
			|| mat_type == 3 ) {
			if( mat ) {
				auto env_map_tint = mat->find_var( xor_str( "$envmaptint" ), nullptr );
				if( env_map_tint )
					env_map_tint->set_value( col.r( ) / 255.f, col.g( ) / 255.f, col.b( ) / 255.f );

				auto alpha = mat->find_var( xor_str( "$alpha" ), nullptr );
				if( alpha )
					alpha->set_value( col.a( ) / 255.f );
			}
		}
		else {

			if( mat ) {
				mat->clr_modulate( col.r( ) / 255.f, col.g( ) / 255.f, col.b( ) / 255.f );
				mat->alpha_modulate( col.a( ) / 255.f );
			}
		}

		mat->set_flag( 1024, ignore_z );
		mat->set_flag( 16384, ignore_z );
		mat->set_flag( 32768, ignore_z );

		valve::g_studio_render->forced_mat_override( mat );
	}

	std::optional< valve::bones_t > c_chams::try_to_lerp_bones( const int index ) const {
		const auto& entry = g_lag_comp->entry( index - 1 );

		if( entry.m_lag_records.size( ) <= 3 )
			return std::nullopt;

		auto& front = entry.m_lag_records.front( );
		
		if( front->m_broke_lc || front->m_dormant )
			return std::nullopt;

		const auto end = entry.m_lag_records.end( );
		for( auto it = entry.m_lag_records.begin( ); it != end; ++it ) {
			lag_record_t* last_first{ nullptr };
			lag_record_t* last_second{ nullptr };
			
			if( it->get( )->valid( ) && it + 1 != end 
				&& !( it + 1 )->get( )->valid( ) 
				&& !( it + 1 )->get( )->m_dormant ) {
				last_first = ( it + 1 )->get( );
				last_second = ( it )->get( );
			}

			if( !last_first || !last_second )
				continue;

			const auto& first_invalid = last_first;
			const auto& last_invalid = last_second;

			if( !last_invalid || !first_invalid ||
				last_invalid->m_sim_time - first_invalid->m_sim_time > 1.f ||
				( last_invalid->m_origin - first_invalid->m_origin ).length( ) < 1.5f )
		 		continue;

			const auto curtime = valve::g_global_vars.get( )->m_cur_time;

			auto delta = 1.f - ( curtime - last_invalid->m_interp_time ) /( last_invalid->m_sim_time - first_invalid->m_sim_time );
			if( delta < 0.f || delta > 1.f )
				last_invalid->m_interp_time = curtime;

			delta = 1.f - ( curtime - last_invalid->m_interp_time ) /( last_invalid->m_sim_time - first_invalid->m_sim_time );

			const auto lerp = sdk::lerp( last_invalid->m_origin, first_invalid->m_origin, std::clamp( delta, 0.f, 1.f ) );

            auto lerped_bones = last_second->m_bones;

            const auto origin_delta = lerp - last_second->m_origin;

            for( std::size_t i { }; i < lerped_bones.size( ); ++i ) {
               lerped_bones [ i ][ 0 ][ 3 ] += origin_delta.x( );
               lerped_bones [ i ][ 1 ][ 3 ] += origin_delta.y( );
               lerped_bones [ i ][ 2 ][ 3 ] += origin_delta.z( );
            }

			return lerped_bones;
		}

		return std::nullopt;
	}

	bool c_chams::draw_mdl( void* ecx, uintptr_t ctx, const valve::draw_model_state_t& state, const valve::model_render_info_t& info, sdk::mat3x4_t* bone ) {
		if( info.m_model && strstr( info.m_model->m_path, xor_str( "models/player" ) ) != nullptr ) {
			dm_ecx = ecx;
			dm_ctx [ info.m_index ] = ctx;
			dm_state [ info.m_index ] = &state;
			dm_info [ info.m_index ] = &info;
			
			auto entity = valve::g_entity_list->get_entity( info.m_index );
			if( !entity )
				return false;

			if( entity->is_player( ) ) {
				auto player = static_cast < valve::cs_player_t* >( entity );

				if( !player->alive( ) )
					return false;

				auto enemy = g_local_player->self( ) && !player->friendly( g_local_player->self( ) );
				auto local = g_local_player->self( ) && player == g_local_player->self( );

				if( enemy && ( m_cfg->m_enemy_chams || m_cfg->m_history_chams ) ) {

					if( m_cfg->m_history_chams ) {
						auto lerp_bones = try_to_lerp_bones( player->networkable( )->index( ) );

						if( lerp_bones.has_value( ) ) {
							override_mat( m_cfg->m_history_chams_type, sdk::col_t( m_cfg->m_history_clr [ 0 ] * 255, m_cfg->m_history_clr [ 1 ] * 255, m_cfg->m_history_clr [ 2 ] * 255, m_cfg->m_history_clr [ 3 ] * 255 ), true );
							hooks::orig_draw_mdl_exec( ecx, ctx, state, info, lerp_bones.value( ).data( ) );
							valve::g_studio_render->forced_mat_override( nullptr );
						}
					}

					if( m_cfg->m_enemy_chams ) {

						if( m_cfg->m_enemy_chams_invisible ) {
							override_mat( m_cfg->m_invisible_enemy_chams_type,
								sdk::col_t( m_cfg->m_invisible_enemy_clr[ 0 ] * 255, m_cfg->m_invisible_enemy_clr[ 1 ] * 255, m_cfg->m_invisible_enemy_clr[ 2 ] * 255, m_cfg->m_invisible_enemy_clr[ 3 ] * 255 ), true );
						}

						hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
						valve::g_studio_render->forced_mat_override( nullptr );

						override_mat( m_cfg->m_enemy_chams_type, sdk::col_t( m_cfg->m_enemy_clr [ 0 ] * 255, m_cfg->m_enemy_clr [ 1 ] * 255, m_cfg->m_enemy_clr [ 2 ] * 255, m_cfg->m_enemy_clr [ 3 ] * 255 ), false );
					}

					hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
					valve::g_studio_render->forced_mat_override( nullptr );
					return true;
				}

				if( local && g_local_player->self( )->alive( ) ) {
				
					if( g_visuals->cfg( ).m_blend_in_scope && valve::g_input->m_camera_in_third_person ) {
						if( g_local_player->self( )->scoped( ) ) {
							valve::g_render_view->set_blend( g_visuals->cfg( ).m_blend_in_scope_val / 100.f );
						}
					}

					if( m_cfg->m_local_chams )
					override_mat( m_cfg->m_local_chams_type,
						sdk::col_t( m_cfg->m_local_clr [ 0 ] * 255, m_cfg->m_local_clr [ 1 ] * 255, m_cfg->m_local_clr [ 2 ] * 255, m_cfg->m_local_clr [ 3 ] * 255 ),
						false );

					hooks::orig_draw_mdl_exec( ecx, ctx, state, info, g_ctx->anim_data( ).m_local_data.m_bones.data( ) );
					valve::g_studio_render->forced_mat_override( nullptr );
					return true;
				}
			}
		}
		else if( g_local_player->self( )
			&& strstr( info.m_model->m_path, xor_str( "weapons/v_" ) ) != nullptr
			&& strstr( info.m_model->m_path, xor_str( "sleeve" ) ) == nullptr 
			&& strstr( info.m_model->m_path, xor_str( "arms" ) ) == nullptr ) {

			if( m_cfg->m_wpn_chams )
				override_mat( m_cfg->m_wpn_chams_type,
					sdk::col_t( m_cfg->m_wpn_clr[ 0 ] * 255, m_cfg->m_wpn_clr[ 1 ] * 255, m_cfg->m_wpn_clr[ 2 ] * 255, m_cfg->m_wpn_clr[ 3 ] * 255 ),
					false );

			hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
			valve::g_studio_render->forced_mat_override( nullptr );
			return true;
		}
		else if( g_local_player->self( )
			&& strstr( info.m_model->m_path, xor_str( "arms" ) ) != nullptr ) {

			if( m_cfg->m_arms_chams )
				override_mat( m_cfg->m_arms_chams_type,
					sdk::col_t( m_cfg->m_arms_clr [ 0 ] * 255, m_cfg->m_arms_clr [ 1 ] * 255, m_cfg->m_arms_clr [ 2 ] * 255, m_cfg->m_arms_clr [ 3 ] * 255 ),
					false );

			hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
			valve::g_studio_render->forced_mat_override( nullptr );
			return true;
		}

		return false;
	}

	void c_visuals::removals( ) {
		if( g_local_player->self( ) ) {
			if( g_local_player->self( )->flash_dur( ) && m_cfg->m_remove_flash )
				g_local_player->self( )->flash_dur( ) = 0.f;
		}

		static std::vector < std::string > smoke_str = {
		xor_str( "particle/vistasmokev1/vistasmokev1_smokegrenade" ),
		xor_str( "particle/vistasmokev1/vistasmokev1_emods" ),
		xor_str( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ),
		xor_str( "particle/vistasmokev1/vistasmokev1_fire" ),
		};

		for( const auto& smoke_mat : smoke_str ) {
			auto cur_mat = valve::g_mat_sys->find_mat( smoke_mat.c_str( ), "Other textures" );

			if( !cur_mat )
				continue;

			cur_mat->set_flag( 1 << 2, m_cfg->m_remove_smoke );
		}

		if( m_cfg->m_remove_smoke ) {
			if( *reinterpret_cast < std::int32_t* >( *reinterpret_cast < std::uint32_t** >( reinterpret_cast < std::uint32_t >( g_ctx->addresses( ).m_smoke_count ) ) ) != 0 ) {
				*reinterpret_cast < std::int32_t* >( *reinterpret_cast < std::uint32_t** >( reinterpret_cast < std::uint32_t >( g_ctx->addresses( ).m_smoke_count ) ) ) = 0;
			}
		}

		**reinterpret_cast < bool** >( reinterpret_cast < std::uint32_t >( g_ctx->addresses( ).m_post_process ) ) = m_cfg->m_remove_post_processing;

		static auto cl_wpn_sway_amt = valve::g_cvar->find_var( xor_str( "cl_wpn_sway_scale" ) );

		if( m_cfg->m_remove_hands_shaking ) {
			cl_wpn_sway_amt->set_float( 0.f );
		}
		else
			cl_wpn_sway_amt->set_float( 1.f );

		static auto blur_overlay = valve::g_mat_sys->find_mat( xor_str( "dev/scope_bluroverlay" ), "Other textures" );
		static auto scope_dirt = valve::g_mat_sys->find_mat( xor_str( "models/weapons/shared/scope/scope_lens_dirt" ), "Other textures" );

		blur_overlay->set_flag( ( 1 << 2 ), false );
		scope_dirt->set_flag( ( 1 << 2 ), false );

		if( g_local_player->self( )
			&& g_local_player->self( )->alive( ) ) {
			auto wpn = g_local_player->self( )->weapon( );
			if( wpn ) {
				if( wpn->info( )
					&& wpn->info( )->m_type == valve::e_weapon_type::sniper ) {
					if( g_local_player->self( )->scoped( ) ) {
						if( m_cfg->m_remove_scope ) {
							blur_overlay->set_flag( ( 1 << 2 ), true );
							scope_dirt->set_flag( ( 1 << 2 ), true );
						}
					}
				}
			}
		}
	}

	void c_visuals::draw_scope_lines( ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		auto wpn = g_local_player->self( )->weapon( );

		if( wpn ) {
			auto wpn_data = wpn->info( );

			if( wpn_data
				&& wpn_data->m_type == valve::e_weapon_type::sniper ) {
				if( g_local_player->self( )->scoped( ) ) {
					if( m_cfg->m_remove_scope ) { 
						int x, y;
						valve::g_engine->get_screen_size( x, y );
						g_render->line( sdk::vec2_t( 0, y / 2 ), sdk::vec2_t( x, y / 2 ), sdk::col_t( 0, 0, 0, 255 ) );
						g_render->line( sdk::vec2_t( x / 2, 0 ), sdk::vec2_t( x / 2, y ), sdk::col_t( 0, 0, 0, 255 ) );
					}
				}
			}
		}
	}
}