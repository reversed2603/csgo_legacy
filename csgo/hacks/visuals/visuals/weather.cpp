#include "../../../csgo.hpp"

namespace csgo::hacks {
	constexpr float max_pos = 32768.0f;
	void c_weather::reset( ) {
		if( !m_has_created_rain )
			return;

		for( int i{ }; i < game::g_entity_list->highest_ent_index( ); ++i ) { 
			game::base_entity_t* entity = game::g_entity_list->get_entity( i );
			if( !entity 
				|| !entity->networkable( )
				|| entity->networkable(	)->dormant( ) )
				continue;

			const game::client_class_t* client_class = entity->networkable( )->client_class( );
			if( !client_class )
				continue;

			if( client_class->m_class_id ==	118 ) {
				entity->networkable( )->release( );
			}
		}
	}

	/* TO-DO: ADD THIS FOR BETTER EFFECT !.! https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/client/c_effects.cpp#L223 
	   OOOOOOOOOOOOOOOOOOR ADD SOMETHING THAT MAKES UR FLOOR WET LOL!.
	   LLAMA PASTE DID THAT SO WHY NOT DO THAT HERE TOO HAHA
	*/

	void c_weather::update( ) {
		if( !g_visuals->cfg( ).m_rain
			|| m_has_created_rain )
			return;

		static game::client_class_t* precipation{ nullptr }	;
		if( !precipation ) {
			for( auto client_class = game::g_client->all_classes( ); client_class && !precipation; client_class = client_class->m_next ) {
				if( client_class->m_class_id == 118 ) {
					precipation = client_class;
				}
			}
		}

		if( precipation 
			&& precipation->m_create_fn ) {
			game::networkable_t* networkable = ( ( game::networkable_t * ( * )( int, int ) )( precipation->m_create_fn ) )( ( 1 << 11 ) - 1, 0 );
			if( !networkable )
				return;

			game::i_client_unknown* rain_unk = reinterpret_cast< game::i_client_renderable* >( networkable )->get_client_unknown( );
			if( !rain_unk )
				return;

			game::base_entity_t* rain_entity = rain_unk->get_base_entity( );
			if( !rain_entity 
				|| !rain_entity->networkable( ) )
				return;

			networkable->pre_data_upd( 0 );
			networkable->pre_data( 0 );

			rain_entity->precipitation( ) = 1;
			rain_entity->obb_min( ) = sdk::vec3_t( -max_pos, -max_pos, -max_pos );
			rain_entity->obb_max( ) = sdk::vec3_t( max_pos, max_pos, max_pos );

			rain_entity->networkable( )->data( 0 );
			rain_entity->networkable( )->post_data( 0 );

			m_has_created_rain = true;
		}
	}
}