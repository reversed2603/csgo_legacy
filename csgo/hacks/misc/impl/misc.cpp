#include "../../../csgo.hpp"

namespace csgo::hacks {

	void c_misc::clan_tag( ) const {

		using set_clan_tag_t = int( __fastcall* )( const char*, const char* );

		if( m_cfg->m_clan_tag ) {
			const auto i = ( valve::g_global_vars.get( )->m_tick_count / valve::to_ticks( 1.f ) ) % 34;
			if( i != m_cfg->m_prev_tag ) {
				hacks::g_misc->cfg( ).g_reset_tag = true;

				auto tag = "";

				switch( i )
				{
					case 0:
						tag = ( "xetra" );
						break;
					case 1:
						tag = ( "xetrahack" );
						break;
					case 2:
						tag = ( "xetrahack xd" );
						break;
					case 3:
						tag = ( "xetrahack" );
						break;
					case 4:
						tag = ( "xetrahack" );
						break;
					case 5:
						tag = ( "xetra" );
						break;
				}

				reinterpret_cast< set_clan_tag_t >( g_ctx->addresses( ).m_set_clan_tag )( tag, tag );

				hacks::g_misc->cfg( ).m_prev_tag = i;
			}
		}
		else if( hacks::g_misc->cfg( ).g_reset_tag ) {
		    hacks::g_misc->cfg( ).g_reset_tag = false;

			reinterpret_cast< set_clan_tag_t >( g_ctx->addresses( ).m_set_clan_tag )( "", "" );
		}
	}

	void c_misc::kill_feed( ) {
		if( !valve::g_engine->in_game( )
			|| !m_cfg->m_kill_feed )
			return;

		valve::kill_feed_t* feed = ( valve::kill_feed_t* ) valve::g_hud->find_element( HASH( "SFHudDeathNoticeAndBotStatus" ) );

		if( !feed )
			return;

		const auto size = feed->m_notices.size( );

		if( !size )
			return;

		for( std::size_t i{ }; i < size; ++i ) {
			valve::notice_text_t* notice = &feed->m_notices.at( i );

			if( notice->m_fade == 1.5f )
				notice->m_fade = std::numeric_limits < float >::max( );
		}
	}

	void c_misc::third_person( ) { 
		bool is_enable = g_key_binds->get_keybind_state( &m_cfg->m_third_person_key );
		static float distance = { m_cfg->m_third_person_dist };

		if( !is_enable ) {
			valve::g_input->m_camera_in_third_person = false;
			return;
		}

		if( !g_local_player || !g_local_player->self( ) )
			return;

		if( !g_local_player->self( )->alive( ) )
		{
			if( m_cfg->m_force_thirdperson_dead )
			{
				valve::g_input->m_camera_in_third_person = false;
				distance = 50.f;

				g_local_player->self( )->observer_mode( ) = 5;
			}

			return;
		}

		sdk::vec3_t eye_pos = g_ctx->shoot_pos( );
			
		sdk::qang_t view_angles = valve::g_engine->view_angles( );

		valve::g_input->m_camera_in_third_person = true;
		valve::g_input->m_camera_offset = sdk::vec3_t( view_angles.x( ), view_angles.y( ), distance );

		valve::trace_t trace;

		float extent = 12.f + m_cfg->m_third_person_dist / 4.8f - 18.f;

		sdk::vec3_t vec_forward = sdk::vec3_t( 0, 0, 0 );
		sdk::ang_vecs( sdk::qang_t( view_angles.x( ), view_angles.y( ), 0.0f ), &vec_forward, nullptr, nullptr );

		valve::trace_filter_world_only_t filter;

		valve::ray_t ray( eye_pos, eye_pos - vec_forward * valve::g_input->m_camera_offset.z( ) );
		ray.m_extents = sdk::vec3_t( extent, extent, extent );
		ray.m_ray = false;

		valve::g_engine_trace->trace_ray( ray, MASK_NPCWORLDSTATIC, &filter, &trace );

		valve::g_input->m_camera_offset.z( ) *= trace.m_frac;
	}

	int c_skins::get_knife_index( ) {
		switch( m_cfg->m_knife_type ) {
		case 0:
			return 0;
		case 1:
			return 500;
		case 2:
			return 505;
		case 3:
			return 506;
		case 4:
			return 507;
		case 5:
			return 508;
		case 6:
			return 509;
		}

		return 0;
	}

	const char* c_skins::get_world_mdl_str( ) {
		switch( m_cfg->m_knife_type ) {
		case 0:
			return 0;
		case 1:
			return xor_str( "models/weapons/w_knife_bayonet.mdl" );
		case 2:
			return xor_str( "models/weapons/w_knife_flip.mdl" );
		case 3:
			return xor_str( "models/weapons/w_knife_gut.mdl" );
		case 4:
			return xor_str( "models/weapons/w_knife_karam.mdl" );
		case 5:
			return xor_str( "models/weapons/w_knife_m9_bay.mdl" );
		case 6:
			return xor_str( "models/weapons/w_knife_tactical.mdl" );
		}

		return nullptr;
	}

	const char* c_skins::get_model_str( ) {
		switch( m_cfg->m_knife_type ) {
		case 0:
			return 0;
		case 1:
			return xor_str( "models/weapons/v_knife_bayonet.mdl" );
		case 2:
			return xor_str( "models/weapons/v_knife_flip.mdl" );
		case 3:
			return xor_str( "models/weapons/v_knife_gut.mdl" );
		case 4:
			return xor_str( "models/weapons/v_knife_karam.mdl" );
		case 5:
			return xor_str( "models/weapons/v_knife_m9_bay.mdl" );
		case 6:
			return xor_str( "models/weapons/v_knife_tactical.mdl" );
		}

		return nullptr;
	}

	const char* c_skins::get_glove_model( ) {
		switch( m_cfg->m_glove_type ) {
		case 5027:
			return xor_str( "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" );
		case 5030:
			return xor_str( "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" );
		case 5031:
			return xor_str( "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" );
		case 5032:
			return xor_str( "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" );
		case 5033:
			return xor_str( "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" );
		case 5034:
			return xor_str( "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" );
		}

		return nullptr;
	}

	const char* c_skins::get_killicon_str( ) {
		switch( m_cfg->m_knife_type ) {
		case 0:
			return 0;
		case 1:
			return xor_str( "bayonet" );
		case 2:
			return xor_str( "knife_flip" );
		case 3:
			return xor_str( "knife_gut" );
		case 4:
			return xor_str( "knife_karambit" );
		case 5:
			return xor_str( "knife_m9_bayonet" );
		case 6:
			return xor_str( "knife_tactical" );
		}

		return nullptr;
	}

	int c_skins::get_current_weapon_id( ) {
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 42;

		auto weapon = g_local_player->weapon( );
		if( !weapon )
			return 42;

		return get_weapon_id( weapon );
	}

	int c_skins::get_weapon_id( valve::cs_weapon_t* weapon ) {

		if( weapon->info( )->m_type == valve::e_weapon_type::grenade )
			return 42;

		if( static_cast < std::ptrdiff_t >( weapon->item_index( ) ) == 31 )
			return 42;

		if( weapon->is_knife( ) )
			return static_cast < int >( weapon->item_index( ) );

		return std::clamp< int >( static_cast < std::ptrdiff_t >( weapon->item_index( ) ), 0, 64 );
	}

	int c_skins::get_skin( valve::cs_weapon_t* weapon ) {
		switch( weapon->item_index( ) ) {
		case valve::e_item_index::scar20:
			return m_cfg->m_cur_skin_scar20;
			break;
		case valve::e_item_index::g3sg1:
			return m_cfg->m_cur_skin_g3sg1;
			break;
		case valve::e_item_index::deagle:
			return m_cfg->m_cur_skin_deagle;
			break;
		case valve::e_item_index::elite:
			return m_cfg->m_cur_skin_elite;
			break;
		case valve::e_item_index::five_seven:
			return m_cfg->m_cur_skin_five_seven;
			break;
		case valve::e_item_index::glock:
			return m_cfg->m_cur_skin_glock;
			break;
		case valve::e_item_index::ak47:
			return m_cfg->m_cur_skin_ak47;
			break;
		case valve::e_item_index::aug:
			return m_cfg->m_cur_skin_aug;
			break;
		case valve::e_item_index::awp:
			return m_cfg->m_cur_skin_awp;
			break;
		case valve::e_item_index::famas:
			return m_cfg->m_cur_skin_famas;
			break;
		case valve::e_item_index::galil:
			return m_cfg->m_cur_skin_galil;
			break;
		case valve::e_item_index::m249:
			return m_cfg->m_cur_skin_m249;
			break;
		case valve::e_item_index::m4a4:
			return m_cfg->m_cur_skin_m4a4;
			break;
		case valve::e_item_index::mac10:
			return m_cfg->m_cur_skin_mac10;
			break;
		case valve::e_item_index::p90:
			return m_cfg->m_cur_skin_p90;
			break;
		case valve::e_item_index::ump45:
			return m_cfg->m_cur_skin_ump45;
			break;
		case valve::e_item_index::xm1014:
			return m_cfg->m_cur_skin_xm1014;
			break;
		case valve::e_item_index::bizon:
			return m_cfg->m_cur_skin_bizon;
			break;
		case valve::e_item_index::mag7:
			return m_cfg->m_cur_skin_mag7;
			break;
		case valve::e_item_index::negev:
			return m_cfg->m_cur_skin_negev;
			break;
		case valve::e_item_index::sawed_off:
			return m_cfg->m_cur_skin_sawedoff;
			break;
		case valve::e_item_index::tec9:
			return m_cfg->m_cur_skin_tec9;
			break;
		case valve::e_item_index::p2000:
			return m_cfg->m_cur_skin_p2000;
			break;
		case valve::e_item_index::mp7:
			return m_cfg->m_cur_skin_mp7;
			break;
		case valve::e_item_index::mp9:
			return m_cfg->m_cur_skin_mp9;
			break;
		case valve::e_item_index::nova:
			return m_cfg->m_cur_skin_nova;
			break;
		case valve::e_item_index::p250:
			return m_cfg->m_cur_skin_p250;
			break;
		case valve::e_item_index::sg553:
			return m_cfg->m_cur_skin_sg553;
			break;
		case valve::e_item_index::ssg08:
			return m_cfg->m_cur_skin_ssg08;
			break;
		case valve::e_item_index::m4a1s:
			return m_cfg->m_cur_skin_m4_s;
			break;
		case valve::e_item_index::usps:
			return m_cfg->m_cur_skin_usp_s;
			break;
		case valve::e_item_index::cz75a:
			return m_cfg->m_cur_skin_cz75a;
			break;
		case valve::e_item_index::revolver:
			return m_cfg->m_cur_skin_revolver;
			break;
		case valve::e_item_index::knife_bayonet:
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_flip:
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_gut:
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_karambit:
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_m9_bayonet:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < valve::e_item_index >( 509 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_falchion:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < valve::e_item_index >( 514 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		case valve::e_item_index::knife_butterfly:
			return m_cfg->m_cur_skin_knifes;
			break;
		case static_cast < valve::e_item_index >( 516 ):
			return m_cfg->m_cur_skin_knifes;
			break;
		default:
			return -1;
			break;
		}
	}

	void c_skins::override_weapon( valve::cs_weapon_t* weapon, std::vector < valve::cs_weapon_t* >& wpns ) {
		if( !weapon )
			return;

		if( m_last_index.at( static_cast < int >( weapon->item_index( ) ) ) != get_skin( weapon ) 
			&& get_skin( weapon ) != -1 ) {
			m_skins.at( get_current_weapon_id( ) ) = get_skin( weapon );
			m_update = true;

			m_last_index.at( static_cast < int >( weapon->item_index( ) ) ) = get_skin( weapon );
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

	void c_skins::fix_seq( valve::base_entity_t* entity ) {
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
	}

	void c_skins::handle_ctx( ) {
		if( valve::g_engine->in_game( ) 
			&& g_local_player->self( ) 
			&& g_local_player->self( )->alive( ) ) {
			auto info = g_local_player->self( )->info( );

			if( !info.has_value( ) )
				return;

			std::vector < valve::cs_weapon_t* > weapons_to_update{ };

			if( g_local_player->self( )->weapon( )
				&& g_local_player->self( )->weapon( )->info( ) ) {
				for( int i{ }; i < valve::g_entity_list->highest_ent_index( ); ++i ) {
					auto entity = valve::g_entity_list->get_entity( i );

					if( !entity )
						continue;

					auto client_class = entity->networkable( )->client_class( );

					if( !client_class )
						continue;

					if( static_cast < int >( client_class->m_class_id ) == 118 ) {
						auto cur_wpn = ( static_cast < valve::cs_weapon_t* >( entity ) )->get_wpn( );

						if( !cur_wpn
							|| !cur_wpn->info( ) 
							|| cur_wpn->info( )->m_type != valve::e_weapon_type::knife
							|| cur_wpn->item_index( ) == valve::e_item_index::taser )
							continue;

						if( !get_knife_index( ) )
							continue;

						if( cur_wpn->orig_owner_xuid_low( ) != info.value( ).m_xuid_low )
							continue;

						valve::cs_weapon_t* const weapon_world_mdl = static_cast < valve::cs_weapon_t* >( valve::g_entity_list->get_entity( cur_wpn->wpn_world_mdl( ) ) );

						if( !weapon_world_mdl )
							continue;

						entity->model_idx( ) = valve::g_model_info->model_index( get_model_str( ) );
						weapon_world_mdl->model_idx( ) = valve::g_model_info->model_index( get_world_mdl_str( ) );
					}

					else if( entity->is_base_combat_wpn( ) ) {
						auto cur_wpn = ( static_cast < valve::cs_weapon_t* >( entity ) );

						if( !cur_wpn
							|| !cur_wpn->info( ) )
							continue;

						if( cur_wpn->orig_owner_xuid_low( ) != info.value( ).m_xuid_low )
							continue;

						if( cur_wpn->info( )->m_type == valve::e_weapon_type::knife
							&& cur_wpn->item_index( ) != valve::e_item_index::taser ) {
							if( get_knife_index( ) ) {
								cur_wpn->item_index( ) = static_cast < valve::e_item_index >( get_knife_index( ) );
								cur_wpn->model_idx( ) = valve::g_model_info->model_index( get_model_str( ) );
								cur_wpn->entity_quality( ) = 3;
							}
						}

						if( cur_wpn->info( )->m_type != valve::e_weapon_type::knife )
							override_weapon( cur_wpn, weapons_to_update );

					}
				}
				if( m_update 
					 && valve::g_global_vars.get( )->m_cur_time >= m_update_time ) {
					for( auto& wpn : weapons_to_update ) {
						if( !wpn
							|| !wpn->is_base_combat_wpn( ) )
							continue;
						if( valve::g_client_state.get( )->m_delta_tick != -1 ) {
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

							( *reinterpret_cast< fn_t** >( networkable ) )[ 7u ]( networkable, 0 );
							( *reinterpret_cast< fn_t** >( networkable ) )[ 5u ]( networkable, 0 );

							valve::hud_element_t* weapon_selection = valve::g_hud->find_element( HASH( "SFWeaponSelection" ) );
							valve::fn_show_and_update_selection( weapon_selection, 0, wpn, 0 );
						}
					}

					m_update = false;
					m_update_time = valve::g_global_vars.get( )->m_cur_time + 1.f;
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
				std::string buy { };

				switch( m_cfg->m_buy_bot_snipers )
				{
				case 1:
					buy += xor_str( "buy g3sg1; " );
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
					buy += xor_str( "buy elite; " );
					break;
				case 2:
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

				valve::g_engine->exec_cmd( buy.data( ) );
			}
		}
	}

	void c_misc::draw_spectators( bool im_gui_suck )
	{

		if( !m_cfg->m_spectators )
			return;

		std::vector < std::string > spectator_list;

		int offset{ 0 };
		static int whole_shit_alphas{ 255 };
		static int spectators_background{ 175 };

		if( g_local_player && g_local_player->self( ) && g_local_player->self( )->alive( ) )
		{
			for( int i = 1; i <= valve::g_global_vars.get( )->m_max_clients; i++ )
			{
				auto player = ( valve::cs_player_t* )valve::g_entity_list->get_entity( i );

				if( !player || player->alive( ) || !player->is_player( ) )
					continue;

				auto observer_target = valve::g_entity_list->get_entity( player->observer_target_handle( ) );

				if( !observer_target || observer_target != g_local_player->self( ) )
					continue;

				valve::player_info_t info;

			    valve::g_engine->get_player_info( player->networkable( )->index( ), &info );

				spectator_list.emplace_back( ( std::string )( info.m_name ) );
			}
		}

		if( spectator_list.empty( ) ) {
			whole_shit_alphas = std::lerp( whole_shit_alphas, 0, 15.f * valve::g_global_vars.get( )->m_frame_time );
			spectators_background = std::lerp( spectators_background, 0, 15.f * valve::g_global_vars.get( )->m_frame_time );
		}
		else {
			whole_shit_alphas = std::lerp( whole_shit_alphas, 255, 15.f * valve::g_global_vars.get( )->m_frame_time );
			spectators_background = std::lerp( spectators_background, 175, 15.f * valve::g_global_vars.get( )->m_frame_time );
		}

		whole_shit_alphas = std::clamp( whole_shit_alphas, 0, 255 ); // ayo
		spectators_background = std::clamp( spectators_background, 0, 175 ); // ayo

		if( spectators_background < 5.f )
			return;

		ImGui::Begin( "Hello, world!!!!!!!!!!!!!!!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar );
		{
			ImGui::PushFont( hacks::g_misc->m_fonts.m_xiaomi );
			ImVec2 pos;
			ImDrawList* draw;
			pos = ImGui::GetWindowPos( );
			draw = ImGui::GetWindowDrawList( );

			ImGui::SetWindowSize( ImVec2( 200, 200 ) );

			for( const auto& it : spectator_list ) {
				draw->AddText( ImVec2( pos.x + 5, pos.y + 25 + offset ), ImColor( 255, 255, 255, whole_shit_alphas ), it.c_str( ) );
				offset += 16;
			}

			draw->AddRectFilled( ImVec2( pos.x, pos.y ), ImVec2( pos.x + 200, pos.y + 23 ), ImColor( 25, 25, 25, spectators_background ), 5.f );
			draw->AddText( ImVec2( pos.x + 76, pos.y + 5 ), ImColor( 255, 255, 255, whole_shit_alphas ), "spectators" );

			ImGui::PopFont( );
		}
		ImGui::End( );
	}

	void c_misc::draw_watermark( )
	{

		return;

		std::string water_mark = xor_str( "xetra_hack |" );
		auto net_channel = valve::g_engine->net_channel_info( );

		if( valve::g_engine->in_game( ) )
		{
			if( net_channel )
			{
				auto latency = net_channel->avg_latency( 0 );

				if( latency )
				{
					static auto cl_updaterate = valve::g_cvar->find_var( xor_str( "cl_updaterate" ) );
					latency -= 0.5f / cl_updaterate->get_float( );
				}

				water_mark += std::string( " ms: " ) + std::to_string( ( int )( std::max( 0.0f, latency ) * 1000.0f ) ) + " | ";
			}
		}
		else
			water_mark += xor_str( " disconnected |" );

		int screen_size_x, screen_size_y;
		valve::g_engine->get_screen_size( screen_size_x, screen_size_y );

		static float current_time = 0.f;
		static int last_fps = ( int )( 1.0f / valve::g_global_vars.get( )->m_abs_frame_time );

		if( current_time > 0.5f )
		{ 
			last_fps = ( int )( 1.0f / valve::g_global_vars.get( )->m_abs_frame_time );
			current_time = 0.f;
		}

		current_time += valve::g_global_vars.get( )->m_abs_frame_time;

		water_mark += xor_str( "FPS: " ) + std::to_string( last_fps );

		int text_length = m_fonts.m_xiaomi->CalcTextSizeA( 15.f, FLT_MAX, NULL, water_mark.c_str( ) ).x + 25;

		ImGui::GetForegroundDrawList( )->AddRectFilled( ImVec2( screen_size_x - text_length, 11 ), ImVec2( screen_size_x - 12, 32 ), sdk::col_t( 25, 25, 25, 175 ).hex( ), 5.f );

		ImGui::PushFont( m_fonts.m_xiaomi );
		ImGui::GetForegroundDrawList( )->AddText( ImVec2( screen_size_x - text_length + 5, 14 ), ImColor( 255, 255, 255, 255 ), water_mark.c_str( ) );
		ImGui::PopFont( );
	}
}