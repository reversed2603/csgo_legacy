#pragma once
#include "../anti_aim.h"

namespace csgo::hacks {
	__forceinline bool& c_anti_aim::can_choke( ) {
		return m_can_choke;
	}

	__forceinline bool c_anti_aim::should_disable( game::user_cmd_t& user_cmd ) {
		const auto cur_move_type = g_local_player->self( )->move_type( );

		game::e_move_type cmd_move_type{ }, cmd_pred_move_type{ };
		cmd_move_type = g_local_player->self( )->move_type( );
		cmd_pred_move_type = g_local_player->self( )->move_type( );

		return cur_move_type != game::e_move_type::noclip
			&& cur_move_type != game::e_move_type::ladder
			&& ( 
				( cmd_move_type != game::e_move_type::ladder
					&& cmd_pred_move_type != game::e_move_type::ladder ) ) // user_cmd->m_view_angles.y != 58.f
			&& !( g_local_player->self( )->flags( ) & game::e_ent_flags::frozen )
			&& !( g_ctx->anim_data( ).m_local_data.m_shot );
	}

	__forceinline const sdk::qang_t& c_anti_aim::last_anim_ang( ) {
		return m_last_anim_ang;
	}

	__forceinline float c_anti_aim::get_manual_rotate( ) {
		static std::ptrdiff_t type { };

		auto current = -1;

		if( g_key_binds->get_keybind_state( &m_cfg->m_forward_manual_key ) )
			current = 3;
		if( g_key_binds->get_keybind_state( &m_cfg->m_left_manual_key ) )
			current = 2;
		if( g_key_binds->get_keybind_state( &m_cfg->m_right_manual_key ) )
			current = 1;
		if( g_key_binds->get_keybind_state( &m_cfg->m_back_manual_key ) )
			current = 4;

		static bool prev_state;
		const auto state = current >= 0;
		if( prev_state != state ) {
			if( state ) {
				if( current == type )
					type = -1;
				else
					type = current;
			}

			prev_state = state;
		}

		if( !m_cfg->m_manual_antiaim )
			type = -1;

		if( type == 4 ) {
			g_visuals->m_cur_yaw_dir = 4;
			return std::numeric_limits < float > ::max( );
		}

		if( type == 1 )
			g_visuals->m_cur_yaw_dir = 2;
				
		if( type == 2 )
			g_visuals->m_cur_yaw_dir = 1;

		if( type == 3 )
			g_visuals->m_cur_yaw_dir = 3;

		float return_value = 0.f;

		if( type == 1 )
			return_value = -80.f;
		if( type == 2 )
			return_value = 80.f;
		if( type == 3 )
			return_value = 345.f;

		if( type != -1 )
			return return_value;

		g_visuals->m_cur_yaw_dir = 0;
		return std::numeric_limits < float > ::max( );
	}
}