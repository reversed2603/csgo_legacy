#pragma once
#include "../lag_comp.h"

namespace csgo::hacks {
	__forceinline void lag_record_t::store ( valve::cs_player_t* player ) {
		m_player = player;
		m_sim_time = m_received_sim_time = player->sim_time ( );
		m_old_sim_time = player->old_sim_time ( );
		m_flags = player->flags ( );
		m_receive_tick = valve::g_client_state.get ( )->m_server_tick;
		m_origin = player->origin ( );
		m_abs_origin = player->abs_origin ( );
		m_eye_angles = player->eye_angles ( );
		m_abs_angles = player->abs_ang ( );
		m_lby = player->lby ( );
		m_mins = player->obb_min ( );
		m_maxs = player->obb_max ( );
		m_duck_amt = player->duck_amt ( );
		m_anim_velocity = player->velocity ( );
		m_third_person_recoil = player->third_person_recoil( );

		m_dormant = player->networkable( )->dormant( );

		m_anim_layers = player->anim_layers ( );
		m_pose_params = player->pose_params ( );
		m_wpn = player->weapon ( );

		m_last_shot_time = m_wpn ? m_wpn->last_shot_time( ) : 0.f;

		m_choked_cmds = m_lag_ticks = valve::to_ticks ( m_sim_time - m_old_sim_time );
	}

	__forceinline void player_entry_t::reset ( ) {
		m_player = nullptr;
		m_alive_loop_cycle = -1.f;
		m_alive_loop_rate = -1.f;
		m_render_origin = {};
		m_cur_sim = 0.f;
		m_old_sim = 0.f;
		m_air_misses = 0;
		m_records_count = 0;
		m_walk_record = {};
		m_stand_moved_misses = 0;
		m_backwards_misses = 0;
		m_freestand_misses = 0;
		m_stand_not_moved_misses = 0;
		m_moving_misses = 0;
		m_lby_misses = 0;
		m_moved = false;

		m_lag_records.clear ( );
	}

	__forceinline player_entry_t& c_lag_comp::entry ( const std::size_t i ) { return m_entries.at ( i ); }

	__forceinline bool lag_record_t::valid ( ) {
		const auto& net_info = g_ctx->net_info( );

		// get correct based on out latency + in latency + lerp time and clamp on sv_maxunlag
		const auto correct = std::clamp(
			net_info.m_lerp + net_info.m_latency.m_in + net_info.m_latency.m_out,
			0.f, g_ctx->cvars( ).m_sv_maxunlag->get_float( )
		);

		auto tick_base = g_local_player->self ( )->tick_base( );
		if ( g_exploits->m_next_shift_amount > 0 )
			tick_base -= g_exploits->m_next_shift_amount;

		return !m_dormant && fabs( correct - ( valve::to_time( tick_base ) - valve::to_time( m_sim_time ) ) ) < 0.19f;
	}

	__forceinline void lag_record_t::simulate( cc_def ( previous_lag_data_t* ) previous, player_entry_t& entry ) {
		if ( previous.get ( ) ) {
			if ( ( m_origin - previous.get( )->m_origin ).length_sqr( 2u ) > crypt_float ( 4096.f ) )
				m_broke_lc = true;
		}

		const auto& cur_alive_loop_layer = m_anim_layers.at( 11u );

		/* es0 should touch some grass */
		if ( !previous.get( )
			|| previous.get( )->m_dormant ) {
			if ( !previous.get( ) ) {
				if ( ( m_flags & valve::e_ent_flags::on_ground ) ) {
					auto max_speed = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;

					if ( m_anim_layers.at( 6u ).m_weight > 0.f && m_anim_layers.at( 6u ).m_playback_rate > 0.f
						&& m_anim_velocity.length( 2u ) > 0.f ) {
						if ( ( m_flags & valve::e_ent_flags::ducking ) )
							max_speed *= 0.34f;
						else if ( entry.m_player->walking( ) )
							max_speed *= 0.52f;

						const auto move_multiplier = m_anim_layers.at( 6u ).m_weight * max_speed;
						const auto speed_multiplier = move_multiplier / m_anim_velocity.length( 2u );

						m_velocity_step = 1;

						m_anim_velocity.x( ) *= speed_multiplier;
						m_anim_velocity.y( ) *= speed_multiplier;
					}
				}
			}

			return;
		}
 
		/* recalculating simulation time via 11th layer, s/o eso */
		if ( m_wpn == previous.get( )->m_wpn
			&& m_anim_layers.at( 11u ).m_playback_rate > 0.f && previous.get( )->m_anim_layers.at( 11u ).m_playback_rate > 0.f ) {

			const auto cur_11th_cycle = cur_alive_loop_layer.m_cycle;
			auto prev_11th_cycle = previous.get( )->m_anim_layers.at( 11u ).m_cycle;

			const auto cycles_delta = cur_11th_cycle - prev_11th_cycle;

			if ( cycles_delta != crypt_float( 0.f ) ) {
				const auto sim_ticks_delta = valve::to_ticks( m_sim_time - m_old_sim_time );

				if ( sim_ticks_delta != crypt_int( 1 ) ) {
					auto prev_11th_rate = previous.get( )->m_anim_layers.at( 11u ).m_playback_rate;
					std::ptrdiff_t resimulated_sim_ticks{};

					if ( cycles_delta >= crypt_float( 0.f ) ) {
						resimulated_sim_ticks = valve::to_ticks( cycles_delta / prev_11th_rate );
					}
					else {
						std::ptrdiff_t ticks_iterated{};
						float cur_simulated_cycle{ 0.f };
						while ( true ) {
							++resimulated_sim_ticks;

							if ( ( prev_11th_rate * valve::g_global_vars.get( )->m_interval_per_tick ) + prev_11th_cycle >= 1.f )
								prev_11th_rate = m_anim_layers.at( 11u ).m_playback_rate;

							cur_simulated_cycle = ( prev_11th_rate * valve::g_global_vars.get( )->m_interval_per_tick ) + prev_11th_cycle;
							prev_11th_cycle = cur_simulated_cycle;
							if ( cur_simulated_cycle > crypt_float( 0.f ) )
								break;

							if ( ++ticks_iterated >= crypt_int( 16 ) ) {
								goto leave_cycle;
							}
						}

						const auto first_val = prev_11th_cycle - cur_simulated_cycle;

						auto recalc_everything = cur_11th_cycle - first_val;
						recalc_everything /= m_anim_layers.at( 11u ).m_playback_rate;
						recalc_everything /= valve::g_global_vars.get( )->m_interval_per_tick;
						recalc_everything += 0.5f;

						resimulated_sim_ticks += recalc_everything;
					}

				leave_cycle:
					if ( resimulated_sim_ticks < sim_ticks_delta ) {
						if ( resimulated_sim_ticks 
								&& valve::g_client_state.get( )->m_server_tick - valve::to_ticks( m_sim_time ) == resimulated_sim_ticks ) {
							entry.m_player->sim_time( ) = entry.m_cur_sim = m_sim_time = ( resimulated_sim_ticks * valve::g_global_vars.get( )->m_interval_per_tick ) + m_old_sim_time;
						}
					}
				}
			}
		}

		if ( previous.get( ) ) {
			const auto& prev_alive_loop_layer = previous.get( )->m_anim_layers.at( 11u );

			auto sim_ticks = valve::to_ticks( m_sim_time - previous.get( )->m_sim_time );

			if ( ( sim_ticks - crypt_int ( 1 ) ) > crypt_int ( 31 ) || previous.get( )->m_sim_time == crypt_float ( 0.f ) ) {
				sim_ticks = crypt_int( 1 );
			}

			auto cur_cycle = m_anim_layers.at( 11 ).m_cycle;
			auto prev_rate = previous.get( )->m_anim_layers.at( 11 ).m_playback_rate;

			if ( prev_rate > crypt_float ( 0.f ) &&
				m_anim_layers.at( 11 ).m_playback_rate > crypt_float ( 0.f ) &&
				m_wpn == previous.get( )->m_wpn ) {
				auto prev_cycle = previous.get( )->m_anim_layers.at( 11 ).m_cycle;
				sim_ticks = 0;

				if ( prev_cycle > cur_cycle )
					cur_cycle += crypt_float ( 1.f );

				while ( cur_cycle > prev_cycle ) {
					const auto last_cmds = sim_ticks;

					const auto next_rate = valve::g_global_vars.get ( )->m_interval_per_tick * prev_rate;
					prev_cycle += valve::g_global_vars.get ( )->m_interval_per_tick * prev_rate;

					if ( prev_cycle >= crypt_float ( 1.f ) )
						prev_rate = m_anim_layers.at( 11 ).m_playback_rate;

					++sim_ticks;

					if ( prev_cycle > cur_cycle && ( prev_cycle - cur_cycle ) > ( next_rate * crypt_float ( 0.5f ) ) )
						sim_ticks = last_cmds;
				}
			}

			m_choked_cmds = std::clamp( sim_ticks, 0, 17 );

			if ( m_choked_cmds >= 2 ) {
				auto origin_diff = m_origin - previous.get( )->m_origin;

				if ( !( previous.get( )->m_flags & valve::e_ent_flags::on_ground ) || ( m_flags & valve::e_ent_flags::on_ground ) ) {
					const auto is_ducking = m_flags & valve::e_ent_flags::ducking;

					if ( ( previous.get( )->m_flags & valve::e_ent_flags::ducking ) != is_ducking ) {
						float duck_mod{};

						if ( is_ducking )
							duck_mod = crypt_float ( 9.f );
						else
							duck_mod = -9.f;

						origin_diff.z( ) -= duck_mod;
					}
				}

				const auto total_cmds_time = valve::to_time( m_choked_cmds );
				if ( total_cmds_time > crypt_float( 0.f )
					&& total_cmds_time < crypt_float( 1.f ) )
					m_anim_velocity = origin_diff * ( 1.f / total_cmds_time );

				sdk::validate( &m_anim_velocity );
			}
		}

		if ( m_wpn
			&& m_last_shot_time > ( m_sim_time - valve::to_time( m_choked_cmds ) )
			&& m_sim_time >= m_last_shot_time )
			m_shot = true;
	
		/* s/o onetap, not to mention that it looks like they pasted it from skeet lol */
		if ( m_choked_cmds >= 2 ) {		
			if ( ( m_flags & valve::e_ent_flags::on_ground )
				&& ( !previous.get( ) || ( previous.get( )->m_flags & valve::e_ent_flags::on_ground ) ) ) {
				if ( m_anim_layers.at( 6u ).m_playback_rate == crypt_float ( 0.f ) )
					m_anim_velocity = {};
				else {
					if ( m_wpn == previous.get( )->m_wpn ) {				
	    				if ( cur_alive_loop_layer.m_weight > 0.f && cur_alive_loop_layer.m_weight < 1.f ) {
							const auto speed_2d = m_anim_velocity.length( 2u );
							const auto max_speed = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;
							if ( speed_2d ) {
								const auto reversed_val = ( crypt_float ( 1.f ) - cur_alive_loop_layer.m_weight ) * crypt_float ( 0.35f );

								if ( reversed_val > crypt_float ( 0.f ) && reversed_val < crypt_float ( 1.f ) ) {
									const auto speed_as_portion_of_run_top_speed = ( ( reversed_val + crypt_float ( 0.55f ) ) * max_speed ) / speed_2d;
									if ( speed_as_portion_of_run_top_speed ) {
										m_velocity_step += 3;

										m_anim_velocity.x( ) *= speed_as_portion_of_run_top_speed;
										m_anim_velocity.y( ) *= speed_as_portion_of_run_top_speed;
									}
								}
							}
						}
					}
				}
			}

			/* pasta from onepasta but its pasted from skeet :| */
			if ( ( m_flags & valve::e_ent_flags::on_ground )
				&& ( previous.get( )->m_flags & valve::e_ent_flags::on_ground ) ) {
				if ( m_anim_layers.at( 6u ).m_playback_rate == 0.f )
					m_anim_velocity = {};
				else {
					if ( m_anim_layers.at( 6u ).m_weight >= 0.1f ) {
						if ( ( cur_alive_loop_layer.m_weight <= crypt_float( 0.f ) || cur_alive_loop_layer.m_weight >= crypt_float( 1.f ) )
							&& m_anim_velocity.length( 2u ) > 0.f) {
							const bool valid_6th_layer = ( m_anim_layers.at( 6u ).m_weight < crypt_float( 1.f ) )
								&& ( m_anim_layers.at( 6u ).m_weight >= previous.get( )->m_anim_layers.at( 6u ).m_weight );
							const auto max_spd = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;

							if ( valid_6th_layer ) {
								const auto max_duck_speed = max_spd * crypt_float( 0.34f );
								const auto speed_multiplier = std::max( 0.f, ( max_spd * crypt_float( 0.52f ) ) - ( max_spd * crypt_float( 0.34f ) ) );
								const auto duck_modifier = crypt_float( 1.f ) - m_duck_amt;

								m_velocity_step += 5;

								const auto speed_via_6th_layer = ( ( ( duck_modifier * speed_multiplier ) + max_duck_speed ) * m_anim_layers.at( 6u ).m_weight ) / m_anim_velocity.length( 2u );

								m_anim_velocity.x( ) *= speed_via_6th_layer;
								m_anim_velocity.y( ) *= speed_via_6th_layer;
							}
						}
					}
					else {
						if ( m_anim_layers.at( 6u ).m_weight ) {
							auto weight = m_anim_layers.at( 6 ).m_weight;
							const auto length_2d = m_anim_velocity.length( 2u );

							if ( m_flags & valve::e_ent_flags::ducking )
								weight *= crypt_float( 0.34f );
							else {
								if ( entry.m_player->walking( ) ) {
									weight *= crypt_float( 0.52f );
								}
							}
							if ( length_2d ) {
								m_velocity_step += 7;
								m_anim_velocity.x( ) = ( m_anim_velocity.x( ) / length_2d ) * weight;
								m_anim_velocity.y( ) = ( m_anim_velocity.y( ) / length_2d ) * weight;
							}
						}
					}
				}
			}

			if ( previous.get( ) ) {
				if ( previous.get( )->m_anim_velocity.length( 2u )
					&& !m_anim_velocity.length( 2u )
					&& std::abs( previous.get( )->m_lby - m_lby ) > 100.f )
					entry.m_has_fake_flick = true;
				else {
					entry.m_has_fake_flick = false;
				}
			}
		}
	}
}