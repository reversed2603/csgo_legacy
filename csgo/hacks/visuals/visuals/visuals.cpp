#include "../../../csgo.hpp"

namespace csgo::hacks { 

	RECT c_visuals::get_bbox( game::cs_player_t* ent, bool is_valid ) { 
		if( ent->is_player( ) ) { 
			float x{ }, y{ }, w{ }, h{ };

			sdk::vec3_t pos = ent->abs_origin( );

			sdk::vec3_t top = pos + sdk::vec3_t( 0, 0, ent->obb_max( ).z( ) );

			sdk::vec3_t pos_screen{ }, top_screen{ };

			if( !g_render->world_to_screen( pos, pos_screen ) ||
				!g_render->world_to_screen( top, top_screen ) ) { 
				is_valid = false;
			}

			x = int( top_screen.x( ) - ( ( pos_screen.y( ) - top_screen.y( ) ) / 2 ) / 2 );
			y = int( top_screen.y( ) );

			w = int( ( ( pos_screen.y( ) - top_screen.y( ) ) ) / 2 );
			h = int( ( pos_screen.y( ) - top_screen.y( ) ) );

			const bool out_of_fov = pos_screen.x( ) + w + 20 < 0 || pos_screen.x( ) - w - 20 > screen_x || pos_screen.y( ) + 20 < 0 || pos_screen.y( ) - h - 20 > screen_y;

			is_valid = !out_of_fov;
			return RECT{ long( x ), long( y ), long( x + w ), long( y + h ) };
		}
		is_valid = false;
		return RECT{ };
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

	void c_visuals::oof_indicators( game::cs_player_t* player ) { 
		if( !player->weapon( ) )
			return;

		sdk::qang_t viewangles = game::g_engine->view_angles( );

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

	void c_visuals::handle_player_drawings( ) { 
		g_dormant_esp->start( );
		for( int i = 1; i < game::g_global_vars.get( )->m_max_clients; ++i )
		{ 
			auto player = reinterpret_cast< game::cs_player_t* >( game::g_entity_list->get_entity( i ) );

			if( !g_local_player || !g_local_player->self( )
				|| !player
				|| !player->networkable( )
				|| player->friendly( g_local_player->self( ) )
				|| player == g_local_player->self( ) ) {

				if( player && player->networkable( ) )
					m_dormant_data[ i ].m_alpha = std::lerp( m_dormant_data[ i ].m_alpha, 0.f, 14.f * game::g_global_vars.get( )->m_frame_time );
				continue;
			}

			bool alive_check{ false };

			if( !player->alive( ) ) { 
				m_dormant_data[ i ].m_alpha = std::lerp( m_dormant_data[ i ].m_alpha, 0.f, 10.f * game::g_global_vars.get( )->m_frame_time );
				alive_check = true;
			}

			m_dormant_data[ i ].m_alpha = std::clamp( m_dormant_data[ i ].m_alpha, 0.f, 255.f );

			if( !m_dormant_data[ i ].m_alpha
				&& alive_check )
				continue;

			if( !alive_check ) { 
				if( player->networkable( )->dormant( ) ) { 
					float last_shared_time = game::g_global_vars.get( )->m_real_time - m_dormant_data.at( i ).m_last_shared_time;
					bool is_valid_dormancy = g_dormant_esp->adjust_sound( player );
					if( !m_dormant_data.at( i ).m_use_shared && last_shared_time > 1.5f ) { 
						if( is_valid_dormancy ) { 
							m_dormant_data[ i ].m_alpha = std::lerp( m_dormant_data[ i ].m_alpha, 140.f, 4.f * game::g_global_vars.get( )->m_frame_time );
						}
						else if( !is_valid_dormancy || ( !m_dormant_data.at( i ).m_use_shared
							&& last_shared_time > 4.f ) ) { 
								m_dormant_data[ i ].m_alpha = std::lerp( m_dormant_data[ i ].m_alpha, 0.f, 4.f * game::g_global_vars.get( )->m_frame_time );
						}
					}
					else if( m_dormant_data.at( i ).m_use_shared ){ 
						m_dormant_data[ i ].m_alpha = std::lerp( m_dormant_data[ i ].m_alpha, 190.f, 4.f * game::g_global_vars.get( )->m_frame_time );
					}

					if( player->weapon( ) ) { 
						if( m_dormant_data.at( i ).m_weapon_id > 0 )
							player->weapon( )->item_index( ) = static_cast< game::e_item_index >( m_dormant_data.at( i ).m_weapon_id );

						if( player->weapon( )->info( ) )
							if( m_dormant_data.at( i ).m_weapon_type > -1 )
								player->weapon( )->info( )->m_type = static_cast< game::e_weapon_type >( m_dormant_data.at( i ).m_weapon_type );
					}
				}
				else { 
					g_dormant_esp->m_sound_players[ i ].reset( true, player->abs_origin( ), static_cast < int > ( player->flags( ) ) );
					m_dormant_data[ i ].m_origin = sdk::vec3_t( );
					m_dormant_data[ i ].m_receive_time = 0.f;
					m_dormant_data[ i ].m_alpha += 255.f / 0.68f * game::g_global_vars.get( )->m_frame_time;
					m_dormant_data[ i ].m_alpha = std::clamp( m_dormant_data[ i ].m_alpha, 0.f, 255.f );
					m_dormant_data[ i ].m_weapon_id = 0;
					m_dormant_data[ i ].m_weapon_type = -1;
					m_dormant_data.at( i ).m_use_shared = false;
					m_dormant_data.at( i ).m_last_shared_time = 0.f;
				}
			}

			game::g_engine->get_screen_size( screen_x, screen_y );

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

			bool valid_bbox{ true };

			RECT rect = get_bbox( player, valid_bbox );

			if( !valid_bbox )
				continue;

			draw_name( player, rect );
			draw_health( player, rect );
			draw_box( player, rect );
			draw_wpn( player, rect );
			draw_ammo( player, rect );
			draw_lby_upd( player, rect );
			draw_flags( player, rect );
		}
	}

	void c_visuals::draw_wpn( game::cs_player_t* player, RECT& rect ) { 
		if( !m_cfg->m_wpn_icon && !m_cfg->m_wpn_text )
			return;

		if( !player->weapon( ) || !player->weapon( )->info( ) )
			return;

		int offset{ 3 };

		if( m_cfg->m_wpn_ammo 
			&& m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) )
			offset += 5;

		if( m_cfg->m_draw_lby 
			&& m_change_offset_due_to_lby.at( player->networkable( )->index( ) ) )
			offset += 5;

		auto wpn_alpha = std::clamp( ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha, 0, 225 );
		if( m_cfg->m_wpn_text ) { 
			g_render->text( get_weapon_name( player->weapon( ) ), sdk::vec2_t( rect.left + ( abs( rect.right - rect.left ) * 0.5f ), rect.bottom + offset )
				, sdk::col_t( 255, 255, 255, wpn_alpha ), g_misc->m_fonts.m_smallest_pixel, true, true, false, false, false );

			offset += 9;
		}

		if( m_cfg->m_wpn_icon )
			g_render->text( get_weapon_icon( player->weapon( ) ), sdk::vec2_t( rect.left + ( abs( rect.right - rect.left ) * 0.5f ), rect.bottom + offset ),
				sdk::col_t( 255, 255, 255, wpn_alpha ), g_misc->m_fonts.m_icon_font, false, true, false, false, true );
	}

	void c_visuals::draw_ammo( game::cs_player_t* player, RECT& rect ) { 
		auto plr_idx = player->networkable( )->index( );

		static float ammo_array[ 64 ]{ 0.f };

		if( !m_cfg->m_wpn_ammo ) { 
			ammo_array[ plr_idx ] = 0.f;
		}

		if( player->networkable( )->dormant( ) 
			&& m_dormant_data[ plr_idx ].m_alpha <= 10.f )
		{ 
			ammo_array[ plr_idx ] = std::lerp( ammo_array[ plr_idx ], 0.f, game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure this shit is good
		}

		if( m_dormant_data[ plr_idx ].m_alpha >= 50.f ) { 
			if( ammo_array[ plr_idx ] < 1.f ) { 
				ammo_array[ plr_idx ] = std::lerp( ammo_array[ plr_idx ], 1.f, game::g_global_vars.get( )->m_frame_time * 10.f );
			}
			else { 
				ammo_array[ plr_idx ] = std::lerp( ammo_array[ plr_idx ], 1.f, game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure x2
			}
		}

		auto wpn = player->weapon( );

		if( !wpn )
			return;

		auto wpn_data = wpn->info( );

		if( !wpn_data )
			return;

		m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) = false;

		if( !m_cfg->m_wpn_ammo ) { 
			return;
		}

		std::clamp( ammo_array[ plr_idx ], 0.f, 1.f );

		if( wpn->clip1( ) < 0 )
			wpn->clip1( ) = 0;

		if( m_cfg->m_wpn_ammo
			&& wpn_data
			&& wpn_data->m_type != game::e_weapon_type::knife
			&& wpn_data->m_type < game::e_weapon_type::c4
			&& wpn_data->m_max_clip1 != -1 ) { 

			m_change_offset_due_to_ammo.at( player->networkable( )->index( ) ) = true;

			float box_width = std::abs( rect.right - rect.left );
			float current_box_width = ( box_width * wpn->clip1( ) / wpn_data->m_max_clip1 );

			if( player->lookup_seq_act( player->anim_layers( ).at( 1 ).m_seq ) == 967 )
				current_box_width = ( box_width * player->anim_layers( ).at( 1 ).m_cycle );

			float size = ( current_box_width * ammo_array[ plr_idx ] );

			size = std::clamp( size, 0.f, ( current_box_width * ammo_array[ plr_idx ] ) );

			auto clr = sdk::col_t( m_cfg->m_wpn_ammo_clr[ 0 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 1 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 2 ] * 255.f, m_cfg->m_wpn_ammo_clr[ 3 ] * m_dormant_data [ player->networkable( )->index( ) ].m_alpha );

			//background kek
			g_render->rect_filled( sdk::vec2_t( rect.right + 1, rect.bottom + 2 ), sdk::vec2_t( rect.left - 1, rect.bottom + 6 ),
				sdk::col_t( 0.f, 0.f, 0.f, 170.f * ( m_dormant_data.at( player->networkable( )->index( ) ).m_alpha / 255.f ) ) );

			g_render->rect_filled( sdk::vec2_t( rect.left, rect.bottom + 3 ), sdk::vec2_t( rect.left + size, rect.bottom + 5 ), clr );

			// less than 90% ammo
			if( wpn->clip1( ) < ( wpn_data->m_max_clip1 * 0.9 ) )
				g_render->text( std::to_string( wpn->clip1( ) ), sdk::vec2_t( rect.left + size, rect.bottom ), sdk::col_t( 255, 255, 255, m_dormant_data.at( player->networkable( )->index( ) ).m_alpha ), g_misc->m_fonts.m_esp.m_04b, true, false, false );
		}
	}

	void c_visuals::draw_lby_upd( game::cs_player_t* player, RECT& rect ) { 
		auto plr_idx = player->networkable( )->index( );

		m_change_offset_due_to_lby.at( plr_idx ) = false;

		if( !m_cfg->m_draw_lby )
			return;

		const auto& entry = g_lag_comp->entry( plr_idx - 1 );
		if( entry.m_lag_records.empty( ) )
			return;

		const auto& lag_record = entry.m_lag_records.front( );

		if( entry.m_lby_misses >= crypt_int( 2 ) )
			return;

		if( lag_record->m_fake_walking )
			return;

		if( lag_record->m_anim_velocity.length( 2u ) >= 0.1f )
			return;

		float cycle = std::clamp<float> ( entry.m_body_data.m_realign_timer - lag_record->m_anim_time, 0.f, 1.1f );

		float scale = ( cycle / 1.1f );

		if( cycle > 1.0f )
			m_change_offset_due_to_lby.at( plr_idx ) = false;
			return;

		m_change_offset_due_to_lby.at( plr_idx ) = true;

		const float box_width = std::abs( rect.right - rect.left );
		int offset{ };
		if( m_cfg->m_wpn_ammo &&
			m_change_offset_due_to_ammo.at( plr_idx ) )
			offset += 6;

		static float lby_array[ 64 ]{ 0.f };

		if( !m_cfg->m_wpn_ammo ) { 
			lby_array[ plr_idx ] = 0.f;
			return;
		}
		
		if( player->networkable( )->dormant( ) 
			&& m_dormant_data[ plr_idx ].m_alpha <= 10.f )
		{ 
			lby_array[ plr_idx ] = std::lerp( lby_array[ plr_idx ], 0.f, game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure this shit is good
		}

		if( m_dormant_data[ plr_idx ].m_alpha >= 50.f ) { 

			if( lby_array[ plr_idx ] < 1.f ) { 
				lby_array[ plr_idx ] = std::lerp( lby_array[ plr_idx ], 1.f, game::g_global_vars.get( )->m_frame_time * 10.f );
			}
			else { 
				lby_array[ plr_idx ] = std::lerp( lby_array[ plr_idx ], 1.f, game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure x3
			}
		}
		
		std::clamp( lby_array[ plr_idx ], 0.f, 1.f );

		auto clr = sdk::col_t( m_cfg->m_lby_upd_clr[ 0 ] * 255.f, m_cfg->m_lby_upd_clr[ 1 ] * 255.f,
			m_cfg->m_lby_upd_clr[ 2 ] * 255.f, m_cfg->m_lby_upd_clr[ 3 ] * m_dormant_data[ plr_idx ].m_alpha );

		// background kek
		g_render->rect_filled( sdk::vec2_t( rect.right + 1, rect.bottom + 2 + offset ), sdk::vec2_t( rect.left - 1, rect.bottom + 6 + offset ), 
			sdk::col_t( 0.f, 0.f, 0.f, 170.f * ( m_dormant_data.at( plr_idx ).m_alpha / 255.f ) ) );

		g_render->rect_filled( sdk::vec2_t( rect.left, rect.bottom + 3 + offset ), sdk::vec2_t( rect.left + ( box_width * scale ) * lby_array[ plr_idx ], rect.bottom + 5 + offset ), clr );
	}

	void c_visuals::draw_flags( game::cs_player_t* player, RECT& rect ) { 
		if( !player->weapon( ) )
			return;

		if( !m_cfg->m_draw_flags )
			return;

		auto cfg = g_visuals->cfg( );

		std::vector < flags_data_t > flags_data { };

		std::string money_str{  };

		if( cfg.m_player_flags & 1 )
			flags_data.push_back( { ( "$" + std::to_string( player->money( ) ) ), sdk::col_t( 155, 210, 100 ) } );

		const auto& entry = g_lag_comp->entry( player->networkable( )->index( ) - 1 );

		// std::string_view solve_method{ "unk" };
		//  kevlar
		if( player->armor_val( ) > 0 )
		{ 
			auto kevlar = player->armor_val( ) > 0;
			auto helmet = player->has_helmet( );

			std::string text;
			sdk::col_t  clr;

			if( helmet && kevlar ) { 
				text = xor_str( "HK" );
				clr  = sdk::col_t( 240, 240, 240 );
			}
			else if( kevlar ) { 
				text = xor_str( "K" );
				clr  = sdk::col_t( 240, 240, 240 );
			}

			if( cfg.m_player_flags & 4 )
				flags_data.push_back( { text, clr } );
		}

		// scoped
		{ 
			if( cfg.m_player_flags & 8 && player->scoped( ) )
				flags_data.push_back( { xor_str( "ZOOM" ), sdk::col_t( 0, 175, 255, 255 ) } );
		}

		if( cfg.m_player_flags & 2 )
			flags_data.push_back( { std::to_string( player->ping( ) ) + "MS", player->ping( ) < 70 ? sdk::col_t( 255, 255, 255 ) :
				player->ping( ) > 250 ? sdk::col_t( 217, 39, 39 ) : sdk::col_t( 255, 145, 0 ) } );

		if( !entry.m_lag_records.empty( ) ) { 
			//for( auto record = entry.m_lag_records.rbegin( ); record != entry.m_lag_records.rend( ); ++record )
			//{ 
			//	if( record->get( )->valid( ) && record->get( ) ) { 
			//		flags_data.push_back( { std::to_string( record->get( )->m_anim_layers.at( 6u ).m_playback_rate ), sdk::col_t( 99, 175, 201 ) } );
			//		flags_data.push_back( { std::to_string( record->get( )->m_anim_layers.at( 12u ).m_playback_rate ), sdk::col_t( 99, 175, 201 ) } );

			//		flags_data.push_back( { std::to_string( record->get( )->m_lby ), sdk::col_t( 255, 175, 201 ) } );
			//		flags_data.push_back( { std::to_string( record->get( )->m_old_lby ), sdk::col_t( 255, 175, 201 ) } );
			//	}
			//}
			auto lag_record = entry.m_lag_records.front( ).get( );

			if( lag_record && !lag_record->m_dormant && cfg.m_player_flags & 16 ) { 
				if( lag_record->m_broke_lc ) { 
					flags_data.push_back( { xor_str( "LC" ), 
						sdk::col_t( 255, 16, 16 ) } );
				}
			}
		}

		// iterate flags.
		for( int i{ }; i < flags_data.size( ); ++i ) { 
			// get flag job ( pair ).
			const auto& f = flags_data[ i ];

			int offset = i * 9;

			// draw flag.
			auto flags_alpha = std::clamp( ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha, 0, 225 );
			sdk::col_t clr = sdk::col_t( f.m_clr.r( ), f.m_clr.g( ), f.m_clr.b( ), flags_alpha );

			g_render->text( f.m_name, sdk::vec2_t( rect.right + 5, rect.top + offset - 1 ), clr, g_misc->m_fonts.m_esp.m_04b, true, false, false, false, false );
		}
	}

	void c_visuals::draw_box( game::cs_player_t* player, RECT& rect ) { 
		if( !m_cfg->m_draw_box )
			return;

		auto bg_alpha = std::clamp( ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha, 0, 170 );

		g_render->rect( sdk::vec2_t( rect.left + 1, rect.top + 1 ), sdk::vec2_t( rect.right - 1, rect.bottom - 1 ), sdk::col_t( 0, 0, 0, bg_alpha ) );
		g_render->rect( sdk::vec2_t( rect.left - 1, rect.top - 1 ), sdk::vec2_t( rect.right + 1, rect.bottom + 1 ), sdk::col_t( 0, 0, 0, bg_alpha ) );
		g_render->rect( sdk::vec2_t( rect.left, rect.top ), sdk::vec2_t( rect.right, rect.bottom ), sdk::col_t( 255, 255, 255, bg_alpha ) );
	}

	
	bool is_grenade( const int id )
	{
		return id == 9 || id == 98 || id == 134;
	}

	void c_visuals::draw_glow( ) { 
		if( !g_local_player->self( ) )
			return;

		if( !game::g_glow->m_object_definitions.size( ) )
			return;

		for( int i{ }; i < game::g_glow->m_object_definitions.size( ); ++i ) { 
			auto obj = &game::g_glow->m_object_definitions.at( i );

			if( !obj->m_entity )
				continue;

			static float alpha[ 64 ]{ };
			auto client_class = obj->m_entity->networkable( )->client_class( );

			auto class_id = client_class->m_class_id;

			if( obj->m_entity->is_player( ) ) {
				auto player = static_cast < game::cs_player_t* > ( obj->m_entity );
				if( !player->networkable( ) )
					break;

				auto idx = player->networkable( )->index( );

				if( m_cfg->m_glow )
					alpha[ idx ] = std::lerp( alpha[ idx ], 1.f, 4.5f * game::g_global_vars.get( )->m_frame_time );
				else {
					alpha[ idx ] = 0.f;
					break;
				}

				if( player->friendly( g_local_player->self( ), false ) )
					continue;

				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_glow_clr[ 0 ], m_cfg->m_glow_clr[ 1 ], m_cfg->m_glow_clr[ 2 ] };
				obj->m_alpha = ( m_cfg->m_glow_clr[ 3 ] * ( m_dormant_data[ player->networkable( )->index( ) ].m_alpha / 255 ) * alpha[ idx ] );
			}

			if( is_grenade( class_id ) )
			{
				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_draw_grenade_glow_clr[ 0 ], m_cfg->m_draw_grenade_glow_clr[ 1 ], m_cfg->m_draw_grenade_glow_clr[ 2 ] };
				obj->m_alpha = m_cfg->m_draw_grenade_glow ? m_cfg->m_draw_grenade_glow_clr[ 3 ] : 0.f;
			}

			if( obj->m_entity->is_base_combat_wpn( )
				&& !is_grenade( class_id ) )
			{
				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_draw_weapon_glow_clr[ 0 ], m_cfg->m_draw_weapon_glow_clr[ 1 ], m_cfg->m_draw_weapon_glow_clr[ 2 ] };
				obj->m_alpha = m_cfg->m_draw_weapon_glow ? m_cfg->m_draw_weapon_glow_clr[ 3 ] : 0.f;
			}
		}
	}

	void c_visuals::draw_shot_mdl( ) { 
		if( !game::g_engine->in_game( ) )
			return m_shot_mdls.clear( );

		if( !g_chams->cfg( ).m_shot_chams
			|| m_shot_mdls.empty( ) )
			return;

		const auto context = game::g_mat_sys->render_context( );
		if( !context )
			return;

		auto& cfg = g_chams->cfg( );

		for( auto i = m_shot_mdls.begin( ); i != m_shot_mdls.end( ); ) { 
			const float delta = ( i->m_time + 1.25f ) - game::g_global_vars.get( )->m_real_time;

			float alpha = 255.f * ( m_dormant_data [ i->m_player_index ].m_alpha / 255.f );
			alpha = std::clamp( alpha, 0.f, 255.f );

			if( delta <= 0.f ) { 
				i = m_shot_mdls.erase( i );
				continue;
			}

			if( !i->m_bones.data( ) )
				continue;

			if( i->m_is_death ) { 
				if( cfg.m_enemy_chams ) { 
					if( cfg.m_enemy_chams_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_enemy_chams_type,
							sdk::col_t( cfg.m_invisible_enemy_clr[ 0 ] * 255, cfg.m_invisible_enemy_clr[ 1 ] * 255, cfg.m_invisible_enemy_clr[ 2 ] * 255, cfg.m_invisible_enemy_clr[ 3 ] * alpha ), true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}

					if( cfg.m_enemy_chams_overlay_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_enemy_chams_overlay_type,
							sdk::col_t( cfg.m_invisible_enemy_clr_overlay[ 0 ] * 255, cfg.m_invisible_enemy_clr_overlay[ 1 ] * 255,
								cfg.m_invisible_enemy_clr_overlay[ 2 ] * 255, cfg.m_invisible_enemy_clr_overlay[ 3 ] * alpha ), true, true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}

					g_chams->override_mat( cfg.m_enemy_chams_type, sdk::col_t( cfg.m_enemy_clr[ 0 ] * 255, cfg.m_enemy_clr[ 1 ] * 255, cfg.m_enemy_clr[ 2 ] * 255, cfg.m_enemy_clr[ 3 ] * alpha ), false );
					hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
					game::g_studio_render->forced_mat_override( nullptr );

					if( cfg.m_enemy_chams_overlay ) { 
						g_chams->override_mat( cfg.m_enemy_chams_overlay_type,
							sdk::col_t( cfg.m_enemy_clr_overlay[ 0 ] * 255, cfg.m_enemy_clr_overlay[ 1 ] * 255,
								cfg.m_enemy_clr_overlay[ 2 ] * 255, cfg.m_enemy_clr_overlay[ 3 ] * alpha ), false, true );
											
						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}
				}
			}

			i = std::next( i );
		}
	}

	void c_visuals::add_shot_mdl( game::cs_player_t* player, const sdk::mat3x4_t* bones, bool is_death ) { 
		const auto model = player->renderable( )->model( );
		if( !model )
			return;

		if( !bones )
			return;

		if( !player )
			return;

		const auto mdl_data = * ( game::studio_hdr_t** ) player->studio_hdr( );
		if( !mdl_data )
			return;

		auto& shot_mdl = m_shot_mdls.emplace_back( );

		static int skin = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nSkin" ) );
		static int body = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nBody" ) );

		shot_mdl.m_player_index = player->networkable( )->index( );
		shot_mdl.m_time = game::g_global_vars.get( )->m_real_time;
		shot_mdl.m_is_death = is_death;
		shot_mdl.m_state.m_studio_hdr = mdl_data;
		shot_mdl.m_state.m_studio_hw_data = game::g_mdl_cache->lookup_hw_data( model->m_studio );
		shot_mdl.m_state.m_cl_renderable = player->renderable( );
		shot_mdl.m_state.m_draw_flags = 0;
		shot_mdl.m_info.m_renderable = player->renderable( );
		shot_mdl.m_info.m_model = model;
		shot_mdl.m_info.m_hitboxset = player->hitbox_set_index( );
		shot_mdl.m_info.m_skin = * ( int* ) ( uintptr_t ( player ) + skin );
		shot_mdl.m_info.m_body = * ( int* ) ( uintptr_t ( player ) + body );
		shot_mdl.m_info.m_index = player->networkable( )->index( );
		shot_mdl.m_info.m_origin = player->origin( );
		shot_mdl.m_info.m_angles.y( ) = player->anim_state( )->m_foot_yaw;

		shot_mdl.m_info.m_instance = player->renderable( )->mdl_instance( );
		shot_mdl.m_info.m_flags = 1;		

		std::memcpy( shot_mdl.m_bones.data( ), bones, sizeof( sdk::mat3x4_t ) * player->bone_cache( ).m_size );

		g_ctx->addresses( ).m_angle_matrix( shot_mdl.m_info.m_angles, shot_mdl.m_world_matrix );

		shot_mdl.m_world_matrix[ 0 ][ 3 ] = player->origin( ).x( );
		shot_mdl.m_world_matrix[ 1 ][ 3 ] = player->origin( ).y( );
		shot_mdl.m_world_matrix[ 2 ][ 3 ] = player->origin( ).z( );

		shot_mdl.m_info.m_model_to_world = shot_mdl.m_state.m_bones = &shot_mdl.m_world_matrix;
	}

	void c_visuals::draw_health( game::cs_player_t* player, RECT& rect ) { 
		auto plr_idx = player->networkable( )->index( );

		static bool first_toggled = true;
		static float hp_array[ 64 ]{ };

		if( !m_cfg->m_draw_health ) { 
			hp_array[ plr_idx ] = 0.f;
			first_toggled = true;
			return;
		}

		if( player->networkable( )->dormant( ) 
			&& m_dormant_data[ plr_idx ].m_alpha <= 15.f )
		{ 
			hp_array[ plr_idx ] = std::lerp( hp_array[ plr_idx ], 0.f, game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure this shit is good
			first_toggled = true;
		}

		if( first_toggled )
		{ 
			if( m_dormant_data[ plr_idx ].m_alpha >= 50.f ) { 
				if( player->health( ) > hp_array[ plr_idx ] ) { 
					hp_array[ plr_idx ] = std::lerp( hp_array[ plr_idx ], player->health( ), game::g_global_vars.get( )->m_frame_time * 10.f );
				}
				else { 
					hp_array[ plr_idx ] = std::lerp( hp_array[ plr_idx ], player->health( ), game::g_global_vars.get( )->m_frame_time * 16.f ); // make sure this shit is good
					first_toggled = false;
				}
			}
		}
		else { 
			if( hp_array[ plr_idx ] > player->health( ) ) { 
				hp_array[ plr_idx ] = std::lerp( hp_array[ plr_idx ], player->health( ), game::g_global_vars.get( )->m_frame_time * 16.f );
			}
			else { 
				hp_array[ plr_idx ] = player->health( );
			}
		}

		std::clamp( int( hp_array[ plr_idx ] ), 0, player->health( ) );

		float box_height = static_cast< float > ( rect.bottom - rect.top );

		float health_multiplier = 12.f / 360.f;
		health_multiplier *= std::ceil( player->health( ) / 10.f ) - 1;

		sdk::col_t color = sdk::col_t::from_hsb( health_multiplier, 1, 1 ).alpha( ( int ) m_dormant_data [ plr_idx ].m_alpha );

		auto bg_alpha = std::clamp( ( int ) m_dormant_data [ plr_idx ].m_alpha, 0, 140 );

		float colored_bar_height = ( ( box_height * std::fmin( hp_array[ plr_idx ], 100.f ) ) / 100.0f );
		float colored_max_bar_height = ( ( box_height * 100.0f ) / 100.0f );

		g_render->rect_filled( sdk::vec2_t( rect.left - 6.0f, rect.top - 1 ), sdk::vec2_t( rect.left - 2.0f, rect.top + colored_max_bar_height + 1 ), sdk::col_t( 0.0f, 0.0f, 0.0f,( float ) bg_alpha ) );
		g_render->rect_filled( sdk::vec2_t( rect.left - 5.0f, rect.top + ( colored_max_bar_height - colored_bar_height ) ), sdk::vec2_t( rect.left - 3.0f, rect.top + colored_max_bar_height ), color );

		if( player->health( ) <= 92 || player->health( ) > 100 )
		{ 
			g_render->text( std::to_string( player->health( ) ), sdk::vec2_t( rect.left - 5.f,
				( rect.top + ( colored_max_bar_height - colored_bar_height ) - 1 ) ), sdk::col_t( 255, 255, 255,( int ) m_dormant_data [ plr_idx ].m_alpha ), g_misc->m_fonts.m_esp.m_04b, true, true, false, false, false );
		}
	}

	void c_visuals::draw_name( game::cs_player_t* player, RECT& rect ) { 
		if( !m_cfg->m_draw_name )
			return;

		game::player_info_t info;

		game::g_engine->get_player_info( player->networkable( )->index( ), &info );

		std::string name = info.m_name;

		if( name.length( ) > 36 )
		{ 
			name.erase( 36, name.length( ) - 36 );
			name.append( xor_str( "..." ) );
		}

		auto width = abs( rect.right - rect.left );

		auto size = g_misc->m_fonts.m_font_for_fkin_name->CalcTextSizeA( 14.f, FLT_MAX, NULL, name.c_str( ) );

		auto name_alpha = std::clamp( ( int ) m_dormant_data [ player->networkable( )->index( ) ].m_alpha, 0, 200 );

		g_render->text( name, sdk::vec2_t( rect.left + width * 0.5f, rect.top - size.y - 2 ), sdk::col_t( 255, 255, 255, name_alpha ), g_misc->m_fonts.m_font_for_fkin_name, false, true, false, false, true );
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