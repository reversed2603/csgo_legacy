#include "../../csgo.hpp"

void* dm_ecx = 0;
std::map < int, uintptr_t > dm_ctx = { };
std::map < int, const csgo::game::draw_model_state_t* > dm_state = { };
std::map < int, const csgo::game::model_render_info_t* > dm_info = { };

namespace csgo::hacks {
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

		m_reg_mat = game::g_mat_sys->find_mat( xor_str( "debug/debugambientcube" ), xor_str( "Model textures" ) );
		m_reg_mat->increment_ref_count( );
		m_flat_mat = game::g_mat_sys->find_mat( xor_str( "debug/debugdrawflat" ), xor_str( "Model textures" ) );
		m_flat_mat->increment_ref_count( );
		m_glow_mat = game::g_mat_sys->find_mat( xor_str( "desync_glow" ), xor_str( "Model textures" ) );
		m_glow_mat->increment_ref_count( );
		m_glow_overlay_mat = game::g_mat_sys->find_mat( xor_str( "dev/glow_armsrace" ), nullptr );
		m_glow_overlay_mat->increment_ref_count( );
		m_metallic_mat = game::g_mat_sys->find_mat( xor_str( "metallic_chams" ), xor_str( "Model textures" ) );
		m_metallic_mat->increment_ref_count( );
	}

	void c_chams::override_mat( int mat_type, sdk::col_t col, bool ignore_z, bool is_overlay ) { 
		game::c_material* mat { };
		
		if( is_overlay ) {
			switch( mat_type ) {
			case 0:
				mat = m_glow_mat;
				break;
			case 1:
				mat = m_glow_overlay_mat;
				break;
			}
		}
		else {
			switch( mat_type ) { 
			case 0:
				mat = m_reg_mat;
				break;
			case 1:
				mat = m_flat_mat;
				break;
				break;
			case 2:
				mat = m_metallic_mat;
				break;
			}
		}

		if( !mat )
			return;

		if( ( is_overlay && ( mat_type == 0 || mat_type == 1 ) ) || ( mat_type == 2 
			|| mat_type == 3 ) ) { 
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

		game::g_studio_render->forced_mat_override( mat );
	}

	std::optional< game::bones_t > c_chams::try_to_lerp_bones( hacks::player_entry_t entry ) const { 
		if( entry.m_lag_records.size( ) < 2 )
			return std::nullopt;

		auto& front = entry.m_lag_records.front( );
		
		if( front->m_broke_lc 
			|| front->m_dormant )
			return std::nullopt;

		for( auto it = entry.m_lag_records.begin( ); 
			it != entry.m_lag_records.end( ); 
			++it ) { 
			lag_record_t* last_first{ nullptr };
			lag_record_t* last_second{ nullptr };
			
			if( it->get( )->valid( ) && it + 1 != entry.m_lag_records.end( )  
				&& !( it + 1 )->get( )->valid( ) 
				&& !( it + 1 )->get( )->m_dormant ) {
				last_first = ( it + 1 )->get( );
				last_second = ( it )->get( );
			}

			if( !last_first
				|| !last_second )
				continue;

			const auto& first_invalid = last_first;
			const auto& last_invalid = last_second;

			if( !last_invalid
				|| !first_invalid
				|| ( last_invalid->m_origin - front->m_origin ).length( ) < 1.0f )
		 		continue;

			g_chams->m_total_distance = ( entry.m_player->abs_origin( ) - last_first->m_abs_origin ).length( );

			const auto curtime = game::g_global_vars.get( )->m_cur_time;

			auto delta = 1.f - ( curtime - last_invalid->m_interp_time ) / ( last_invalid->m_sim_time - first_invalid->m_sim_time );
			if( delta < 0.f 
				|| delta > 1.f )
				last_invalid->m_interp_time = curtime;

			delta = 1.f - ( curtime - last_invalid->m_interp_time ) / ( last_invalid->m_sim_time - first_invalid->m_sim_time );

			const auto lerp = sdk::lerp( last_invalid->m_origin, first_invalid->m_origin, std::clamp( delta, 0.f, 1.f ) );

			auto lerped_bones = last_second->m_bones;

			const auto origin_delta = lerp - last_second->m_origin;

			for( std::size_t i { }; i < lerped_bones.size( ); ++i ) { 
				lerped_bones[ i ][ 0 ][ 3 ] += origin_delta.x( );
				lerped_bones[ i ][ 1 ][ 3 ] += origin_delta.y( );
				lerped_bones[ i ][ 2 ][ 3 ] += origin_delta.z( );
			}

			return lerped_bones;
		}

		return std::nullopt;
	}

	bool c_chams::draw_mdl( void* ecx, uintptr_t ctx, const game::draw_model_state_t& state, const game::model_render_info_t& info, sdk::mat3x4_t* bone ) { 
		auto entity = game::g_entity_list->get_entity( info.m_index );

		if( entity ) {
			if( !entity->networkable( ) )
				return false;
			if( info.m_model && strstr( info.m_model->m_path, xor_str( "models/player" ) ) != nullptr ) { 
				dm_ecx = ecx;
				dm_ctx [ info.m_index ] = ctx;
				dm_state [ info.m_index ] = &state;
				dm_info [ info.m_index ] = &info;

				if( entity->is_player( ) ) { 
					auto player = static_cast < game::cs_player_t* > ( entity );
					bool local = g_local_player->self( ) && player == g_local_player->self( );
					bool enemy = g_local_player->self( ) && !player->friendly( g_local_player->self( ) );

					if( !player->alive( ) && !m_cfg->m_ragdoll_chams
						|| ( !m_cfg->m_ragdoll_chams 
							&& entity->networkable( )->client_class( )->m_class_id == 37 
							&& enemy ) )
						return false;

					if( enemy && ( m_cfg->m_enemy_chams || m_cfg->m_history_chams || m_cfg->m_ragdoll_chams && entity->networkable( )->client_class( )->m_class_id == 37 ) ) {
						float alpha = g_dormancy->m_data[ player->networkable( )->index( ) ].m_alpha;

						if( m_cfg->m_history_chams ) { 
							const auto& entry = g_lag_comp->entry( player->networkable( )->index( ) - 1 );
							auto lerp_bones = try_to_lerp_bones( entry );
							auto max_distance = 24.f;

							if( !entry.m_lag_records.empty( ) ) {
								float distance = g_chams->m_total_distance;
								float total_alpha = ( m_cfg->m_history_clr[ 3 ] * alpha );

								if( distance <= max_distance )
									total_alpha *= distance / max_distance; // measure alpha lol

								if( lerp_bones.has_value( ) ) { 
									override_mat( m_cfg->m_history_chams_type, sdk::col_t( m_cfg->m_history_clr[ 0 ] * 255.f, m_cfg->m_history_clr[ 1 ] * 255.f, m_cfg->m_history_clr[ 2 ] * 255.f, total_alpha ), true );
									hooks::orig_draw_mdl_exec( ecx, ctx, state, info, lerp_bones.value( ).data( ) );
									game::g_studio_render->forced_mat_override( nullptr );
								}
							}
						}

						if( m_cfg->m_enemy_chams ) { 
							if( m_cfg->m_enemy_chams_invisible ) { 
								override_mat( m_cfg->m_invisible_enemy_chams_type,
									sdk::col_t( m_cfg->m_invisible_enemy_clr[ 0 ] * 255.f, m_cfg->m_invisible_enemy_clr[ 1 ] * 255.f, m_cfg->m_invisible_enemy_clr[ 2 ] * 255.f, m_cfg->m_invisible_enemy_clr[ 3 ] * alpha ), true );

								hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
								game::g_studio_render->forced_mat_override( nullptr );
							}

							if( m_cfg->m_enemy_chams_overlay_invisible ) { 
								override_mat( m_cfg->m_invisible_enemy_chams_overlay_type,
									sdk::col_t( m_cfg->m_invisible_enemy_clr_overlay[ 0 ] * 255.f, m_cfg->m_invisible_enemy_clr_overlay[ 1 ] * 255.f,
										m_cfg->m_invisible_enemy_clr_overlay[ 2 ] * 255.f, m_cfg->m_invisible_enemy_clr_overlay[ 3 ] * alpha ), true, true );

								hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
								game::g_studio_render->forced_mat_override( nullptr );
							}

							override_mat( m_cfg->m_enemy_chams_type, sdk::col_t( m_cfg->m_enemy_clr[ 0 ] * 255.f, m_cfg->m_enemy_clr[ 1 ] * 255.f, m_cfg->m_enemy_clr[ 2 ] * 255.f, m_cfg->m_enemy_clr[ 3 ] * alpha ), false );
							hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
							game::g_studio_render->forced_mat_override( nullptr );

							if( m_cfg->m_enemy_chams_overlay ) { 
								override_mat( m_cfg->m_enemy_chams_overlay_type,
									sdk::col_t( m_cfg->m_enemy_clr_overlay[ 0 ] * 255.f, m_cfg->m_enemy_clr_overlay[ 1 ] * 255.f,
										m_cfg->m_enemy_clr_overlay[ 2 ] * 255.f, m_cfg->m_enemy_clr_overlay[ 3 ] * alpha ), false, true );
											
								hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
								game::g_studio_render->forced_mat_override( nullptr );
							}
						}

						return true;
					}

					if( local 
						&& g_local_player->self( )->alive( ) ) { 
						static float blend_main{ 0.f };
						if( g_visuals->cfg( ).m_blend_in_scope && game::g_input->m_camera_in_third_person ) { 
							if( g_local_player->self( )->scoped( ) ) { 
								blend_main = std::lerp( blend_main, ( g_visuals->cfg( ).m_blend_in_scope_val / 100.f ), 15.f * game::g_global_vars.get( )->m_frame_time );
								game::g_render_view->set_blend( blend_main );
							}
							else if( blend_main < 1.f ) { 
								blend_main = std::lerp( blend_main, 1.f, 15.f * game::g_global_vars.get( )->m_frame_time );

								if( blend_main > 0.95f ) // lol retarded fix idc
									blend_main = 1.f;

								game::g_render_view->set_blend( blend_main );
							}
						}
								
						if( m_cfg->m_local_chams ) {
							override_mat( m_cfg->m_local_chams_type,
								sdk::col_t( m_cfg->m_local_clr[ 0 ] * 255.f, m_cfg->m_local_clr[ 1 ] * 255.f, m_cfg->m_local_clr[ 2 ] * 255.f, m_cfg->m_local_clr[ 3 ] * 255 ),
								false );
						}

						hooks::orig_draw_mdl_exec( ecx, ctx, state, info, g_ctx->anim_data( ).m_local_data.m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );

						if( m_cfg->m_local_chams_overlay ) {
							override_mat( m_cfg->m_local_overlay_type,
								sdk::col_t( m_cfg->m_local_overlay_clr[ 0 ] * 255.f, m_cfg->m_local_overlay_clr[ 1 ] * 255.f,
									m_cfg->m_local_overlay_clr[ 2 ] * 255.f, m_cfg->m_local_overlay_clr[ 3 ] * 255 ), false, true );
											
							hooks::orig_draw_mdl_exec( ecx, ctx, state, info, g_ctx->anim_data( ).m_local_data.m_bones.data( ) );
							game::g_studio_render->forced_mat_override( nullptr );
						}
						return true;
					}
				}
			}
		}
		else if( g_local_player->self( )
			&& strstr( info.m_model->m_path, xor_str( "weapons/v_" ) ) != nullptr
			&& strstr( info.m_model->m_path, xor_str( "sleeve" ) ) == nullptr 
			&& strstr( info.m_model->m_path, xor_str( "arms" ) ) == nullptr ) { 

			if( m_cfg->m_wpn_chams ) {
				override_mat( m_cfg->m_wpn_chams_type,
					sdk::col_t( m_cfg->m_wpn_clr[ 0 ] * 255.f, m_cfg->m_wpn_clr[ 1 ] * 255.f, m_cfg->m_wpn_clr[ 2 ] * 255.f, m_cfg->m_wpn_clr[ 3 ] * 255 ),
					false );
			}

			hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
			game::g_studio_render->forced_mat_override( nullptr );
			return true;
		}
		else if( g_local_player->self( )
			&& strstr( info.m_model->m_path, xor_str( "arms" ) ) != nullptr ) { 

			if( m_cfg->m_arms_chams ) {
				override_mat( m_cfg->m_arms_chams_type,
					sdk::col_t( m_cfg->m_arms_clr[ 0 ] * 255.f, m_cfg->m_arms_clr[ 1 ] * 255.f, m_cfg->m_arms_clr[ 2 ] * 255.f, m_cfg->m_arms_clr[ 3 ] * 255 ),
					false );
			}

			hooks::orig_draw_mdl_exec( ecx, ctx, state, info, bone );
			game::g_studio_render->forced_mat_override( nullptr );
			return true;
		}

		return false;
	}

	void c_visuals::draw_shot_mdl( ) { 
		if( !game::g_engine->in_game( ) )
			return m_shot_mdls.clear( );

		if( m_shot_mdls.empty( ) )
			return;

		const auto context = game::g_mat_sys->render_context( );
		if( !context )
			return;

		auto& cfg = g_chams->cfg( );

		for( auto i = m_shot_mdls.begin( ); i != m_shot_mdls.end( ); ) { 
			const float delta = ( i->m_time + 1.5f ) - game::g_global_vars.get( )->m_real_time;

			float alpha = 255.f * ( g_dormancy->m_data.at( i->m_player_index ).m_alpha / 255.f );

			alpha = std::clamp( alpha, 0.f, 255.f );

			/* checking if player is alive will fix problems with when player has appeared death chams still stay there... */
			if( i->m_is_death ? ( alpha <= 5.f || i->m_player->alive( ) ) : delta <= 0.f ) { 
				i = m_shot_mdls.erase( i );
				continue;
			}

			if( !i->m_bones.data( ) )
				continue;

			if( i->m_is_death ) { 
				if( cfg.m_enemy_chams ) { 
					if( cfg.m_enemy_chams_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_enemy_chams_type,
							sdk::col_t( cfg.m_invisible_enemy_clr[ 0 ] * 255.f, cfg.m_invisible_enemy_clr[ 1 ] * 255.f, cfg.m_invisible_enemy_clr[ 2 ] * 255.f, cfg.m_invisible_enemy_clr[ 3 ] * alpha ), true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}

					if( cfg.m_enemy_chams_overlay_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_enemy_chams_overlay_type,
							sdk::col_t( cfg.m_invisible_enemy_clr_overlay[ 0 ] * 255.f, cfg.m_invisible_enemy_clr_overlay[ 1 ] * 255.f,
								cfg.m_invisible_enemy_clr_overlay[ 2 ] * 255.f, cfg.m_invisible_enemy_clr_overlay[ 3 ] * alpha ), true, true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}

					g_chams->override_mat( cfg.m_enemy_chams_type, sdk::col_t( cfg.m_enemy_clr[ 0 ] * 255.f, cfg.m_enemy_clr[ 1 ] * 255.f, cfg.m_enemy_clr[ 2 ] * 255.f, cfg.m_enemy_clr[ 3 ] * alpha ), false );
					hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
					game::g_studio_render->forced_mat_override( nullptr );

					if( cfg.m_enemy_chams_overlay ) { 
						g_chams->override_mat( cfg.m_enemy_chams_overlay_type,
							sdk::col_t( cfg.m_enemy_clr_overlay[ 0 ] * 255.f, cfg.m_enemy_clr_overlay[ 1 ] * 255.f,
								cfg.m_enemy_clr_overlay[ 2 ] * 255.f, cfg.m_enemy_clr_overlay[ 3 ] * alpha ), false, true );
											
						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}
				}
			}
			else {
				if( cfg.m_shot_chams ) {
					if( cfg.m_shot_chams_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_shot_chams_type,
							sdk::col_t( cfg.m_invisible_shot_clr[ 0 ] * 255.f, cfg.m_invisible_shot_clr[ 1 ] * 255.f, cfg.m_invisible_shot_clr[ 2 ] * 255.f, cfg.m_invisible_shot_clr[ 3 ] * ( 255.f * delta ) ), true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}

					if( cfg.m_shot_chams_overlay_invisible ) { 
						g_chams->override_mat( cfg.m_invisible_shot_chams_overlay_type,
							sdk::col_t( cfg.m_invisible_shot_clr_overlay[ 0 ] * 255.f, cfg.m_invisible_shot_clr_overlay[ 1 ] * 255.f,
								cfg.m_invisible_shot_clr_overlay[ 2 ] * 255.f, cfg.m_invisible_shot_clr_overlay[ 3 ] * ( 255.f * delta ) ), true, true );

						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}
 
					g_chams->override_mat( cfg.m_shot_chams_type, sdk::col_t( cfg.m_shot_clr[ 0 ] * 255.f, cfg.m_shot_clr[ 1 ] * 255.f, cfg.m_shot_clr[ 2 ] * 255.f, cfg.m_shot_clr[ 3 ] * ( 255.f * delta ) ), false );
					hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
					game::g_studio_render->forced_mat_override( nullptr );

					if( cfg.m_shot_chams_overlay ) { 
						g_chams->override_mat( cfg.m_shot_chams_overlay_type,
							sdk::col_t( cfg.m_shot_clr_overlay[ 0 ] * 255.f, cfg.m_shot_clr_overlay[ 1 ] * 255.f,
								cfg.m_shot_clr_overlay[ 2 ] * 255.f, cfg.m_shot_clr_overlay[ 3 ] * ( 255.f * delta ) ), false, true );
											
						hooks::orig_draw_mdl_exec( game::g_mdl_render, *context, i->m_state, i->m_info, i->m_bones.data( ) );
						game::g_studio_render->forced_mat_override( nullptr );
					}
				}
			}
				
			i = std::next( i );
		}
	}

	void c_visuals::add_shot_mdl( game::cs_player_t* player, const sdk::mat3x4_t* bones, bool is_death ) { 
		if( !player )
			return;

		const auto model = player->renderable( )->model( );
		if( !model
			|| !bones )
			return;

		const auto mdl_data = * ( game::studio_hdr_t** ) player->studio_hdr( );
		
		if( !mdl_data )
			return;

		auto& shot_mdl = m_shot_mdls.emplace_back( );

		static int skin = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nSkin" ) );
		static int body = find_in_datamap( player->get_pred_desc_map( ), xor_str( "m_nBody" ) );

		shot_mdl.m_player = player;
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
}