#include "../.././../csgo.hpp"

namespace csgo::hacks {
	void c_lag_comp::handle_net_update ( ) {
		if ( !g_local_player->self ( )
			|| !valve::g_engine->in_game ( ) )
			return;

		const auto tick_rate = valve::to_ticks ( 1.f );

		for ( std::ptrdiff_t i { 1 }; i <= valve::g_global_vars.get ( )->m_max_clients; ++i ) {
			auto& entry = m_entries.at ( i - 1 );

			const auto player = static_cast< valve::cs_player_t* >(
				valve::g_entity_list->get_entity ( i )
				);

			if ( !player )
			{
				entry.reset ( );

				continue;
			}

			if ( player == g_local_player->self ( ) ) {
				entry.reset ( );

				continue;
			}

			if ( player->team ( ) == g_local_player->self ( )->team ( ) ) {
				entry.reset ( );

				continue;
			}

			if ( entry.m_player != player )
				entry.reset ( );

			entry.m_player = player;

			if ( !player
				|| !player->alive ( ) ) {
				entry.reset ( );

				if ( player )
					entry.m_player = player;

				continue;
			}

			const auto anim_state = player->anim_state ( );
			if ( !anim_state ) {
				entry.reset ( );

				continue;
			}

			if ( player->networkable ( )->dormant( ) ) {
				entry.m_previous_record = std::nullopt;
				if ( entry.m_lag_records.empty( ) ) {
					entry.m_lag_records.emplace_front(
						std::make_shared< lag_record_t >( player )
					);

					continue;
				}

				if ( !entry.m_lag_records.back( )->m_dormant ) {
					entry.m_lag_records.clear( );

					entry.m_lag_records.emplace_front(
						std::make_shared< lag_record_t >( player )
					);

					continue;
				}

				continue;
			}

			bool invalid_processing{ false };

			if ( player->old_sim_time( ) >= player->sim_time( )
				|| player->sim_time( ) == crypt_float( 0.f ) ) {
				invalid_processing = true;
				continue;
			}

			if ( player->anim_layers( ).at( 11u ).m_cycle == entry.m_alive_loop_cycle
				&& player->anim_layers( ).at( 11u ).m_playback_rate == entry.m_alive_loop_rate ) {
				player->sim_time( ) = entry.m_cur_sim;
				player->old_sim_time( ) = entry.m_old_sim;
				continue;
			}

			++entry.m_records_count;

			entry.m_old_sim = entry.m_cur_sim;
			entry.m_cur_sim = player->sim_time( );

			if ( !invalid_processing )
				entry.m_last_sim = player->sim_time( );

			entry.m_alive_loop_cycle = player->anim_layers ( ).at ( 11 ).m_cycle;
			entry.m_alive_loop_rate = player->anim_layers ( ).at ( 11 ).m_playback_rate;

			while ( entry.m_lag_records.size( ) > tick_rate )
				entry.m_lag_records.pop_back ( );

			entry.m_receive_time = valve::g_global_vars.get ( )->m_real_time;

			if ( entry.m_spawn_time != player->spawn_time ( ) ) {
				anim_state->reset ( );
				entry.m_cur_sim = 0.f;
				entry.m_old_sim = 0.f;
				entry.m_last_sim = 0.f;
				entry.m_previous_record = std::nullopt;
				entry.m_air_misses = 0;
				entry.m_lby_misses = 0;
				entry.m_moving_misses = 0;
				entry.m_stand_moved_misses = 0;
				entry.m_freestand_misses = 0;
				entry.m_stand_not_moved_misses = 0;
				entry.m_lag_records.clear ( );
			}

			entry.m_spawn_time = player->spawn_time ( );

			previous_lag_data_t* previous{}, *pre_previous{};
			if ( entry.m_previous_record.has_value( ) )
				previous = &entry.m_previous_record.value( );

			if ( entry.m_pre_previous_record.has_value( ) )
				pre_previous = &entry.m_pre_previous_record.value( );

			entry.m_lag_records.emplace_front ( std::make_shared < lag_record_t > ( player ) );

			const auto current = entry.m_lag_records.front ( ).get ( );
			current->m_dormant = false;
			current->m_invalid = invalid_processing;

			entry.m_render_origin = current->m_origin;

			g_anim_sync->handle_player_update ( current, previous, pre_previous, entry );

			if ( entry.m_previous_record.has_value( ) )
				entry.m_pre_previous_record.emplace( entry.m_previous_record.value( ) );

			entry.m_previous_record.emplace( current );
		}
	}
}