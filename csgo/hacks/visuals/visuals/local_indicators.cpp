#include "../../../csgo.hpp"

namespace csgo::hacks {
	void c_visuals::draw_auto_peek( )
	{ 
		if( !g_local_player->self( ) ||
			!g_local_player->self( )->alive( ) )
			return;

		if( !g_local_player->self( )->weapon( ) )
			return;

		if( g_local_player->self( )->weapon( )->is_knife( ) )
			return;

		if( g_local_player->self( )->weapon( )->item_index( ) == game::e_item_index::taser )
			return;

		if( !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == static_cast < game::e_weapon_type > ( 9 ) )
			return;
		
		static float alpha = 0.f;

        bool auto_peek_enabled = g_key_binds->get_keybind_state( &hacks::g_move->cfg( ).m_auto_peek_key );

		if( auto_peek_enabled && alpha < 1.f )
			alpha += 0.05f;
		else if( !( auto_peek_enabled ) && alpha > 0.f )
			alpha -= 0.05f;

		auto pos = g_ctx->get_auto_peek_info( ).m_start_pos;

        if( alpha || auto_peek_enabled ) { 
			float step = sdk::pi * 2.0f / 60;
			std::vector<ImVec2> points;
			for( float lat = 0.f; lat <= sdk::pi * 2.0f; lat += step )
			{ 
				const auto& point3d = sdk::vec3_t( sin( lat ), cos( lat ), 0.f ) * ( 12.f * alpha );
				sdk::vec3_t point2d;
				if( g_render->world_to_screen( pos + point3d, point2d ) )
					points.push_back( ImVec2( point2d.x( ), point2d.y( ) ) );
			}
			auto flags_backup = g_render->m_draw_list->Flags;

			auto move_cfg = g_move->cfg( );

			g_render->m_draw_list->Flags |= ImDrawListFlags_AntiAliasedFill;
			g_render->m_draw_list->AddConvexPolyFilled( points.data( ), points.size( ), ImColor( move_cfg.m_auto_peek_clr[ 0 ], move_cfg.m_auto_peek_clr[ 1 ], move_cfg.m_auto_peek_clr[ 2 ], 0.4f * alpha ) );
			g_render->m_draw_list->AddPolyline( points.data( ), points.size( ), ImColor( move_cfg.m_auto_peek_clr[ 0 ], move_cfg.m_auto_peek_clr[ 1 ], move_cfg.m_auto_peek_clr[ 2 ], 0.8f * alpha ), true, 1.f );
			g_render->m_draw_list->Flags = flags_backup;
		}
	}

	
	void c_visuals::manuals_indicators( ) { 

		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) || !m_cfg->m_manuals_indication )
			return;

		auto center = sdk::vec2_t( screen_x / 2.f, screen_y / 2.f );

		float distance = 1.f;

		auto color = sdk::col_t( m_cfg->m_manuals_indication_clr[ 0 ] * 255, m_cfg->m_manuals_indication_clr[ 1 ] * 255, m_cfg->m_manuals_indication_clr[ 2 ] * 255, m_cfg->m_manuals_indication_clr[ 3 ] * 255 );

		if( m_cur_yaw_dir == 1 ) { 
			g_render->triangle_filled( center.x( ) - 29 - distance, center.y( ) + 7,
				center.x( ) - 44 - distance, center.y( ), center.x( ) - 29 - distance, center.y( ) - 7, color );
		}
		if( m_cur_yaw_dir == 2 ) { 
			g_render->triangle_filled( center.x( ) + 30 + distance, center.y( ) - 7,
				center.x( ) + 45 + distance, center.y( ), center.x( ) + 30 + distance, center.y( ) + 7, color );
		}
		if( m_cur_yaw_dir == 4 ) { 
			g_render->triangle_filled( center.x( ), center.y( ) + 45 + distance ,
				center.x( ) - 7, center.y( ) + 30 + distance, center.x( ) + 7, center.y( ) + 30 + distance, color );
		}
		if( m_cur_yaw_dir == 3 ) { 
			g_render->triangle_filled( center.x( ), center.y( ) - 45 - distance,
				center.x( ) - 7, center.y( ) - 30 - distance, center.x( ) + 7, center.y( ) - 30 - distance, color );
		}
	}

	void c_visuals::draw_key_binds( ) { 
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) || !g_misc->cfg( ).m_key_binds )
			return;

		int  padding{ 16 };

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
			ind.text = "baim";

			indicators.push_back( ind );
		}

		if( g_key_binds->get_keybind_state( &g_exploits->cfg( ).m_dt_key ) ) { 
			ind_t ind{ };
			ind.clr = sdk::col_t::lerp( sdk::col_t( 255, 0, 0 ),
				sdk::col_t( 255, 255, 255 ), g_exploits->m_ticks_allowed ).alpha( 200 );
			ind.has_progression_bar = true;

			ind.text = "dt";

			if( ind.has_progression_bar ) { 
				ind.fill_bar = std::clamp( g_exploits->m_ticks_allowed, 0, 1 );
			}

			indicators.push_back( ind );
		}

		if( g_aim_bot->get_min_dmg_override_state( ) ) { 
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "dmg";

			indicators.push_back( ind );
		}

		if( g_key_binds->get_keybind_state( &g_anti_aim->cfg( ).m_freestand_key ) ) { 
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "freestand";

			indicators.push_back( ind );
		}

		if( game::g_engine->net_channel_info( ) ) { 
			auto incoming_latency = game::g_engine->net_channel_info( )->latency( game::e_net_flow::in );
			float percent = std::clamp( ( ( incoming_latency * 1000.f ) / g_ping_spike->cfg( ).m_to_spike ), 0.f, 1.f );

			if( g_key_binds->get_keybind_state( &g_ping_spike->cfg( ).m_ping_spike_key ) ) { 
				ind_t ind{ };
				ind.clr = sdk::col_t::lerp( sdk::col_t( 255, 0, 0, 255 ), 
					sdk::col_t( 150, 200, 60, 255 ), percent );

				ind.text = "ping";
				ind.has_progression_bar = true;

				if( ind.has_progression_bar ) { 
					ind.fill_bar = percent;
				}

				indicators.push_back( ind );
			}
		}

		if( g_key_binds->get_keybind_state( &g_anti_aim->cfg( ).m_fake_flick_key ) ) { 
			ind_t ind{ };
			ind.clr = sdk::col_t( 255, 255, 255, 255 );
			ind.has_progression_bar = false;

			ind.text = "f-body";

			indicators.push_back( ind );		
		}	

		if( indicators.empty( ) )
			return;	

		// iterate and draw indicators.
		for( int i{ }; i < indicators.size( ); ++i ) { 
			auto& indicator = indicators[ i ];
			
			auto size = g_misc->m_fonts.m_esp.m_verdana->CalcTextSizeA( 18.f, FLT_MAX, NULL, indicator.text.data( ) );
			int add { };
			add = 50 + padding + ( ( size.y + 4 ) * i );

			//if( indicator.has_progression_bar ) { 
			//	g_render->draw_rect_filled( 25, ( y / 2 + add + 3 ),
			//		( size.x + 2 ), 2, sdk::col_t( 0, 0, 0, 175 ), 0 );

			//	/* background above, below is the bar fill-up */

			//	g_render->draw_rect_filled( 25, ( y / 2 + add + 3 ),
			//		( ( size.x + 2 ) * indicator.fill_bar ), 2, indicator.clr, 0 );
			//}

			g_render->draw_rect_filled( 13, ( screen_y / 2 + add ),
				( size.x + 4 ), size.y + 2, sdk::col_t( 0, 0, 0, 75 ), 0 );

			g_render->draw_rect_filled( 13, ( screen_y / 2 + add + size.y + 1 ),
				indicator.has_progression_bar ? ( size.x + 4 ) * indicator.fill_bar : ( size.x + 4 ), 2, indicator.clr, 0 );
			
			g_render->text( indicator.text, sdk::vec2_t( 15, screen_y / 2 + add ),
				indicator.clr, g_misc->m_fonts.m_esp.m_verdana, false, false, false, false, true );
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
				&& wpn_data->m_type == game::e_weapon_type::sniper ) { 
				if( g_local_player->self( )->scoped( ) ) { 
					if( m_cfg->m_removals & 1 ) { 
						g_render->m_draw_list->AddLine( ImVec2( 0, screen_y / 2 ), ImVec2( screen_x, screen_y / 2 ), ImColor( 0, 0, 0, 255 ) );
						g_render->m_draw_list->AddLine( ImVec2( screen_x / 2, 0 ), ImVec2( screen_x / 2, screen_y ), ImColor( 0, 0, 0, 255 ) );
					}
				}
			}
		}
	}

}