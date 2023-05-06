#include "../../../csgo.hpp"

namespace csgo::hacks {

	bool is_armored( game::cs_player_t* player, int hit_group ) {
		const bool has_helmet = player->has_helmet( );
		const bool has_heavy_armor = player->has_heavy_armor( );
		const float armor_val = player->armor_val( );

		if( armor_val > 0.f ) {
			switch( hit_group ) {
			case 2:
			case 3:
			case 4:
			case 5:
				return true;
				break;
			case 1:
				return has_helmet || has_heavy_armor;
				break;
			default:
				return has_heavy_armor;
				break;
			}
		}

		return false;
	}

	void c_auto_wall::scale_dmg( game::cs_player_t* player, game::trace_t& trace, game::weapon_info_t* wpn_info, float& dmg, const int hit_group ) {
		if( !player || !player->is_player( ) )
			return;

		const bool armored = is_armored( player, hit_group );
		const bool has_heavy_armor = player->has_heavy_armor( );
		const bool is_zeus = g_local_player->self( )->weapon( ) ? g_local_player->self( )->weapon( )->item_index( ) == game::e_item_index::taser : false;
		const float armor_val = static_cast < float >( player->armor_val( ) );

		if( !is_zeus ) {
			switch( hit_group ) {
			case 1:
				dmg *= 4.f;
				break;
			case 3:
				dmg *= 1.25f;
				break;
			case 6:
			case 7:
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

	__forceinline bool hit_world( game::trace_t tr )
	{
		return tr.m_entity == game::g_entity_list->get_entity( 0 );
	}

	__forceinline bool did_hit_non_world_ent( game::trace_t tr )  {
		return tr.m_entity->is_valid_ptr( ) && !hit_world( tr );
	}

	bool c_auto_wall::trace_to_exit( sdk::vec3_t start, sdk::vec3_t dir,
		sdk::vec3_t& end, game::trace_t& tr_start, game::trace_t& tr_exit
	 )
	{
		float distance = 0;
		sdk::vec3_t last = start;
		int start_content = 0;

		while( distance <= 90.f )
		{
			distance += 4.f;

			end = start + ( dir * distance );

			sdk::vec3_t tr_end = end - ( dir * 4.f );

			if( start_content == 0 )
				start_content = game::g_engine_trace->get_point_contents( end, CS_MASK_SHOOT | CONTENTS_HITBOX );

			int current_content = game::g_engine_trace->get_point_contents( end, CS_MASK_SHOOT | CONTENTS_HITBOX );

			if( ( current_content & CS_MASK_SHOOT ) == 0 || ( ( current_content & CONTENTS_HITBOX ) && start_content != current_content ) )
			{
				// this gets a bit more complicated and expensive when we have to deal with displacements
				game::g_engine_trace->trace_ray( game::ray_t( end, tr_end ), CS_MASK_SHOOT_PLAYER, nullptr, &tr_exit );

				// we exited the wall into a player's hitbox
				if( tr_exit.m_start_solid && ( tr_exit.m_surface.m_flags & SURF_HITBOX ) )
				{
					// do another trace, but skip the player to get the actual exit surface 
					game::trace_filter_simple_t trace_filter{ tr_exit.m_entity, 0 };
					game::g_engine_trace->trace_ray( game::ray_t( end, start ), CS_MASK_SHOOT, reinterpret_cast< game::base_trace_filter_t* >( &trace_filter ), &tr_exit );

					if( tr_exit.hit( ) && !tr_exit.m_start_solid )
					{
						end = tr_exit.m_end;
						return true;
					}
				}
				else if( tr_exit.hit( ) && !tr_exit.m_start_solid )
				{
					bool start_no_draw = !!( tr_start.m_surface.m_flags & ( SURF_NODRAW ) );
					bool exit_no_draw = !!( tr_exit.m_surface.m_flags & ( SURF_NODRAW ) );
					if( exit_no_draw && g_auto_wall->is_breakable( tr_exit.m_entity ) && g_auto_wall->is_breakable( tr_start.m_entity ) )
					{
						// we have a case where we have a breakable object, but the mapper put a nodraw on the backside
						end = tr_exit.m_end;
						return true;
					}
					else if( !exit_no_draw || ( start_no_draw && exit_no_draw ) ) // exit nodraw is only valid if our entrace is also nodraw
					{
						float dot = dir.dot( tr_exit.m_plane.m_normal );
						if( dot <= 1.0f )
						{
							// get the real end pos
							end = end - ( dir * ( 4.f * tr_exit.m_frac ) );
							return true;
						}
					}
				}
				else if( did_hit_non_world_ent( tr_start ) && g_auto_wall->is_breakable( tr_start.m_entity ) )
				{
					// if we hit a breakable, make the assumption that we broke it if we can't find an exit ( hopefully.. )
					// fake the end pos
					tr_exit = tr_start;
					tr_exit.m_end = start + ( dir * 1.0f );
					return true;
				}
			}
		}

		return false;
	}

	bool c_auto_wall::handle_bullet_penetration( 
		game::weapon_info_t* wpn_data, game::trace_t& enter_trace, sdk::vec3_t& eye_pos, const sdk::vec3_t& direction,
		int& possible_hits_remain, float& cur_dmg, float penetration_power, float trace_len
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
		game::trace_t exit_trace{ };

		sdk::vec3_t pen_end{ };

		if( !trace_to_exit( enter_trace.m_end, direction, pen_end, enter_trace, exit_trace )
			&& !( game::g_engine_trace->get_point_contents( enter_trace.m_end, CS_MASK_SHOOT ) & CS_MASK_SHOOT ) )
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
		game::cs_player_t* const player, const game::should_hit_fn_t& should_hit_fn
	 )
	{
		if( !player || !player->networkable( ) 
			|| player->networkable( )->dormant( ) || !player->alive( ) )
			return;

		const sdk::vec3_t mins = player->obb_min( );
		const sdk::vec3_t maxs = player->obb_max( );

		sdk::vec3_t dir( src - dst );
		dir.normalize( );

		sdk::vec3_t
			center = ( maxs + mins ) / 2,
			pos( center + player->origin( ) );

		sdk::vec3_t to = pos - src;
		float range_along = dir.dot( to );

		float range;
		if( range_along < 0.f )
			range = -to.length( );

		else if( range_along > dir.length( ) )
			range = - ( pos - dst ).length( );

		else
		{
			auto ray( pos - ( dir * range_along + src ) );
			range = ray.length( );
		}

		if( range < 0.f || range > 60.f )
			return;

		game::trace_t trace_;
		game::g_engine_trace->clip_ray_to_entity( { src, dst }, CS_MASK_SHOOT_PLAYER, player, &trace_ );
		if( trace.m_frac > trace_.m_frac )
			trace = trace_;
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

		game::trace_t enter_trace{ };

		cur_dmg = ( float ) wpn_data->m_dmg;

		auto eye_pos = pos;
		auto cur_dist = 0.0f;
		auto max_range = wpn_data->m_range;
		auto pen_dist = 3000.0f;
		auto pen_power = wpn_data->m_penetration;
		auto possible_hit_remain = 4;
		remaining_pen = 4;
		while( cur_dmg > 0.f )
		{
			max_range -= cur_dist;
			auto end = eye_pos + direction * max_range;

			game::trace_filter_simple_t filter{ };
			filter.m_ignore_entity = g_local_player->self( );

			game::g_engine_trace->trace_ray( { eye_pos, end }, CS_MASK_SHOOT_PLAYER, reinterpret_cast< game::base_trace_filter_t* >( &filter ), &enter_trace );
			if( entity ) {
				clip_trace_to_player( eye_pos, end, enter_trace, static_cast < game::cs_player_t* >( entity ), filter.m_should_hit_fn );
			}

			game::surface_data_t* enter_surf_data = game::g_surface_data->get( enter_trace.m_surface.m_surface_props );
			float enter_surf_pen_mod = enter_surf_data->m_game.m_pen_modifier;

			if( enter_trace.m_frac == 1.0f )
				break;

			cur_dist += enter_trace.m_frac * max_range;
			cur_dmg *= pow( wpn_data->m_range_modifier, cur_dist / 500.0f );

			game::cs_player_t* hit_player = static_cast < game::cs_player_t* >( enter_trace.m_entity );

			auto can_do_dmg = enter_trace.m_hitgroup != game::e_hitgroup::gear && enter_trace.m_hitgroup != game::e_hitgroup::generic;
			auto is_player = ( ( game::cs_player_t* ) enter_trace.m_entity )->is_player( );
			auto is_enemy = ( ( game::cs_player_t* ) enter_trace.m_entity )->team( ) != g_local_player->self( )->team( );

			if( can_do_dmg 
				&& is_player 
				&& is_enemy
				&& hit_player
				&& hit_player->is_player( ) )
			{
				scale_dmg( hit_player, enter_trace, wpn_data, cur_dmg, static_cast< std::ptrdiff_t >( enter_trace.m_hitgroup ) );
				hitbox = static_cast < int >( enter_trace.m_hitbox );
				hit_group = static_cast< int >( enter_trace.m_hitgroup );
				return true;
			}
			
			if( ( cur_dist > pen_dist && wpn_data->m_penetration > 0.f ) || enter_surf_pen_mod < 0.1f )
				break;

			if( !possible_hit_remain )
				break;

			if( !handle_bullet_penetration( wpn_data, enter_trace, eye_pos, direction,
				possible_hit_remain, cur_dmg, pen_power, cur_dist ) ) {
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