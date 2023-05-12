#include "../.././../csgo.hpp"

namespace csgo::hacks { 
	std::string resolver_mode( int solve_method )
	{
		std::string return_result{ "unknown" };

		switch( solve_method ) { 
		case e_solve_methods::low_lby:
			return_result = xor_str( "low lby" );
			break;
		case e_solve_methods::lby_delta:
			return_result = xor_str( "lby delta" );
			break;
		case e_solve_methods::fake_walk:
			return_result = xor_str( "fake walk" );
			break;
		case e_solve_methods::last_move_lby:
			return_result = xor_str( "last move" );
			break;
		case e_solve_methods::body_flick:
			return_result = xor_str( "flick" );
			break;
		case e_solve_methods::backwards:
			return_result = xor_str( "backwards" );
			break;
		case e_solve_methods::forwards:
			return_result = xor_str( "forwards" );
			break;
		case e_solve_methods::freestand:
			return_result = xor_str( "anti-freestanding" );
			break;
		case e_solve_methods::brute:
			return_result = xor_str( "brute" );
			break;
		case e_solve_methods::brute_not_moved:
			return_result = xor_str( "brute(n)" );
			break;
		case e_solve_methods::just_stopped:
			return_result = xor_str( "anim lby" );
			break;
		case e_solve_methods::body_flick_res:
			return_result = xor_str( "body flick" );
		break;
		case e_solve_methods::air:
			return_result = xor_str( "in air" );
			break;
		case e_solve_methods::move:
			return_result = xor_str( "move" );
			break;
		default:
			return_result = xor_str( "unk" );
			break;
		}
		return return_result;
	}

	void c_lag_comp::handle_net_update( ) { 
		if( !g_local_player->self( ) 
			|| !game::g_engine->in_game( ) )
			return;

		for( std::ptrdiff_t i { 1 }; i <= game::g_global_vars.get( )->m_max_clients; ++i ) { 
			auto& entry = m_entries.at( i - 1 );

			game::cs_player_t* player = static_cast< game::cs_player_t* > (
				game::g_entity_list->get_entity( i )
				 );

			if( !player 
				|| player == g_local_player->self( ) ) { 
				entry.reset( );
				continue;
			}

			if( !player->alive( ) ) { 
				if( ( !entry.m_lag_records.empty( ) 
					&& entry.m_lag_records.front( )
					&& entry.m_lag_records.front( )->m_has_valid_bones ) ) {
					g_visuals->add_shot_mdl( player, entry.m_lag_records.front( )->m_bones.data( ), true );
				}

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
				if( entry.m_lag_records.empty( ) 
					|| !entry.m_lag_records.front( )->m_dormant ) { 

					entry.m_previous_record = std::nullopt;

					entry.m_lag_records.emplace_front( 
						std::make_shared< lag_record_t > ( player )
					 );

					if( !entry.m_lag_records.empty( ) )
						entry.m_lag_records.front( )->m_dormant = true;

					// reset body & moving data
					entry.m_moving_data.reset( );
					entry.m_body_data.reset( true );

					entry.m_stand_not_moved_misses = entry.m_stand_moved_misses = entry.m_last_move_misses =
						entry.m_forwards_misses = entry.m_backwards_misses = entry.m_freestand_misses,
						entry.m_lby_misses = entry.m_just_stopped_misses = entry.m_low_lby_misses =
						entry.m_moving_misses = entry.m_low_lby_misses = entry.m_air_misses = 0;
				}

				if( entry.m_lag_records.size( ) > 2 )
					entry.m_lag_records.pop_back( );

				// reset simulation data ( will force update as soon as they go out of dormancy, can be helpful )
				entry.m_alive_loop_cycle = -1.f;
				continue;
			}

			// player hasn't updated yet
			if( player->sim_time( ) == crypt_float( 0.f ) )
				continue;

			// if both are set to -1 it means they were dormant
			// meaning we should force update them as soon as they go outside of dormancy
			if( entry.m_alive_loop_cycle != -1.f 
				&& entry.m_alive_loop_rate != -1.f ) {
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
					player->sim_time( ) = player->old_sim_time( );
					continue;
				}
			}

			entry.m_alive_loop_cycle = player->anim_layers( ).at( 11 ).m_cycle;
			entry.m_alive_loop_rate = player->anim_layers( ).at( 11 ).m_playback_rate;

			entry.m_receive_time = game::g_global_vars.get( )->m_real_time;

			if( entry.m_spawn_time != player->spawn_time( ) ) { 
				anim_state->reset( );
				entry.m_previous_record = std::nullopt;

				entry.m_stand_not_moved_misses = entry.m_stand_moved_misses = entry.m_last_move_misses =
					entry.m_forwards_misses = entry.m_backwards_misses = entry.m_freestand_misses,
					entry.m_lby_misses = entry.m_just_stopped_misses = entry.m_low_lby_misses =
					entry.m_moving_misses = entry.m_low_lby_misses = 0;

				entry.m_moving_data.reset( );
				entry.m_body_data.reset( true );
				entry.m_alive_loop_cycle = -1.f;
				entry.m_lag_records.clear( );
				entry.m_spawn_time = player->spawn_time( );
			}

			previous_lag_data_t* previous{ };

			if( entry.m_previous_record.has_value( ) )
				previous = &entry.m_previous_record.value( );
			
			entry.m_lag_records.emplace_front( std::make_shared < lag_record_t > ( player ) );

			lag_record_t* current = entry.m_lag_records.front( ).get( );

			if( current ) { 
				current->m_dormant = false;

				entry.m_render_origin = current->m_origin;

				g_anim_sync->handle_player_update( current, previous, entry );

				current->m_shifting = ( game::to_ticks( current->m_sim_time ) - game::g_global_vars.get( )->m_tick_count ) < -2;

				entry.m_previous_record.emplace( current );

				while( entry.m_lag_records.size( ) > 2
					&& current->m_broke_lc ) // we don't want to shoot invalid ticks
					entry.m_lag_records.pop_back( );
			}

			while( entry.m_lag_records.size( ) > g_ctx->ticks_data( ).m_tick_rate )
				entry.m_lag_records.pop_back( );
		}
	}
}