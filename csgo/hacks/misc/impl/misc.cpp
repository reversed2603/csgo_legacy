#include "../../../csgo.hpp"

namespace csgo::hacks { 

	void c_misc::manipulate_ragdolls( ) {
		static auto ragdoll_gravity_cvar = game::g_cvar->find_var( "cl_ragdoll_gravity" );
		static auto backup_cvar = game::g_cvar->find_var( "cl_ragdoll_gravity" );

		if( !m_cfg->m_force_ragdoll_gravity || !m_cfg->m_modulate_ragdolls ) {
			ragdoll_gravity_cvar->set_int( backup_cvar->get_int( ) );
		}
		else if( m_cfg->m_force_ragdoll_gravity && m_cfg->m_modulate_ragdolls ) {
			ragdoll_gravity_cvar->set_int( m_cfg->m_force_ragdoll_gravity_amt );
		}
		
		for( int i{ }; i < game::g_entity_list->highest_ent_index( ); ++i ) { 
			auto entity = game::g_entity_list->get_entity( i );

			if( !entity
				|| !entity->networkable( )
				|| entity->networkable( )->dormant( ) )
				continue;

			if( entity->networkable( )->client_class( )->m_class_id == 37 ) {
				game::ragdoll_t* ragdoll_ptr = reinterpret_cast< game::ragdoll_t* >( entity );

				if( !ragdoll_ptr )
					return;

				static auto& backup_vec_force = ragdoll_ptr->vec_force( );

				if( !m_cfg->m_modulate_ragdolls 
					|| !m_cfg->m_force_ragdoll ) {
					ragdoll_ptr->vec_force( ) = backup_vec_force;
				}
				else if( m_cfg->m_force_ragdoll && m_cfg->m_modulate_ragdolls ) {
					ragdoll_ptr->vec_force( ) = m_cfg->m_force_ragdoll_amt * 1500.f;
				}
			}
		}
	}

	void c_misc::clan_tag( ) const { 
		using set_clan_tag_t = int( __fastcall* )( const char*, const char* );

		if( m_cfg->m_clan_tag ) { 
			const auto i = ( game::g_global_vars.get( )->m_tick_count / game::to_ticks( 1.f ) ) % 11;
			if( i != m_cfg->m_prev_tag ) { 
				hacks::g_misc->cfg( ).g_reset_tag = true;

				auto tag = "";

				switch( i )
				{ 
					case 0:
						tag = xor_str( "s" );
						break;
					case 1:
						tag = xor_str( "sh" );
						break;
					case 2:
						tag = xor_str( "sha xd" );
						break;
					case 3:
						tag = xor_str( "shac" );
						break;
					case 4:
						tag = xor_str( "shack" );
						break;
					case 5:
						tag = xor_str( "shack 24" );
						break;
					case 6:
						tag = xor_str( "shack" );
						break;
					case 7:
						tag = xor_str( "shac" );
						break;
					case 8:
						tag = xor_str( "sha" );
						break;
					case 9:
						tag = xor_str( "sh" );
						break;
					case 10:
						tag = xor_str( "s" );
						break;
				}

				reinterpret_cast< set_clan_tag_t > ( g_ctx->addresses( ).m_set_clan_tag )( tag, tag );

				hacks::g_misc->cfg( ).m_prev_tag = i;
			}
		}
		else if( hacks::g_misc->cfg( ).g_reset_tag ) { 
		    hacks::g_misc->cfg( ).g_reset_tag = false;

			reinterpret_cast< set_clan_tag_t > ( g_ctx->addresses( ).m_set_clan_tag )( "", "" );
		}
	}

	void c_misc::kill_feed( ) { 
		if( !game::g_engine->in_game( )
			|| !m_cfg->m_kill_feed )
			return;

		game::kill_feed_t* feed = ( game::kill_feed_t* ) game::g_hud->find_element( HASH( "SFHudDeathNoticeAndBotStatus" ) );

		if( !feed )
			return;

		const auto size = feed->m_notices.size( );

		if( !size )
			return;

		for( std::size_t i{ }; i < size; ++i ) { 
			game::notice_text_t* notice = &feed->m_notices.at( i );

			if( notice->m_fade == 1.5f )
				notice->m_fade = std::numeric_limits < float >::max( );
		}
	}

	void c_misc::third_person( ) { 
		bool is_enable = g_key_binds->get_keybind_state( &m_cfg->m_third_person_key );
		static float distance{ 35.f };

		if( !g_local_player 
			|| !g_local_player->self( ) )
			return;

		if( !g_local_player->self( )->alive( ) )
		{ 
			if( m_cfg->m_force_thirdperson_dead )
			{ 
				game::g_input->m_camera_in_third_person = false;
				g_local_player->self( )->observer_mode( ) = 5;
			}

			return;
		}

		if( is_enable && distance != m_cfg->m_third_person_dist )
			distance = std::lerp( distance, m_cfg->m_third_person_dist, 18.f * game::g_global_vars.get( )->m_frame_time );
		else if( !is_enable )
			distance = std::lerp( distance, 15.f, 15.f * game::g_global_vars.get( )->m_frame_time );

		if( distance <= 30.f )
		{ 
			game::g_input->m_camera_in_third_person = false;
			return;
		}

		sdk::vec3_t eye_pos = g_ctx->shoot_pos( );
			
		sdk::qang_t view_angles = game::g_engine->view_angles( );

		game::g_input->m_camera_in_third_person = true;

		game::trace_t trace;

		view_angles.z( ) = distance;

		sdk::vec3_t vec_forward = sdk::vec3_t( 0, 0, 0 );
		sdk::ang_vecs( sdk::qang_t( view_angles.x( ), view_angles.y( ), view_angles.z( ) ), &vec_forward, nullptr, nullptr );

		game::trace_filter_simple_t filter{ g_local_player->self( ), 0 };

		game::ray_t ray( game::ray_t( eye_pos, eye_pos - ( vec_forward * view_angles.z( ) ),  { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ) );

		game::g_engine_trace->trace_ray( ray, MASK_NPCWORLDSTATIC, reinterpret_cast< game::base_trace_filter_t* >( &filter ), &trace );

		view_angles.z( ) *= trace.m_frac;

		game::g_input->m_camera_offset = sdk::vec3_t( view_angles.x( ), view_angles.y( ), view_angles.z( ) );
	}

	void c_misc::buy_bot( ) { 
		if( m_cfg->m_buy_bot && g_ctx->buy_bot( ) )
		{ 
			--g_ctx->buy_bot( );

			if( !g_ctx->buy_bot( ) )
			{ 
				std::string buy{ };

				switch( m_cfg->m_buy_bot_snipers )
				{ 
				case 1:
					buy += xor_str( "buy g3sg1; buy scar20; " );
					break;
				case 2:
					buy += xor_str( "buy awp; " );
					break;
				case 3:
					buy += xor_str( "buy ssg08; " );
					break;
				}

				switch( m_cfg->m_buy_bot_pistols )
				{ 
				case 1:
					buy += xor_str( "buy fiveseven; " );
				case 2:
					buy += xor_str( "buy tec9; " );
				case 3:
					buy += xor_str( "buy elite; " );
					break;
				case 4:
					buy += xor_str( "buy deagle; buy revolver; " );
					break;
				}

				if( m_cfg->m_buy_bot_additional & 2 )
					buy += xor_str( "buy vesthelm; buy vest; " );

				if( m_cfg->m_buy_bot_additional & 4 )
					buy += xor_str( "buy taser; " );

				if( m_cfg->m_buy_bot_additional & 1 )
					buy += xor_str( "buy molotov; buy hegrenade; buy smokegrenade; " );

				if( m_cfg->m_buy_bot_additional & 8 )
					buy += xor_str( "buy defuser; " );

				game::g_engine->exec_cmd( buy.data( ) );
			}
		}
	}

	void c_misc::draw_spectators( )
	{ 
		if( !m_cfg->m_spectators )
			return;

		std::vector < std::string > spectator_list;

		if( g_local_player && g_local_player->self( ) && g_local_player->self( )->alive( ) )
		{ 
			for( int i = 1; i <= game::g_global_vars.get( )->m_max_clients; i++ )
			{ 
				auto player = reinterpret_cast< game::cs_player_t* > ( game::g_entity_list->get_entity( i ) );

				if( !player || player == g_local_player->self( ) 
					||  player->alive( ) 
					|| !player->is_player( )	
					|| !player->networkable( ) 
					||  player->networkable( )->dormant( ) )
					continue;

				auto observer_target = game::g_entity_list->get_entity( player->observer_target_handle( ) );

				if( !observer_target || observer_target != g_local_player->self( ) )
					continue;

				game::player_info_t info;

			    game::g_engine->get_player_info( player->networkable( )->index( ), &info );

				spectator_list.push_back( std::string( info.m_name ).substr( 0, 24 ) );
			}

			for( int i{ }; i < spectator_list.size( ); ++i ) { 
				const std::string& name = spectator_list[ i ];
				int text_length = m_fonts.m_verdana->CalcTextSizeA( 12.f, FLT_MAX, NULL, name.c_str( ) ).x + 5;

				g_render->text( name, sdk::vec2_t( g_visuals->screen_x - text_length, ( i * 18 ) + 5 ),
					sdk::col_t( 255, 255, 255, 220 ), g_misc->m_fonts.m_verdana, false, false, false, false, true );
			}
		}
	}

	void c_misc::draw_watermark( )
	{ 
		return;
        std::string water_mark = xor_str( "secret_hack24 | " );

        auto net_channel = game::g_engine->net_channel_info( );
        if( game::g_engine->in_game( ) )
        { 
            if( net_channel )
            { 
                auto latency = net_channel->avg_latency( game::e_net_flow::out );

                if( latency )
                { 
                    game::cvar_t* cl_updaterate = game::g_cvar->find_var( xor_str( "cl_updaterate" ) );
                    latency -= 0.5f / cl_updaterate->get_float( );
                }

                water_mark += std::string( xor_str( " ms: " ) ) + std::to_string( ( int )( std::max( 0.0f, latency ) * 1000.0f ) ) + " | ";
            }
        }
        else
            water_mark += xor_str( " not connected |" );

        static float last_fps{ };
        static float curr_fps{ };

        curr_fps = ( int )( 1.0f / game::g_global_vars.get( )->m_abs_frame_time );

        if( last_fps != curr_fps )
            last_fps = std::lerp( last_fps, curr_fps, 2.f * game::g_global_vars.get( )->m_frame_time );

        water_mark += xor_str( " fps: " ) + std::to_string( int( last_fps ) );

		auto size_text = m_fonts.m_xiaomi->CalcTextSizeA( 15.f, FLT_MAX, NULL, water_mark.c_str( ) );

		auto draw_list = gui::GetForegroundDrawList( );
        post_process::perform_blur( draw_list, ImVec2( g_visuals->screen_x + 5, 4 ), ImVec2( g_visuals->screen_x + size_text.x + 10, size_text.y + 4 ), 0.85f );

        gui::PushFont( m_fonts.m_xiaomi );
        draw_list->AddText( ImVec2( g_visuals->screen_x + 6, 4 ), ImColor( 255, 255, 255, 255 ), water_mark.c_str ( ) );
        gui::PopFont ( );
	}
}