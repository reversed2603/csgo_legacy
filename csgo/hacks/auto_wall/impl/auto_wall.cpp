#include "../../../csgo.hpp"

namespace csgo::hacks { 
	
	constexpr int   max_pen  = 4;
	constexpr float pen_dist = 3000.0f;

	bool is_armored( game::cs_player_t* player, game::e_hitgroup hit_group ) { 
;
		const bool has_armor = player->armor_val( ) > 0.1f;
		bool ret = false;
	
		switch( hit_group ) { 
		case game::e_hitgroup::chest:
		case game::e_hitgroup::stomach:
		case game::e_hitgroup::left_arm:
		case game::e_hitgroup::right_arm:
			ret = has_armor;
			break;
		case game::e_hitgroup::head:
			ret = player->has_helmet( ) && has_armor;
			break;
		default:
			break;
		}

		return ret;
	}

	void c_auto_wall::scale_dmg( game::cs_player_t* player, game::trace_t& trace, game::weapon_info_t* wpn_info, float& dmg, const game::e_hitgroup hit_group ) { 
		if( !player || !player->is_player( )
			|| !player->networkable( ) || player->networkable( )->dormant( ) )
			return;

		const bool armored = is_armored( player, hit_group );
		const bool is_zeus = g_local_player->self( )->weapon( ) ? g_local_player->self( )->weapon( )->item_index( ) == game::e_item_index::taser : false;
		const float armor_val = static_cast < float >( player->armor_val( ) );

		if( !is_zeus ) { 
			switch( hit_group ) { 
			case game::e_hitgroup::head:
				dmg *= 4.f;
				break;
			case game::e_hitgroup::stomach:
				dmg *= 1.25f;
				break;
			case game::e_hitgroup::right_leg:
			case game::e_hitgroup::left_leg:
				dmg *= 0.75f;
				break;
			default:
				break;
			}
		}

		if( !g_local_player->self( )
			|| !g_local_player->self( )->weapon( ) 
			|| !g_local_player->self( )->weapon( )->info( ) )
			return;

		const auto armor_ratio = g_local_player->self( )->weapon( )->info( )->m_armor_ratio;

		if( armored ) { 
			float armor_scale = 1.f;
			float armor_bonus_ratio = 0.5f;
			float armor_ratio_calced = armor_ratio * 0.5f;
			float dmg_to_health = dmg * armor_ratio_calced;

			float dmg_to_armor = ( dmg - dmg_to_health ) * ( armor_scale * armor_bonus_ratio );

			if( dmg_to_armor > armor_val )
				dmg_to_health = dmg - ( armor_val / armor_bonus_ratio );

			dmg = dmg_to_health;
		}

		dmg = std::floor( dmg );
	}

	bool c_auto_wall::trace_to_exit( 
		const sdk::vec3_t& src, const sdk::vec3_t& dir,
		const game::trace_t& enter_trace, game::trace_t& exit_trace
	 ) 
	{ 
		sdk::vec3_t end;
		float distance = 0.f;
		int first_contents{ };

		while( distance <= 90.f )
		{ 
			distance += 4.f;
			end = src + ( dir * distance );

			if( !first_contents )
				first_contents = game::g_engine_trace->get_point_contents( end, CS_MASK_SHOOT_PLAYER, nullptr );

			int curr_contents = game::g_engine_trace->get_point_contents( end, CS_MASK_SHOOT_PLAYER, nullptr );

			if ( ( curr_contents & CS_MASK_SHOOT ) == 0 || (( curr_contents & CONTENTS_HITBOX ) && first_contents != curr_contents ) )
			{ 
				game::ray_t exit_ray{ end, end - dir * 4.f };
				game::g_engine_trace->trace_ray( exit_ray, CS_MASK_SHOOT_PLAYER, nullptr, &exit_trace );

				if( exit_trace.m_start_solid && exit_trace.m_surface.m_flags & SURF_HITBOX )
				{ 
					game::trace_filter_simple_t trace_filter { exit_trace.m_entity, 0 };

					game::g_engine_trace->trace_ray( game::ray_t( src, end ), CS_MASK_SHOOT, reinterpret_cast< game::base_trace_filter_t* >( &trace_filter ), &exit_trace );

					if( exit_trace.hit( ) && !exit_trace.m_start_solid )
						return true;

					continue;
				}

				if( exit_trace.hit( ) && !exit_trace.m_start_solid )
				{ 
					if( enter_trace.m_surface.m_flags & SURF_NODRAW || !( exit_trace.m_surface.m_flags & SURF_NODRAW ) )
					{ 
						if( exit_trace.m_plane.m_normal.dot( dir ) <= 1.f )
							return true;

						continue;
					}

					if( is_breakable( enter_trace.m_entity ) && is_breakable( exit_trace.m_entity ) )
						return true;

					continue;
				}

				if( ( !enter_trace.m_entity || !enter_trace.m_entity->networkable( )->index( ) ) && ( is_breakable( enter_trace.m_entity ) ) )
				{ 
					exit_trace = enter_trace;
					exit_trace.m_end = src + dir;
					return true;
				}

				continue;
			}

		};

		return false;
	}

	bool c_auto_wall::handle_bullet_penetration( 
		game::weapon_info_t* wpn_data, game::trace_t& enter_trace, sdk::vec3_t& eye_pos, const sdk::vec3_t& direction,
		int& possible_hits_remain, float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, float trace_len
	 ) { 
		if( !wpn_data )
			return false;

		if( possible_hits_remain <= 0 || wpn_data->m_penetration <= 0.0f )
			return false;

		const bool contents_grate = ( enter_trace.m_contents & CONTENTS_GRATE );
		const bool surf_nodraw = ( enter_trace.m_surface.m_flags & SURF_NODRAW );

		game::surface_data_t* enter_surf_data = game::g_surface_data->get( enter_trace.m_surface.m_surface_props );

		if( !enter_surf_data )
			return false;

		const std::uint16_t enter_material = enter_surf_data->m_game.m_material;
		game::trace_t exit_trace;

		if( !trace_to_exit( enter_trace.m_end, direction, enter_trace, exit_trace ) 
			&& !( game::g_engine_trace->get_point_contents( enter_trace.m_end, CS_MASK_SHOOT, nullptr ) & CS_MASK_SHOOT ) )
			return false;

		game::surface_data_t* exit_surface_data = game::g_surface_data->get( exit_trace.m_surface.m_surface_props );

		if( !exit_surface_data )
			return false;

		const std::uint16_t exit_material = exit_surface_data->m_game.m_material;

		// percent of total damage lost automatically on impacting a surface
		// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/cstrike15/cs_player_shared.cpp#L2023
		float combined_damage_modifier = 0.16f;

		// check the exit material and average the exit and entrace values
		// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/cstrike15/cs_player_shared.cpp#L2058
		float combined_penetration_modifier = ( enter_surf_data->m_game.m_pen_modifier + exit_surface_data->m_game.m_pen_modifier ) * 0.5f;

		// since some railings in de_inferno are CONTENTS_GRATE but CHAR_TEX_CONCRETE, we'll trust the
		// CONTENTS_GRATE and use a high damage modifier.
		// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/cstrike15/cs_player_shared.cpp#L2027
		if( contents_grate || surf_nodraw || enter_material == CHAR_TEX_WOOD || enter_material == CHAR_TEX_GRATE )
		{ 
			// If we're a concrete grate ( TOOLS/TOOLSINVISIBLE texture ) allow more penetrating power.
			if( enter_material == CHAR_TEX_WOOD || enter_material == CHAR_TEX_GRATE )
			{ 
				combined_penetration_modifier = 3.0f;
				combined_damage_modifier = 0.05f;
			}
			else
				combined_penetration_modifier = 1.0f;
		}

		// if enter & exit point is wood we assume this is 
		// a hollow crate and give a penetration bonus
		if( enter_material == exit_material )
		{ 
			if( exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD )
				combined_penetration_modifier = 3.0f;
			else if( exit_material == CHAR_TEX_PLASTIC )
				combined_penetration_modifier = 2.0f;
		}

		trace_len = ( exit_trace.m_end - enter_trace.m_end ).length( );

		float pen_mod = std::max( 0.f,( 1.f / combined_penetration_modifier ) );
		float percent_damage_chunk = cur_dmg * combined_damage_modifier;
		float pen_wep_mod = percent_damage_chunk + std::max( 0.f,( 3.f / wpn_data->m_penetration ) * 1.25f ) * ( pen_mod * 3.f );
		float lost_damage_obj = ( ( pen_mod * ( trace_len * trace_len ) ) / 24.f );
		float total_lost_dam = pen_wep_mod + lost_damage_obj;
		
		if( total_lost_dam > cur_dmg )
			return false;

		// subtract from damage.
		cur_dmg -= std::max( 0.f, total_lost_dam );
		if( cur_dmg < 1.f )
			return false;

		eye_pos = exit_trace.m_end;
		--possible_hits_remain;

		return true;
	}

	void clip_trace_to_player( 
		const sdk::vec3_t& src, const sdk::vec3_t& dst, game::trace_t& trace,
		game::cs_player_t* const player
	 )
	{ 
		if( !player || !player->networkable( ) 
			|| player->networkable( )->dormant( ) || !player->alive( ) )
			return;

		// get bounding box
		const sdk::vec3_t mins = player->obb_min( );
		const sdk::vec3_t maxs = player->obb_max( );
		const sdk::vec3_t center = ( maxs + mins ) / 2.f;

		// calculate world space center
		const sdk::vec3_t vec_position = center + player->origin( );

		const sdk::vec3_t vec_to = vec_position - src;

		sdk::vec3_t vec_direction = ( dst - src );
		const float length = vec_direction.normalize( );

		const float range_along = vec_direction.dot( vec_to );
		float range{ };

		// calculate distance to ray
		if( range_along < 0.0f )
			range = -vec_to.length( ); // off start point
		else if( range_along > length )
			range = - ( vec_position - dst ).length( ); // off end point
		else
			range = ( vec_position - ( vec_direction * range_along + src ) ).length( );	// within ray bounds

		if( range < 0.f || range > 60.f )
			return;

		game::trace_t plr_tr;
		game::g_engine_trace->clip_ray_to_entity( game::ray_t{ src, dst }, CS_MASK_SHOOT_PLAYER, player, &plr_tr );
		
		if( plr_tr.m_frac > trace.m_frac )
			plr_tr = trace;
	}

	bool c_auto_wall::fire_bullet( game::cs_weapon_t* wpn, sdk::vec3_t& direction, bool& visible, 
		float& cur_dmg, int& remaining_pen, int& hit_group, int& hitbox, game::base_entity_t* entity, float length, const sdk::vec3_t& pos )
	{ 
		if( !g_local_player->self( ) 
			|| !g_local_player->self( )->alive( ) )
			return false;

		if( !wpn )
			return false;

		game::weapon_info_t* wpn_data = wpn->info( );

		if( !wpn_data )
			return false;

		game::cvar_t* dmg_reduction_bullets = game::g_cvar->find_var( xor_str( "ff_damage_reduction_bullets" ) );
		game::cvar_t* dmg_bullet_pen = game::g_cvar->find_var( xor_str( "ff_damage_bullet_penetration" ) );

		game::trace_t enter_trace{ };

		cur_dmg = float( wpn_data->m_dmg );

		sdk::vec3_t start_pos = pos;
		sdk::vec3_t end{ };

		game::trace_filter_skip_two_entities_t trace_filter{ };
		trace_filter.m_ignore_entity0 = g_local_player->self( );
		trace_filter.m_ignore_entity1 = nullptr; // should be last hit entity, but useless to set that

		float cur_dist = 0.f;
		float max_range = wpn_data->m_range;
		float pen_power = wpn_data->m_penetration;

		int possible_hit_remain = remaining_pen = ( wpn->item_index( ) == game::e_item_index::taser ? 0 : max_pen );

		while( cur_dmg > 0.f )
		{ 
			max_range -= cur_dist;
			end = start_pos + direction * max_range;
			sdk::vec3_t ext_end = end + ( direction * 40.f );

			game::g_engine_trace->trace_ray(
				game::ray_t{ pos, end }, CS_MASK_SHOOT_PLAYER,
				reinterpret_cast< game::trace_filter_t* >( &trace_filter ), &enter_trace
			);

			if( entity->is_valid_ptr( ) ) 
				clip_trace_to_player( pos, ext_end, enter_trace, static_cast < game::cs_player_t* >( entity ) );

			game::surface_data_t* enter_surf_data = game::g_surface_data->get( enter_trace.m_surface.m_surface_props );
			float enter_surf_pen_mod = enter_surf_data->m_game.m_pen_modifier;

			if( enter_trace.m_frac == 1.0f )
				break;

			cur_dist += enter_trace.m_frac * max_range;
			cur_dmg *= std::pow( wpn_data->m_range_modifier, cur_dist / 500.f );

			game::cs_player_t* hit_player = static_cast < game::cs_player_t* >( enter_trace.m_entity );

			if( hit_player->is_valid_ptr( ) ) { 
				const bool can_do_dmg = enter_trace.m_hitgroup != game::e_hitgroup::gear && enter_trace.m_hitgroup != game::e_hitgroup::generic;
				const bool is_player = ( reinterpret_cast< game::cs_player_t*>( enter_trace.m_entity ))->is_player( );
				const bool is_enemy = !( reinterpret_cast< game::cs_player_t*>( enter_trace.m_entity )->friendly( g_local_player->self( ) ) );

				if( can_do_dmg 
					&& is_player 
					&& is_enemy
					&& hit_player->is_player( ) )
				{ 
					scale_dmg( hit_player, enter_trace, wpn_data, cur_dmg, enter_trace.m_hitgroup );
					hitbox = static_cast < int >( enter_trace.m_hitbox );
					hit_group = static_cast< int >( enter_trace.m_hitgroup );
					return true;
				}
			}
			
			if( ( cur_dist > pen_dist && wpn_data->m_penetration > 0.f ) || enter_surf_pen_mod < 0.1f )
				break;

			if( !possible_hit_remain )
				break;

			if( !handle_bullet_penetration( wpn_data, enter_trace, start_pos, direction,
				possible_hit_remain, cur_dmg, pen_power, dmg_reduction_bullets->get_float( ), dmg_bullet_pen->get_float( ), cur_dist ) ) { 
				remaining_pen = possible_hit_remain;
				break;
			}

			remaining_pen = possible_hit_remain;
			visible = false;
		}

		return false;
	}

	bool c_auto_wall::wall_penetration( sdk::vec3_t& eye_pos, hacks::point_t* point, game::cs_player_t* e )
	{ 
		// only run awall if input data was given
		if( !point )
			return false;

		// only run awall if player is valid
		if( !e 	
			|| e->networkable( )->dormant( ) 
			|| !e->alive( ) ) 
			return false;

		const sdk::vec3_t tmp = point->m_pos - eye_pos;

		sdk::qang_t angles = sdk::qang_t( 0.f, 0.f, 0.f );
		sdk::vec_angs( tmp, angles );

		sdk::vec3_t direction = sdk::vec3_t( 0.f, 0.f, 0.f );
		sdk::ang_vecs( angles, &direction, nullptr, nullptr );

		direction.normalize( );

		bool visible = true;
		float damage = -1.f;
		int hitbox = -1;
		int remaining_pen { -1 };
		int hitgroup{ -1 };

		game::cs_weapon_t* weapon = g_local_player->self( )->weapon( );

		if( !weapon->is_valid_ptr( ) )
			return false;

		bool has_data = fire_bullet( weapon, direction, visible, damage, remaining_pen, hitgroup, hitbox, e, 0.0f, eye_pos );

		if( has_data ) { 
			point->m_remaining_pen = remaining_pen;
			point->m_dmg = damage;
			point->m_hitgroup = hitgroup;
			point->m_hitbox = hitbox;
		}

		return has_data;
	}
}