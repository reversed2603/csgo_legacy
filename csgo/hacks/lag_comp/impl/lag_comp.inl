#pragma once
#include "../lag_comp.h"

namespace csgo::hacks { 
	__forceinline void lag_record_t::store( game::cs_player_t* player ) { 
		m_player = player;
		m_sim_time = m_received_sim_time = player->sim_time( );
		m_old_sim_time = player->old_sim_time( );
		m_flags = player->flags( );
		m_receive_tick = game::g_client_state.get( )->m_server_tick;
		m_origin = player->origin( );
		m_old_origin = player->old_origin( );
		m_abs_origin = player->abs_origin( );
		m_eye_angles = player->eye_angles( );
		m_abs_angles = player->abs_ang( );
		m_lby = player->lby( );
		m_mins = player->obb_min( );
		m_maxs = player->obb_max( );
		m_duck_amt = player->duck_amt( );
		m_anim_velocity = player->velocity( );
		m_third_person_recoil = player->third_person_recoil( );

		m_dormant = player->networkable( )->dormant( );

		m_anim_layers = player->anim_layers( );
		m_pose_params = player->pose_params( );
		m_wpn = player->weapon( );

		m_last_shot_time = m_wpn ? m_wpn->last_shot_time( ) : 0.f;

		m_anim_time = m_old_sim_time + game::g_global_vars.get( )->m_interval_per_tick;

		m_choked_cmds = m_lag_ticks = game::to_ticks( m_sim_time - m_old_sim_time );
	}

	__forceinline void player_entry_t::reset( ) { 
		m_player = nullptr;
		m_alive_loop_cycle = -1.f;
		m_render_origin = { };
		m_air_misses = 0;

		m_stand_not_moved_misses = m_stand_moved_misses = m_last_move_misses =
			m_forwards_misses = m_backwards_misses = m_freestand_misses,
			m_lby_misses = m_just_stopped_misses = m_low_lby_misses =
			m_moving_misses = m_low_lby_misses = 0;

		m_moving_data.reset( );
		m_lag_records.clear( );
		m_body_data.reset( true );
	}

	__forceinline player_entry_t& c_lag_comp::entry( const std::size_t i ) { return m_entries.at( i ); }

	__forceinline bool lag_record_t::valid( ) { 
		const auto& net_info = g_ctx->net_info( );

		// get correct based on out latency + in latency + lerp time and clamp on sv_maxunlag
		const auto correct = std::clamp(
			net_info.m_lerp + net_info.m_latency.m_in + net_info.m_latency.m_out,
			0.f, g_ctx->cvars( ).m_sv_maxunlag->get_float( )
		);

		auto tick_base = g_local_player->self( )->tick_base( );
		if( g_exploits->m_next_shift_amount > 0 )
			tick_base -= g_exploits->m_next_shift_amount;

		return std::fabs( correct - ( game::to_time( tick_base ) - m_sim_time ) ) < crypt_float( 0.2f );
	}

	__forceinline void lag_record_t::simulate( cc_def( previous_lag_data_t* ) previous, player_entry_t& entry ) { 

		if( previous.get( ) ) { 
			if( ( m_origin - m_old_origin ).length_sqr( 2u ) > crypt_float( 4096.f ) )
				m_broke_lc = true;
		}

		const auto& cur_alive_loop_layer = m_anim_layers.at( 11u );

		/* es0 should touch some grass */
		if( !previous.get( )
			|| previous.get( )->m_dormant ) { 
			if( !previous.get( ) ) { 
				if( ( m_flags & game::e_ent_flags::on_ground ) ) { 
					auto max_speed = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;

					if( m_anim_layers.at( 6u ).m_weight > 0.f && m_anim_layers.at( 6u ).m_playback_rate > 0.f
						&& m_anim_velocity.length( 2u ) > 0.f ) { 
						if( ( m_flags & game::e_ent_flags::ducking ) )
							max_speed *= 0.34f;
						else if( entry.m_player->walking( ) )
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
 
		if( previous.get( ) ) { 

			auto sim_ticks = game::to_ticks( m_sim_time - previous.get( )->m_sim_time );

			m_choked_cmds = std::clamp( sim_ticks, 0, 17 );

			if( m_choked_cmds >= 2 ) { 
				sdk::vec3_t origin_diff = m_origin - previous.get( )->m_origin;

				if( !( previous.get( )->m_flags & game::e_ent_flags::on_ground ) || ( m_flags & game::e_ent_flags::on_ground ) ) { 
					
					const bool was_ducking = ( previous.get( )->m_flags & game::e_ent_flags::ducking );
					const bool is_ducking = ( m_flags & game::e_ent_flags::ducking );

					if( was_ducking != is_ducking ) { 
						float duck_mod{ -9.f };

						if( is_ducking )
							duck_mod = 9.f;

						origin_diff.z( ) -= duck_mod;
					}
				}

				const auto total_cmds_time = game::to_time( m_choked_cmds );
				if( total_cmds_time > ( 0.f )
					&& total_cmds_time < ( 1.f ) )
					m_anim_velocity = origin_diff * ( 1.f / total_cmds_time );

				sdk::validate( &m_anim_velocity );
			}
		}

		if( m_wpn
			&& m_last_shot_time > ( m_sim_time - game::to_time( m_choked_cmds ) )
			&& m_sim_time >= m_last_shot_time )
			m_shot = true;
	
		/* s/o onetap, not to mention that it looks like they pasted it from skeet lol */
		if( m_choked_cmds >= 2 ) { 		
			if( ( m_flags & game::e_ent_flags::on_ground )
				&& ( !previous.get( ) || ( previous.get( )->m_flags & game::e_ent_flags::on_ground ) ) ) { 
				if( m_anim_layers.at( 6u ).m_playback_rate == ( 0.f ) )
					m_anim_velocity = { };
				else { 
					if( m_wpn == previous.get( )->m_wpn ) { 				
	    				if( cur_alive_loop_layer.m_weight > 0.f && cur_alive_loop_layer.m_weight < 1.f ) { 
							const auto speed_2d = m_anim_velocity.length( 2u );
							const auto max_speed = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;
							if( speed_2d ) { 
								const auto reversed_val = ( ( 1.f ) - cur_alive_loop_layer.m_weight ) * ( 0.35f );

								if( reversed_val > ( 0.f ) && reversed_val < ( 1.f ) ) { 
									const auto speed_as_portion_of_run_top_speed = ( ( reversed_val + ( 0.55f ) ) * max_speed ) / speed_2d;
									if( speed_as_portion_of_run_top_speed ) { 
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
			if( ( m_flags & game::e_ent_flags::on_ground )
				&& ( previous.get( )->m_flags & game::e_ent_flags::on_ground ) ) { 
				if( m_anim_layers.at( 6u ).m_playback_rate == 0.f )
					m_anim_velocity = { };
				else { 
					if( m_anim_layers.at( 6u ).m_weight >= 0.1f ) { 
						if( ( cur_alive_loop_layer.m_weight <= ( 0.f ) || cur_alive_loop_layer.m_weight >= ( 1.f ) )
							&& m_anim_velocity.length( 2u ) > 0.f ) { 
							const bool valid_6th_layer = ( m_anim_layers.at( 6u ).m_weight < ( 1.f ) )
								&& ( m_anim_layers.at( 6u ).m_weight >= previous.get( )->m_anim_layers.at( 6u ).m_weight );
							const auto max_spd = m_wpn ? std::max( 0.1f, m_wpn->max_speed( entry.m_player->scoped( ) ) ) : 260.f;

							if( valid_6th_layer ) { 
								const auto max_duck_speed = max_spd * ( 0.34f );
								const auto speed_multiplier = std::max( 0.f, ( max_spd * ( 0.52f ) ) - ( max_spd * ( 0.34f ) ) );
								const auto duck_modifier = ( 1.f ) - m_duck_amt;

								m_velocity_step += 5;

								const auto speed_via_6th_layer = ( ( ( duck_modifier * speed_multiplier ) + max_duck_speed ) * m_anim_layers.at( 6u ).m_weight ) / m_anim_velocity.length( 2u );

								m_anim_velocity.x( ) *= speed_via_6th_layer;
								m_anim_velocity.y( ) *= speed_via_6th_layer;
							}
						}
					}
					else { 
						if( m_anim_layers.at( 6u ).m_weight ) { 
							auto weight = m_anim_layers.at( 6 ).m_weight;
							const auto length_2d = m_anim_velocity.length( 2u );

							if( m_flags & game::e_ent_flags::ducking )
								weight *= ( 0.34f );
							else { 
								if( entry.m_player->walking( ) ) { 
									weight *= ( 0.52f );
								}
							}
							if( length_2d ) { 
								m_velocity_step += 7;
								m_anim_velocity.x( ) = ( m_anim_velocity.x( ) / length_2d ) * weight;
								m_anim_velocity.y( ) = ( m_anim_velocity.y( ) / length_2d ) * weight;
							}
						}
					}
				}
			}

			if( previous.get( ) ) { 
				if( previous.get( )->m_anim_velocity.length( 2u )
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