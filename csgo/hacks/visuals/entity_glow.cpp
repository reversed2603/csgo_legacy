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

		for( int i{ }; i < game::g_glow->m_object_definitions.size( ); ++i ) { 
			auto obj = &game::g_glow->m_object_definitions.at( i );

			if( !obj->m_entity )
				continue;

			static float alpha[ 64 ]{ };
			auto client_class = obj->m_entity->networkable( )->client_class( );

			auto class_id = client_class->m_class_id;

			if( obj->m_entity->is_player( ) ) {
				auto player = static_cast < game::cs_player_t* > ( obj->m_entity );
				if( !player->networkable( ) )
					break;

				auto idx = player->networkable( )->index( );

				if( m_cfg->m_glow )
					alpha[ idx ] = std::lerp( alpha[ idx ], 1.f, 4.5f * game::g_global_vars.get( )->m_frame_time );
				else {
					alpha[ idx ] = 0.f;
					break;
				}

				if( player->friendly( g_local_player->self( ), false ) )
					continue;

				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_glow_clr[ 0 ], m_cfg->m_glow_clr[ 1 ], m_cfg->m_glow_clr[ 2 ] };
				obj->m_alpha = ( m_cfg->m_glow_clr[ 3 ] * ( m_dormant_data[ player->networkable( )->index( ) ].m_alpha / 255 ) * alpha[ idx ] );
			}

			if( is_grenade( class_id ) )
			{
				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_draw_grenade_glow_clr[ 0 ], m_cfg->m_draw_grenade_glow_clr[ 1 ], m_cfg->m_draw_grenade_glow_clr[ 2 ] };
				obj->m_alpha = m_cfg->m_draw_grenade_glow ? m_cfg->m_draw_grenade_glow_clr[ 3 ] : 0.f;
			}

			if( obj->m_entity->is_base_combat_wpn( )
				&& !is_grenade( class_id ) )
			{
				obj->m_render_occluded = true;
				obj->m_render_unoccluded = false;
				obj->m_render_full_bloom = false;
				obj->m_color = { m_cfg->m_draw_weapon_glow_clr[ 0 ], m_cfg->m_draw_weapon_glow_clr[ 1 ], m_cfg->m_draw_weapon_glow_clr[ 2 ] };
				obj->m_alpha = m_cfg->m_draw_weapon_glow ? m_cfg->m_draw_weapon_glow_clr[ 3 ] : 0.f;
			}
		}
	}
}