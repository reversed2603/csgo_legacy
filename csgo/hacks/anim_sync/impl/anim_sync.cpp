#include "../../../csgo.hpp"

namespace csgo::hacks { 
	void c_anim_sync::handle_player_update( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry ) { 
		const sdk::vec3_t origin = entry.m_player->origin( );
		const sdk::vec3_t velocity = entry.m_player->velocity( );
		const sdk::vec3_t abs_velocity = entry.m_player->abs_velocity( );
		const int ieflags = entry.m_player->ieflags( );
		const game::e_ent_flags flags = entry.m_player->flags( );
		const float duck_amt = entry.m_player->duck_amt( );
		const float lby = entry.m_player->lby( );

		const int   frame_count = game::g_global_vars.get( )->m_frame_count;
		const int   tick_count = game::g_global_vars.get( )->m_tick_count;
		const float real_time = game::g_global_vars.get( )->m_real_time;
		const float cur_time = game::g_global_vars.get( )->m_cur_time;
		const float frame_time = game::g_global_vars.get( )->m_frame_time;
		const float abs_frame_time = game::g_global_vars.get( )->m_abs_frame_time;
		const float interp_amt = game::g_global_vars.get( )->m_interp_amt;

		// NOTE: 2018 skeet does old_sim_time + interval but new one does sim_time
		game::g_global_vars.get( )->m_real_time = current.get( )->m_anim_time;
		game::g_global_vars.get( )->m_cur_time = current.get( )->m_anim_time;
		game::g_global_vars.get( )->m_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_abs_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_frame_count = game::to_ticks( current.get( )->m_anim_time );
		game::g_global_vars.get( )->m_tick_count = game::to_ticks( current.get( )->m_anim_time );
		game::g_global_vars.get( )->m_interp_amt = crypt_float( 0.0f );

		if( current.get( )->m_lag_ticks > 1 )
			current.get( )->simulate( previous, entry );

		game::anim_state_t* anim_state = entry.m_player->anim_state( );

		entry.m_player->set_abs_origin( current.get( )->m_origin );

		entry.m_player->anim_layers( ) = current.get( )->m_anim_layers;

		if( previous.get( ) && !previous.get( )->m_dormant ) { 
			// NOTE: current skeet does this atm ( it might be wrong )
			// but to make sure i will just like copy what they do ig
			// if this breaks anything blame eso typls

			anim_state->m_move_weight = previous.get( )->m_anim_layers.at( 6u ).m_weight;
			anim_state->m_primary_cycle = previous.get( )->m_anim_layers.at( 6u ).m_cycle;
			anim_state->m_strafe_weight = previous.get( )->m_anim_layers.at( 7u ).m_weight;
			anim_state->m_strafe_sequence = previous.get( )->m_anim_layers.at( 7u ).m_seq;
			anim_state->m_strafe_cycle = previous.get( )->m_anim_layers.at( 7u ).m_cycle;
			anim_state->m_acceleration_weight = previous.get( )->m_anim_layers.at( 12u ).m_weight;

			// apparently skeet does this:
			// foot_yaw = previous_foot_yaw;
			// updatecsa 
			// previous_foot_yaw = foot_yaw;
			anim_state->m_foot_yaw = previous.get( )->m_foot_yaw;
			anim_state->m_move_yaw = previous.get( )->m_move_yaw;
			anim_state->m_move_yaw_cur_to_ideal = previous.get( )->m_move_yaw_cur_to_ideal;
			anim_state->m_move_yaw_ideal = previous.get( )->m_move_yaw_ideal;
			anim_state->m_move_weight_smoothed = previous.get( )->m_move_weight_smoothed;

			// set layers & poseparams to latest networked before this update
			entry.m_player->anim_layers( ) = previous.get( )->m_anim_layers;
			entry.m_player->pose_params( ) = previous.get( )->m_pose_params;

			catch_ground( current.get( ), previous.get( ), entry );
		}
		else { 
			if( current.get( )->m_flags & game::e_ent_flags::on_ground ) { 
				anim_state->m_on_ground = true;
				anim_state->m_landing = false;
			}

			anim_state->m_primary_cycle = current.get( )->m_anim_layers.at( 6u ).m_cycle;
			anim_state->m_move_weight = current.get( )->m_anim_layers.at( 6u ).m_weight;
			anim_state->m_strafe_weight = current.get( )->m_anim_layers.at( 7u ).m_weight;
			anim_state->m_strafe_sequence = current.get( )->m_anim_layers.at( 7u ).m_seq;	
			anim_state->m_strafe_cycle = current.get( )->m_anim_layers.at( 7u ).m_cycle;
			anim_state->m_acceleration_weight = current.get( )->m_anim_layers.at( 12u ).m_weight;
			anim_state->m_last_update_time = current.get( )->m_anim_time - game::g_global_vars.get( )->m_interval_per_tick;
		}

		entry.m_player->duck_amt( ) = current.get( )->m_duck_amt;
		entry.m_player->abs_velocity( ) = entry.m_player->velocity( ) = current.get( )->m_anim_velocity;

		if( current.get( )->m_lag_ticks > crypt_int( 2u )
			&& current.get( )->m_anim_layers.at( 6u ).m_weight == crypt_float( 0.f )
			&& current.get( )->m_anim_layers.at( 6u ).m_playback_rate == crypt_float( 0.f )
			&& entry.m_player->flags( ) & game::e_ent_flags::on_ground
			&& entry.m_player->velocity( ).length( 2u ) > 0.1f ) { 

			current.get( )->m_fake_walking = true;
			entry.m_player->abs_velocity( ) = entry.m_player->velocity( ) = current.get( )->m_anim_velocity = { };
		}

		
		if( game::to_ticks( current.get( )->m_last_shot_time ) !=
			game::to_ticks( current.get( )->m_sim_time ) ) {
			int tick_rate = game::to_ticks( 1.f );
			if( game::to_ticks( current.get( )->m_last_shot_time ) >= ( ( tick_rate * current.get( )->m_anim_time ) ) 
				&& game::to_ticks( current.get( )->m_last_shot_time ) <= ( ( tick_rate * current.get( )->m_sim_time ) ) ) {
				if( !previous.get( )
					|| current.get ( )->m_choked_cmds < 2 )
					current.get( )->m_eye_angles.x( ) = crypt_float( 89.f );
				else {
					current.get( )->m_eye_angles.x( ) = previous.get( )->m_eye_angles.x( );
				}
			}
		}

		if( previous.get( ) ) { 
			g_resolver->handle_ctx( current, previous, entry );
		}

		entry.m_player->origin( ) = current.get( )->m_origin;
		entry.m_player->lby( ) = current.get( )->m_lby;

		entry.m_player->ieflags( ) &= ~( game::cs_player_t::entityflags_t::EFL_DIRTY_ABSVELOCITY 
			| game::cs_player_t::entityflags_t::EFL_DIRTY_ABSTRANSFORM );

		entry.m_player->eye_angles( ) = current.get( )->m_eye_angles;

		entry.m_player->anim_state( )->m_last_update_frame = game::g_global_vars.get( )->m_frame_count - crypt_int( 1 );

		g_ctx->anim_data( ).m_allow_update = entry.m_player->client_side_anim_proxy( ) = true;
		entry.m_player->update_client_side_anim( );
		g_ctx->anim_data( ).m_allow_update = entry.m_player->client_side_anim_proxy( ) = false;

		current.get( )->m_pose_params = entry.m_player->pose_params( );

		reinterpret_cast < void( __thiscall* )( void*, int ) > ( g_ctx->addresses( ).m_invalidate_physics_recursive )( entry.m_player, 0x8 );

		entry.m_player->anim_layers( ) = current.get( )->m_anim_layers;

		current.get( )->m_foot_yaw = entry.m_player->anim_state( )->m_foot_yaw;
		current.get( )->m_move_yaw = entry.m_player->anim_state( )->m_move_yaw;
		current.get( )->m_move_yaw_cur_to_ideal = entry.m_player->anim_state( )->m_move_yaw_cur_to_ideal;
		current.get( )->m_move_yaw_ideal = entry.m_player->anim_state( )->m_move_yaw_ideal;
		current.get( )->m_move_weight_smoothed = entry.m_player->anim_state( )->m_move_weight_smoothed;

		setup_bones( entry.m_player, current.get( )->m_bones, current.get( )->m_anim_time );
		current.get( )->m_has_valid_bones = current.get( )->m_bones.data( ) != nullptr;

		std::memcpy( entry.m_bones.data( ), current.get( )->m_bones.data( ), sizeof( sdk::mat3x4_t ) * game::k_max_bones );

		game::g_global_vars.get( )->m_real_time = real_time;
		game::g_global_vars.get( )->m_cur_time = cur_time;
		game::g_global_vars.get( )->m_frame_time = frame_time;
		game::g_global_vars.get( )->m_abs_frame_time = abs_frame_time;
		game::g_global_vars.get( )->m_interp_amt = interp_amt;
		game::g_global_vars.get( )->m_frame_count = frame_count;
		game::g_global_vars.get( )->m_tick_count = tick_count;

		const auto mdl_data = entry.m_player->mdl_ptr( );
		if( mdl_data ) { 
	    	current.get( )->m_bones_count = mdl_data->m_studio->m_bones_count;
		}
		entry.m_player->origin( ) = origin;
		entry.m_player->velocity( ) = velocity;
		entry.m_player->abs_velocity( ) = abs_velocity;
		entry.m_player->flags( ) = flags;
		entry.m_player->ieflags( ) = ieflags;
		entry.m_player->duck_amt( ) = duck_amt;
		entry.m_player->lby( ) = lby;
	}

	void c_anim_sync::catch_ground( 
		cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry
	 ) { 

		if( current.get( )->m_lag_ticks < 2 )
			return;

		if( current.get( )->m_flags & game::e_ent_flags::on_ground ) { 
			if( current.get( )->m_anim_layers.at( 5u ).m_weight > crypt_int( 0.f )
				&& previous.get( )->m_anim_layers.at( 5u ).m_weight <= crypt_int( 0.f )
				&& !( previous.get( )->m_flags & game::e_ent_flags::on_ground ) ) { 
				const int land_seq = current.get( )->m_anim_layers.at( 5u ).m_seq;

				if( land_seq >= crypt_int( 2 ) ) { 
					const int land_act = entry.m_player->lookup_seq_act( land_seq );
					if( land_act == crypt_int( 988 ) || land_act == crypt_int( 989 ) ) { 
						const float cur_cycle = current.get( )->m_anim_layers.at( 5u ).m_cycle;
						const float cur_rate = current.get( )->m_anim_layers.at( 5u ).m_playback_rate;
						if( cur_cycle != crypt_int( 0.f ) && cur_rate != crypt_int( 0.f ) ) { 
							const float  land_time = cur_cycle / cur_rate;
							if( land_time != 0.f ) { 
								current.get( )->m_on_ground = true;
								current.get( )->m_act_time = current.get( )->m_anim_time - land_time;
							}
						}
					}
				}
			}

			current.get( )->m_anim_velocity.z( ) = 0.f;
		}
		else { 

			const int jump_seq = current.get( )->m_anim_layers.at( 4u ).m_seq;
			static int crypt_jmp_seq = crypt_int( 2 );
			static float crypt_jmp_act = crypt_float( 985 );

			if( !( previous.get( )->m_flags & game::e_ent_flags::on_ground ) ) { 
				if( jump_seq >= crypt_jmp_seq ) { 

					const int jump_act = entry.m_player->lookup_seq_act( jump_seq );

					if( jump_act == crypt_jmp_act ) { 
						const float cur_cycle = current.get( )->m_anim_layers.at( 4u ).m_cycle;
						const float cur_rate = current.get( )->m_anim_layers.at( 4u ).m_playback_rate;

						if( cur_cycle != crypt_int( 0.f ) &&
							cur_rate != crypt_int( 0.f ) ) { 

							const float jump_time = cur_cycle / cur_rate;
							if( jump_time != crypt_int( 0.f ) ) { 
								current.get( )->m_on_ground = false;
								current.get( )->m_act_time = current.get( )->m_anim_time - jump_time;
							}
						}
					}
				}
			}

			if( current.get( )->m_anim_layers.at( 4u ).m_weight > crypt_int( 0.f )
				&& current.get( )->m_anim_layers.at( 4u ).m_playback_rate > crypt_int( 0.f )
				&& entry.m_player->lookup_seq_act( jump_seq ) == crypt_jmp_act ) { 
				const float jump_time = ( ( ( current.get( )->m_anim_layers.at( 4u ).m_cycle / current.get( )->m_anim_layers.at( 4u ).m_playback_rate )
					/ game::g_global_vars.get( )->m_interval_per_tick ) + 0.5f ) * game::g_global_vars.get( )->m_interval_per_tick;

				const float update_time = ( game::to_ticks( ( current.get( )->m_anim_time ) ) * game::g_global_vars.get( )->m_interval_per_tick ) - ( ( ( ( 
					current.get( )->m_anim_layers.at( 4u ).m_cycle / current.get( )->m_anim_layers.at( 4u ).m_playback_rate ) / game::g_global_vars.get( )->m_interval_per_tick
					 ) + 0.5f
					 ) * game::g_global_vars.get( )->m_interval_per_tick );

				if( entry.m_player->flags( ) & game::e_ent_flags::on_ground ) { 
					if( update_time > entry.m_player->anim_state( )->m_last_update_time ) { 
						entry.m_player->anim_state( )->m_on_ground = false;
						entry.m_player->pose_params( ).at( 6u ) = crypt_int( 0.f );
						entry.m_player->anim_state( )->m_time_since_in_air = crypt_int( 0.f );
						entry.m_player->anim_state( )->m_last_update_time = update_time;
					}
				}

				static game::cvar_t* sv_gravity = game::g_cvar->find_var( xor_str( "sv_gravity" ) );
				static game::cvar_t* sv_jump_impulse = game::g_cvar->find_var( xor_str( "sv_jump_impulse" ) );
				current.get( )->m_anim_velocity.z( ) = sv_jump_impulse->get_float( ) - sv_gravity->get_float( ) * jump_time;
			}
		}

		if( current.get( )->m_on_ground.has_value( ) ) { 
			const int anim_tick = game::to_ticks( current.get( )->m_anim_time ) - current.get( )->m_lag_ticks;

			if( !current.get( )->m_on_ground.value( ) ) { 
				const int jump_tick = game::to_ticks( current.get( )->m_act_time ) + crypt_int( 1 );

				if( jump_tick == anim_tick )
					entry.m_player->flags( ) |= game::e_ent_flags::on_ground;
				else if( jump_tick == anim_tick + crypt_int( 1 ) ) { 
					entry.m_player->anim_layers( ).at( 4u ).m_playback_rate = current.get( )->m_anim_layers.at( 4u ).m_playback_rate;
					entry.m_player->anim_layers( ).at( 4u ).m_seq = current.get( )->m_anim_layers.at( 4u ).m_seq;
					entry.m_player->anim_layers( ).at( 4u ).m_cycle = crypt_int( 0.f );
					entry.m_player->anim_layers( ).at( 4u ).m_weight = crypt_int( 0.f );
					entry.m_player->flags( ) &= ~game::e_ent_flags::on_ground;
				}
			}
			else { 
				const int land_tick = game::to_ticks( current.get( )->m_act_time ) + crypt_int( 1 );

				if( land_tick == anim_tick ) 
					entry.m_player->flags( ) &= ~game::e_ent_flags::on_ground;
				else if( land_tick == anim_tick + crypt_int( 1 ) ) { 
					entry.m_player->anim_layers( ).at( 5u ).m_playback_rate = current.get( )->m_anim_layers.at( 5u ).m_playback_rate;
					entry.m_player->anim_layers( ).at( 5u ).m_seq = current.get( )->m_anim_layers.at( 5u ).m_seq;
					entry.m_player->anim_layers( ).at( 5u ).m_cycle = crypt_int( 0.f );
					entry.m_player->anim_layers( ).at( 5u ).m_weight = crypt_int( 0.f );
					entry.m_player->flags( ) |= game::e_ent_flags::on_ground;
				}
			}
		}

		if( !( current.get( )->m_flags & game::e_ent_flags::on_ground ) ) { 
			if( current.get( )->m_anim_layers.at( 4 ).m_weight != crypt_int( 0.f )
				&& current.get( )->m_anim_layers.at( 4 ).m_playback_rate != crypt_int( 0.f ) ) { 
				const int cur_seq = current.get( )->m_anim_layers.at( 4 ).m_seq;

				if( entry.m_player->lookup_seq_act( cur_seq ) == crypt_int( 985 ) ) { 

					const float cur_cycle = current.get( )->m_anim_layers.at( 4 ).m_cycle;
					const float previous_cycle = previous.get( )->m_anim_layers.at( 4 ).m_cycle;
					const int previous_seq = previous.get( )->m_anim_layers.at( 4 ).m_seq;

					if( ( cur_cycle != previous_cycle || previous_seq != cur_seq ) && previous_cycle > cur_cycle ) { 
						entry.m_player->pose_params( ).at( 6 ) = crypt_int( 0.f );
						entry.m_player->anim_state( )->m_time_since_in_air = cur_cycle / current.get( )->m_anim_layers.at( 4 ).m_playback_rate;
					}
				}
			}
		}
	}

	void c_anim_sync::setup_bones( game::cs_player_t* player, std::array < sdk::mat3x4_t, game::k_max_bones >& out, float time, bool visual_matrix ) { 
		if( player->team( ) == g_local_player->self( )->team( ) )
			return;

		const auto effects = player->effects( );
		const auto lod_flags = player->anim_lod_flags( );
		const auto anim_occlusion_frame_count = player->anim_occlusion_frame_count( );
		const auto ik_ctx = player->ik( );
		const auto client_effects = player->client_effects( );

		player->effects( ) |= 8u;
		player->anim_lod_flags( ) &= ~2u;
		player->anim_occlusion_frame_count( ) = 0;
		player->ik( ) = nullptr;
		player->client_effects( ) |= 2u;
		player->last_setup_bones_time( ) = 0.f;
		player->invalidate_bone_cache( );

		g_ctx->anim_data( ).m_allow_setup_bones = true;
		player->renderable( )->setup_bones( out.data( ), game::k_max_bones, 0x7FF00, time );
		g_ctx->anim_data( ).m_allow_setup_bones = false;

		player->ik( ) = ik_ctx;

		if( !visual_matrix )
			player->effects( ) = effects;

		player->anim_lod_flags( ) = lod_flags;
		player->anim_occlusion_frame_count( ) = anim_occlusion_frame_count;
		player->client_effects( ) = client_effects;
	}

	void c_resolver::handle_ctx( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry ) { 

		if( !previous.get( ) )
			return;

		set_solve_mode( current, entry );

		 if( current.get( )->m_mode == e_solve_modes::solve_stand )
			solve_stand( current, previous, entry );

		else if( current.get( )->m_mode == e_solve_modes::solve_move )
			solve_walk( current, entry );

		else if( current.get( )->m_mode == e_solve_modes::solve_air )
			solve_air( current, previous, entry );

		current.get( )->m_eye_angles.y( ) = sdk::norm_yaw( current.get( )->m_eye_angles.y( ) );
	}

	void c_resolver::set_solve_mode( cc_def( lag_record_t* )current, player_entry_t& entry ) { 
		auto enemy_weapon = entry.m_player->weapon( );
		if( enemy_weapon ) {
			if( entry.m_player->pin_pulled( )
				&& enemy_weapon->throw_time( ) > 0.0f ) // noone really has anti-aim on nade throw, so don't run resolver if so
				return;
		}

		// check if on ground
		if( entry.m_player->flags( ) & game::e_ent_flags::on_ground ) { 

			// check if moving
			// NOTE: game uses length_2d so we're gonna go ahead and do the same
			if( current.get( )->m_anim_velocity.length( 2u ) > 0.1f && !current.get( )->m_fake_walking )
				current.get( )->m_mode = e_solve_modes::solve_move; // cheat will use moving solve method
			else
				current.get( )->m_mode = e_solve_modes::solve_stand; // cheat will use standing solve method

			// exit from solve mode func
			return;
		}

		// if we arrived here, our flag indicates they're not on ground
		// so use air solve method
		current.get( )->m_mode = e_solve_modes::solve_air;
	}

	void c_resolver::parse_lby_proxy( game::cs_player_t* player, float* new_lby ) { 
		auto& player_entry = hacks::g_lag_comp->entry( player->networkable( )->index( ) - 1 );

		if( !player || player->velocity( ).length( 2u ) > 0.1f 
			|| !( player->flags( ) & game::e_ent_flags::on_ground ) 
			|| player->networkable( )->dormant( ) ) { 
			player_entry.m_body_proxy_updated = false;
			return;
		}

		player_entry.m_old_lby = player_entry.m_lby;
		player_entry.m_lby = *new_lby;

		// lol
		if( std::abs( sdk::angle_diff( player_entry.m_old_lby, player_entry.m_lby ) ) > 17.5f ) { 
			player_entry.m_body_proxy_updated = true;
		}
	}

	void c_resolver::solve_stand( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry ) { 
		hacks::g_resolver->anti_freestand( entry, current.get( )->m_lby );

		float move_anim_time = FLT_MAX;
		float move_delta = FLT_MAX;
		bool temp_moved = false;

		if( entry.m_moving_data.m_time != FLT_MAX ) { 
			move_anim_time = entry.m_moving_data.m_time - current.get( )->m_anim_time;
			move_delta = std::abs( sdk::angle_diff( entry.m_moving_data.m_lby, current.get( )->m_lby ) );
			temp_moved = ( ( entry.m_moving_data.m_origin - current.get( )->m_origin ).length( 3u ) <= crypt_float( 128.f ) ) ? true : false;
		}

		entry.m_moving_data.m_moved = temp_moved;

		const auto at_target_angle = sdk::calc_ang( g_local_player->self( )->origin( ), entry.m_player->origin( ) );

		// NOTE: we do not need move data to do this
		if( previous.get( ) 
			&& !current.get( )->m_fake_walking ) { 
			// or anim lby changed	
			// NOTE: here i remove proxy stuff
			// cus im not sure if proxy is more or less accurate
			// as its not on animation time but just on server
			// which means it will trigger fake updates on break lc etc.. (lol...)
			bool timer_update = entry.m_body_data.m_realign_timer <= current.get( )->m_anim_time && entry.m_body_data.m_has_updated;
			bool body_update = std::abs( sdk::angle_diff( current.get( )->m_lby, previous.get( )->m_lby ) ) >= 17.5f; // will trigger more accurately in case he has a slight direction change

			if( entry.m_lby_misses < crypt_int( 2 ) ) { 
				// update this value
				entry.m_body_data.m_has_updated = body_update || timer_update;

				if( body_update || timer_update ) { 
					// note: this is probably inaccurate, should be simtime and not animtime	
					// ^ reading abt uc makes me think it is actually right but im not sure
					// cus in 2018 update is handled diff, data is sent on lag == 0 and not on m_bSendPacket = true;
					// aka 1 tick after sending packet ( would explain why old sim time + interval )
					entry.m_body_data.m_realign_timer = current.get( )->m_anim_time + game::k_lower_realign_delay;
			
					current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
					current.get( )->m_broke_lby = current.get( )->m_resolved = true;
					current.get( )->m_resolver_method = e_solve_methods::body_flick;
					return;
				}
			}
		}

		if( entry.m_moving_data.m_moved ) { 
			entry.m_had_last_move = !current.get( )->m_fake_walking 
                && current.get( )->m_valid_move 
				&& move_delta != FLT_MAX 
				&& move_delta <= crypt_float( 12.5f ) 
                && entry.m_last_move_misses < crypt_int( 1 );

			if( current.get( )->m_lby == previous.get( )->m_lby
				&& entry.m_lby == entry.m_old_lby
				&& !entry.m_body_proxy_updated
				&& !entry.m_body_data.m_has_updated
				&& entry.m_low_lby_misses < 1 ) { 
                current.get( )->m_resolver_method = e_solve_methods::low_lby;
                current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
            }
            // just stopped will also be the one we use to detect if they broke lby or not
            else if( !current.get( )->m_fake_walking 
                && move_anim_time < crypt_float( 0.2f )
                && !entry.m_body_data.m_has_updated 
				&& !current.get( )->m_broke_lby
                && entry.m_just_stopped_misses < crypt_int( 1 ) 
				&& !entry.m_body_proxy_updated
				) { 
                current.get( )->m_resolver_method = e_solve_methods::just_stopped;
                current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
            }
			else if( entry.m_had_last_move )
            { 
                current.get( )->m_resolver_method = e_solve_methods::last_move_lby;
                current.get( )->m_eye_angles.y( ) = entry.m_moving_data.m_lby;
            }
            else if( !current.get( )->m_fake_walking
                && current.get( )->m_valid_move && is_sideways( current.get( ), entry.m_moving_data.m_lby, true ) 
				&& move_delta != FLT_MAX 
				&& move_delta <= crypt_float( 15.f ) 
                && entry.m_last_move_misses < crypt_int( 1 ) )
            { 
                current.get( )->m_resolver_method = e_solve_methods::last_move_lby;
                current.get( )->m_eye_angles.y( ) = entry.m_moving_data.m_lby;
            }
			else if( is_sideways( current.get( ), current.get( )->m_lby, false )
				&& entry.m_freestand_misses < crypt_int( 2 ) && entry.m_has_freestand )
			{ 
				current.get( )->m_resolver_method = e_solve_methods::freestand;
				current.get( )->m_eye_angles.y( ) = entry.m_freestand_angle;
			}
			else if( !is_sideways( current.get( ), current.get( )->m_lby, false )
				&& entry.m_forwards_misses < crypt_int( 1 ) )
			{ 
				current.get( )->m_resolver_method = e_solve_methods::forwards;
				current.get( )->m_eye_angles.y( ) = at_target_angle.y( );
			}
			else if( !is_sideways( current.get( ), current.get( )->m_lby, false ) && std::abs( sdk::angle_diff( current.get( )->m_lby, at_target_angle.y( ) ) ) <= crypt_float( 65.f )
				&& entry.m_backwards_misses < crypt_int( 1 ) )
			{ 
				current.get( )->m_resolver_method = e_solve_methods::backwards;
				current.get( )->m_eye_angles.y( ) = at_target_angle.y( );
			}
			else if( !current.get( )->m_fake_walking 
                && entry.m_low_lby_misses < crypt_int( 1 ) && ( ( entry.m_body_data.m_has_updated &&
                sdk::angle_diff( entry.m_lby, entry.m_old_lby ) <= 20.f )
                && ( previous.get( ) && sdk::angle_diff( previous.get( )->m_lby, current.get( )->m_lby ) <= 20.f ) ) )
            { 
                current.get( )->m_resolver_method = e_solve_methods::body_flick_res;
                current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
            }
			else { 
				current.get( )->m_resolver_method = e_solve_methods::brute;

				switch( entry.m_stand_moved_misses % 4 ) { 
				case 0:
					current.get( )->m_eye_angles.y( ) = entry.m_moving_data.m_lby;
					break;
				case 1:
					current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
					break;
				case 2:
					current.get( )->m_eye_angles.y( ) = current.get( )->m_lby + crypt_float( 110.f );
					break;
				case 3:
					current.get( )->m_eye_angles.y( ) = current.get( )->m_lby - crypt_float( 110.f );
					break;
				default:
					break;
				}
			}
		}
		else { 
			current.get( )->m_resolver_method = e_solve_methods::brute_not_moved;
			switch( entry.m_stand_not_moved_misses % 5 ) { 
			case 0:
				current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
				break;
			case 1:
				current.get( )->m_eye_angles.y( ) = entry.m_freestand_angle;
				break;
			case 2:
				current.get( )->m_eye_angles.y( ) = at_target_angle.y( ) + 180.f;
				break;
			case 3:
				current.get( )->m_eye_angles.y( ) =	current.get( )->m_lby - crypt_float( 110.f );
				break;
			case 4:
				current.get( )->m_eye_angles.y( ) = current.get( )->m_lby + crypt_float( 110.f );
				break;
			}
		}
	}

	void c_resolver::solve_walk( cc_def( lag_record_t* ) current, player_entry_t& entry ) { 
		const float speed = current.get( )->m_anim_velocity.length( 2u );

		current.get( )->m_resolver_method = e_solve_methods::move;
		current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
		current.get( )->m_broke_lby = false;

		if( speed >= 20.f ) { 
			current.get( )->m_resolved = true;
		}

		// only reset body changed if vel is high
		entry.m_body_data.reset( speed > 30.f );

		// note: above * 0.33f -> slide/walk animation
		if( current.get( )->m_anim_velocity.length( 2u ) > entry.m_player->max_speed( ) * 0.33f )
			current.get( )->m_valid_move = true;

		// fuck this, lets try syncing cycle using actual updates
		// not by trying to guess it cus that fails on micromove, fakewalk and decelerations(slowing down)
		if( speed >= 25.f ) {
			if( entry.m_moving_misses <= 2 )
				entry.m_body_data.m_realign_timer = current.get( )->m_anim_time + 0.22f;
		}

		entry.m_stand_not_moved_misses = entry.m_stand_moved_misses = entry.m_last_move_misses =
		entry.m_forwards_misses = entry.m_backwards_misses = entry.m_freestand_misses, 
		entry.m_lby_misses = entry.m_just_stopped_misses = entry.m_low_lby_misses =
		entry.m_moving_misses = 0;
		entry.m_moving_data.m_moved = false;
		entry.m_moving_data.m_origin = current.get( )->m_origin;
		entry.m_moving_data.m_lby = current.get( )->m_lby;
		entry.m_moving_data.m_time = current.get( )->m_anim_time;

		// note: we wanna set those to lastmove
		// so it uses lastmoving if they walk then stop
		// better / more accurate since that will be animation values
		// ^ put it back to was it was if it rly is seemingly worse
		entry.m_lby = current.get( )->m_lby;
		entry.m_old_lby = current.get( )->m_lby;
	}	

	void c_resolver::solve_air( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry ) { 
		current.get( )->m_resolver_method = e_solve_methods::air;

		const auto vel_yaw = sdk::to_deg( std::atan2( current.get( )->m_anim_velocity.y( ), current.get( )->m_anim_velocity.x( ) ) );

		bool has_body_updated = fabsf( sdk::angle_diff( entry.m_old_lby, entry.m_lby ) ) >= 35.f 
			|| previous.get( ) 
			&& fabsf( sdk::angle_diff( current.get( )->m_lby, previous.get( )->m_lby ) ) >= 35.f;

		float move_diff = fabsf( entry.m_moving_data.m_lby - current.get( )->m_lby );
		float back_diff = fabsf( get_away_angle( current.get( ) ) - current.get( )->m_lby );
		float vel_yaw_diff = fabsf( vel_yaw + crypt_float( 180.f ) - current.get( )->m_lby );

		float low_neg_delta = 17.5f;
		float neg_delta = 35.f;
		
		// NOTE: we do not need move data to do this
		if( previous.get( ) ) { 
			// or anim lby changed	
			// NOTE: here i remove proxy stuff
			// cus im not sure if proxy is more or less accurate
			// as its not on animation time but just on server
			// which means it will trigger fake updates on break lc etc.. (lol...)
			bool timer_update = entry.m_body_data.m_realign_timer <= current.get( )->m_anim_time && entry.m_body_data.m_has_updated;
			bool body_update = std::abs( sdk::angle_diff( current.get( )->m_lby, previous.get( )->m_lby ) ) >= 17.5f; // will trigger more accurately in case he has a slight direction change

			if( entry.m_lby_misses < crypt_int( 2 ) ) { 
				// update this value
				entry.m_body_data.m_has_updated = body_update || timer_update;

				if( body_update || timer_update ) { 
					// note: this is probably inaccurate, should be simtime and not animtime	
					// ^ reading abt uc makes me think it is actually right but im not sure
					// cus in 2018 update is handled diff, data is sent on lag == 0 and not on m_bSendPacket = true;
					// aka 1 tick after sending packet ( would explain why old sim time + interval )
					entry.m_body_data.m_realign_timer = current.get( )->m_anim_time + game::k_lower_realign_delay;
			
					current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
					current.get( )->m_broke_lby = current.get( )->m_resolved = true;
					current.get( )->m_resolver_method = e_solve_methods::body_flick;
					return;
				}
			}
		}

		// lby breaker stuff above everything so we detect lby updates in airs too
		bool use_lby = fabsf( current.get( )->m_lby - low_neg_delta ) >= ( back_diff || vel_yaw_diff ); // detect retards constantly swapping their angles

		if( use_lby && fabsf( previous.get( )->m_lby - current.get( )->m_lby ) <= neg_delta )
			current.get( )->m_eye_angles.y( ) = current.get( )->m_lby + neg_delta;

		if( move_diff <= low_neg_delta && !has_body_updated && entry.m_air_misses < 1 )
			current.get( )->m_eye_angles.y( ) = entry.m_moving_data.m_lby;

		if( back_diff <= vel_yaw_diff && fabsf( vel_yaw_diff - low_neg_delta ) <= ( neg_delta + 10.f ) )
			current.get( )->m_eye_angles.y( ) = vel_yaw + crypt_float( 180.f );

		else if( fabsf( back_diff - neg_delta ) <= ( neg_delta - 7.5f ) )
			current.get( )->m_eye_angles.y( ) = get_away_angle( current.get( ) );

		else if( back_diff >= neg_delta && vel_yaw_diff >= neg_delta )
			current.get( )->m_eye_angles.y( ) = current.get( )->m_lby;
	}

	__forceinline sdk::vec3_t origin( sdk::mat3x4_t who ) { 
		return sdk::vec3_t( who [ 0 ] [ 3 ], who [ 1 ] [ 3 ], who [ 2 ] [ 3 ] );
	}

	void c_setup_bones::handle_ctx( game::cs_player_t* player, sdk::mat3x4_t* bones, int bone_count, int bone_mask, float time ) { 
		if( bone_mask == -1 ) { 
			bone_mask = player->prev_bone_mask( );
		}

		bone_mask |= 0x80000;

		int lod{ };

		int mask = 0x00000400;
		for( ; lod < 8; ++lod, mask <<= 1 ) { 
			if( bone_mask & mask )
				break;
		}
		for( ; lod < 8; ++lod, mask <<= 1 ) { 
			bone_mask |= mask;
		}

		game::bone_accessor_t* bone_accessor = &player->bone_accessor( );

		auto model_bone_counter = ** ( unsigned long** )( g_ctx->addresses( ).m_invalidate_bone_cache + 0x000a );

		if( !bone_accessor )
			return;

		if( player->mdl_bone_cnt( ) != model_bone_counter ) { 
			if( std::numeric_limits < float >::max( ) >= player->last_setup_bones_time( )
				|| time < player->last_setup_bones_time( ) ) { 
				bone_accessor->m_readable_bones = 0.f;
				bone_accessor->m_writable_bones = 0.f;
				player->last_setup_bones_time( ) = time;
			}

			player->prev_bone_mask( ) = player->accumulated_bone_mask( );
		}

		player->accumulated_bone_mask( ) |= bone_mask;

		player->anim_lod_flags( ) = 0;
		player->anim_occlusion_frame_count( ) = 0;

		player->mdl_bone_cnt( ) = model_bone_counter;

		auto origin = player->origin( );
		auto angles = player->abs_ang( );

		alignas( 16 ) sdk::mat3x4_t parent_transform { };

		g_ctx->addresses( ).m_angle_matrix( angles, parent_transform );

		parent_transform[ 0 ][ 3 ] = origin.x( );
		parent_transform[ 1 ][ 3 ] = origin.y( );
		parent_transform[ 2 ][ 3 ] = origin.z( );

		bone_mask |= player->prev_bone_mask( );

		bone_accessor->m_bones = bones;

		auto old_readable_bones = bone_accessor->m_readable_bones;
		auto new_writable_bones = old_readable_bones | bone_mask;
		bone_accessor->m_readable_bones = new_writable_bones;
		bone_accessor->m_writable_bones = new_writable_bones;
	}

	void c_resolver::anti_freestand( player_entry_t& entry, float angle ) { 
		if( !entry.m_player 
			|| entry.m_lag_records.empty( ) )
			return;

		if( !entry.m_player->alive( )
			|| entry.m_player->team( ) == g_local_player->self( )->team( )
			|| entry.m_player->networkable( )->dormant( ) )	
			return;

		const auto at_target_angle = sdk::calc_ang( g_local_player->self( )->origin( ), entry.m_player->origin( ) );

		// constants
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 28.f };

		// best target.
		sdk::vec3_t enemypos = entry.m_player->wpn_shoot_pos( );

		// construct vector of angles to test.
		std::vector< adaptive_angle > angles{ };
		angles.emplace_back( at_target_angle.y( ) - 180.f );
		angles.emplace_back( at_target_angle.y( ) + 90.f );
		angles.emplace_back( at_target_angle.y( ) - 90.f );
		angles.emplace_back( angle );

		// start the trace at the your shoot pos.
		sdk::vec3_t start = g_ctx->shoot_pos( );

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for( auto it = angles.begin( ); it != angles.end( ); ++it ) { 

			// compute the 'rough' estimation of where our head will be.
			sdk::vec3_t end{ enemypos.x( ) + std::cos( sdk::to_rad( it->m_yaw ) ) * RANGE,
				enemypos.y( ) + std::sin( sdk::to_rad( it->m_yaw ) ) * RANGE,
				enemypos.z( ) };

			// draw a line for debugging purposes.
			// g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

			// compute the direction.
			sdk::vec3_t dir = end - start;
			float len = dir.normalize( );

			// should never happen.
			if( len <= 0.f )
				continue;

			// step thru the total distance, 4 units per step.
			for( float i{ 0.f }; i < len; i += STEP ) { 
				// get the current step position.
				sdk::vec3_t point = start + ( dir * i );

				// get the contents at this point.
				auto contents = game::g_engine_trace->get_point_contents( point, CS_MASK_SHOOT, nullptr );

				// contains nothing that can stop a bullet.
				if( !( contents & CS_MASK_SHOOT ) )
					continue;

				float mult = 1.f;

				// over 50% of the total length, prioritize this shit.
				if( i > ( len * 0.5f ) )
					mult = 1.25f;

				// over 90% of the total length, prioritize this shit.
				if( i > ( len * 0.75f ) )
					mult = 1.5f;

				// over 90% of the total length, prioritize this shit.
				if( i > ( len * 0.9f ) )
					mult = 2.f;

				// append 'penetrated distance'.
				it->m_dist += ( STEP * mult );

				// mark that we found anything.
				valid = true;
			}
		}

		if( !valid ) { 
			entry.m_has_freestand = false;
			return;
		}

		// put the most distance at the front of the container.
		std::sort( angles.begin( ), angles.end( ),
			[]( const adaptive_angle& a, const adaptive_angle& b ) { 
				return a.m_dist > b.m_dist;
			} );

		// the best angle should be at the front now.
		adaptive_angle* best = &angles.front( );

		entry.m_has_freestand = true;
		entry.m_freestand_angle = best->m_yaw;
	}

	void c_local_sync::handle_ctx ( const game::user_cmd_t& user_cmd, bool& send_packet ) { 
		if( game::g_client_state.get( )->m_choked_cmds /*|| g_exploits->is_in_defensive*/) // prevent animations from double update since we want to update only last received command
			return;

		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		auto anim_state = g_local_player->self( )->anim_state( );

		if( !anim_state )
			return;

		const auto cur_time = game::g_global_vars.get( )->m_cur_time;
		const auto real_time = game::g_global_vars.get( )->m_real_time;
		const auto frame_time = game::g_global_vars.get( )->m_frame_time;
		const auto abs_frame_time = game::g_global_vars.get( )->m_abs_frame_time;
		const auto tick_count = game::g_global_vars.get( )->m_tick_count;
		const auto interp_amt = game::g_global_vars.get( )->m_interp_amt;
		const auto frame_count = game::g_global_vars.get( )->m_frame_count;

		g_ctx->anim_data( ).m_local_data.m_anim_frame = game::to_time( g_local_player->self( )->tick_base( ) ) - g_ctx->anim_data( ).m_local_data.m_anim_time;
		g_ctx->anim_data( ).m_local_data.m_anim_time = game::to_time( g_local_player->self( )->tick_base( ) );

		game::g_global_vars.get( )->m_cur_time = game::to_time( g_local_player->self( )->tick_base( ) );
		game::g_global_vars.get( )->m_real_time = game::to_time( g_local_player->self( )->tick_base( ) );
		game::g_global_vars.get( )->m_abs_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_tick_count = g_local_player->self( )->tick_base( );
		game::g_global_vars.get( )->m_frame_count = g_local_player->self( )->tick_base( );
		game::g_global_vars.get( )->m_interp_amt = 0.0f;

		if( g_local_player->self( )->spawn_time( ) != g_ctx->anim_data( ).m_local_data.m_spawn_time ) { 
			g_ctx->anim_data( ).m_allow_update = true;
			g_local_player->self( )->update_client_side_anim( );
			g_ctx->anim_data( ).m_allow_update = false;
			g_ctx->anim_data( ).m_local_data.m_anim_event.m_flags = static_cast < std::uint32_t > ( g_local_player->self( )->flags( ) );
			g_ctx->anim_data( ).m_local_data.m_anim_event.m_move_type = static_cast < std::uint8_t > ( g_local_player->self( )->move_type( ) );
			g_ctx->anim_data( ).m_local_data.m_spawn_time = g_local_player->self( )->spawn_time( );
		}
		
		std::memcpy( g_local_player->self( )->anim_layers( ).data( ), get_anim_layers( ).data( ), sizeof( game::anim_layer_t ) * 13 );

		//if( !( g_anti_aim->m_fake_moving && !g_anti_aim->can_choke( ) ) ) // lol ye	
		g_ctx->anim_data( ).m_local_data.m_anim_ang = user_cmd.m_view_angles;

		anim_state->m_last_update_frame = game::g_global_vars.get( )->m_frame_count - crypt_int( 1 );

		g_local_player->self( )->ieflags( ) &= ~( game::cs_player_t::entityflags_t::EFL_DIRTY_ABSVELOCITY 
			| game::cs_player_t::entityflags_t::EFL_DIRTY_ABSTRANSFORM );

		g_local_player->self( )->abs_velocity( ) = g_local_player->self( )->velocity( );

		anim_state->m_move_weight = crypt_float( 0.f );

		game::g_prediction->set_local_view_angles( g_ctx->anim_data( ).m_local_data.m_anim_ang );

		g_local_player->self( )->set_abs_ang( sdk::qang_t( 0.f, g_ctx->anim_data( ).m_local_data.m_abs_ang, 0.f ) );

		bool cl_side_backup = g_local_player->self( )->client_side_anim_proxy( );

		do_anim_event( );

		for( int layer = 0u; layer < crypt_int( 13 ); layer++ )
			g_local_player->self( )->anim_layers( ).at( layer ).m_owner = g_local_player->self( );

		m_last_upd = game::g_global_vars.get( )->m_cur_time;
		m_last_choke = std::max ( game::g_global_vars.get( )->m_cur_time - anim_state->m_last_update_time, game::g_global_vars.get( )->m_interval_per_tick );

		g_local_player->self( )->client_side_anim_proxy( ) = true;
		g_ctx->anim_data( ).m_allow_update = true;
		g_local_player->self( )->update_client_side_anim( );
		g_ctx->anim_data( ).m_allow_update = false;
		g_local_player->self( )->client_side_anim_proxy( ) = cl_side_backup;

		m_old_layers = m_anim_layers;
		m_old_params = m_pose_params;
		std::memcpy ( m_pose_params.data( ), g_local_player->self( )->pose_params( ).data( ), sizeof ( float_t ) * 24 );
		std::memcpy ( m_anim_layers.data( ), g_local_player->self( )->anim_layers( ).data( ), sizeof ( game::anim_layer_t ) * 13 );

		g_ctx->anim_data( ).m_local_data.m_abs_ang = anim_state->m_foot_yaw;

		auto weight_12 = g_local_player->self( )->anim_layers( ).at ( 12 ).m_weight;

		g_local_player->self( )->anim_layers( ).at ( 12 ).m_weight = crypt_float( 0.f );

		if( anim_state->m_on_ground 
			&& !g_ctx->anim_data( ).m_local_data.m_on_ground ) { 
			g_ctx->anim_data( ).m_local_data.m_lby = g_ctx->anim_data( ).m_local_data.m_anim_ang.y( );
			g_ctx->anim_data( ).m_local_data.m_lby_upd = g_ctx->anim_data( ).m_local_data.m_anim_time;

			if( hacks::g_anti_aim->m_fake_moving )
				g_ctx->anim_data( ).m_local_data.m_can_break = true;
			else
				g_ctx->anim_data( ).m_local_data.m_can_break = false;
		}
		else if( anim_state->m_speed_2d > crypt_float( 0.1f ) ) { 
			if( anim_state->m_on_ground )
				g_ctx->anim_data( ).m_local_data.m_lby = g_ctx->anim_data( ).m_local_data.m_anim_ang.y( );

			if( hacks::g_anti_aim->m_fake_moving )
				g_ctx->anim_data( ).m_local_data.m_can_break = true;
			else
				g_ctx->anim_data( ).m_local_data.m_can_break = false;

			g_ctx->anim_data( ).m_local_data.m_lby_upd = g_ctx->anim_data( ).m_local_data.m_anim_time + ( game::k_lower_realign_delay * 0.2f );
		}
		else if( g_ctx->anim_data( ).m_local_data.m_anim_time > g_ctx->anim_data( ).m_local_data.m_lby_upd ) { 
			g_ctx->anim_data( ).m_local_data.m_lby = g_ctx->anim_data( ).m_local_data.m_anim_ang.y( );
			g_ctx->anim_data( ).m_local_data.m_lby_upd = g_ctx->anim_data( ).m_local_data.m_anim_time + game::k_lower_realign_delay;
			g_ctx->anim_data( ).m_local_data.m_can_break = true;
		}

		setup_bones ( g_ctx->anim_data( ).m_local_data.m_bones, g_local_player->self( )->sim_time( ) );

		g_local_player->self( )->anim_layers( ).at ( 12 ).m_weight = weight_12;

		std::memcpy ( g_local_player->self( )->anim_layers( ).data( ), get_anim_layers( ).data( ), sizeof ( game::anim_layer_t ) * 13 );
		std::memcpy ( g_local_player->self( )->pose_params( ).data( ), m_pose_params.data( ), sizeof ( float_t ) * 24 );

		for( std::ptrdiff_t i { }; i < game::k_max_bones; ++i ) { 
			g_ctx->anim_data( ).m_local_data.m_bone_origins.at ( i ) = g_local_player->self( )->abs_origin( ) - origin ( g_ctx->anim_data ( ).m_local_data.m_bones.at ( i ) );
		}

		game::g_global_vars.get( )->m_cur_time = cur_time;
		game::g_global_vars.get( )->m_real_time = real_time;
		game::g_global_vars.get( )->m_abs_frame_time = abs_frame_time;
		game::g_global_vars.get( )->m_frame_time = frame_time;
		game::g_global_vars.get( )->m_tick_count = tick_count;
		game::g_global_vars.get( )->m_frame_count = frame_count;
		game::g_global_vars.get( )->m_interp_amt = interp_amt;

		g_ctx->anim_data( ).m_local_data.m_on_ground = anim_state->m_on_ground;
		g_ctx->anim_data( ).m_local_data.m_speed_2d = anim_state->m_speed_2d;
	}

	void c_local_sync::simulate( ) { 
		if( game::g_client_state.get( )->m_choked_cmds )
			return;

		const auto anim_state = g_local_player->self( )->anim_state( );
		if( !anim_state )
			return; 

		g_ctx->anim_data( ).m_local_data.m_max_body_yaw = anim_state->m_aim_yaw_max;
		g_ctx->anim_data( ).m_local_data.m_min_body_yaw = anim_state->m_aim_yaw_min;

		auto speed_frac = std::max( 0.f, std::min( anim_state->m_speed_as_portion_of_walk_speed, 1.f ) );
		auto speed_factor = std::max( 0.f, std::min( anim_state->m_speed_as_portion_of_crouch_speed, 1.f ) );

		auto max_aim_width = ( ( anim_state->m_walk_to_run_transition * -0.30000001f ) - crypt_float( 0.19999999f ) ) * speed_frac + crypt_float( 1.f );

		if( anim_state->m_duck_amount > 0.0f )
			max_aim_width += ( ( anim_state->m_duck_amount * speed_factor ) * ( 0.5f - max_aim_width ) );

		g_ctx->anim_data( ).m_local_data.m_max_body_yaw *= max_aim_width;
		g_ctx->anim_data( ).m_local_data.m_min_body_yaw *= max_aim_width;
	}

	void c_local_sync::do_anim_event( ) { 
		auto fifth_layer = &g_local_player->self( )->anim_layers( ).at( 5u );
		auto fourth_layer = &g_local_player->self( )->anim_layers( ).at( 4u );

		if( !fifth_layer
			|| !fourth_layer )
			return;

		if( g_ctx->anim_data( ).m_local_data.m_anim_event.m_move_type != static_cast < std::uint8_t > ( game::e_move_type::ladder )
			&& g_local_player->self( )->move_type( ) == game::e_move_type::ladder )
			g_local_player->self( )->anim_state( )->set_layer_seq( fifth_layer, crypt_int( 987 ) );
		else if( g_ctx->anim_data( ).m_local_data.m_anim_event.m_move_type == static_cast < std::uint8_t > ( game::e_move_type::ladder )
			&& g_local_player->self( )->move_type( ) != game::e_move_type::ladder )
			g_local_player->self( )->anim_state( )->set_layer_seq( fifth_layer, crypt_int( 986 ) );
		else { 
			if( g_local_player->self( )->flags( ) & game::e_ent_flags::on_ground ) { 
				if( !( g_ctx->anim_data( ).m_local_data.m_anim_event.m_flags & static_cast < std::uint32_t > ( game::e_ent_flags::on_ground ) ) ) { 
					g_local_player->self( )->anim_state( )->set_layer_seq( fifth_layer, g_local_player->self( )->anim_state( )->m_time_since_in_air > crypt_float( 1.0f ) ? crypt_int( 989 ) : crypt_int( 988 ) );
				}
			}
			else if( g_ctx->anim_data( ).m_local_data.m_anim_event.m_flags & static_cast < std::uint32_t > ( game::e_ent_flags::on_ground ) ) { 
				if( g_local_player->self( )->velocity( ).z( ) > 0.f ) { 
					g_local_player->self( )->anim_state( )->set_layer_seq( fourth_layer, crypt_int( 985 ) );
				}
				else { 
					g_local_player->self( )->anim_state( )->set_layer_seq( fourth_layer, crypt_int( 986 ) );
				}
			}
		}

		g_ctx->anim_data( ).m_local_data.m_anim_event.m_move_type = static_cast < std::uint8_t > ( g_local_player->self( )->move_type( ) );
		g_ctx->anim_data( ).m_local_data.m_anim_event.m_flags = static_cast < std::uint32_t > ( g_local_player->self( )->flags( ) );
	}

	void c_local_sync::handle_anim_interp( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( )
			|| !g_local_player->self( )->anim_state( ) )
			return;

		if( !( g_local_player->self( )->anim_state( )->m_on_ground && g_local_player->self( )->anim_state( )->m_landing ) ) { 
			for( size_t i { }; i < 18; ++i ) { 
				float lerp = std::min ( m_last_choke, game::g_global_vars.get( )->m_interval_per_tick * 2.f );
				float update_delta = m_last_choke;
				float update_lerp = std::clamp ( update_delta - lerp, 0.f, 1.f );
				if( update_delta > 0.f )
					lerp = std::clamp ( lerp, 0.f, update_delta );

				float lerp_progress = ( m_last_upd + lerp - game::g_global_vars.get( )->m_cur_time ) / lerp;

				float old_param = m_old_params.at ( i );
				float param = m_pose_params.at ( i );

				if( i == 11 )
					continue;

				if( !isfinite ( old_param ) || !isfinite ( param ) )
					continue;

				float jmp_param = sdk::lerp_dir ( old_param, param, update_lerp / update_delta );
				float final_param = sdk::lerp_dir ( param, jmp_param, std::clamp ( lerp_progress, 0.f, 1.f ) );

				g_local_player->self( )->pose_params( ).at ( i ) = final_param;
			}

			float lerp = std::min ( m_last_choke, game::g_global_vars.get( )->m_interval_per_tick * 2.f );
			float update_delta = m_last_choke;
			float update_lerp = std::clamp ( update_delta - lerp, 0.f, 1.f );
			if( update_delta > 0.f )
				lerp = std::clamp ( lerp, 0.f, update_delta );

			for( size_t i { }; i < 13; ++i ) { 
				auto old_cycle = m_old_layers.at ( i ).m_cycle;
				auto cycle = m_anim_layers.at ( i ).m_cycle;

				float lerp_progress = ( m_last_upd + lerp - game::g_global_vars.get( )->m_cur_time ) / lerp;

				float jmp_cycle = sdk::lerp_dir ( old_cycle, cycle, update_lerp / update_delta );
				float final_cycle = sdk::lerp_dir ( cycle, jmp_cycle, std::clamp ( lerp_progress, 0.f, 1.f ) );

				g_local_player->self( )->anim_layers( ).at ( i ).m_cycle = final_cycle;
			}
		}
	}

	void c_local_sync::setup_bones( std::array < sdk::mat3x4_t, 256 >& out, float time, int custom_max ) { 
		
		const float cur_time = game::g_global_vars.get( )->m_cur_time;
		const float real_time = game::g_global_vars.get( )->m_real_time;
		const float frame_time = game::g_global_vars.get( )->m_frame_time;
		const float abs_frame_time = game::g_global_vars.get( )->m_abs_frame_time;
		const int tick_count = game::g_global_vars.get( )->m_tick_count;
		const float interp_amt = game::g_global_vars.get( )->m_interp_amt;
		const int frame_count = game::g_global_vars.get( )->m_frame_count;

		game::g_global_vars.get( )->m_cur_time = time;
		game::g_global_vars.get( )->m_real_time = time;
		game::g_global_vars.get( )->m_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_abs_frame_time = game::g_global_vars.get( )->m_interval_per_tick;
		game::g_global_vars.get( )->m_frame_count = game::to_ticks( time );
		game::g_global_vars.get( )->m_tick_count = game::to_ticks( time );
		game::g_global_vars.get( )->m_interp_amt = 0.f;

		const uint32_t effects = g_local_player->self( )->effects( );
		const int lod_flags = g_local_player->self( )->anim_lod_flags( );
		const int anim_occlusion_frame_count = g_local_player->self( )->anim_occlusion_frame_count( );
		const int client_effects = g_local_player->self( )->client_effects( );
		const auto ik_ctx = g_local_player->self( )->ik( );

		g_local_player->self( )->effects( ) |= 8u;
		g_local_player->self( )->anim_lod_flags( ) &= ~2u;
		g_local_player->self( )->anim_occlusion_frame_count( ) = 0;
		g_local_player->self( )->ik( ) = nullptr;
		g_local_player->self( )->client_effects( ) |= 2u;
		g_local_player->self( )->last_setup_bones_time( ) = 0.f;
		g_local_player->self( )->invalidate_bone_cache( );

		g_ctx->anim_data( ).m_allow_setup_bones = true;

		if( custom_max == -1 )
			g_local_player->self( )->renderable( )->setup_bones( out.data( ), game::k_max_bones, 0x7FF00, time );
		else 
			g_local_player->self( )->renderable( )->setup_bones( out.data( ), game::k_max_bones, 0x100, time );

		g_ctx->anim_data( ).m_allow_setup_bones = false;

		g_local_player->self( )->ik( ) = ik_ctx;
		g_local_player->self( )->effects( ) = effects;
		g_local_player->self( )->anim_lod_flags( ) = lod_flags;
		g_local_player->self( )->anim_occlusion_frame_count( ) = anim_occlusion_frame_count;
		g_local_player->self( )->client_effects( ) = client_effects;

		game::g_global_vars.get( )->m_cur_time = cur_time;
		game::g_global_vars.get( )->m_real_time = real_time;
		game::g_global_vars.get( )->m_abs_frame_time = abs_frame_time;
		game::g_global_vars.get( )->m_frame_time = frame_time;
		game::g_global_vars.get( )->m_tick_count = tick_count;
		game::g_global_vars.get( )->m_frame_count = frame_count;
		game::g_global_vars.get( )->m_interp_amt = interp_amt;
	}
}