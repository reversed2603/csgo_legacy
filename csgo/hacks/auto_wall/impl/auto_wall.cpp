#include "../../../csgo.hpp"

namespace csgo::hacks {

	bool is_armored( valve::cs_player_t* player, int hit_group ) {
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

	void c_auto_wall::scale_dmg( valve::cs_player_t* player, valve::trace_t& trace, valve::weapon_info_t* wpn_info, float& dmg, const int hit_group ) {
		if( !player || !player->is_player( ) )
			return;

		const bool armored = is_armored( player, hit_group );
		const bool has_heavy_armor = player->has_heavy_armor( );
		const bool is_zeus = g_local_player->self( )->weapon( ) ? g_local_player->self( )->weapon( )->item_index( ) == valve::e_item_index::taser : false;
		const float armor_val = static_cast <float>( player->armor_val( ) );

		if( !is_zeus ) {
			switch ( hit_group ) {
			case 1:
				dmg = ( dmg * 4.f );

				if( has_heavy_armor )
					dmg *= ( dmg * 0.5f );
				break;
			case 3:
				dmg = ( dmg * 1.25f );
				break;
			case 6:
			case 7:
				dmg = ( dmg * 0.75f );
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
			float dmg_to_health = 0.f;

			if( has_heavy_armor ) {
				armor_ratio_calced = armor_ratio * 0.25f;
				armor_bonus_ratio = 0.33f;
				armor_scale = 0.33f;

				dmg_to_health = ( dmg * armor_ratio_calced ) * 0.85f;
			}
			else
				dmg_to_health = dmg * armor_ratio_calced;

			float dmg_to_armor = ( dmg - dmg_to_health ) * ( armor_scale * armor_bonus_ratio );

			if( dmg_to_armor > armor_val )
				dmg_to_health = dmg - ( armor_val / armor_bonus_ratio );

			dmg = dmg_to_health;
		}

		dmg = std::floor( dmg );
	}


	inline void c_auto_wall::util_traceline( 
		const sdk::vec3_t& vec_abs_start,
		const sdk::vec3_t& vec_abs_end,
		unsigned int mask,
		valve::base_entity_t* ignore,
		int collision_group,
		valve::trace_t* ptr ) {

		valve::trace_filter_simple_t trace_filter( ignore, collision_group );
		valve::g_engine_trace->trace_ray( valve::ray_t( vec_abs_start, vec_abs_end ), mask, ( valve::base_trace_filter_t* )&trace_filter, ptr );
	}

	static bool trace_to_exit_game( sdk::vec3_t start, sdk::vec3_t dir, sdk::vec3_t& end, valve::trace_t& tr_enter, valve::trace_t& exit_trace, float step_size, float max_dist )
	{
		float tr_dist = 0;
		int start_contents{ };

		while( tr_dist <= max_dist ) {

			tr_dist += step_size;
			end = start + ( dir * tr_dist );
			sdk::vec3_t tr_end = end - ( dir * step_size );

			int current_contents = valve::g_engine_trace->get_point_contents( end, CS_MASK_SHOOT_PLAYER );

			if( !start_contents )
				start_contents = current_contents;

			if( ( current_contents & CS_MASK_SHOOT ) == 0 || ( ( current_contents & CONTENTS_HITBOX ) && start_contents != current_contents ) )
			{
				// this gets a bit more complicated and expensive when we have to deal with displacements
				g_auto_wall->util_traceline( end, tr_end, CS_MASK_SHOOT_PLAYER, NULL, 0,  &exit_trace );

				// we exited the wall into a player's hitbox
				if( exit_trace.m_start_solid && ( exit_trace.m_surface.m_flags & SURF_HITBOX ) )
				{
					// do another trace, but skip the player to get the actual exit surface 
					g_auto_wall->util_traceline( end, start, CS_MASK_SHOOT, exit_trace.m_entity, 0, &exit_trace );

					if( exit_trace.hit( ) && !exit_trace.m_start_solid ) {
						end = exit_trace.m_end;
						return true;
					}
				}
				else if( exit_trace.hit( ) && !exit_trace.m_start_solid )
				{
					const bool start_is_nodraw = !!( tr_enter.m_surface.m_flags & ( SURF_NODRAW ) );
					const bool exit_is_nodraw = !!( exit_trace.m_surface.m_flags & ( SURF_NODRAW ) );

					// we have a case where we have a breakable object
					// but the mapper put a nodraw on the backside
					if( exit_is_nodraw 
						&& g_auto_wall->is_breakable( exit_trace.m_entity ) 
						&& g_auto_wall->is_breakable( tr_enter.m_entity ) ) {
						end = exit_trace.m_end;
						return true;
					}
					// exit nodraw is only valid if our entrace is also nodraw
					else if( !exit_is_nodraw || ( start_is_nodraw && exit_is_nodraw ) ) {
						if( dir.dot( exit_trace.m_plane.m_normal ) <= 1.0f ) {
							// get the real end pos
							end -= ( dir * ( step_size * exit_trace.m_frac ) );
							return true;
						}
					}
				}
				else if( ( tr_enter.m_entity != valve::g_entity_list->get_entity( 0 ) && tr_enter.m_entity != NULL ) 
					&& g_auto_wall->is_breakable( tr_enter.m_entity ) ) {
					// if we hit a breakable, make the assumption that we broke it if we can't find an exit ( hopefully.. )
					// fake the end pos
					exit_trace = tr_enter;
					exit_trace.m_end = start + ( dir * 1.f );
					return true;
				}
			}
		}
		return false;
	}

	bool c_auto_wall::handle_bullet_penetration( 
		valve::weapon_info_t* wpn_data, valve::trace_t& enter_trace, sdk::vec3_t& eye_pos, const sdk::vec3_t& direction,
		int& possible_hits_remain, float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, float& trace_len
	,	float pen_modifier, 
		float dmg_modifier ) {
		if( !wpn_data )
			return false;

		if( possible_hits_remain <= 0 || wpn_data->m_penetration <= 0.f )
			return false;

		const bool contents_grate = ( enter_trace.m_contents & CONTENTS_GRATE );
		const bool surf_nodraw = ( enter_trace.m_surface.m_flags & SURF_NODRAW );

		valve::surface_data_t* enter_surf_data = valve::g_surface_data->get( enter_trace.m_surface.m_surface_props );

		if( !enter_surf_data )
			return false;

		const std::uint16_t enter_material = enter_surf_data->m_game.m_material;
		valve::trace_t exit_trace;
		sdk::vec3_t end_;

		if( !trace_to_exit_game( enter_trace.m_end, direction, end_, enter_trace, exit_trace, 4.f, 90.f ) ) {
			
			if( !( valve::g_engine_trace->get_point_contents( enter_trace.m_end, CS_MASK_SHOOT ) & CS_MASK_SHOOT ) )
				return false; // ended up in solid
		}

		valve::surface_data_t* exit_surface_data = valve::g_surface_data->get( exit_trace.m_surface.m_surface_props );

		if( !exit_surface_data )
			return false;

		const std::uint16_t exit_material = exit_surface_data->m_game.m_material;

		// percent of total damage lost automatically on impacting a surface
		float dmg_lost_percent = 0.16f;

		// since some railings in de_inferno are CONTENTS_GRATE but CHAR_TEX_CONCRETE, we'll trust the
		// CONTENTS_GRATE and use a high damage modifier.
		if( contents_grate || surf_nodraw || enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE ) {

			// If we're a concrete grate ( TOOLS/TOOLSINVISIBLE texture ) allow more penetrating power.
			if( enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE ) {
				pen_modifier = 3.0f;
				dmg_lost_percent = 0.05f;
			}
			else
				pen_modifier = 1.0f;
		}
		else if( enter_material == CHAR_TEX_FLESH 
			&& ff_damage_reduction_bullets == 0.f
			&& enter_trace.m_entity 
			&& enter_trace.m_entity->is_player( ) 
			&& ( ( valve::cs_player_t* )enter_trace.m_entity )->team( ) == g_local_player->self( )->team( ) ) {

			if( ff_damage_bullet_penetration == 0.f ) 
				return true;

			pen_modifier = ff_damage_bullet_penetration;
		}
		else {
			// check the exit material and average the exit and entrace values
			float exit_pen_mod = exit_surface_data->m_game.m_pen_modifier;
			pen_modifier = ( pen_modifier + exit_pen_mod ) / 2.f;
		}

		// if enter & exit point is wood we assume this is 
		// a hollow crate and give a penetration bonus
		if( enter_material == exit_material ) {
			if( exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD )
				pen_modifier = 3.f;
			else if( exit_material == CHAR_TEX_PLASTIC )
				pen_modifier = 2.f;
		}

		// NOTE: game src uses float and not float& which means this does not exist / work
		// trace_len = ( exit_trace.m_end - enter_trace.m_end ).length( );

		float pen_mod = std::max( 0.f, ( 1.f / pen_modifier ) );
		float percent_damage_chunk = cur_dmg * dmg_lost_percent;
		float pen_wep_mod = percent_damage_chunk + std::max( 0.f, ( 3.f / wpn_data->m_penetration ) * 1.25f ) * ( pen_mod * 3.f );
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
		const sdk::vec3_t& src, const sdk::vec3_t& dst, valve::trace_t& trace,
		valve::cs_player_t* const player, const valve::should_hit_fn_t& should_hit_fn
	 )
	{
		if( !player || !player->alive( ) )
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
			range = -( pos - dst ).length( );

		else
		{
			auto ray( pos - ( dir * range_along + src ) );
			range = ray.length( );
		}

		if( range <= 60.f && range > 0.0f )
		{
			valve::trace_t trace_;
			valve::g_engine_trace->clip_ray_to_entity( { src, dst }, CS_MASK_SHOOT_PLAYER, player, &trace_ );
			if( trace.m_frac > trace_.m_frac )
				trace = trace_;
		}
	}

	bool c_auto_wall::fire_bullet( valve::cs_weapon_t* wpn, sdk::vec3_t& direction, bool& visible, float& cur_dmg, int& remaining_pen, int& hit_group, int& hitbox, valve::base_entity_t* e, float length, const sdk::vec3_t& pos )
	{
		if( !wpn || !wpn->info( ) )
			return false;

		auto wpn_data = wpn->info( );

		if( !wpn_data )
			return false;

		static auto dmg_reduction_bullets = valve::g_cvar->find_var( xor_str( "ff_damage_reduction_bullets" ) );
		static auto dmg_bullet_pen = valve::g_cvar->find_var( xor_str( "ff_damage_bullet_penetration" ) );

		valve::trace_t enter_trace;

		cur_dmg = ( float )wpn_data->m_dmg;

		auto eye_pos = pos;
		auto cur_dist = 0.0f;
		auto max_range = wpn_data->m_range;
		auto pen_dist = 3000.0f;
		auto pen_power = wpn_data->m_penetration;
		auto possible_hit_remain = 4;

		float dmg_modifier = 0.5f;
		float pen_modifier = 1.0f;

		remaining_pen = 4;
		while( cur_dmg > 0.f )
		{
			max_range -= cur_dist;
			auto end = eye_pos + direction * max_range;

			valve::trace_filter_simple_t filter;
			filter.m_ignore_entity = g_local_player->self( );

			valve::g_engine_trace->trace_ray( valve::ray_t( eye_pos, end ), CS_MASK_SHOOT_PLAYER, reinterpret_cast< valve::base_trace_filter_t* >( &filter ), &enter_trace );
			if( e ) {
				clip_trace_to_player( eye_pos, end, enter_trace, static_cast <valve::cs_player_t*>( e ), filter.m_should_hit_fn );
			}

			if( enter_trace.m_frac == 1.0f )
				break;

			cur_dist += enter_trace.m_frac * max_range;
			cur_dmg *= pow( wpn_data->m_range_modifier, cur_dist / 500.0f );

			valve::cs_player_t* hit_player = static_cast <valve::cs_player_t*>( enter_trace.m_entity );

			if( hit_player ) {

				const bool can_do_dmg = enter_trace.m_hitgroup <= valve::e_hitgroup::right_leg && enter_trace.m_hitgroup > valve::e_hitgroup::generic;
				const bool is_player = ( ( valve::cs_player_t* )enter_trace.m_entity )->is_player( );
				const bool is_enemy = ( ( valve::cs_player_t* )enter_trace.m_entity )->team( ) != g_local_player->self( )->team( );

				if( can_do_dmg
					&& is_player
					&& is_enemy
					&& hit_player
					&& hit_player->is_player( ) )
				{
					scale_dmg( hit_player, enter_trace, wpn_data, cur_dmg, static_cast< std::ptrdiff_t >( enter_trace.m_hitgroup ) );
					hitbox = static_cast <int>( enter_trace.m_hitbox );
					hit_group = static_cast<int>( enter_trace.m_hitgroup );
					return true;
				}
			}
			
			valve::surface_data_t* surface_data = valve::g_surface_data->get( enter_trace.m_surface.m_surface_props );

			if( !surface_data )
				break;

			pen_modifier = surface_data->m_game.m_pen_modifier;
			dmg_modifier = surface_data->m_game.m_dmg_modifier;

			if( ( cur_dist > pen_dist && wpn_data->m_penetration > 0.f ) || pen_modifier < 0.1f )
				break;

			if( !possible_hit_remain )
				break;

			if( !handle_bullet_penetration( wpn_data, enter_trace, eye_pos, direction,
				possible_hit_remain, 
				cur_dmg,
				pen_power,
				dmg_reduction_bullets->get_float( ), 
				dmg_bullet_pen->get_float( ), 
				cur_dist, 
				pen_modifier, 
				dmg_modifier ) ) {
				remaining_pen = possible_hit_remain;
				break;
			}

			remaining_pen = possible_hit_remain;
			visible = false;
		}

		return false;
	}

	auto_wall_data_t c_auto_wall::wall_penetration( sdk::vec3_t& eye_pos, sdk::vec3_t& point, valve::cs_player_t* e )
	{
		if( !e || !e->alive( ) )
			return auto_wall_data_t( -1, -1, -1 );

		auto tmp = point - eye_pos;

		auto angles = sdk::qang_t( 0.f, 0.f, 0.f );
		sdk::vec_angs( tmp, angles );

		auto direction = sdk::vec3_t( 0.f, 0.f, 0.f );
		sdk::ang_vecs( angles, &direction, nullptr, nullptr );

		direction.normalize( );

		auto visible = true;
		auto damage = -1.0f;
		auto hitbox = -1;
		int remaining_pen{ -1 };
		int hitgroup{ -1 };

		auto weapon = g_local_player->self( )->weapon( );

		if( fire_bullet( weapon, direction, visible, damage, remaining_pen, hitgroup, hitbox, e, 0.0f, eye_pos ) )
			return auto_wall_data_t( ( int )damage, hitbox, hitgroup );

		return auto_wall_data_t( -1, -1, -1 );
	}
}