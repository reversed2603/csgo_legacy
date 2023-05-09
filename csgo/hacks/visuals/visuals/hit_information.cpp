#include "../../../csgo.hpp"

namespace csgo::hacks {
	void c_visuals::draw_hitmarkers( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if( ~m_cfg->m_hit_markers_selection & 1
			&& ~m_cfg->m_hit_markers_selection & 2 ) {
			return m_hit_markers.clear( );
		}

		if( !m_hit_markers.empty( ) ) { 
			auto it = m_hit_markers.begin( );

			while( it != m_hit_markers.end( ) ) { 
				auto& cur_it = *it;
				const auto life_time = game::g_global_vars.get( )->m_cur_time - cur_it.m_spawn_time;

				if( cur_it.m_alpha < 1.f && life_time < 2.f ) { 
					cur_it.m_alpha = std::lerp( cur_it.m_alpha, 1.f, 8.f * game::g_global_vars.get( )->m_frame_time );
				}
				else if( cur_it.m_alpha > 0.f && life_time > 2.f ) { 
					cur_it.m_alpha = std::lerp( cur_it.m_alpha, 0.f, 8.f * game::g_global_vars.get( )->m_frame_time );
				}

				auto col = sdk::col_t( m_cfg->m_hit_markers_clr[ 0 ] * 255.f, m_cfg->m_hit_markers_clr[ 1 ] * 255.f,
					m_cfg->m_hit_markers_clr[ 2 ] * 255.f, ( m_cfg->m_hit_markers_clr[ 3 ] * 255.f ) * cur_it.m_alpha );

				sdk::vec3_t on_screen{ };
				if( g_render->world_to_screen( cur_it.m_pos, on_screen ) ) { 
					float k_size = 6.f * cur_it.m_alpha;
					int padding{ 0 };

					sdk::vec2_t screen_center = sdk::vec2_t( screen_x / 2, screen_y / 2 );

					if( m_cfg->m_hit_markers_selection & 1 ) {
						auto col = sdk::col_t( m_cfg->m_screen_hit_markers_clr[ 0 ] * 255.f, m_cfg->m_screen_hit_markers_clr[ 1 ] * 255.f,
							m_cfg->m_screen_hit_markers_clr[ 2 ] * 255.f, ( m_cfg->m_screen_hit_markers_clr[ 3 ] * 255.f ) * cur_it.m_alpha );

						float k_size = 10.f * cur_it.m_alpha;
						g_render->line( 
							{ screen_center.x( ) - k_size, screen_center.y( ) - k_size },
							{ screen_center.x( ) - ( k_size / 2 ), screen_center.y( ) - ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ screen_center.x( ) - k_size, screen_center.y( ) + k_size },
							{ screen_center.x( ) - ( k_size / 2 ), screen_center.y( ) + ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ screen_center.x( ) + k_size, screen_center.y( ) + k_size } ,
							{ screen_center.x( ) + ( k_size / 2 ), screen_center.y( ) + ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ screen_center.x( ) + k_size, screen_center.y( ) - k_size },
							{ screen_center.x( ) + ( k_size / 2 ), screen_center.y( ) - ( k_size / 2 ) }, col
						 );
					}

					if( m_cfg->m_hit_markers_selection & 2 ) {
						g_render->line( 
							{ on_screen.x( ) - k_size, on_screen.y( ) - k_size },
							{ on_screen.x( ) - ( k_size / 2 ), on_screen.y( ) - ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ on_screen.x( ) - k_size, on_screen.y( ) + k_size },
							{ on_screen.x( ) - ( k_size / 2 ), on_screen.y( ) + ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ on_screen.x( ) + k_size, on_screen.y( ) + k_size } ,
							{ on_screen.x( ) + ( k_size / 2 ), on_screen.y( ) + ( k_size / 2 ) }, col
						 );
						g_render->line( 
							{ on_screen.x( ) + k_size, on_screen.y( ) - k_size },
							{ on_screen.x( ) + ( k_size / 2 ), on_screen.y( ) - ( k_size / 2 ) }, col
						 );
						padding += k_size;
					}

					if( m_cfg->m_hit_markers_selection & 4 ) {
						auto col = sdk::col_t( m_cfg->m_damage_markers_clr[ 0 ] * 255.f, m_cfg->m_damage_markers_clr[ 1 ] * 255.f,
							m_cfg->m_damage_markers_clr[ 2 ] * 255.f, ( m_cfg->m_damage_markers_clr[ 3 ] * 255.f ) * cur_it.m_alpha );

						g_render->text( std::to_string( cur_it.m_damage ), 
							sdk::vec2_t( on_screen.x( ), on_screen.y( ) - 10.f * cur_it.m_alpha - padding ),
							col, hacks::g_misc->m_fonts.m_verdana, false, true, true, false, true );
					}
				}

				life_time > 2.f && cur_it.m_alpha < 0.025f ? it = m_hit_markers.erase( it ) : it++;
			}
		}
	}

	void c_visuals::draw_beam( ) { 
		if( !g_local_player->self( ) )
			return;

		bool final_impact = false;
		for( size_t i{ 0 }; i < bullet_trace_info.size( ); i++ ) { 
			auto& curr_impact = bullet_trace_info.at( i );

			if( std::abs( game::g_global_vars.get( )->m_real_time - curr_impact.m_exp_time ) > 3.f ) { 
				bullet_trace_info.erase( bullet_trace_info.begin( ) + i );
				continue;
			}

			if( curr_impact.m_ignore )
				continue;

			if( curr_impact.m_ring ) {
				game::beam_info_t beam_info{ };
				beam_info.m_type = 7;
				beam_info.m_model_name = xor_str( "sprites/purplelaser1.vmt" );
				beam_info.m_model_index = game::g_model_info->model_index( xor_str( "sprites/purplelaser1.vmt" ) );
				beam_info.m_halo_name = xor_str( "sprites/purplelaser1.vmt" );
				beam_info.m_halo_index = game::g_model_info->model_index( xor_str( "sprites/purplelaser1.vmt" ) );
				beam_info.m_halo_scale = 5.f;
				beam_info.m_life = 1.5f;
				beam_info.m_width = 6.0f;
				beam_info.m_end_width = 6.0f;
				beam_info.m_fade_length = 0.0f;
				beam_info.m_amplitude = 0.0f;//2.f
				beam_info.m_brightness = curr_impact.col.a( );
				beam_info.m_speed = 10.f;
				beam_info.m_start_frame = 0;
				beam_info.m_frame_rate = 0.f;
				beam_info.m_red = curr_impact.col.r( );
				beam_info.m_green = curr_impact.col.g( );
				beam_info.m_blue = curr_impact.col.b( );
				beam_info.m_segments = 1;
				beam_info.m_renderable = true;
				beam_info.m_flags = 0;
				beam_info.m_center = curr_impact.m_start_pos + sdk::vec3_t( 0, 0, 5 );
				beam_info.m_start_radius = 1;
				beam_info.m_end_radius = 550;

				const auto beam = game::g_beams->create_beam_ring_point( beam_info );
				if( !beam )
					return;

				game::g_beams->draw_beam( beam );

				curr_impact.m_ignore = true;
				continue;
			}
		
			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if( i == ( bullet_trace_info.size( ) - 1 ) )
				final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if( ( i + 1 ) < bullet_trace_info.size( ) && curr_impact.m_tickbase != bullet_trace_info.operator[ ]( i + 1 ).m_tickbase )
				final_impact = true;

			else
				final_impact = false;

			if( final_impact || curr_impact.m_player_index != g_local_player->self( )->networkable( )->index( ) ) { 
				auto start = curr_impact.m_start_pos;
				auto end = curr_impact.m_end_pos;

				sdk::vec3_t ang_orientation = ( start - end );

				constexpr auto thickness = 0.4f;
 
				sdk::vec3_t mins = sdk::vec3_t( 0.f, -thickness, -thickness );
				sdk::vec3_t maxs = sdk::vec3_t( ang_orientation.length( ), thickness, thickness );
 
				game::g_glow->add_glow_box( end, ang_orientation.angles( ), mins, maxs, curr_impact.col, 2.f );

				curr_impact.m_ignore = true;
			}
		}
	}

	void c_visuals::push_beam_info( bullet_trace_data_t beam_info ) { 
		bullet_trace_info.emplace_back( beam_info );
	}
}