#include "../../../csgo.hpp"
#include <execution>

namespace csgo::hacks { 
	void c_aim_bot::setup_threading( ) { 
		if( m_setupped_threading )
			return;

		const auto fn = reinterpret_cast<int( _cdecl* )( )> ( 
				GetProcAddress( GetModuleHandle( xor_str( "tier0.dll" ) ), xor_str( "AllocateThreadID" ) )
				 );

		std::counting_semaphore<> sem{ 0u };

		for( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
			sdk::g_thread_pool->enqueue( 
				[]( decltype( fn ) fn, std::counting_semaphore<>& sem ) { 
					sem.acquire( );
					fn( );
				}, fn, std::ref( sem )
					 );

		for( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
			sem.release( );

		sdk::g_thread_pool->wait( );
		m_setupped_threading = true;
	}

	void c_aim_bot::handle_ctx( game::user_cmd_t& user_cmd, bool& send_packet ) { 
		m_targets.clear( );
		m_angle = { };
		hacks::g_move->allow_early_stop( ) = true;

		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if( !g_local_player->self( )->weapon( )
			|| !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::knife )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::c4
			|| g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::grenade )
			return;

		setup_threading( );

		if( !m_setupped_threading )
			return;

		if( !m_cfg->m_rage_bot )
			return;

		add_targets( );

		if( m_targets.empty( ) )
			return;

		setup_hitboxes( m_hit_boxes );

		if( m_hit_boxes.empty( ) )
			return;

		g_exploits->m_had_target = false;
		m_silent_aim = false;

		if( g_ctx->can_shoot( ) && !game::g_client_state.get( )->m_choked_cmds
			&& !g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key ) 
			&& g_local_player->weapon_info( )->m_type != game::e_weapon_type::grenade ) { 
			send_packet = false;
			m_silent_aim = true;
		}

		select( user_cmd, send_packet );

		if( m_silent_aim )
			user_cmd.m_buttons &= ~game::e_buttons::in_attack;
	}

	void c_aim_bot::get_hitbox_data( c_hitbox* rtn, game::cs_player_t* ent, int ihitbox, const game::bones_t& matrix )
	{ 
		if( ihitbox < 0 
			|| ihitbox > 19 ) 
			return;

		if( !ent ) 
			return;

		game::studio_hdr_t* const studio_hdr = ent->mdl_ptr( );

		if( !studio_hdr )
			return;

		game::studio_bbox_t* const hitbox = studio_hdr->m_studio->p_hitbox( ihitbox, ent->hitbox_set_index( ) );

		if( !hitbox )
			return;

		const bool is_capsule = hitbox->m_radius != -1.f;

		sdk::vec3_t min, max;
		if( is_capsule ) { 
			min = hitbox->m_mins.transform( matrix [ hitbox->m_bone ] );
			max = hitbox->m_maxs.transform( matrix [ hitbox->m_bone ] );
		}
		else { 
			min = sdk::vector_rotate( hitbox->m_mins, hitbox->m_rotation );
			max = sdk::vector_rotate( hitbox->m_maxs, hitbox->m_rotation );

			min = min.transform( matrix [ hitbox->m_bone ] );
			max = max.transform( matrix [ hitbox->m_bone ] );
		}

		rtn->m_hitbox_id = ihitbox;
		rtn->m_is_oob = !is_capsule;
		rtn->m_radius = hitbox->m_radius;
		rtn->m_mins = min;
		rtn->m_maxs = max;
		rtn->m_hitgroup = hitbox->m_group;
		rtn->m_hitbox = hitbox;
		rtn->m_start_scaled = g_ctx->shoot_pos( ).i_transform( matrix [ hitbox->m_bone ] );
		rtn->m_bone = hitbox->m_bone;
	}

	static const int total_seeds = 128u;

	__forceinline sdk::vec2_t calculate_spread( float recoil_index, const game::e_item_index item_index, int seed, float inaccuracy, float spread, bool revolver2 = false ) { 
		float r1, r2, r3, r4, s1, c1, s2, c2;

		// seed randomseed.
		g_ctx->addresses( ).m_random_seed( seed );

		// generate needed floats.
		r1 = g_ctx->addresses( ).m_random_float( 0.f, 1.f );
		r2 = g_ctx->addresses( ).m_random_float( 0.f, sdk::pi * 2 );

		r3 = g_ctx->addresses( ).m_random_float( 0.f, 1.f );
		r4 = g_ctx->addresses( ).m_random_float( 0.f, sdk::pi * 2 );

		if( item_index == game::e_item_index::negev && recoil_index < 3.f ) { 
			for( int i{ 3 }; i > recoil_index; --i ) { 
				r1 *= r1;
				r3 *= r3;
			}

			r1 = 1.f - r1;
			r3 = 1.f - r3;
		}

		// get needed sine / cosine values.
		c1 = std::cosf( r2 );
		c2 = std::cosf( r4 );
		s1 = std::sinf( r2 );
		s2 = std::sinf( r4 );

		// calculate spread vector.
		return { 
			( c1 * ( r1 * inaccuracy ) ) + ( c2 * ( r3 * spread ) ),
			( s1 * ( r1 * inaccuracy ) ) + ( s2 * ( r3 * spread ) )
		};
	}

	int c_aim_bot::get_min_dmg_override_key( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return -1;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return -1;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_awp_dmg_key );
		case game::e_item_index::ssg08:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_scout_dmg_key );
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_scar_dmg_key );
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_other_dmg_key );
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_heavy_pistol_dmg_key );
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_pistol_dmg_key );
		default:
			return -1;
		}

		return -1;
	}

	int c_aim_bot::get_min_dmg_override( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_awp_min_dmg_on_key;
		case game::e_item_index::ssg08:
			return m_cfg->m_scout_min_dmg_on_key;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_scar_min_dmg_on_key;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_min_dmg_on_key;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_pistol_min_dmg_on_key;
		default:
			return m_cfg->m_other_min_dmg_on_key;
		}

		return 0.f;
	}

	int c_aim_bot::get_force_head_conditions( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_force_head_conditions_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_force_head_conditions_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_force_head_conditions_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_force_head_conditions_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_force_head_conditions_pistol;
		default:
			return m_cfg->m_force_head_conditions_other;
		}

		return 0;
	}

	int c_aim_bot::get_force_body_conditions( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_force_body_conditions_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_force_body_conditions_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_force_body_conditions_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_force_body_conditions_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_force_body_conditions_pistol;
		default:
			return m_cfg->m_force_body_conditions_other;
		}

		return 0;
	}

	bool c_aim_bot::get_min_dmg_override_state( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return false;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_awp_dmg_key );
		case game::e_item_index::ssg08:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scout_dmg_key );
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scar_dmg_key );
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_heavy_pistol_dmg_key );
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_pistol_dmg_key );
		default:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_other_dmg_key );
		}

		return false;
	}

	int c_aim_bot::get_min_dmg_to_set_up( )
	{ 
		if( !g_local_player->self( ) 
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_min_dmg_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_min_dmg_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_min_dmg_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_min_dmg_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_min_dmg_pistol;
		default:
			return m_cfg->m_min_dmg_other;
		}

		return 0.f;
	}
	
	int c_aim_bot::get_dt_stop_type( )
	{ 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		int ret = m_cfg->m_auto_stop_type_dt_other;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			ret = m_cfg->m_auto_stop_type_dt_awp;
			break;
		case game::e_item_index::ssg08:
			ret = m_cfg->m_auto_stop_type_dt_scout;
			break;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			ret = m_cfg->m_auto_stop_type_dt_scar;
			break;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			ret = m_cfg->m_auto_stop_type_dt_heavy_pistol;
			break;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			ret = m_cfg->m_auto_stop_type_dt_pistol;
			break;
		default:
			break;
		}

		return ret;
	}

	int c_aim_bot::get_autostop_type( )
	{ 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;
		
		bool shifting = game::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick <= 16;
		int dt_type = get_dt_stop_type( );

		if( shifting )  // if in shift 
			return ( dt_type == 2 || dt_type == 1 ) ? dt_type : 0; // return dt stop type

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		int ret = m_cfg->m_auto_stop_type_other;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			ret = m_cfg->m_auto_stop_type_awp;
			break;
		case game::e_item_index::ssg08:
			ret = m_cfg->m_auto_stop_type_scout;
			break;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			ret = m_cfg->m_auto_stop_type_scar;
			break;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			ret = m_cfg->m_auto_stop_type_heavy_pistol;
			break;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			ret = m_cfg->m_auto_stop_type_pistol;
			break;
		default:
			break;
		}

		return ret;
	}

	int c_aim_bot::get_hitboxes_setup( )
	{ 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_awp_hitboxes;
		case game::e_item_index::ssg08:
			return m_cfg->m_scout_hitboxes;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_scar_hitboxes;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_hitboxes;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_pistol_hitboxes;
		default:
			return g_aim_bot->cfg( ).m_other_hitboxes;
		}

		return 0;
	}

	float c_aim_bot::get_pointscale( )
	{ 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_awp_point_scale;
		case game::e_item_index::ssg08:
			return m_cfg->m_scout_point_scale;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_scar_point_scale;
	
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_point_scale;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_pistol_point_scale;
		default:
			return g_aim_bot->cfg( ).m_other_point_scale;
		}

		return 0;
	}

	float c_aim_bot::get_hit_chance( ) { 
		if( !g_local_player->self( ) 
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		bool shifting = game::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick <= 16 && g_key_binds->get_keybind_state( &g_exploits->cfg( ).m_dt_key );

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return shifting ? m_cfg->m_dt_hit_chance_awp : m_cfg->m_hit_chance_awp;
		case game::e_item_index::ssg08:
			return shifting ? m_cfg->m_dt_hit_chance_scout : m_cfg->m_hit_chance_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return shifting ? m_cfg->m_dt_hit_chance_scar : m_cfg->m_hit_chance_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return shifting ? m_cfg->m_dt_hit_chance_heavy_pistol: m_cfg->m_hit_chance_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return shifting ? m_cfg->m_dt_hit_chance_pistol : m_cfg->m_hit_chance_pistol;
		default:
			return shifting ? m_cfg->m_dt_hit_chance_other : m_cfg->m_hit_chance_other;
		}

		return 0;
	}

	bool c_aim_bot::calc_hit_chance( 
		game::cs_player_t* player, const sdk::qang_t& angle, sdk::vec3_t pos
	 ) { 
		sdk::vec3_t fwd { }, right { }, up { };
		sdk::ang_vecs( angle, &fwd, &right, &up );

		float hits{ };
		const float needed_hits{ ( get_hit_chance( ) / 100.f ) * static_cast< float > ( total_seeds ) };
		game::cs_weapon_t* weapon = g_local_player->self( )->weapon( );

		if( !weapon )
			return false;

		game::weapon_info_t* wpn_data = weapon->info( );

		if( !wpn_data )
			return false;

		const float recoil_index = weapon->recoil_index( );
		const float wpn_range = wpn_data->m_range;
		const game::e_item_index item_id = weapon->item_index( );
		sdk::vec3_t dir{ }, end{ }, start{ g_ctx->shoot_pos( ) };
		sdk::vec2_t spread_angle{ };
		game::trace_t tr{ };

		// lets not overshoot
		const float dist = std::clamp( ( pos - g_ctx->shoot_pos( ) ).length( 3u ) + 64.f, 0.f, wpn_range );

		for( int i { 0 }; i < total_seeds; i++ ) { 

			spread_angle = calculate_spread( recoil_index, item_id, i, g_eng_pred->inaccuracy( ), g_eng_pred->spread( ), recoil_index );
			dir = fwd + ( right * spread_angle.x( ) ) + ( up * spread_angle.y( ) );
			dir.normalize( );
			
			end = start + ( dir * dist );

			game::g_engine_trace->clip_ray_to_entity( game::ray_t( start, end ), CS_MASK_SHOOT_PLAYER, player, &tr );

			// check if we hit a valid player / hitgroup on the player and increment total hits.
			if( tr.m_entity == player && game::is_valid_hitgroup( static_cast< int > ( tr.m_hitgroup ) ) )
				++hits;
		}

		return hits >= needed_hits;
	}

	void c_aim_bot::add_targets( ) { 
		m_targets.reserve( game::g_global_vars.get( )->m_max_clients );

		for( int i = 1; i <= game::g_global_vars.get( )->m_max_clients; ++i ) { 
			auto& entry = hacks::g_lag_comp->entry( i - 1 );

			if( !entry.m_player
				|| !entry.m_player->is_valid_ptr( )
				|| !entry.m_player->alive( )
				|| !entry.m_player->networkable( )
				|| entry.m_player->networkable( )->dormant( )
				|| !entry.m_player->renderable( )
				|| !entry.m_player->bone_cache( ).m_mem.m_ptr
				|| entry.m_lag_records.empty( )
				|| entry.m_lag_records.size( ) <= 1 )
				continue;

			if( entry.m_player->team( )
				== g_local_player->self( )->team( ) )
				continue;

			auto record = select_ideal_record( entry );

			if( !record.has_value( ) 
				|| !record.value( ).m_lag_record->get( )->m_has_valid_bones ) { 
				continue;
			}

			m_targets.emplace_back( aim_target_t( &entry, record.value( ).m_lag_record ) );
		}

		// run sorting and target limit in last
		run_sorting( );
	}

	void c_aim_bot::scan_center_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, sdk::vec3_t shoot_pos, std::vector < point_t >& points ) const { 
		const game::studio_hitbox_set_t* hitbox_set = target.m_entry->m_player->mdl_ptr( )->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );
		
		if( !hitbox_set )
			return;
		
		const game::studio_bbox_t* hitbox_head = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::head ) );
		const game::studio_bbox_t* hitbox_stomach = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::stomach ) );
		const game::studio_bbox_t* hitbox_l_foot = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::left_foot ) );
		const game::studio_bbox_t* hitbox_r_foot = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::right_foot ) );
		const game::studio_bbox_t* hitbox_chest = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::chest ) );

		if( !hitbox_stomach 
			|| !hitbox_head 
			|| !hitbox_l_foot
			|| !hitbox_r_foot
			|| !hitbox_chest )
			return;

		sdk::vec3_t body_point{ };
		sdk::vec3_t head_point{ };
		sdk::vec3_t chest_point{ };
		sdk::vec3_t l_foot_point{ };
		sdk::vec3_t r_foot_point{ };

		sdk::vector_transform( 
			( hitbox_stomach->m_mins + hitbox_stomach->m_maxs ) / 2.f,
			record->m_bones[ hitbox_stomach->m_bone ], body_point
		 );

		sdk::vector_transform( 
			( hitbox_head->m_mins + hitbox_head->m_maxs ) / 2.f,
			record->m_bones[ hitbox_head->m_bone ], head_point
		 );

		// additional scan here
		if( m_cfg->m_backtrack_intensity > 2 ) { // is above medium
			if( m_cfg->m_backtrack_intensity == 4 ) { // is maximum
				sdk::vector_transform( 
					( hitbox_chest->m_mins + hitbox_chest->m_maxs ) / 2.f,
					record->m_bones[ hitbox_chest->m_bone ], chest_point
				 );
			}

			sdk::vector_transform( 
				( hitbox_l_foot->m_mins + hitbox_l_foot->m_maxs ) / 2.f,
				record->m_bones[ hitbox_l_foot->m_bone ], l_foot_point
			 );

			sdk::vector_transform( 
				( hitbox_r_foot->m_mins + hitbox_r_foot->m_maxs ) / 2.f,
				record->m_bones[ hitbox_r_foot->m_bone ], r_foot_point
			 );
		}
		
		points.clear( );

		/* fkin constructors gone crazy mf visual studio kys */
		point_t body_point_{ };
		body_point_.m_center = true;
		body_point_.m_index = game::e_hitbox::stomach;
		body_point_.m_pos = body_point;
		points.push_back( body_point_ );
		
		point_t head_point_{ };
		head_point_.m_center = true;
		head_point_.m_index = game::e_hitbox::head;
		head_point_.m_pos = head_point;
		points.push_back( head_point_ );

		// additional scan here
		if( m_cfg->m_backtrack_intensity > 2 ) { // is above medium
			if( m_cfg->m_backtrack_intensity == 4 ) { // is maximum
				point_t chest_point_{ };
				chest_point_.m_center = true;
				chest_point_.m_index = game::e_hitbox::chest;
				chest_point_.m_pos = chest_point;
				points.push_back( chest_point_ );
			}

			point_t l_foot_point_{ };
			l_foot_point_.m_center = true;
			l_foot_point_.m_index = game::e_hitbox::left_foot;
			l_foot_point_.m_pos = l_foot_point;
			points.push_back( l_foot_point_ );

			point_t r_foot_point_{ };
			r_foot_point_.m_center = true;
			r_foot_point_.m_index = game::e_hitbox::right_foot;
			r_foot_point_.m_pos = r_foot_point;
			points.push_back( r_foot_point_ );
		}

		record->adjust( target.m_entry->m_player );

		// note: made it use 1 dmg override cus we dont rly care if it uses mindmg or not
		for( auto& point : points ) { 
			scan_point( target.m_entry, point, 1.f, true, shoot_pos );

			if( point.m_valid )
				break;
		}
	}

	std::optional < aim_target_t > c_aim_bot::select_ideal_record( const player_entry_t& entry ) const { 
		if( entry.m_lag_records.empty( ) 
			|| entry.m_lag_records.size( ) < 1u
			|| !entry.m_lag_records.front( )->m_has_valid_bones
			|| entry.m_lag_records.front( )->m_dormant ) { 
			return std::nullopt;
		}

		// get front
		const auto& front = entry.m_lag_records.front( );

		// if he's breaking lc, extrapolate him 
		// we have only 1 record available
		if( front->m_broke_lc 
			|| entry.m_lag_records.size( ) == 1u ) { 
			return extrapolate( entry );
		}

		// backup matrixes
		lag_backup_t lag_backup{ };
		lag_backup.setup( entry.m_player );

		// note: ok this is ghetto, but it works in a simple way
		// when you shift tickbase, your tickbase goes backward by your shift amount
		// making the front record not hittable, now if you're lucky enough or the record is slow or standing
		// you'll be able to still shoot at the front lagrecord without missing it
		if( front && ( front->valid( ) || front->m_anim_velocity.length( 2u ) <= 40.f ) ) { 

			std::vector < point_t > points_front{ };
			aim_target_t target_front{ const_cast <player_entry_t*> ( &entry ), front };

			// generate & scan points
			scan_center_points( target_front, front, g_ctx->shoot_pos( ), points_front );

			bool can_hit_front = scan_points( &target_front, points_front, false );

			// restore matrixes etc..
			lag_backup.restore( entry.m_player );
		
			// if we can hit first record, dont try backtracking
			// note: saves up fps & processing time
			if( can_hit_front ) { 
				return get_latest_record( entry );
			}
		}

		// if we only have few records, force front
		if( entry.m_lag_records.size( ) < 2u
			|| m_cfg->m_backtrack_intensity == 0u )
			return get_latest_record( entry );

		// -> we arrived here and couldnt hit front record
		// start backtracking process
		std::shared_ptr< lag_record_t > best_record{ }; // fix for shooting at invalid record probably
		std::optional< point_t > best_aim_point{ };
		const auto rend = entry.m_lag_records.end( );
		sdk::vec3_t last_origin{ 0, 0, 0 };

		for( auto i = entry.m_lag_records.begin( ); i != rend; i = std::next( i ) ) { 
			const auto& lag_record = *i;

			// we already scanned this record
			// and it was not hittable, skip it
			if( lag_record == entry.m_lag_records.front( ) )
				continue;

			// record isnt valid, skip it
			if( !lag_record->valid( ) || ( ( lag_record->m_origin - last_origin ).length( ) < 1.f ) )
				continue;

			// did we find a context smaller than target time ?
			if( !front->m_fake_walking && front->m_sim_time <= lag_record->m_sim_time )
				return get_latest_record( entry );

			std::vector < point_t > points{ };
			aim_target_t target{ const_cast< player_entry_t* > ( &entry ), lag_record };

			// generate and scan points for this record
			scan_center_points( target, lag_record, g_ctx->shoot_pos( ), points );

			// save latest origin
			last_origin = lag_record->m_origin;

			// no hittable point have been found, skip this record
			if( !scan_points( &target, points, false ) ) {
				if( !best_record )
					best_record = lag_record;
				continue;
			}

			// if we have no best point, it means front wasnt hittable
			if( !best_aim_point.has_value( ) ) { 
				best_record = lag_record;

				// lol this is ghetto but will do i suppose
				if( target.m_best_point ) 
					best_aim_point = *target.m_best_point;
				else if( target.m_best_body_point )
					best_aim_point = *target.m_best_body_point;
				else
					break; // somehow theyre all invalid just break

				continue;
			}

			if( !best_record ) {
				best_record = lag_record;
				continue;
			}

			const int health = target.m_entry->m_player->health( );
		
			// if best point isnt valid override to body point
			if( !target.m_best_point ) 
				target.m_best_point = target.m_best_body_point;

			// if its this valid, skip
			if( !target.m_best_point )
				continue;

			// if body point is valid
			if( target.m_best_body_point ) { 
				// and we have more damage or best point isnt valid
				if( target.m_best_point->m_dmg < 1 || target.m_best_point->m_dmg < target.m_best_body_point->m_dmg )
					target.m_best_point = target.m_best_body_point;
			}

			// skip if both were invalid
			if( target.m_best_point->m_dmg < 1 )
				continue;

			// this record's priority is different than current record
			if( lag_record->m_resolved != best_record->m_resolved ) { 
				// this record is resolved but not the best record
				if( lag_record->m_resolved ) { 
					// this record is lethal and has more damage or less than 5 damage
					// note: shoot for lower damage but on a safer record
					if( target.m_best_point->m_dmg >= health 
						|| target.m_best_point->m_dmg - best_aim_point->m_dmg > -5.f ) { 
						
						// replace best record by current record
						best_record = lag_record;

						// lol this is ghetto but will do i suppose
						if( target.m_best_point )
							best_aim_point = *target.m_best_point;
					}
				}

				// note: here you could make a dmg check or something but atm i just prioritize resolved record

				continue;
			}

			// we dealt more damage
			if( target.m_best_point->m_dmg > best_aim_point->m_dmg )
			{ 
				best_record = lag_record;
				best_aim_point = *target.m_best_point;

				// if this record is lethal, stop here
				if( best_aim_point->m_dmg >= health )
					break;

				// go to next
				continue;
			}
		}

		lag_backup.restore( entry.m_player );

		if( !best_record )
			return std::nullopt;

		if( best_record->m_broke_lc ) { // player broke lc let's extrapolate him
			return extrapolate ( entry );
		}
		else {
			return aim_target_t{ const_cast < player_entry_t* > ( &entry ), best_record };
		}
	}

	std::optional < aim_target_t > c_aim_bot::get_latest_record( const player_entry_t& entry ) const { 
		const auto& latest = entry.m_lag_records.front( );
		if( latest->m_lag_ticks <= 0
			|| latest->m_lag_ticks > 19	
			|| latest->m_dormant
			|| !latest->m_has_valid_bones ) { 

			return std::nullopt;
		}

		// yo, wanna see some ghetto shit?
		if( !latest->valid( ) 
			&& latest->m_anim_velocity.length( 2u ) > 40.f ) { // here u go
			return std::nullopt;
		}

		if( latest->m_broke_lc ) { 
			const float adjusted_arrive_tick = std::clamp( game::to_ticks( ( ( g_ctx->net_info( ).m_latency.m_out ) + game::g_global_vars.get( )->m_real_time )
				- entry.m_receive_time ), 0, 100 );

			if( ( adjusted_arrive_tick - latest->m_choked_cmds ) >= 0 ) { 
				return std::nullopt;
			}
		}

		return aim_target_t{ const_cast< player_entry_t* > ( &entry ), latest };
	}	

	__forceinline float calc_point_scale( 
		const float spread, const float max,
		const float dist, const sdk::vec3_t& dir,
		const sdk::vec3_t& right, const sdk::vec3_t& up
	 )
	{ 
		const float accuracy = g_eng_pred->inaccuracy( ) + spread;
		sdk::vec3_t angle = ( right * accuracy + dir + up * accuracy ).normalized( );

		const float delta = sdk::angle_diff( sdk::to_deg( std::atan2( dir.y( ), dir.x( ) ) ), sdk::to_deg( std::atan2( angle.y( ), angle.x( ) ) ) );
		const float scale = max + dist / std::tan( sdk::to_rad( 180.f - ( delta + 90.f ) ) );
		if( scale > max )
			return 0.9f;

		float final_scale{ 0.3f };

		if( scale >= 0.f )
			final_scale = scale;

		return final_scale / max;
	}

	void c_aim_bot::setup_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, game::e_hitbox index, e_hit_scan_mode mode
	 ) { 
		game::studio_hdr_t* hdr = target.m_entry->m_player->mdl_ptr( );
		if( !hdr )
			return;

		game::studio_hitbox_set_t* set = hdr->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );
		if( !set )
			return;

		game::studio_bbox_t* hitbox = set->get_bbox( static_cast < std::ptrdiff_t > ( index ) );
		if( !hitbox )
			return;

		sdk::vec3_t point{ };

		// center.
		const sdk::vec3_t center = ( hitbox->m_mins + hitbox->m_maxs ) / 2.f;
		sdk::vector_transform( center, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, true );

		// get hitbox scales.
		float scale = g_aim_bot->get_pointscale( ) / 100.f;
		
		if( scale <= 0.0f ) { 

			const float max = ( hitbox->m_maxs - hitbox->m_mins ).length( ) * 0.5f + hitbox->m_radius;
			sdk::vec3_t dir = ( point - g_ctx->shoot_pos( ) );

			const float dist = dir.normalize( );

			sdk::vec3_t right{ }, up{ };

			if( dir.x( ) == 0.f && dir.y( ) == 0.f ) { 
				right = { 0.f, -1.f, 0.f };
				up = { -dir.z( ), 0.f, 0.f };
			}
			else { 
				right = dir.cross( { 0.f, 0.f, 1.f } ).normalized( );
				up = right.cross( dir ).normalized( );
			}

			scale = calc_point_scale( g_eng_pred->spread( ), max, dist, dir, right, up );

			if( scale <= 0.3f && g_eng_pred->spread( ) > g_eng_pred->min_inaccuracy( ) )
				scale = calc_point_scale( g_eng_pred->min_inaccuracy( ), max, dist, dir, right, up );

			scale = std::clamp( scale, 0.3f, 0.91f );
			
		}

		// pain
		if( scale <= 0.f )
			return;

		// feet
		if( hitbox->m_radius <= 0.f )
		{ 
			if( index == game::e_hitbox::left_foot || index == game::e_hitbox::right_foot ) { 
				// front
				point = { center.x( ) + ( hitbox->m_mins.x( ) - center.x( ) ) * scale, center.y( ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// back
				point = { center.x( ) + ( hitbox->m_maxs.x( ) - center.x( ) ) * scale, center.y( ), center.z( )};
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			return;
		}

		if( index != game::e_hitbox::head ) { 
				
			if( index == game::e_hitbox::pelvis ) { 

				// back
				point = { center.x( ), hitbox->m_maxs.y( ) - ( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// front
				point = { center.x( ), hitbox->m_maxs.y( ) + ( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// this is cancer but its to avoid issues with going outside of the model
				scale *= 0.8f;

				// left
				point = { center.x( ), center.y( ), hitbox->m_maxs.z( ) + ( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// right
				point = { center.x( ), center.y( ), hitbox->m_mins.z( ) - ( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			else if( index == game::e_hitbox::chest ) { 
				// back
				point = { center.x( ), hitbox->m_maxs.y( ) - ( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				scale *= 0.8f;

				// left
				point = { center.x( ), center.y( ), hitbox->m_maxs.z( ) + ( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// right
				point = { center.x( ), center.y( ), hitbox->m_mins.z( ) - ( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			else if( index == game::e_hitbox::stomach || index == game::e_hitbox::lower_chest || index == game::e_hitbox::upper_chest ) { 
				// back
				point = { center.x( ), hitbox->m_maxs.y( ) - ( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				return;
			}

			// exit
			return;
		}

		// top back
		point = { hitbox->m_maxs.x( ) + 0.70710678f * ( hitbox->m_radius * scale ), hitbox->m_maxs.y( ) - 0.70710678f * ( hitbox->m_radius * scale ), hitbox->m_maxs.z( ) };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// side
		point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) + hitbox->m_radius * scale };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// side
		point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) - hitbox->m_radius * scale };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// back
		point = { center.x( ), hitbox->m_maxs.y( ) - hitbox->m_radius * scale, center.z( ) };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );
	}

	void c_aim_bot::setup_hitboxes( std::vector < hit_box_data_t >& hitboxes ) { 
		hitboxes.clear( );

		if( g_local_player->self( )->weapon( )->item_index( ) == game::e_item_index::taser ) { 
			hitboxes.push_back( { game::e_hitbox::chest, e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::stomach, e_hit_scan_mode::normal } );

			return;
		}

		const int hitboxes_selected = g_aim_bot->get_hitboxes_setup( );

		if( hitboxes_selected & 1 ) { 
			hitboxes.push_back( { game::e_hitbox::head, e_hit_scan_mode::normal } );
		}
		
		if( hitboxes_selected & 4 ) { 
			hitboxes.push_back( { game::e_hitbox::pelvis, e_hit_scan_mode::normal } );

			hitboxes.push_back( { game::e_hitbox::stomach, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 2 ) { 
			hitboxes.push_back( { game::e_hitbox::lower_chest, e_hit_scan_mode::normal } );

			hitboxes.push_back( { game::e_hitbox::chest, e_hit_scan_mode::normal } );

			hitboxes.push_back( { game::e_hitbox::upper_chest, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 16 ) { 
			hitboxes.push_back( { game::e_hitbox::left_thigh, e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::right_thigh,e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::left_calf, e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::right_calf,e_hit_scan_mode::normal } );

			hitboxes.push_back( { game::e_hitbox::left_foot, e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::right_foot, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 8 ) { 
			hitboxes.push_back( { game::e_hitbox::left_upper_arm,e_hit_scan_mode::normal } );
			hitboxes.push_back( { game::e_hitbox::right_upper_arm, e_hit_scan_mode::normal } );
		}
	}

	void c_aim_bot::scan_point( player_entry_t* entry,
		point_t& point, float min_dmg_key, bool min_dmg_key_pressed, sdk::vec3_t& shoot_pos ) { 

		if( !g_auto_wall->wall_penetration( shoot_pos, &point, entry->m_player ) )
			return;

		const int hp = entry->m_player->health( );
		int min_dmg = g_aim_bot->get_min_dmg_to_set_up( );
		
		if( min_dmg_key_pressed )
			min_dmg = min_dmg_key;

		if( min_dmg >= hp )
			min_dmg = hp + ( min_dmg - 100.f );

		point.m_valid = ( point.m_dmg >= hp || point.m_dmg >= min_dmg );
	}

	bool c_aim_bot::scan_points( cc_def ( aim_target_t* ) target, std::vector < point_t >& points, bool additional_scan ) const {
		std::array < point_t*, 20 > best_points {};

		for( auto& point : points ) {

			if( additional_scan )
				scan_point( target.get( )->m_entry, point, static_cast < int > ( g_aim_bot->get_min_dmg_override( ) ), g_aim_bot->get_min_dmg_override_state( ) );

			if( !point.m_valid 
				|| point.m_dmg < 1 )
				continue;

			const auto hp = target.get( )->m_entry->m_player->health( );

			auto& best_point = best_points.at ( static_cast < std::ptrdiff_t > ( point.m_index ) );

			if( !best_point ) {
				best_point = &point; // init best point that we can compare to next points
				continue;
			}

			const auto& best_pen_data = best_point;
			const auto& pen_data = point;

			if( point.m_center ) {
				if( ( best_pen_data->m_hitgroup == pen_data.m_hitgroup )
					|| ( best_pen_data->m_remaining_pen == pen_data.m_remaining_pen && std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) <= crypt_int ( 1 ) )
					|| ( best_pen_data->m_dmg > hp && pen_data.m_dmg > hp ) ) {
					best_point = &point;
				}

				continue;
			}	

			auto& cur_dmg = pen_data.m_dmg;
			auto& last_dmg = best_pen_data->m_dmg;

			if( last_dmg == cur_dmg ) {
				continue;
			}

			if( cur_dmg >= hp
				&& last_dmg < hp ) {
				best_point = &point;
				break; // lethal point, its perfect
			}

			if( best_pen_data->m_hitgroup != pen_data.m_hitgroup
				|| best_pen_data->m_remaining_pen != pen_data.m_remaining_pen ) {
				if( best_pen_data->m_remaining_pen != pen_data.m_remaining_pen
					|| std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 ) {
					if( best_pen_data->m_dmg <= hp || pen_data.m_dmg <= hp ) {
						if( pen_data.m_dmg > best_pen_data->m_dmg )
							best_point = &point;

						break; // nah escape from scan
					}
				}
			}

			if( last_dmg < hp
				&& cur_dmg < hp ) {
				if( std::abs( last_dmg - cur_dmg ) <= 1 ) {
					continue;
				}
	
				if( cur_dmg > last_dmg + 5 ) {
					best_point = &point;
					break; // point is fine
				}
			}

		}

		std::vector < point_t > next_points {};

		for( auto& best_point : best_points ) {
			if( best_point )
				next_points.emplace_back ( std::move ( *best_point ) );

			points = next_points;
		}

		if( points.empty ( ) )
			return false;

		const auto hp = target.get( )->m_entry->m_player->health( );

		for( auto& point : points ) {
			if( !target.get( )->m_best_point )
				target.get( )->m_best_point = &point;
			else {
				const auto& best_pen_data = target.get( )->m_best_point;
				const auto& pen_data = point;

				auto point_has_more_damage = false;
				if( std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data->m_dmg <= hp ) )
					point_has_more_damage = pen_data.m_dmg > best_pen_data->m_dmg;

				if( point_has_more_damage )
					target.get( )->m_best_point = &point;
			}

			if( point.m_index == game::e_hitbox::stomach
				|| point.m_index == game::e_hitbox::pelvis ) {
				if( !target.get( )->m_best_body_point )
					target.get( )->m_best_body_point = &point;
				else {
					const auto& best_pen_data = target.get( )->m_best_body_point;
					const auto& pen_data = point;

					auto point_has_more_damage = false;
					if( std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data->m_dmg <= hp ) )
						point_has_more_damage = pen_data.m_dmg > best_pen_data->m_dmg;

					if( point_has_more_damage )
						target.get( )->m_best_body_point = &point;
				}
			}
		}

		if( target.get( )->m_best_body_point ) {
			target.get( )->m_best_point = target.get( )->m_best_body_point;
		}

		return true;
	}

	point_t* c_aim_bot::select_point( cc_def( aim_target_t* ) target, const int cmd_num ) { 

		static float crypt_lethalx2 = crypt_float( 2.f );
		static int crypt_cmd = crypt_int( 150 );
		static int crypt_dmg = crypt_int( 1 );

		if( !target.get( )->m_best_body_point
			|| target.get( )->m_best_body_point->m_dmg < crypt_dmg ) { 
			return target.get( )->m_best_point;
		}

		if( !target.get( )->m_best_point || target.get( )->m_best_point->m_dmg < crypt_dmg )
			return target.get( )->m_best_body_point;

		if( target.get( )->m_best_body_point->m_dmg >= target.get( )->m_best_point->m_dmg
			|| target.get( )->m_best_body_point->m_dmg >= target.get( )->m_entry->m_player->health( ) )
			return target.get( )->m_best_body_point; 

		const int hp = target.get( )->m_entry->m_player->health( );

		if( g_key_binds->get_keybind_state( &m_cfg->m_baim_key ) )
			return target.get( )->m_best_body_point;

		const int body_cond = get_force_body_conditions( );

		if( body_cond & 1 ) { 
			if( !( target.get( )->m_lag_record.value( )->m_flags & game::e_ent_flags::on_ground ) ) { 
				return target.get( )->m_best_body_point;
			}
		}

		if( body_cond & 2 ) { 
			if( !target.get( )->m_entry->m_moving_data.m_moved ) { 
				return target.get( )->m_best_body_point;
			}
		}

		if( body_cond & 4 ) { 
			if( !target.get( )->m_lag_record.value( )->m_resolved ) { 
				return target.get( )->m_best_body_point;
			}
		}

		int head_cond = get_force_head_conditions( );

		if( target.get( )->m_best_body_point->m_dmg < hp ) { 

			if( head_cond & 1
				&& ( target.get( )->m_entry->m_body_data.m_has_updated || target.get( )->m_entry->m_had_last_move ) ) { 
				return target.get( )->m_best_point;
			}

			if( head_cond & 2
				&& target.get( )->m_lag_record.value( )->m_anim_velocity.length( 2u ) > 75.f
				&& !target.get( )->m_lag_record.value( )->m_fake_walking 
				&& ( target.get( )->m_lag_record.value( )->m_flags & game::e_ent_flags::on_ground ) ) { 
				return target.get( )->m_best_point;
			}

			if( head_cond & 4
				&& target.get( )->m_lag_record.value( )->m_resolved ) { 
				return target.get( )->m_best_point;
			}
		}

		const auto& shots = g_shot_construct->m_shots;

		if( !shots.empty( ) && body_cond & 8 ) { 
			const auto& last_shot = shots.back( );

			if( last_shot.m_target->m_entry->m_player == target.get( )->m_entry->m_player
				&& std::abs( last_shot.m_cmd_num - cmd_num ) <= crypt_cmd ) { 
				const auto hp_left = last_shot.m_target->m_entry->m_player->health( ) - last_shot.m_damage;

				if( hp_left
					&& target.get( )->m_best_body_point->m_dmg >= hp_left )
					return target.get( )->m_best_body_point;
			}
		}

		if( target.get( )->m_best_body_point->m_dmg >= target.get( )->m_entry->m_player->health( ) )
			return target.get( )->m_best_body_point;

		if( g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key )
			&& target.get( )->m_best_body_point->m_dmg * crypt_lethalx2 >= target.get( )->m_entry->m_player->health( )
			&& g_local_player->weapon( )->item_index( ) != game::e_item_index::ssg08 
			&&  ( body_cond & 8 )
			&& ( std::abs( game::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick ) <= 16
				|| g_exploits->m_ticks_allowed >= 14 ) ) { 
			return target.get( )->m_best_body_point;
		}

		return target.get( )->m_best_point;
	}

	bool c_aim_bot::can_shoot( 
		bool skip_r8, const int shift_amount, const bool lol
	 ) const { 


		static int crypt_seq = crypt_int( 967 );
		static int crypt_clip = crypt_int( 0 );

		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto weapon = g_local_player->self( )->weapon( );
		if( !weapon )
			return false;

		auto weapon_data = weapon->info( );
		if( !weapon_data )
			return false;

		if( g_local_player->self( )->flags( ) & game::e_ent_flags::frozen )
			return false;

		if( g_local_player->self( )->wait_for_no_attack( ) )
			return false;

		if( g_local_player->self( )->defusing( ) )
			return false;

		auto anim_layer = g_local_player->self( )->anim_layers( ).at( 1u );
		
		if( anim_layer.m_owner ) { 
			if( g_local_player->self( )->lookup_seq_act( anim_layer.m_seq ) == crypt_seq && anim_layer.m_weight != 0.f )
				return false;
		}

		if( weapon_data->m_type >= game::e_weapon_type::pistol && weapon_data->m_type <= game::e_weapon_type::machine_gun && weapon->clip1( ) == crypt_clip )
			return false;

		float curtime = game::to_time( g_local_player->self( )->tick_base( ) - shift_amount );
		if( curtime < g_local_player->self( )->next_attack( ) )
			return false;

		if( lol )
			return true;

		if( ( weapon->item_index( ) == game::e_item_index::glock || weapon->item_index( ) == game::e_item_index::famas ) && weapon->burst_shots_remaining( ) > crypt_clip ) { 
			if( curtime >= weapon->next_burst_shot( ) )
				return true;
		}

		if( curtime < weapon->next_primary_attack( ) )
			return false;

		if( weapon->item_index( ) != game::e_item_index::revolver )
			return true;

		if( skip_r8 )
			return true;

		return curtime >= weapon->postpone_fire_ready_time( );
	}

	bool c_aim_bot::can_hit( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box ) { 
		auto hdr = * ( game::studio_hdr_t** ) record->m_player->mdl_ptr( );
		if( !hdr )
			return false;

		auto hitbox_set = hdr->m_studio->get_hitbox_set( record->m_player->hitbox_set_index( ) );

		if( !hitbox_set )
			return false;

		auto hitbox_ptr = hitbox_set->get_bbox( box );

		if( !hitbox_ptr )
			return false;

		bool is_capsule = hitbox_ptr->m_radius != -1.0f;

		auto mins = hitbox_ptr->m_mins.transform( record->m_bones[ hitbox_ptr->m_bone ] );
		auto maxs = hitbox_ptr->m_maxs.transform( record->m_bones[ hitbox_ptr->m_bone ] );

		bool is_intersected{ };

		if( !is_capsule ) { 
			is_intersected = sdk::intersect_bounding_box( start, end, mins, maxs );
		}
		else
		{ 
			is_intersected = sdk::intersect( start, end, mins, maxs, hitbox_ptr->m_radius );
		}

		return is_intersected;
	}


	void c_aim_bot::run_sorting( ) { 

		if( m_targets.empty( ) || m_targets.size( ) <= 2 )
			return;

		// max threads
		static int max_threads = std::jthread::hardware_concurrency( );

		// should be static i think?
		static auto sort_targets =[&]( aim_target_t& a, aim_target_t& b )  { 

			// this is the same player
			// in that case, do nothing
			if( a.m_entry->m_player == b.m_entry->m_player 
				|| a.m_entry->m_player->networkable( )->index( ) == b.m_entry->m_player->networkable( )->index( ) )
				return false;

			// get fov of player a
			float fov_a = sdk::calc_fov( game::g_engine->view_angles( ), g_ctx->shoot_pos( ), a.m_entry->m_player->world_space_center( ) );
	
			// get fov of player b
			float fov_b = sdk::calc_fov( game::g_engine->view_angles( ), g_ctx->shoot_pos( ), b.m_entry->m_player->world_space_center( ) );
		
			// if player a fov lower than player b fov prioritize him
			return fov_a < fov_b;
		};

		// std::execution::par -> parallel sorting( multithreaded )
		// NOTE: not obligated, std::sort doesnt take alot of cpu power but its still better
		std::sort( std::execution::par, m_targets.begin( ), m_targets.end( ), sort_targets );

		// calc max allowed targets 
		const int max_allowed_size =
			m_cfg->m_dynamic_limit ? std::clamp( int( m_targets.size( ) / 2 ), 2, max_threads - 2 ) 
			: std::clamp( m_cfg->m_max_targets, 2, max_threads - 2 );

		// target limit based on our prioritized targets
		while( m_targets.size( ) > max_allowed_size )
			m_targets.pop_back( );
	}
	
	aim_target_t* c_aim_bot::select_target( ) { 
		if( m_targets.empty( ) )
			return nullptr;

		if( m_targets.size( ) == 1 )
			return &m_targets.front( );

		aim_target_t* best_target{ };

		const auto end = m_targets.end( );
		for( auto it = std::next( m_targets.begin( ) ); it != end; it = std::next( it ) ) { 
			const int hp = it->m_entry->m_player->health( );

			if( it->m_best_point ) {
				const float cur_dmg = it->m_best_point->m_dmg;

				if( !best_target ) { 
					best_target = &*it;

					if( cur_dmg >= hp )
						break;

					continue;
				}

				if( cur_dmg > best_target->m_best_point->m_dmg ||
					cur_dmg >= hp ) { 
					best_target = &*it;
				
					if( it->m_best_point->m_dmg >= hp )
						break;
				}
			}
		}

		return best_target;
	}

	void c_aim_bot::select( game::user_cmd_t& user_cmd, bool& send_packet ) { 
		struct ideal_target_t { 
			game::cs_player_t* m_player { }; sdk::vec3_t m_pos{ };
			float m_dmg{ }; game::e_hitbox m_hit_box{ }; std::shared_ptr < lag_record_t > m_record{ }; aim_target_t* m_target{ };
		};
		std::unique_ptr < ideal_target_t > ideal_select = std::make_unique < ideal_target_t > ( );

		// ok this is retarded
		for( auto& target : m_targets ) { 
			// setup backup record
			target.m_backup_record.setup( target.m_entry->m_player );

			// apply record matrixes
			target.m_lag_record.value( )->adjust( target.m_entry->m_player );

			sdk::g_thread_pool->enqueue( [ ]( aim_target_t& target ) { 
				// setup points for this target
				for( const hit_box_data_t& hitbox : g_aim_bot->m_hit_boxes )
					g_aim_bot->setup_points( target, target.m_lag_record.value( ), hitbox.m_index, hitbox.m_mode );

				// scan through all points
				for( auto& point : target.m_points )
					g_aim_bot->scan_point( target.m_entry, point, g_aim_bot->get_min_dmg_override( ), g_aim_bot->get_min_dmg_override_state( ) );

				// check if the entity is hittable ( if we hit any point )
				target.m_hittable_target = g_aim_bot->scan_points( &target, target.m_points, false );

			}, std::ref( target ) );

			target.m_backup_record.restore( target.m_entry->m_player );
		}

		// wait till all threads finish their job
		sdk::g_thread_pool->wait( );

		// erase targets that are not targettable
		m_targets.erase( 
			std::remove_if( 
				m_targets.begin( ), m_targets.end( ),
				[ & ]( aim_target_t& target ) { 
					return !target.m_hittable_target;
				}
			 ),
			m_targets.end( )
					 );

		// get best target
		aim_target_t* target = select_target( );

		if( !target ) 
			return m_targets.clear( );

		hacks::g_move->allow_early_stop( ) = false; 

		const point_t* point = select_point( target, user_cmd.m_number );

		if( point ) { 
			// game::g_cvar->error_print( true, "bestpoint found!\n" );
			ideal_select->m_player = target->m_entry->m_player;
			ideal_select->m_dmg = point->m_dmg;
			ideal_select->m_record = target->m_lag_record.value( );
			ideal_select->m_hit_box = point->m_index;
			ideal_select->m_pos = point->m_pos;
			ideal_select->m_target = target;
		}

		if( ideal_select->m_player && ideal_select->m_record ) { 
			g_exploits->m_had_target = true;

			if( g_exploits->m_type == c_exploits::type_defensive )
				g_exploits->m_type = c_exploits::type_doubletap;

			ideal_select->m_target->m_pos = ideal_select->m_pos;

			g_eng_pred->update_shoot_pos( m_angle );

			m_angle = ( ideal_select->m_pos - g_ctx->shoot_pos( ) ).angles( );

			g_ctx->was_shooting( ) = false;
			
			game::weapon_info_t* wpn_info = g_local_player->weapon( )->info( );

			if( wpn_info ) { 
				bool between_shots = ( m_cfg->m_stop_modifiers & 2 ) 
					&& g_ctx->get_auto_peek_info( ).m_start_pos == sdk::vec3_t( ) 
					&& ( wpn_info->m_full_auto 
						|| wpn_info->m_type == game::e_weapon_type::pistol );

				if( can_shoot( true, 0, between_shots ) 
					|| ( g_exploits->m_ticks_allowed >= 14
							&& can_shoot( true, 14, between_shots ) ) ) { 
					m_should_stop = get_autostop_type( ) + 1;
				}
			}

			if( g_ctx->can_shoot( ) && !m_silent_aim ) { 
				auto wpn_idx = g_local_player->weapon( )->item_index( );
				bool can_scope = !g_local_player->self( )->scoped( ) && ( wpn_idx == game::e_item_index::aug 
					|| wpn_idx == game::e_item_index::sg553 || wpn_idx == game::e_item_index::scar20
					|| wpn_idx == game::e_item_index::g3sg1 
					|| g_local_player->weapon_info( )->m_type == game::e_weapon_type::sniper );

				if( !( user_cmd.m_buttons & game::e_buttons::in_jump ) ) { 
					if( can_scope
						&& m_cfg->m_auto_scope )
						user_cmd.m_buttons |= game::e_buttons::in_attack2;
				}

				lag_backup_t lag_backup{ };
				lag_backup.setup( ideal_select->m_player );
				ideal_select->m_record->adjust( ideal_select->m_player );
				const bool hit_chance = calc_hit_chance( ideal_select->m_player, m_angle, ideal_select->m_pos );

				lag_backup.restore( ideal_select->m_player );

				if( hit_chance ) { 
					std::stringstream msg;

					int idx = ideal_select->m_player->networkable( )->index( );
					game::player_info_t info;

					auto find = game::g_engine->get_player_info( idx, &info );

					auto get_hitbox_name_by_id = [ ]( game::e_hitbox id ) -> const char* { 
						switch( id ) { 
						case game::e_hitbox::head:
							return "head";
							break;
						case game::e_hitbox::neck:
							return "neck";
							break;
						case game::e_hitbox::pelvis:
							return "pelvis";
						case game::e_hitbox::stomach:
							return "stomach";
							break;
						case game::e_hitbox::lower_chest:
							return "lower chest";
							break;
						case game::e_hitbox::chest:
							return "chest";
							break;
						case game::e_hitbox::upper_chest:
							return "upper chest";
							break;
						case game::e_hitbox::right_thigh:
						case game::e_hitbox::right_calf:
						case game::e_hitbox::right_foot:
							return "right leg";
							break;
						case game::e_hitbox::left_thigh:
						case game::e_hitbox::left_calf:
						case game::e_hitbox::left_foot:
							return "left leg";
							break;
						case game::e_hitbox::left_upper_arm:
							return "left arm";
							break;
						case game::e_hitbox::right_upper_arm:
							return "right arm";
						break;
						default:
							return "?";
							break;
						}
					};

					std::string solve_method = resolver_mode( ideal_select->m_record->m_resolver_method );

					if( find ) { 
						int rounded_damage =  ( int )std::round( ideal_select->m_dmg );
						int hitbox = ( int )ideal_select->m_hit_box;
						int rounded_vel = ( int )std::round( ideal_select->m_record->m_anim_velocity.length( 2u ) );

						msg << "[SHOT INFO] player: " << info.m_name;
						msg << " hb: " << std::string( get_hitbox_name_by_id( ideal_select->m_hit_box ) ).data( );
						msg << " (" << std::to_string( hitbox ) << ")";
						msg << " damage: " << std::to_string( rounded_damage );
						msg << " | resolver: " << solve_method.data( ) << " |";
						msg << " animation velocity: " << std::to_string( rounded_vel );
					}

					constexpr uint8_t gray_clr[ 4 ] = { 195, 195, 195, 205 };

					const std::string msg_to_string = msg.str( );

					g_ctx->was_shooting( ) = true;
					if( g_ctx->was_shooting( ) ) { 
						g_ctx->allow_defensive( ) = false;
					}

					static auto weapon_recoil_scale = game::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) );

					g_shots->add( 
						g_ctx->shoot_pos( ), ideal_select->m_target,
						hacks::g_exploits->m_next_shift_amount, user_cmd.m_number, game::g_global_vars.get( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
						 );

					user_cmd.m_buttons |= game::e_buttons::in_attack;
					g_ctx->get_auto_peek_info( ).m_is_firing = true;
					g_ctx->anim_data( ).m_local_data.m_shot = true;

					if( g_ctx->was_shooting( ) ) {
						game::g_cvar->con_print( false, *gray_clr, msg_to_string.c_str( ) );
						game::g_cvar->con_print( false, *gray_clr, xor_str( "\n" ) );
					}

					user_cmd.m_tick = game::to_ticks( ideal_select->m_record->m_sim_time + g_ctx->net_info( ).m_lerp );

					user_cmd.m_view_angles = m_angle;
					user_cmd.m_view_angles -= g_local_player->self( )->aim_punch( ) * weapon_recoil_scale->get_float( );

					user_cmd.m_view_angles.x( ) = std::remainder( user_cmd.m_view_angles.x( ), 360.f );
					user_cmd.m_view_angles.y( ) = std::remainder( user_cmd.m_view_angles.y( ), 360.f );
					user_cmd.m_view_angles.z( ) = std::remainder( user_cmd.m_view_angles.z( ), 360.f );

					user_cmd.m_view_angles.x( ) = std::clamp( user_cmd.m_view_angles.x( ), -89.f, 89.f );
					user_cmd.m_view_angles.y( ) = std::clamp( user_cmd.m_view_angles.y( ), -180.f, 180.f );
					user_cmd.m_view_angles.z( ) = std::clamp( user_cmd.m_view_angles.z( ), -90.f, 90.f );

					g_ctx->anim_data( ).m_local_data.m_last_shot_time = game::g_global_vars.get( )->m_cur_time;
				}
			}			
		}

		m_targets.clear( );
	}

	void c_knife_bot::handle_knife_bot( game::user_cmd_t& cmd )
	{ 
		m_best_index = -1;

		if( !g_local_player->self( )->weapon( ) )
			return;

		if( !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type != game::e_weapon_type::knife )
			return;

		if( !select_target( ) || m_best_player == nullptr )
			return;

		auto best_angle = get_hitbox_pos( 5, m_best_player );
		auto entity_angle = sdk::calc_ang( g_ctx->shoot_pos( ), best_angle );
		auto health = m_best_player->health( );
		auto stab = false;

		if( m_best_player->has_heavy_armor( ) )
		{ 
			if( health <= 55 && health > get_min_dmg( ) )
				stab = true;
		}
		else
		{ 
			if( health <= 65 && health > get_min_dmg( ) )
				stab = true;
		}

		if( health >= 90 )
			stab = true;

		stab = stab && m_best_distance < 60;

		cmd.m_view_angles = sdk::qang_t( entity_angle.x( ), entity_angle.y( ), entity_angle.z( ) );

		if( stab )
			cmd.m_buttons |= game::e_buttons::in_attack2;
		else
			cmd.m_buttons |= game::e_buttons::in_attack;

		cmd.m_tick = game::to_ticks( m_best_player->sim_time( ) + g_ctx->net_info( ).m_lerp );
	}
	__forceinline float dist_to( sdk::vec3_t& from, const sdk::vec3_t& value ) { return ( from - value ).length( ); }

	bool c_knife_bot::select_target( )
	{ 
		float good_distance = 75.0f;

		for( int i = 1; i <= game::g_global_vars.get( )->m_max_clients; i++ )
		{ 
			game::cs_player_t* player = reinterpret_cast< game::cs_player_t* > ( game::g_entity_list->get_entity( i ) );

			if( !player || player == g_local_player->self( ) 
				|| player->networkable( )->dormant( ) || !player->alive( ) 
				|| player->team( ) == g_local_player->self( )->team( ) )
				continue;

			sdk::vec3_t local_position = g_local_player->self( )->origin( );
			local_position.z( ) += 50.0f;

			sdk::vec3_t entity_position = player->origin( );
			entity_position.z( ) += 50.0f;

			float current_distance = dist_to( local_position, entity_position );

			if( current_distance < good_distance )
			{ 
				good_distance = current_distance;
				m_best_index = i;
				m_best_player = player;
			}
		}

		m_best_distance = good_distance;

		return m_best_index != -1;
	}
}