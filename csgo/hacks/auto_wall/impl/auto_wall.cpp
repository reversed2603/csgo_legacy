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
		const float armor_val = static_cast < float >( player->armor_val( ) );

		if( !is_zeus ) {
			switch( hit_group ) {
			case 1:
				dmg = ( dmg * 4.f );

				if( has_heavy_armor )
					dmg *= ( dmg * 0.5f );
				break;
			case 3:
				dmg = ( dmg * 1.25f ) ;
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

	bool c_auto_wall::trace_to_exit( 
		const sdk::vec3_t& src, const sdk::vec3_t& dir,
		const valve::trace_t& enter_trace, valve::trace_t& exit_trace
	 ) 
	{
		sdk::vec3_t end;
		float distance = 0.f;
		int first_contents{ };

		while( distance <= 90.f )
		{
			distance += 4.f;
			end = src + dir * distance;

			if( !first_contents )
				first_contents = valve::g_engine_trace->get_point_contents( end, MASK_SHOT );

			int point_contents = valve::g_engine_trace->get_point_contents( end, MASK_SHOT );

			if( !( point_contents & ( MASK_SHOT_HULL | CONTENTS_HITBOX ) ) || ( point_contents & CONTENTS_HITBOX ) && point_contents != first_contents )
			{
				valve::g_engine_trace->trace_ray( valve::ray_t( end, end - dir * 4.f ), MASK_SHOT, nullptr, &exit_trace );

				if( exit_trace.m_start_solid && exit_trace.m_surface.m_flags & SURF_HITBOX )
				{
					valve::trace_filter_simple_t trace_filter { exit_trace.m_entity, 0 };

					valve::g_engine_trace->trace_ray(valve::ray_t(src, end), CS_MASK_SHOOT_PLAYER, reinterpret_cast< valve::base_trace_filter_t* >( &trace_filter ), &exit_trace);

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

				if( exit_trace.m_surface.m_flags & SURF_NODRAW )
				{
					if( is_breakable( enter_trace.m_entity ) && is_breakable( exit_trace.m_entity ) )
						return true;
					else if( !( enter_trace.m_surface.m_flags & SURF_NODRAW ) )
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
		valve::weapon_info_t* wpn_data, valve::trace_t& enter_trace, sdk::vec3_t& eye_pos, const sdk::vec3_t& direction,
		int& possible_hits_remain, float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, float& trace_len
	 ) {
		if( !wpn_data )
			return false;

		if( possible_hits_remain <= 0 || wpn_data->m_penetration <= 0.0f )
			return false;

		const bool contents_grate = ( enter_trace.m_contents & CONTENTS_GRATE );
		const bool surf_nodraw = ( enter_trace.m_surface.m_flags & SURF_NODRAW );

		valve::surface_data_t* enter_surf_data = valve::g_surface_data->get( enter_trace.m_surface.m_surface_props );

		if( !enter_surf_data )
			return false;

		const std::uint16_t enter_material = enter_surf_data->m_game.m_material;
		valve::trace_t exit_trace;

		if( !trace_to_exit( enter_trace.m_end, direction, enter_trace, exit_trace ) 
			&& !( valve::g_engine_trace->get_point_contents( enter_trace.m_end, CS_MASK_SHOOT ) & CS_MASK_SHOOT ) )
			return false;

		valve::surface_data_t* exit_surface_data = valve::g_surface_data->get( exit_trace.m_surface.m_surface_props );

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
		const sdk::vec3_t& src, const sdk::vec3_t& dst, valve::trace_t& trace,
		valve::cs_player_t* const player, const valve::should_hit_fn_t& should_hit_fn
	 )
	{
		if( !player || !player->networkable( ) || player->networkable( )->dormant( ) || !player->alive( ) )
			return;

		// get bounding box
		const sdk::vec3_t mins = player->obb_min( );
		const sdk::vec3_t maxs = player->obb_max( );
		const sdk::vec3_t center = ( maxs + mins ) / 2.f;

		// calculate world space center
		const sdk::vec3_t vec_position = center + player->origin( );

		const sdk::vec3_t vec_to = vec_position - src;
		sdk::vec3_t vec_direction = dst - src;
		const float length = vec_direction.normalize( );

		const float range_along = vec_direction.dot( vec_to );
		float range = 0.0f;

		// calculate distance to ray
		if( range_along < 0.0f )
			range = -vec_to.length( ); // off start point
		else if( range_along > length )
			range = -( vec_position - dst ).length( ); // off end point
		else
			range = ( vec_position - ( vec_direction * range_along + src ) ).length( );	// within ray bounds

		if( range < 0.0f || range > 60.0f )
			return;

		valve::trace_t plr_tr;
		valve::g_engine_trace->clip_ray_to_entity( { src, dst }, CS_MASK_SHOOT_PLAYER, player, &plr_tr);
		if( plr_tr.m_frac > trace.m_frac  )
			plr_tr = trace;
	}

	bool c_auto_wall::fire_bullet( valve::cs_weapon_t* wpn, sdk::vec3_t& direction, bool& visible, 
		float& cur_dmg, int& remaining_pen, int& hit_group, int& hitbox, valve::base_entity_t* entity, float length, const sdk::vec3_t& pos )
	{
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return false;

		if( !wpn )
			return false;

		auto wpn_data = wpn->info( );

		if( !wpn_data )
			return false;

		static auto dmg_reduction_bullets = valve::g_cvar->find_var( xor_str( "ff_damage_reduction_bullets" ) );
		static auto dmg_bullet_pen = valve::g_cvar->find_var( xor_str( "ff_damage_bullet_penetration" ) );

		valve::trace_t enter_trace;

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

			valve::trace_filter_simple_t filter{ };
			filter.m_ignore_entity = g_local_player->self( );

			valve::g_engine_trace->trace_ray( { eye_pos, end }, CS_MASK_SHOOT_PLAYER, reinterpret_cast< valve::base_trace_filter_t* >( &filter ), &enter_trace );
			if( entity ) {
				clip_trace_to_player( eye_pos, end, enter_trace, static_cast < valve::cs_player_t* >( entity ), filter.m_should_hit_fn );
			}
			auto enter_surf_data = valve::g_surface_data->get( enter_trace.m_surface.m_surface_props );
			auto enter_surf_pen_mod = enter_surf_data->m_game.m_pen_modifier;

			if( enter_trace.m_frac == 1.0f )
				break;

			cur_dist += enter_trace.m_frac * max_range;
			cur_dmg *= pow( wpn_data->m_range_modifier, cur_dist / 500.0f );

			valve::cs_player_t* hit_player = static_cast < valve::cs_player_t* >( enter_trace.m_entity );

			auto can_do_dmg = enter_trace.m_hitgroup != valve::e_hitgroup::gear && enter_trace.m_hitgroup != valve::e_hitgroup::generic;
			auto is_player = ( ( valve::cs_player_t* ) enter_trace.m_entity )->is_player( );
			auto is_enemy = ( ( valve::cs_player_t* ) enter_trace.m_entity )->team( ) != g_local_player->self( )->team( );

			if( can_do_dmg 
				&& is_player 
				&& is_enemy
				&& hit_player
				&& hit_player->is_player( ) )
			{
				scale_dmg( hit_player, enter_trace, wpn_data, cur_dmg, static_cast < std::ptrdiff_t >( enter_trace.m_hitgroup ) );
				hitbox = static_cast < int >( enter_trace.m_hitbox );
				hit_group = static_cast< int >( enter_trace.m_hitgroup );
				return true;
			}
			
			if( ( cur_dist > pen_dist && wpn_data->m_penetration > 0.f ) || enter_surf_pen_mod < 0.1f )
				break;

			if( !possible_hit_remain )
				break;

			if( !handle_bullet_penetration( wpn_data, enter_trace, eye_pos, direction,
				possible_hit_remain, cur_dmg, pen_power, dmg_reduction_bullets->get_float( ), dmg_bullet_pen->get_float( ), cur_dist ) ) {
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
		if( !e || e->networkable( )->dormant( ) || !e->alive( ) )
			return auto_wall_data_t( -1, -1, -1, -1 );

		auto tmp = point - eye_pos;

		auto angles = sdk::qang_t( 0.f, 0.f, 0.f );
		sdk::vec_angs( tmp, angles );

		auto direction = sdk::vec3_t( 0.f, 0.f, 0.f );
		sdk::ang_vecs( angles, &direction, nullptr, nullptr );

		direction.normalize( );

		auto visible = true;
		auto damage = -1.0f;
		auto hitbox = -1;
		int remaining_pen { -1 };
		int hitgroup{ -1 };

		auto weapon = g_local_player->self( )->weapon( );

		if( fire_bullet( weapon, direction, visible, damage, remaining_pen, hitgroup, hitbox, e, 0.0f, eye_pos ) )
		{
			return auto_wall_data_t( ( int ) damage, hitbox, hitgroup, remaining_pen );
		}
		else
		{
			return auto_wall_data_t( -1, -1, -1, -1 );
		}
	}
}