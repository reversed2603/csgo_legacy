#include "../../csgo.hpp"

namespace csgo::hacks {
	bool is_grenade( const int id )
	{
		return id == 9 || id == 98 || id == 134;
	}

	void c_visuals::draw_glow( ) { 
		if( !g_local_player->self( ) )
			return;

		if( !game::g_glow->m_object_definitions.size( ) )
			return;

		const auto player_glow = [ & ]( const int idx )
		{
			game::cs_player_t* player = reinterpret_cast< game::cs_player_t* >( game::g_glow->m_object_definitions.at( idx ).m_entity );

			if( !player 
				|| !player->is_player( ) 
				|| player->friendly( g_local_player->self( ) ) 
				|| !m_cfg->m_glow )
				return;

			game::g_glow->m_object_definitions.at( idx ).m_color = { m_cfg->m_glow_clr[ 0 ], m_cfg->m_glow_clr[ 1 ], m_cfg->m_glow_clr[ 2 ] };
			game::g_glow->m_object_definitions.at( idx ).m_alpha = ( m_cfg->m_glow_clr[ 3 ] * ( m_dormant_data[ player->networkable( )->index( ) ].m_alpha / 255 ) );
			game::g_glow->m_object_definitions.at( idx ).m_render_occluded = true;
			game::g_glow->m_object_definitions.at( idx ).m_render_unoccluded = false;
			game::g_glow->m_object_definitions.at( idx ).m_bloom_amount = 0.8f;
		};

		const auto weapon_glow = [ & ]( const int idx )
		{
			game::base_entity_t* entity = reinterpret_cast< game::base_entity_t* >( game::g_glow->m_object_definitions.at( idx ).m_entity );

			if( !entity )
				return;

			const auto classid = game::g_glow->m_object_definitions.at( idx ).m_entity->networkable( )->client_class( )->m_class_id;
			if( is_grenade( classid ) 
				||	entity->is_player( ) 
				|| !entity->is_base_combat_wpn( ) 
				|| ( classid == game::e_class_id::c_c4 
					|| classid == game::e_class_id::c_planted_c4 )
				|| ( ~m_cfg->m_dropped_weapon_selection & 4 ) )
				return;

			game::g_glow->m_object_definitions.at( idx ).m_color = { m_cfg->m_draw_weapon_glow_clr[ 0 ], m_cfg->m_draw_weapon_glow_clr[ 1 ], m_cfg->m_draw_weapon_glow_clr[ 2 ] };
			game::g_glow->m_object_definitions.at( idx ).m_alpha = m_cfg->m_draw_weapon_glow_clr[ 3 ];
			game::g_glow->m_object_definitions.at( idx ).m_render_occluded = true;
			game::g_glow->m_object_definitions.at( idx ).m_render_unoccluded = false;
			game::g_glow->m_object_definitions.at( idx ).m_bloom_amount = 0.8f;
		};

		const auto grenade_glow = [ & ]( const int idx )
		{
			const auto classid = game::g_glow->m_object_definitions.at( idx ).m_entity->networkable( )->client_class( )->m_class_id;
			if( !is_grenade( classid )
				|| ( ~m_cfg->m_grenade_selection & 4 ) )
				return;

			game::g_glow->m_object_definitions.at( idx ).m_color = { m_cfg->m_draw_grenade_glow_clr[ 0 ], m_cfg->m_draw_grenade_glow_clr[ 1 ], m_cfg->m_draw_grenade_glow_clr[ 2 ] };
			game::g_glow->m_object_definitions.at( idx ).m_alpha = m_cfg->m_draw_grenade_glow_clr[ 3 ];
			game::g_glow->m_object_definitions.at( idx ).m_render_occluded = true;
			game::g_glow->m_object_definitions.at( idx ).m_render_unoccluded = false;
			game::g_glow->m_object_definitions.at( idx ).m_bloom_amount = 0.8f;
		};

		const auto c4_glow = [ & ]( const int idx )
		{
			const auto classid = game::g_glow->m_object_definitions.at( idx ).m_entity->networkable( )->client_class( )->m_class_id;
			if( !( classid == game::e_class_id::c_c4 
				|| classid == game::e_class_id::c_planted_c4 )
				|| ( ~m_cfg->m_draw_bomb_options & 4 ) )
				return;

			game::g_glow->m_object_definitions.at( idx ).m_color = { m_cfg->m_draw_c4_glow_clr[ 0 ], m_cfg->m_draw_c4_glow_clr[ 1 ], m_cfg->m_draw_c4_glow_clr[ 2 ] };
			game::g_glow->m_object_definitions.at( idx ).m_alpha = m_cfg->m_draw_c4_glow_clr[ 3 ];
			game::g_glow->m_object_definitions.at( idx ).m_render_occluded = true;
			game::g_glow->m_object_definitions.at( idx ).m_render_unoccluded = false;
			game::g_glow->m_object_definitions.at( idx ).m_bloom_amount = 0.8f;
		};

		for( int i{ }; i < game::g_glow->m_object_definitions.size( ); ++i ) { 
			auto obj = &game::g_glow->m_object_definitions.at( i );

			if( !obj->m_entity )
				continue;

			player_glow( i );

			weapon_glow( i );

			grenade_glow( i );

			c4_glow( i );
		}
	}
}