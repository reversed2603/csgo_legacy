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
}