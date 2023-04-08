#include "../.././../csgo.hpp"

namespace csgo::hacks {
	void c_lag_comp::handle_net_update( ) {
		if( !g_local_player->self( ) || !valve::g_engine->in_game( ) )
			return;

		for( std::ptrdiff_t i { 1 }; i <= valve::g_global_vars.get( )->m_max_clients; ++i ) {
			auto& entry = m_entries.at( i - 1 );

			const auto player = static_cast< valve::cs_player_t* >( 
				valve::g_entity_list->get_entity( i )
				 );

			if( !player || !player->alive( ) ) {
				entry.reset( );
				continue;
			}

			if( player == g_local_player->self( ) ) {
				entry.reset( );
				continue;
			}

			if( player->team( ) == g_local_player->self( )->team( ) ) {
				player->client_side_anim_proxy( ) = true;
				entry.reset( );
				continue;
			}

			if( entry.m_player != player )
				entry.reset( );

			entry.m_player = player;

			const auto anim_state = player->anim_state( );
			if( !anim_state ) {
				entry.reset( );
				continue;
			}

			if( player->networkable( )->dormant( ) ) {
				entry.m_previous_record = std::nullopt;

				if( entry.m_lag_records.empty( ) || !entry.m_lag_records.front( )->m_dormant ) {
	
					entry.m_lag_records.emplace_front( 
						std::make_shared< lag_record_t >( player )
					 );
				}


				// if dormant only keep 1/2 records
				while( entry.m_lag_records.size( ) > 2 )
					entry.m_lag_records.pop_back( );

				// reset simulation data ( will force update as soon as they go out of dormancy, can be helpful )
				entry.m_alive_loop_cycle = -1.f;
				entry.m_alive_loop_rate = -1.f;
				continue;
			}



			if( player->sim_time( ) == crypt_float( 0.f ) )
				continue;

			// if both are set to -1 it means they were dormant
			// meaning we should force update them as soon as they go outside of dormancy
			if( entry.m_alive_loop_cycle != -1.f && entry.m_alive_loop_rate != -1.f ) {

				// player has not updated yet
				if( player->old_sim_time( ) == player->sim_time( ) ) 
					continue;

				// player has updated, check if its fake update
				// note: moved it down here cus skeet/onetap etc.. check for oldsim == sim before this 
				if( player->anim_layers( ).at( 11u ).m_cycle == entry.m_alive_loop_cycle
					&& player->anim_layers( ).at( 11u ).m_playback_rate == entry.m_alive_loop_rate ) {
				
					// fix simulation data
					// changed it to this and commented out old one
					// note: skeet/nemesis uses this
					player->sim_time( ) =  player->old_sim_time( );

					// player->sim_time( ) = entry.m_cur_sim;
					// player->old_sim_time( ) = entry.m_old_sim;
					continue;
				}
			}

			entry.m_old_sim = entry.m_cur_sim;
			entry.m_cur_sim = player->sim_time( );

			entry.m_alive_loop_cycle = player->anim_layers( ).at( 11 ).m_cycle;
			entry.m_alive_loop_rate = player->anim_layers( ).at( 11 ).m_playback_rate;

			entry.m_receive_time = valve::g_global_vars.get( )->m_real_time;

			if( entry.m_spawn_time != player->spawn_time( ) ) {
				anim_state->reset( );
				entry.m_cur_sim = 0.f;
				entry.m_old_sim = 0.f;
				entry.m_previous_record = std::nullopt;
				entry.m_air_misses = 0;
				entry.m_lby_misses = 0;
				entry.m_moving_misses = 0;
				entry.m_stand_moved_misses = 0;
				entry.m_freestand_misses = 0;
				entry.m_stand_not_moved_misses = 0;
				entry.m_alive_loop_cycle = -1.f;
				entry.m_alive_loop_rate = -1.f;
				entry.m_lag_records.clear( );
				entry.m_spawn_time = player->spawn_time( );
			}

			previous_lag_data_t* previous{ }, *pre_previous{ };
			if( entry.m_previous_record.has_value( ) )
				previous = &entry.m_previous_record.value( );

			if( entry.m_pre_previous_record.has_value( ) )
				pre_previous = &entry.m_pre_previous_record.value( );

			entry.m_lag_records.emplace_front( std::make_shared < lag_record_t >( player ) );

			const auto current = entry.m_lag_records.front( ).get( );
			current->m_dormant = player->networkable( )->dormant( );

			entry.m_render_origin = current->m_origin;


			g_anim_sync->handle_player_update( current, previous, pre_previous, entry );

			if( entry.m_previous_record.has_value( ) )
				entry.m_pre_previous_record.emplace( entry.m_previous_record.value( ) );

			entry.m_previous_record.emplace( current );

			while( entry.m_lag_records.size( ) > 64 )
				entry.m_lag_records.pop_back( );

			// note: changed that to 2, as we want to keep 2 records for anim corrections
			while( entry.m_lag_records.size( ) > 2 
				&& ( current->m_broke_lc || player->sim_time( ) < player->old_sim_time( ) ) )
				entry.m_lag_records.pop_back( );
		}
	}
}