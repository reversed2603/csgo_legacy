#include "../../../csgo.hpp"

namespace csgo::hacks { 

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
			distance = std::lerp( distance, 15.f, 18.f * game::g_global_vars.get( )->m_frame_time );

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

	int c_skins::get_knife_index( ) { 


		int knife_idx = 0;

		switch( m_cfg->m_knife_type ) { 
		case 1:
			knife_idx = 500;
			break;
		case 2:
			knife_idx = 505;
			break;
		case 3:
			knife_idx = 506;
			break;
		case 4:
			knife_idx = 507;
			break;
		case 5:
			knife_idx = 508;
			break;
		case 6:
			knife_idx = 509;
			break;
		default: 
			break;
		}

		return knife_idx;
	}

	 std::string c_skins::get_world_mdl_str( ) { 

		std::string ret = "";
		
		switch( m_cfg->m_knife_type ) { 
		case 1:
			ret = xor_str( "models/weapons/w_knife_bayonet.mdl" );
			break;
		case 2:
			ret = xor_str( "models/weapons/w_knife_flip.mdl" );
			break;
		case 3:
			ret = xor_str( "models/weapons/w_knife_gut.mdl" );
			break;
		case 4:
			ret = xor_str( "models/weapons/w_knife_karam.mdl" );
			break;
		case 5:
			ret = xor_str( "models/weapons/w_knife_m9_bay.mdl" );
			break;
		case 6:
			ret = xor_str( "models/weapons/w_knife_tactical.mdl" );
			break;
		default:
			break;
		}

		return ret;
	}

	std::string c_skins::get_model_str( ) { 


		std::string ret = "";

		switch( m_cfg->m_knife_type ) { 
		case 1:
			ret = xor_str( "models/weapons/v_knife_bayonet.mdl" );
			break;
		case 2:
			ret = xor_str( "models/weapons/v_knife_flip.mdl" );
			break;
		case 3:
			ret = xor_str( "models/weapons/v_knife_gut.mdl" );
			break;
		case 4:
			ret = xor_str( "models/weapons/v_knife_karam.mdl" );
			break;
		case 5:
			ret = xor_str( "models/weapons/v_knife_m9_bay.mdl" );
			break;
		case 6:
			ret = xor_str( "models/weapons/v_knife_tactical.mdl" );
			break;
		default: 
			break;
		}

		return ret;
	}

	std::string c_skins::get_glove_model( ) { 

		std::string ret = "";
		
		switch( m_cfg->m_glove_type ) { 
		case 5027:
			ret = xor_str( "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" );
			break;
		case 5030:
			ret = xor_str( "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" );
			break;
		case 5031:
			ret = xor_str( "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" );
			break;
		case 5032:
			ret = xor_str( "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" );
			break;
		case 5033:
			ret = xor_str( "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" );
			break;
		case 5034:
			ret = xor_str( "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" );
			break;
		}

		return ret;
	}

	std::string c_skins::get_killicon_str( ) { 

		std::string ret = "";

		switch( m_cfg->m_knife_type ) { 
		case 1:
			ret = xor_str( "bayonet" );	
			break;
		case 2:
			ret = xor_str( "knife_flip" );	
			break;
		case 3:
			ret = xor_str( "knife_gut" );	
			break;
		case 4:
			ret = xor_str( "knife_karambit" );	
			break;
		case 5:
			ret = xor_str( "knife_m9_bayonet" );	
			break;
		case 6:
			ret = xor_str( "knife_tactical" );
			break;
		default:
			break;
		}

		return ret;
	}

	int c_skins::get_current_weapon_id( ) { 
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 42;

		auto weapon = g_local_player->weapon( );
		if( !weapon )
			return 42;

		return get_weapon_id( weapon );
	}

	int c_skins::get_weapon_id( game::cs_weapon_t* weapon ) { 

		if( !weapon )
			return -1;

		if( weapon->info( )->m_type == game::e_weapon_type::grenade )
			return 42;

		if( static_cast < std::ptrdiff_t > ( weapon->item_index( ) ) == 31 )
			return 42;

		if( weapon->is_knife( ) )
			return static_cast < int > ( weapon->item_index( ) );

		return std::clamp< int > ( static_cast < std::ptrdiff_t > ( weapon->item_index( ) ), 0, 64 );
	}

	int c_skins::get_skin( game::cs_weapon_t* weapon ) { 
		switch( weapon->item_index( ) ) { 
		case game::e_item_index::scar20:
			return m_cfg->m_cur_skin_scar20;
			break;
		case game::e_item_index::g3sg1:
			return m_cfg->m_cur_skin_g3sg1;
			break;
		case game::e_item_index::deagle:
			return m_cfg->m_cur_skin_deagle;
			break;
		case game::e_item_index::elite:
			return m_cfg->m_cur_skin_elite;
			break;
		case game::e_item_index::five_seven:
			return m_cfg->m_cur_skin_five_seven;
			break;
		case game::e_item_index::glock:
			return m_cfg->m_cur_skin_glock;
			break;
		case game::e_item_index::ak47:
			return m_cfg->m_cur_skin_ak47;
			break;
		case game::e_item_index::aug:
			return m_cfg->m_cur_skin_aug;
			break;
		case game::e_item_index::awp:
			return m_cfg->m_cur_skin_awp;
			break;
		case game::e_item_index::famas:
			return m_cfg->m_cur_skin_famas;
			break;
		case game::e_item_index::galil:
			return m_cfg->m_cur_skin_galil;
			break;
		case game::e_item_index::m249:
			return m_cfg->m_cur_skin_m249;
			break;
		case game::e_item_index::m4a4:
			return m_cfg->m_cur_skin_m4a4;
			break;
		case game::e_item_index::mac10:
			return m_cfg->m_cur_skin_mac10;
			break;
		case game::e_item_index::p90:
			return m_cfg->m_cur_skin_p90;
			break;
		case game::e_item_index::ump45:
			return m_cfg->m_cur_skin_ump45;
			break;
		case game::e_item_index::xm1014:
			return m_cfg->m_cur_skin_xm1014;
			break;
		case game::e_item_index::bizon:
			return m_cfg->m_cur_skin_bizon;
			break;
		case game::e_item_index::mag7:
			return m_cfg->m_cur_skin_mag7;
			break;
		case game::e_item_index::negev:
			return m_cfg->m_cur_skin_negev;
			break;
		case game::e_item_index::sawed_off:
			return m_cfg->m_cur_skin_sawedoff;
			break;
		case game::e_item_index::tec9:
			return m_cfg->m_cur_skin_tec9;
			break;
		case game::e_item_index::p2000:
			return m_cfg->m_cur_skin_p2000;
			break;
		case game::e_item_index::mp7:
			return m_cfg->m_cur_skin_mp7;
			break;
		case game::e_item_index::mp9:
			return m_cfg->m_cur_skin_mp9;
			break;
		case game::e_item_index::nova:
			return m_cfg->m_cur_skin_nova;
			break;
		case game::e_item_index::p250:
			return m_cfg->m_cur_skin_p250;
			break;
		case game::e_item_index::sg553:
			return m_cfg->m_cur_skin_sg553;
			break;
		case game::e_item_index::ssg08:
			return m_cfg->m_cur_skin_ssg08;
			break;
		case game::e_item_index::m4a1s:
			return m_cfg->m_cur_skin_m4_s;
			break;
		case game::e_item_index::usps:
			return m_cfg->m_cur_skin_usp_s;
			break;
		case game::e_item_index::cz75a:
			return m_cfg->m_cur_skin_cz75a;
			break;
		case game::e_item_index::revolver:
			return m_cfg->m_cur_skin_revolver;
			break;
		case game::e_item_index::knife_bayonet:
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_flip:
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_gut:
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_karambit:
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_m9_bayonet:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < game::e_item_index > ( 509 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_falchion:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < game::e_item_index > ( 514 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		case game::e_item_index::knife_butterfly:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < game::e_item_index > ( 516 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		default:
			return -1;
			break;
		}
	}

	void c_skins::override_weapon( game::cs_weapon_t* weapon, std::vector < game::cs_weapon_t* >& wpns ) { 
		if( !weapon )
			return;

		if( m_last_index.at( static_cast < int > ( weapon->item_index( ) ) ) != get_skin( weapon ) 
			&& get_skin( weapon ) != -1 ) { 
			m_skins.at( get_current_weapon_id( ) ) = get_skin( weapon );
			m_update = true;

			m_last_index.at( static_cast < int > ( weapon->item_index( ) ) ) = get_skin( weapon );
		}
		auto info = g_local_player->self( )->info( );

		if( !info.has_value( ) )
			return;

		if( m_skins.at( get_weapon_id( weapon ) ) <= 0 )
			return;

		wpns.push_back( weapon );

		if( m_skins.at( get_weapon_id( weapon ) ) != weapon->fallback_paint_kit( ) ) { 

			weapon->item_id_high( ) = -1;
			weapon->account_id( ) = info.value( ).m_xuid_low;
		}

		weapon->fallback_paint_kit( ) = m_skins.at( get_weapon_id( weapon ) );
		weapon->fallback_stattrak( ) = -1;
		weapon->fallback_seed( ) = g_ctx->addresses( ).m_random_int( 0, 1000 );
		weapon->fallback_wear( ) = 0.0000001f;

	}

	void c_skins::fix_seq( game::base_entity_t* entity ) { 
	}
	
	int c_skins::correct_skin_idx( int cur_idx ) { 
		if( cur_idx <= 2 )
			return cur_idx += 1;

		if( cur_idx == 3 )
			return 5;

		if( cur_idx == 4 )
			return 6;

		if( cur_idx <= 17 ) { 
			return cur_idx + 3;
		}

		if( cur_idx <= 25 ) { 
			return cur_idx + 7;
		}

		if( cur_idx <= 34 )
		return cur_idx + 10;


		if( cur_idx <= 37 )
		return cur_idx + 13;

		if( cur_idx == 38 )
		return cur_idx + 17;

		return -1;
	}

	void c_skins::handle_ctx( ) { 
		if( game::g_engine->in_game( ) 
			&& g_local_player->self( ) 
			&& g_local_player->self( )->alive( ) ) { 
			auto info = g_local_player->self( )->info( );

			if( !info.has_value( ) )
				return;

			std::vector < game::cs_weapon_t* > weapons_to_update{ };

			if( g_local_player->self( )->weapon( )
				&& g_local_player->self( )->weapon( )->info( ) ) { 
				for( int i{ }; i < game::g_entity_list->highest_ent_index( ); ++i ) { 
					auto entity = game::g_entity_list->get_entity( i );

					if( !entity )
						continue;

					auto client_class = entity->networkable( )->client_class( );

					if( !client_class )
						continue;

					if( static_cast < int > ( client_class->m_class_id ) == 118 ) { 
						auto cur_wpn = ( static_cast < game::cs_weapon_t* > ( entity ) )->get_wpn( );

						if( !cur_wpn
							|| !cur_wpn->info( ) 
							|| cur_wpn->info( )->m_type != game::e_weapon_type::knife
							|| cur_wpn->item_index( ) == game::e_item_index::taser )
							continue;

						if( !get_knife_index( ) )
							continue;

						if( cur_wpn->orig_owner_xuid_low( ) != info.value( ).m_xuid_low )
							continue;

						game::cs_weapon_t* const weapon_world_mdl = static_cast < game::cs_weapon_t* > ( game::g_entity_list->get_entity( cur_wpn->wpn_world_mdl( ) ) );

						if( !weapon_world_mdl )
							continue;

						entity->model_idx( ) = game::g_model_info->model_index( get_model_str( ).c_str( ) );
						weapon_world_mdl->model_idx( ) = game::g_model_info->model_index( get_world_mdl_str( ).c_str( ) );
					}

					else if( entity->is_base_combat_wpn( ) ) { 
						auto cur_wpn = ( static_cast < game::cs_weapon_t* > ( entity ) );

						if( !cur_wpn
							|| !cur_wpn->info( ) )
							continue;

						if( cur_wpn->orig_owner_xuid_low( ) != info.value( ).m_xuid_low )
							continue;

						if( cur_wpn->info( )->m_type == game::e_weapon_type::knife
							&& cur_wpn->item_index( ) != game::e_item_index::taser ) { 
							if( get_knife_index( ) ) { 
								cur_wpn->item_index( ) = static_cast < game::e_item_index > ( get_knife_index( ) );
								cur_wpn->model_idx( ) = game::g_model_info->model_index( get_model_str( ).c_str( ) );
								cur_wpn->entity_quality( ) = 3;
							}
						}

						if( cur_wpn->info( )->m_type != game::e_weapon_type::knife )
							override_weapon( cur_wpn, weapons_to_update );

					}
				}

				if( m_update 
					 && game::g_global_vars.get( )->m_cur_time >= m_update_time ) { 
					for( auto& wpn : weapons_to_update ) { 
						if( !wpn
							|| !wpn->is_base_combat_wpn( ) )
							continue;
						if( game::g_client_state.get( )->m_delta_tick != -1 ) { 
							wpn->custom_material_inited( ) = wpn->fallback_paint_kit( ) <= 0;
							wpn->custom_materials( ).remove_all( );
							wpn->custom_materials_2( ).remove_all( );

							size_t count = wpn->visual_data_processors( ).size( );
							for( size_t i{ }; i < count; ++i ) { 
								auto& elem = wpn->visual_data_processors( ).at( i );
								if( elem ) { 
									elem->unreference( );
									elem = nullptr;
								}
							}

							wpn->visual_data_processors( ).remove_all( );

							const auto networkable = ( std::uintptr_t ) wpn->networkable( );

							using fn_t = void( __thiscall* )( const std::uintptr_t, const int );

							( *reinterpret_cast< fn_t** > ( networkable ) )[ 7u ]( networkable, 0 );
							( *reinterpret_cast< fn_t** > ( networkable ) )[ 5u ]( networkable, 0 );

							game::hud_element_t* weapon_selection = game::g_hud->find_element( HASH( "SFWeaponSelection" ) );
							game::fn_show_and_update_selection( weapon_selection, 0, wpn, 0 );
						}
					}

					m_update = false;
					m_update_time = game::g_global_vars.get( )->m_cur_time + 1.f;
				}
			}
		}
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
				int text_length = m_fonts.m_font_for_fkin_name->CalcTextSizeA( 14.f, FLT_MAX, NULL, name.c_str( ) ).x + 5;

				g_render->text( name, sdk::vec2_t( g_visuals->screen_x - text_length, ( i * 18 ) + 5 ),
					sdk::col_t( 255, 255, 255, 220 ), g_misc->m_fonts.m_font_for_fkin_name, false, false, false, false, true );
			}
		}
	}

	void c_misc::draw_watermark( )
	{ 
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