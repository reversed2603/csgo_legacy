#pragma once
#include "../eng_pred.h"

namespace csgo::hacks {
	__forceinline void c_eng_pred::net_vars_t::restore( const int cmd_number ) const {
		if( cmd_number != m_cmd_number )
			return;

		int counter { };

		if( std::abs( g_local_player->self( )->view_offset( ).z( ) - m_view_offset_z ) <= 0.03125f )
			g_local_player->self( )->view_offset( ).z( ) = m_view_offset_z;

		const auto aim_punch_delta = g_local_player->self( )->aim_punch( ) - m_aim_punch;
		if( std::abs( aim_punch_delta.x( ) ) <= 0.03125f
			&& std::abs( aim_punch_delta.y( ) ) <= 0.03125f
			&& std::abs( aim_punch_delta.z( ) ) <= 0.03125f )
			g_local_player->self( )->aim_punch( ) = m_aim_punch;
		else
			counter++;

		const auto aim_punch_vel_delta = g_local_player->self( )->aim_punch_vel( ) - m_aim_punch_vel;
		if( std::abs( aim_punch_vel_delta.x( ) ) <= 0.03125f
			&& std::abs( aim_punch_vel_delta.y( ) ) <= 0.03125f
			&& std::abs( aim_punch_vel_delta.z( ) ) <= 0.03125f )
			g_local_player->self( )->aim_punch_vel( ) = m_aim_punch_vel;
		else
			counter++;

		const auto view_punch_delta = g_local_player->self( )->view_punch( ) - m_view_punch;
		if( std::abs( view_punch_delta.x( ) ) <= 0.03125f
			&& std::abs( view_punch_delta.y( ) ) <= 0.03125f
			&& std::abs( view_punch_delta.z( ) ) <= 0.03125f )
			g_local_player->self( )->view_punch( ) = m_view_punch;
		else
			counter++;

		const auto velocity_modifier_delta = g_local_player->self( )->velocity_modifier( ) - m_velocity_modifier;
		if( std::abs( velocity_modifier_delta ) <= 0.00625f )
			g_local_player->self( )->velocity_modifier( ) = m_velocity_modifier;
		else
			counter++;

		const auto fall_velocity_delta = g_local_player->self( )->fall_velocity( ) - m_fall_velocity;
		if( std::abs( fall_velocity_delta ) <= 0.5f )
			g_local_player->self( )->fall_velocity( ) = m_fall_velocity;
		else
			counter++;

		const auto velocity_delta = g_local_player->self( )->velocity( ) - m_velocity;
		if( std::abs( velocity_delta.x( ) ) <= 0.5f
			&& std::abs( velocity_delta.y( ) ) <= 0.5f
			&& std::abs( velocity_delta.z( ) ) <= 0.5f )
			g_local_player->self( )->velocity( ) = m_velocity;
		else
			counter++;

		const auto origin_delta = g_local_player->self( )->origin( ) - m_origin;
		if( std::abs( origin_delta.x( ) ) <= 0.1f
			&& std::abs( origin_delta.y( ) ) <= 0.1f
			&& std::abs( origin_delta.z( ) ) <= 0.1f )
			g_local_player->self( )->origin( ) = m_origin;
		else
			counter++;

		hacks::g_eng_pred->is_out_of_epsilon( ) = counter > 0 ? true : false;
	}

	__forceinline void c_eng_pred::net_vars_t::store( const int cmd_number ) {
		m_cmd_number = cmd_number;

		m_view_offset_z = std::clamp( g_local_player->self( )->view_offset( ).z( ), 0.f, 64.f );
		m_aim_punch = g_local_player->self( )->aim_punch( );
		m_aim_punch_vel = g_local_player->self( )->aim_punch_vel( );
		m_view_punch = g_local_player->self( )->view_punch( );
		m_velocity = g_local_player->self( )->velocity( );
		m_origin = g_local_player->self( )->origin( );
		m_velocity_modifier = g_local_player->self( )->velocity_modifier( );
		m_fall_velocity = g_local_player->self( )->fall_velocity( );
	}

	__forceinline void c_eng_pred::velocity_modifier_to_data_map( )
	{
		static const auto& net_var = g_ctx->offsets( ).m_cs_player.m_velocity_modifier;

		game::data_map_t* data_map = g_local_player->self( )->get_pred_desc_map( );

		game::type_desc_t* type_desc = g_local_player->self( )->get_data_map_entry( data_map, xor_str( "m_vphysicsCollisionState" ) );

		if( net_var > 0 )
		{
			if( type_desc )
			{
				const auto recovery_rate = 1.f / 2.5f;
				const auto tolerance = recovery_rate * game::g_global_vars.get( )->m_interval_per_tick;

				if( type_desc->m_tolerance != tolerance )
				{
					int offset = net_var;

					type_desc->m_type = ( int )game::e_field_type::_float;
					type_desc->m_tolerance = tolerance;
					type_desc->m_offset = offset;
					type_desc->m_field_size_in_bytes = sizeof( float );
					type_desc->m_flat_offset[ 0 ] = offset;

					data_map->m_packed_offsets_computed = false;
					data_map->m_packed_size = 0;
				}
			}
		}
	}

	__forceinline void c_eng_pred::local_data_t::init( const game::user_cmd_t& user_cmd ) {
		std::memset( this, 0, sizeof( local_data_t ) );

		g_eng_pred->velocity_modifier( ) = g_local_player->self( )->velocity_modifier( );

		if( const auto weapon = g_local_player->self( )->weapon( ) ) {
			g_eng_pred->recoil_index( ) = weapon->recoil_index( );
			g_eng_pred->accuracy_penalty( ) = weapon->accuracy_penalty( );
		}

		m_spawn_time = g_local_player->self( )->spawn_time( );
		m_tick_base = m_adjusted_tick_base = g_local_player->self( )->tick_base( );

		m_user_cmd = user_cmd;
	}

	__forceinline void c_eng_pred::store_data_map( ) {
		m_velocity = g_local_player->self( )->velocity( );
		m_origin = g_local_player->self( )->origin( );
		m_aim_punch = g_local_player->self( )->aim_punch( );
		m_aim_punch_vel = g_local_player->self( )->aim_punch_vel( );
		m_view_punch = g_local_player->self( )->view_punch( );
		m_view_offset = g_local_player->self( )->view_offset( );
		m_tick_base = g_local_player->self( )->tick_base( );
		m_flags = g_local_player->self( )->flags( );
		m_move_type = g_local_player->self( )->move_type( );
		m_duck_amt = g_local_player->self( )->duck_amt( );
		m_duck_speed = g_local_player->self( )->duck_speed( );
		m_base_velocity = g_local_player->self( )->base_velocity( );
		m_velocity_modifier_data = g_local_player->self( )->velocity_modifier( );
		m_fall_velocity = g_local_player->self( )->fall_velocity( );
	}

	__forceinline void c_eng_pred::restore_data_map( ) {
		g_local_player->self( )->velocity( ) = m_velocity;
		g_local_player->self( )->origin( ) = m_origin;
		g_local_player->self( )->aim_punch( ) = m_aim_punch;
		g_local_player->self( )->aim_punch_vel( ) = m_aim_punch_vel;
		g_local_player->self( )->view_punch( ) = m_view_punch;
		g_local_player->self( )->view_offset( ) = m_view_offset;
		g_local_player->self( )->tick_base( ) = m_tick_base;
		g_local_player->self( )->flags( ) = m_flags;
		g_local_player->self( )->move_type( ) = m_move_type;
		g_local_player->self( )->duck_amt( ) = m_duck_amt;
		g_local_player->self( )->duck_speed( ) = m_duck_speed;
		g_local_player->self( )->base_velocity( ) = m_base_velocity;
		g_local_player->self( )->velocity_modifier( ) = m_velocity_modifier_data;
		g_local_player->self( )->fall_velocity( ) = m_fall_velocity;
	}

	__forceinline bool& c_eng_pred::is_out_of_epsilon( ) {
		return m_is_out_of_epsilon;
	}

	__forceinline game::e_frame_stage& c_eng_pred::last_frame_stage( ) {
		return m_last_frame_stage;
	}

	__forceinline float& c_eng_pred::inaccuracy( ) {
		return m_inaccuracy;
	}

	__forceinline std::array< c_eng_pred::net_vars_t, 150u >& c_eng_pred::net_vars( ) {
		return m_net_vars;
	}

	__forceinline std::array< c_eng_pred::local_data_t, 150u >& c_eng_pred::local_data( ) {
		return m_local_data;
	}

	__forceinline void c_eng_pred::reset_on_spawn( ) {
		m_net_velocity_modifier = 1.f;

		std::memset( m_local_data.data( ), 0, sizeof( local_data_t ) * m_local_data.size( ) );
	}

	__forceinline float& c_eng_pred::spread( ) {
		return m_spread;
	}

	__forceinline float& c_eng_pred::min_inaccuracy( ) {
		return m_min_inaccuracy;
	}

	__forceinline float& c_eng_pred::recoil_index( ) {
		return m_recoil_index;
	}

	__forceinline float& c_eng_pred::accuracy_penalty( ) {
		return m_accuracy_penalty;
	}

	__forceinline float& c_eng_pred::velocity_modifier( ) {
		return m_velocity_modifier;
	}

	__forceinline float& c_eng_pred::net_velocity_modifier( ) {
		return m_net_velocity_modifier;
	}

	__forceinline bool& c_eng_pred::r8_can_shoot( ) {
		return m_r8_can_shoot;
	}

	__forceinline float& c_eng_pred::postpone_fire_ready_time( ) {
		return m_postpone_fire_ready_time;
	}

	__forceinline void c_eng_pred::save_view_model( )
	{
		auto view_model = game::g_entity_list->get_entity( g_local_player->self( )->view_model_handle( ) );

		if( !view_model )
			return;

		m_view_model.m_animation_parity = ( ( game::base_view_model_t* ) view_model )->anim_parity( );
		m_view_model.m_view_sequence = ( ( game::base_view_model_t* ) view_model )->sequence( );
		m_view_model.m_view_cycle = view_model->cycle( );
		m_view_model.m_anim_time = view_model->anim_time( );
	}

	__forceinline void c_eng_pred::adjust_view_model( )
	{
		auto view_model = game::g_entity_list->get_entity( g_local_player->self( )->view_model_handle( ) );

		if( !view_model )
			return;

		if( m_view_model.m_view_sequence != ( ( game::base_view_model_t* ) view_model )->sequence( ) || m_view_model.m_animation_parity != ( ( game::base_view_model_t* ) view_model )->anim_parity( ) )
			return;

		view_model->cycle( ) = m_view_model.m_view_cycle;
		view_model->anim_time( ) = m_view_model.m_anim_time;
	}

	/* fuck ups whole prediction on high ping */
	__forceinline void c_eng_pred::recompute_velocity_modifier( 
		const std::ptrdiff_t cmd_num, const bool predicted
	 ) const {
		if( !( g_local_player->self( )->flags( ) & game::e_ent_flags::on_ground ) )
			return;

		const auto diff = cmd_num - game::g_client_state.get( )->m_cmd_ack + ( predicted ? 0 : 1 );

		if( diff < 0 || m_net_velocity_modifier == 1.f )
			return;

		if( diff ) {
			auto calc_vel_mod = ( ( game::g_global_vars.get( )->m_interval_per_tick * 0.4f ) * static_cast < float >( diff ) ) + m_net_velocity_modifier;

			calc_vel_mod = std::clamp( calc_vel_mod, 0.f, 1.f );

			g_local_player->self( )->velocity_modifier( ) = calc_vel_mod;
		}
		else
			g_local_player->self( )->velocity_modifier( ) = m_net_velocity_modifier;
	}
}