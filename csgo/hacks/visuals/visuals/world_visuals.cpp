#include "../../../csgo.hpp"

namespace csgo::hacks {

	void c_visuals::tone_map_modulation( game::base_entity_t* ent ) { 
		if( !m_cfg->m_tone_map_modulation )
			return;

		game::tone_map_t* const entity = static_cast < game::tone_map_t* > ( ent );

		entity->use_custom_bloom_scale( ) = true;
		entity->custom_bloom_scale( ) = m_cfg->m_bloom * 0.01f;

		entity->user_custom_auto_exposure_min( ) = true;
		entity->auto_custom_exposure_min( ) = m_cfg->m_exposure * 0.001f;

		entity->user_custom_auto_exposure_max( ) = true;
		entity->auto_custom_exposure_max( ) = m_cfg->m_exposure * 0.001f;
	}

	void c_visuals::change_shadows( game::base_entity_t* entity ) { 

		auto ent = reinterpret_cast < game::cascade_light_t* > ( entity );
		static sdk::vec3_t backup = ent->shadow_dir( );
		static sdk::vec3_t shadow_dir{ };

		if( !m_cfg->m_shadows_modulation 
			&& game::g_engine->in_game( ) ) { 
			shadow_dir.x( ) = std::lerp( shadow_dir.x( ), backup.x( ), 5.f * game::g_global_vars.get( )->m_frame_time );
			shadow_dir.y( ) = std::lerp( shadow_dir.y( ), backup.y( ), 5.f * game::g_global_vars.get( )->m_frame_time );
			shadow_dir.z( ) = std::lerp( shadow_dir.z( ), backup.z( ), 5.f * game::g_global_vars.get( )->m_frame_time );
		}
		else {
			shadow_dir.x( ) = std::lerp( shadow_dir.x( ), m_cfg->m_x_dir, 2.0f * game::g_global_vars.get( )->m_frame_time );
			shadow_dir.y( ) = std::lerp( shadow_dir.y( ), m_cfg->m_y_dir, 2.0f * game::g_global_vars.get( )->m_frame_time );
			shadow_dir.z( ) = std::lerp( shadow_dir.z( ), m_cfg->m_z_dir, 2.0f * game::g_global_vars.get( )->m_frame_time );
		}

		ent->shadow_dir( ) = shadow_dir;
	}

	void c_visuals::skybox_changer( ) { 
		static auto sv_skyname = game::g_cvar->find_var( xor_str( "sv_skyname" ) );
		std::string skybox = sv_skyname->get_str( );

		using sky_box_fn = void( __fastcall* )( const char* );

		static auto fn = reinterpret_cast < sky_box_fn > ( g_ctx->addresses( ).m_sky_box );

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

	void c_visuals::removals( ) { 
		if( g_local_player->self( ) ) { 
			if( g_local_player->self( )->flash_dur( ) && m_cfg->m_removals & 4 )
				g_local_player->self( )->flash_dur( ) = 0.f;
		}

		static std::vector < std::string > smoke_str = { 
			xor_str( "particle/vistasmokev1/vistasmokev1_smokegrenade" ),
			xor_str( "particle/vistasmokev1/vistasmokev1_emods" ),
			xor_str( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ),
			xor_str( "particle/vistasmokev1/vistasmokev1_fire" ),
		};

		for( const auto& smoke_mat : smoke_str ) { 
			auto cur_mat = game::g_mat_sys->find_mat( smoke_mat.c_str( ), "Other textures" );

			if( !cur_mat )
				continue;

			cur_mat->set_flag( 1 << 2, m_cfg->m_removals & 8 );
		}

		if( m_cfg->m_removals & 8 ) { 
			if( *reinterpret_cast < std::int32_t* > ( *reinterpret_cast < std::uint32_t** > ( reinterpret_cast < std::uint32_t > ( g_ctx->addresses( ).m_smoke_count ) ) ) != 0 ) { 
				*reinterpret_cast < std::int32_t* > ( *reinterpret_cast < std::uint32_t** > ( reinterpret_cast < std::uint32_t > ( g_ctx->addresses( ).m_smoke_count ) ) ) = 0;
			}
		}

		**reinterpret_cast < bool** > ( reinterpret_cast < std::uint32_t > ( g_ctx->addresses( ).m_post_process ) ) = m_cfg->m_removals & 32;

		static auto cl_wpn_sway_amt = game::g_cvar->find_var( xor_str( "cl_wpn_sway_scale" ) );

		if( m_cfg->m_removals & 16 ) { 
			cl_wpn_sway_amt->set_float( 0.f );
		}
		else
			cl_wpn_sway_amt->set_float( 1.f );

		static auto blur_overlay = game::g_mat_sys->find_mat( xor_str( "dev/scope_bluroverlay" ), "Other textures" );
		static auto scope_dirt = game::g_mat_sys->find_mat( xor_str( "models/weapons/shared/scope/scope_lens_dirt" ), "Other textures" );

		blur_overlay->set_flag( ( 1 << 2 ), false );
		scope_dirt->set_flag( ( 1 << 2 ), false );

		if( g_local_player->self( )
			&& g_local_player->self( )->alive( ) ) { 
			auto wpn = g_local_player->self( )->weapon( );
			if( wpn ) { 
				if( wpn->info( )
					&& wpn->info( )->m_type == game::e_weapon_type::sniper ) { 
					if( g_local_player->self( )->scoped( ) ) { 
						if( m_cfg->m_removals & 1 ) { 
							blur_overlay->set_flag( ( 1 << 2 ), true );
							scope_dirt->set_flag( ( 1 << 2 ), true );
						}
					}
				}
			}
		}
	}

	void c_visuals::handle_world_drawings( ) { 
		for( std::ptrdiff_t i{ }; i <= game::g_entity_list->highest_ent_index( ); ++i ) { 
			game::base_entity_t* entity = game::g_entity_list->get_entity( i );

			if( !entity
				|| entity->networkable( )->dormant( )
				|| !entity->networkable( )->client_class( ) )
				continue;

			int class_id = entity->networkable( )->client_class( )->m_class_id;

			bool is_bomb = class_id == game::e_class_id::c_planted_c4
				|| class_id == game::e_class_id::c_c4;

			if( is_bomb )
				draw_c4( entity );

			if( entity->is_player( ) )
				continue;

			if( entity->is_base_combat_wpn( )
				&& !is_bomb ) { 
				game::cs_weapon_t* weapon = reinterpret_cast< game::cs_weapon_t* >( entity );
				sdk::vec3_t screen{ };

				sdk::vec3_t origin = weapon->origin( );

				if( origin.is_zero( ) 
					|| !g_render->world_to_screen( origin, screen ) )
					continue;

				int offset{ 0 };
					
				float dist_world = ( weapon->origin( ) - g_local_player->self( )->origin( ) ).length( );
				float alpha = std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );

				sdk::col_t clr = sdk::col_t( m_cfg->m_proj_wpn_clr[ 0 ] * 255.f, m_cfg->m_proj_wpn_clr[ 1 ] * 255.f, 
					m_cfg->m_proj_wpn_clr[ 2 ] * 255.f, m_cfg->m_proj_wpn_clr[ 3 ] * ( 225 * alpha ) );
				
				sdk::col_t clr_icon = sdk::col_t( m_cfg->m_proj_icon_clr[ 0 ] * 255.f, m_cfg->m_proj_icon_clr[ 1 ] * 255.f,
					m_cfg->m_proj_icon_clr[ 2 ] * 255.f, m_cfg->m_proj_icon_clr[ 3 ] * ( 225 * alpha ) );

				if( m_cfg->m_dropped_weapon_selection & 1 ) {
					g_render->text( get_weapon_name( weapon ), sdk::vec2_t( screen.x( ), screen.y( ) ),
						clr, g_misc->m_fonts.m_verdana, false, true, false, false, true );
					offset += 12;
				}

				if( m_cfg->m_dropped_weapon_selection & 2 ) {
					g_render->text( get_weapon_icon( weapon ), sdk::vec2_t( screen.x( ), screen.y( ) + offset ),
						clr_icon, g_misc->m_fonts.m_icon_font, false, true, false, false, true );
				}
			}

			switch( class_id ) { 
			case game::e_class_id::inferno:
				molotov_timer( entity );
				break;
			case game::e_class_id::smoke_grenade_projectile:
				smoke_timer( entity );
				break;
			case game::e_class_id::cascade_light:
				change_shadows( entity );
				break;
			case game::e_class_id::tone_map_controller:
				tone_map_modulation( entity );
				break;
			default:
				grenade_projectiles( entity );
				break;
			}
		}
	}

	void c_visuals::draw_c4( game::base_entity_t* entity ) { 
		auto owner = ( game::cs_player_t* )game::g_entity_list->get_entity( entity->m_owner_ent( ) );

		if( !entity )
			m_bomb_holder.clear( );
			return;

		if( ( owner 
			&& owner->is_player( ) ) ) {
			if( owner->is_player( ) 
				&& m_cfg->m_player_flags & 32 ) {
				if( entity->is_player( ) 
					&& entity->networkable( ) 
					&& entity != owner ) {
					m_bomb_holder.clear( );
				}
				else {
					m_bomb_holder.push_back( { owner } );
				}
			}
			else if( !owner->is_player( ) 
				|| !owner )
			{
				m_bomb_holder.clear( );
			}

			return;
		}

		if( ( ~m_cfg->m_draw_bomb_options & 1
			&& ~m_cfg->m_draw_bomb_options & 2 ) )
			return;

		int class_id = entity->networkable( )->client_class( )->m_class_id;
		bool is_planted = false/*class_id == game::e_class_id::c_planted_c4*/;

		sdk::vec3_t origin = entity->abs_origin( );

		sdk::vec3_t screen_origin{ };

		if( !g_render->world_to_screen( origin, screen_origin ) ||
			( entity->origin( ) - g_local_player->self( )->origin( ) ).length( ) > 2000.f )
			return;

		auto dist_world = ( entity->origin( ) - g_local_player->self( )->origin( ) ).length( );
		auto alpha = std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );

		int offset{ 0 };

		if( m_cfg->m_draw_bomb_options & 1 ) {
			sdk::col_t clr = is_planted ?
				sdk::col_t( m_cfg->m_draw_planted_c4_clr[ 0 ] * 255.f, m_cfg->m_draw_planted_c4_clr[ 1 ] * 255.f,
					m_cfg->m_draw_planted_c4_clr[ 2 ] * 255.f, ( m_cfg->m_draw_planted_c4_clr[ 3 ] * 255.f ) * alpha )
				:
				sdk::col_t( m_cfg->m_draw_c4_clr[ 0 ] * 255.f, m_cfg->m_draw_c4_clr[ 1 ] * 255.f,
					m_cfg->m_draw_c4_clr[ 2 ] * 255.f, ( m_cfg->m_draw_c4_clr[ 3 ] * 255.f ) * alpha );

			g_render->text( is_planted ? xor_str( "planted bomb" ) : xor_str( "bomb" ), sdk::vec2_t( screen_origin.x( ), screen_origin.y( ) ),
				clr, g_misc->m_fonts.m_verdana, false, true, true, false, true );
			offset += 12;
		}

		if( m_cfg->m_draw_bomb_options & 2 ) {
			sdk::col_t clr = is_planted ?
				sdk::col_t( m_cfg->m_draw_planted_c4_icon_clr[ 0 ] * 255.f, m_cfg->m_draw_planted_c4_icon_clr[ 1 ] * 255.f,
					m_cfg->m_draw_planted_c4_icon_clr[ 2 ] * 255.f, ( m_cfg->m_draw_planted_c4_icon_clr[ 3 ] * 255.f ) * alpha )
				:
				sdk::col_t( m_cfg->m_draw_c4_icon_clr[ 0 ] * 255.f, m_cfg->m_draw_c4_icon_clr[ 1 ] * 255.f,
					m_cfg->m_draw_c4_icon_clr[ 2 ] * 255.f, ( m_cfg->m_draw_c4_icon_clr[ 3 ] * 255.f ) * alpha );

			g_render->text( xor_str( "o" ), sdk::vec2_t( screen_origin.x( ), screen_origin.y( ) + offset ),
				clr, g_misc->m_fonts.m_icon_font, false, true, true, false, true );
		}
	}

	bool add_fire_information( game::inferno_t* inferno ) { 
		bool* fire_is_burning = inferno->fire_is_burning( );
		int* fire_x_delta = inferno->fire_x_delta( );
		int* fire_y_delta = inferno->fire_y_delta( );
		int* fire_z_delta = inferno->fire_z_delta( );
		int fire_count = inferno->fire_count( );

		auto& inf_info = g_visuals.get( )->inferno_information.emplace_back( );

		sdk::vec3_t average_vector = sdk::vec3_t( 0, 0, 0 );

		for( int i = 0; i <= fire_count; i++ ) { 
			if( !fire_is_burning[ i ] )
				continue;

			sdk::vec3_t fire_origin = sdk::vec3_t( fire_x_delta[ i ], fire_y_delta[ i ], fire_z_delta[ i ] );
			float delta = fire_origin.length( ) + 14.4f;
			if( delta > inf_info.range )
				inf_info.range = delta;

			average_vector += fire_origin;

			if( fire_origin == sdk::vec3_t( 0, 0, 0 ) )
				continue;

			inf_info.points.push_back( fire_origin + inferno->abs_origin( ) );
		}

		if( fire_count <= 1 )
			inf_info.origin = inferno->abs_origin( );
		else
			inf_info.origin = ( average_vector / fire_count ) + inferno->abs_origin( );

		return true;
	}

	void add_circle( sdk::vec3_t location, float radius, ImColor clr ) { 
		float step = ( sdk::pi * 2.0f ) / 60;
		std::vector< ImVec2 > points;
		for( float lat = 0.f; lat <= sdk::pi * 2.0f; lat += step )
		{ 
			const auto& point3d = sdk::vec3_t( sin( lat ), cos( lat ), 0.f ) * radius;
			sdk::vec3_t point2d;
			if( g_render->world_to_screen( location + point3d, point2d ) )
				points.push_back( ImVec2( point2d.x( ), point2d.y( ) ) );
		}
		g_render->m_draw_list->AddPolyline( points.data( ), points.size( ), clr, true, 0.5f );
	}

	void c_visuals::molotov_timer( game::base_entity_t* entity ) { 
		if( !m_cfg->m_molotov_timer )
			return;

		auto inferno = reinterpret_cast< game::inferno_t* > ( entity );
		auto origin = inferno->abs_origin( );

		sdk::vec3_t screen_origin{ };

		if( !g_render->world_to_screen( origin, screen_origin ) ||
			( inferno->origin( ) - g_local_player->self( )->origin( ) ).length( ) > 2000.f )
			return;

		auto dist_world = ( inferno->origin( ) - g_local_player->self( )->origin( ) ).length( );
		auto alpha = std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );

		auto spawn_time = inferno->get_spawn_time( );
		auto factor = ( spawn_time + game::inferno_t::get_expiry_time( ) - game::g_global_vars.get( )->m_cur_time ) / game::inferno_t::get_expiry_time( );

		const auto mod = std::clamp( 
			factor,
			0.f, 1.f
		 );

		if( add_fire_information( inferno ) ) { 
			if( !inferno_information.empty( ) ) { 
				inferno_info info = inferno_information.back( );

				add_circle( info.origin, info.range, ImColor( 1.f, 1.f, 1.f, ( ( 0.5f * mod ) * alpha ) ) );
			}
		}

		g_render->m_draw_list->AddCircleFilled( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 18.f, ImColor( 0.1f, 0.1f, 0.1f, ( 0.75f * mod ) * alpha ), 255.f );
		g_render->m_draw_list->AddCircle( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 18.f, ImColor( 0.f, 0.f, 0.f, ( 0.75f * mod ) * alpha ), 255.f );

		g_render->text( xor_str( "l" ), sdk::vec2_t( screen_origin.x( ) + 1, screen_origin.y( ) ),
			sdk::col_t( 255, 255, 255, ( 225 * mod ) * alpha ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
	}

	void c_visuals::smoke_timer( game::base_entity_t* entity ) { 
		if( !m_cfg->m_smoke_timer )	
			return;

		auto smoke = reinterpret_cast< game::smoke_t* > ( entity );

		if( !smoke->smoke_effect_tick_begin( ) || !smoke->did_smoke_effect( ) )
			return;

		auto origin = smoke->abs_origin( );

		sdk::vec3_t screen_origin{ };

		if( !g_render->world_to_screen( origin, screen_origin ) 
			|| ( smoke->origin( ) - g_local_player->self( )->origin( ) ).length( ) > 2000.f )
			return;

		auto dist_world = ( smoke->origin( ) - g_local_player->self( )->origin( ) ).length( );
		auto alpha = std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );

		auto spawn_time = game::to_time( smoke->smoke_effect_tick_begin( ) );
		auto factor = ( spawn_time + game::smoke_t::get_expiry_time( ) - game::g_global_vars.get( )->m_cur_time ) / 
			game::smoke_t::get_expiry_time( );

		const auto mod = std::clamp( 
			factor,
			0.f, 1.f
		 );

		g_render->m_draw_list->AddCircleFilled( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 18.f, ImColor( 0.1f, 0.1f, 0.1f, ( 0.75f * mod ) * alpha ), 255.f );
		g_render->m_draw_list->AddCircle( ImVec2( screen_origin.x( ), screen_origin.y( ) ), 18.f, ImColor( 0.f, 0.f, 0.f, ( 0.75f * mod ) * alpha ), 255.f );

		g_render->text( xor_str( "k" ), sdk::vec2_t( screen_origin.x( ) + 1, screen_origin.y( ) ),
			sdk::col_t( 255, 255, 255, ( 225 * mod ) * alpha ), g_misc->m_fonts.m_warning_icon_font, false, true, true, false, true );
	}

	void c_visuals::grenade_projectiles( game::base_entity_t* entity ) { 

		auto client_class = entity->networkable( )->client_class( );

		auto model = entity->renderable( )->model( );

		if( !model )
			return;

		if( client_class->m_class_id == game::e_class_id::base_cs_grenade_projectile 
			|| client_class->m_class_id == game::e_class_id::molotov_projectile )
		{ 
			auto name = ( std::string_view ) model->m_path;
			auto grenade_origin = entity->abs_origin( );
			auto grenade_position = sdk::vec3_t( );

			if( !g_render->world_to_screen( grenade_origin, grenade_position ) )
				return;

			std::string grenade_name, grenade_icon;

			if( name.find( "flashbang" ) != std::string::npos )
			{ 
				grenade_name = "flashbang";
				grenade_icon = "i";
			}
			else if( name.find( "smokegrenade" ) != std::string::npos )
			{ 
				grenade_name = "smoke";
				grenade_icon = "k";
			}
			else if( name.find( "incendiarygrenade" ) != std::string::npos )
			{ 
				grenade_name = "incendiary";
				grenade_icon = "n";
			}
			else if( name.find( "molotov" ) != std::string::npos )
			{ 
				grenade_name = "molotov";
				grenade_icon = "l";
			}
			else if( name.find( "fraggrenade" ) != std::string::npos )
			{ 
				grenade_name = "he grenade";
				grenade_icon = "j";
			}
			else
				return;

			auto dist_world = ( entity->origin( ) - g_local_player->self( )->origin( ) ).length( );
			auto alpha = std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 0.883f );

			int offset{ 0 };

			sdk::col_t clr = sdk::col_t( m_cfg->m_grenade_projectiles_clr[ 0 ] * 255.f, m_cfg->m_grenade_projectiles_clr[ 1 ] * 255.f,
				m_cfg->m_grenade_projectiles_clr[ 2 ] * 255.f, m_cfg->m_grenade_projectiles_clr[ 3 ] * ( 225 * alpha ) );

			sdk::col_t clr_icon = sdk::col_t( m_cfg->m_grenade_projectiles_icon_clr[ 0 ] * 255.f, m_cfg->m_grenade_projectiles_icon_clr[ 1 ] * 255.f, 
				m_cfg->m_grenade_projectiles_icon_clr[ 2 ] * 255.f, m_cfg->m_grenade_projectiles_icon_clr[ 3 ] * ( 225 * alpha ) );

			if( m_cfg->m_grenade_selection & 1 ) {
				g_render->text( grenade_name.c_str( ), sdk::vec2_t( grenade_position.x( ), grenade_position.y( ) ),
					clr, g_misc->m_fonts.m_verdana, false, true, false, false, true );
				offset += 12;
			}

			if( m_cfg->m_grenade_selection & 2 ) {
				g_render->text( grenade_icon.c_str( ), sdk::vec2_t( grenade_position.x( ), grenade_position.y( ) + offset ),
					clr_icon, g_misc->m_fonts.m_icon_font, false, true, false, false, true );
			}
		}
	}
}