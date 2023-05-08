#include "../../../csgo.hpp"
#include <windows.h>
#include <mmsystem.h>
#include "../hit_sounds.h"

#include <playsoundapi.h>
#pragma comment( lib, "Winmm.lib" )

__forceinline constexpr std::uint32_t hash_1( const char* str )
{ 
	auto hash = 0x811c9dc5u;

	char v4 { };
	do { 
		v4 = *str++;
		hash = 0x1000193u * ( hash ^ v4 );
	} while( v4 );

	return hash;
}

namespace csgo::hacks { 

	void c_shots::on_new_event( game::game_event_t* const event )
	{ 
		if( !g_local_player 
			|| !game::g_engine->in_game( ) )
			return;

		m_elements.erase( std::remove_if( m_elements.begin( ), m_elements.end( ),
			[ & ]( const shot_t& shot ) { 
				if( std::abs( game::g_global_vars.get( )->m_real_time - shot.m_shot_time ) >= 2.5f )
					return true;

				if( shot.m_processed
					|| shot.m_cmd_number == -1
					|| shot.m_process_tick
					|| std::abs( game::g_client_state.get( )->m_cmd_ack - shot.m_cmd_number ) <= 20 )
					return false;

				const auto delta = std::abs( game::g_global_vars.get( )->m_real_time - shot.m_sent_time );
				return delta > 2.5f || delta > shot.m_latency;
			}
		 ), m_elements.end( ) );

		if( strstr( event->name( ), xor_str( "bomb_beginplant" ) ) )
		{
			game::base_entity_t* c4{ };
			sdk::vec3_t explosion_origin{ }, explosion_origin_adjusted{ };
			game::trace_filter_simple_t filter{ };
			game::trace_t tr{ };
			
			game::g_cvar->error_print( true, "ye" );
		}
		else if( strstr( event->name( ), xor_str( "player_footstep" ) ) ) {
			const auto ent = game::g_entity_list->get_entity( game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );
			if( !ent )
				return;

			if( ent->is_player( ) ) {
				game::cs_player_t* player = reinterpret_cast< game::cs_player_t* >( ent );
				if( !player->networkable( ) 
					|| player == g_local_player->self( ) 
					|| player->friendly( g_local_player->self( ) ) )
					return;
					
				auto cfg = g_visuals->cfg( );

				sdk::col_t clr = sdk::col_t( cfg.m_foot_step_esp_clr[ 0 ] * 255.f, cfg.m_foot_step_esp_clr[ 1 ] * 255.f, cfg.m_foot_step_esp_clr[ 2 ] * 255.f, cfg.m_foot_step_esp_clr[ 3 ] * 255.f );

				if( cfg.m_foot_step_esp )
					g_visuals->push_beam_info( { game::g_global_vars.get( )->m_real_time, player->abs_origin( ), { }, clr, player->networkable( )->index( ), player->tick_base( ), false, true } );
			}
		}
		else if( strstr( event->name( ), xor_str( "player_hurt" ) ) ) {
			g_shot_construct->on_hurt( event );
			const auto victim = game::g_entity_list->get_entity( game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );
			if( !victim )
				return;

			auto misc_cfg = g_misc->cfg( );

			const auto hitgroup = event->get_int( xor_str( "hitgroup" ) );

			if( victim->is_player( ) ) {
				game::cs_player_t* player = reinterpret_cast< game::cs_player_t* >( victim );
				const auto attacker = game::g_entity_list->get_entity( game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );

				if( !attacker 
					|| !attacker->networkable( )
					|| !victim )
					return;

				if( !attacker->is_player( ) )
					return;

				if( misc_cfg.m_notification_logs & 2 ) {
					game::player_info_t info{ };
					if( game::g_engine->get_player_info( attacker->networkable( )->index( ), &info ) ) {
						if( attacker != g_local_player->self( )
							&& victim == g_local_player->self( ) ) {
							std::string name{ std::string( info.m_name ).substr( 0, 24 ) };

							std::string out = tfm::format( xor_str( "harmed by %s in the %s for %i damage (%i remain)\n" ),
								name, g_shot_construct->m_groups[ hitgroup ], event->get_int( xor_str( "dmg_health" ) ), event->get_int( xor_str( "health" ) ) );

							g_logs->push_log( out, sdk::col_t( 255, 255, 255, 255 ) );
						}
					}
				}

				if( !player->networkable( ) 
					|| player == g_local_player->self( ) 
					|| player->friendly( g_local_player->self( ) ) )
					return;
					
				auto cfg = g_visuals->cfg( );

				sdk::col_t clr = sdk::col_t( cfg.m_foot_step_esp_clr[ 0 ] * 255.f, cfg.m_foot_step_esp_clr[ 1 ] * 255.f, 
					cfg.m_foot_step_esp_clr[ 2 ] * 255.f, cfg.m_foot_step_esp_clr[ 3 ] * 255.f );

				if( cfg.m_foot_step_esp )
					g_visuals->push_beam_info( { game::g_global_vars.get( )->m_real_time, player->abs_origin( ), { }, clr, player->networkable( )->index( ), player->tick_base( ), false, true } );
			}

			if( misc_cfg.m_hit_marker_sound ) { 
				game::g_engine->exec_cmd( xor_str( "play buttons\\arena_switch_press_02.wav" ) );
			}

			if( hitgroup == int( game::e_hitgroup::gear ) )
				return;

			const auto shot = last_unprocessed( );
			if( !shot
				|| ( shot->m_target.m_entry && shot->m_target_index != victim->networkable( )->index( ) ) )
				return;

			shot->m_server_info.m_hitgroup = hitgroup;
			shot->m_server_info.m_dmg = event->get_int( xor_str( "dmg_health" ) );
			shot->m_server_info.m_hurt_tick = game::g_client_state.get( )->m_server_tick;
		}
		else if( strstr( event->name( ), xor_str( "weapon_fire" ) ) ) {
			g_shot_construct->on_fire( event );

			if( !event || !g_local_player->self( ) )
				return;

			// get attacker, if its not us, screw it.
			auto attacker = game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );

			const auto ent = game::g_entity_list->get_entity( game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );
			if( ent ) {
				if( ent->is_player( ) ) {
					game::cs_player_t* player = reinterpret_cast< game::cs_player_t* >( ent );
					if( !player->networkable( ) 
						|| player == g_local_player->self( ) 
						|| player->friendly( g_local_player->self( ) ) )
						return;
					
					auto cfg = g_visuals->cfg( );

					sdk::col_t clr = sdk::col_t( cfg.m_foot_step_esp_clr[ 0 ] * 255.f, cfg.m_foot_step_esp_clr[ 1 ] * 255.f, cfg.m_foot_step_esp_clr[ 2 ] * 255.f, cfg.m_foot_step_esp_clr[ 3 ] * 255.f );

					if( cfg.m_foot_step_esp )
						g_visuals->push_beam_info( { game::g_global_vars.get( )->m_real_time, player->abs_origin( ), { }, clr, player->networkable( )->index( ), player->tick_base( ), false, true } );
				}
			}

			if( attacker != game::g_engine->get_local_player( ) )
				return;

			if( m_elements.empty( ) )
				return;

			const auto shot = std::find_if( 
				m_elements.begin( ), m_elements.end( ),
				[ ]( const shot_t& shot ) { 
					return shot.m_cmd_number != -1 && !shot.m_server_info.m_fire_tick
						&& std::abs( game::g_client_state.get( )->m_cmd_ack - shot.m_cmd_number ) <= 20;
				}
				);

			if( shot == m_elements.end( ) )
				return;

			shot->m_process_tick = game::g_global_vars.get( )->m_real_time + 2.5f;
			shot->m_server_info.m_fire_tick = game::g_client_state.get( )->m_server_tick;
		}
		else if( strstr( event->name( ), xor_str( "bullet_impact" ) ) ) {
			g_shot_construct->on_impact( event );
			if( !event || !g_local_player->self( ) )
				return;

			// get attacker, if its not us, screw it.
			auto attacker = game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );
			if( attacker != game::g_engine->get_local_player( ) )
				return;

			// decode impact coordinates and convert to vec3.
			sdk::vec3_t pos = { 
				event->get_float( xor_str( "x" ) ),
				event->get_float( xor_str( "y" ) ),
				event->get_float( xor_str( "z" ) )
			};

			if( const auto shot = last_unprocessed( ) )
				shot->m_server_info.m_impact_pos = pos;
		}
		else if( strstr( event->name( ), xor_str( "item_equip" ) ) ) {
			const auto idx = game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );

			hacks::g_visuals->m_dormant_data[ idx ].m_weapon_id = event->get_int( xor_str( "defindex" ) );
			hacks::g_visuals->m_dormant_data[ idx ].m_weapon_type = event->get_int( xor_str( "weptype" ) );
		}
		else if( strstr( event->name( ), xor_str( "round_prestart" ) ) )
		{
			constexpr uint8_t blue_clr [ 4 ] = { 130, 130, 130, 255 };
			game::g_cvar->con_print( false, *blue_clr, xor_str( "\n\n------- NEW ROUND STARTED -------\n\n" ) );
			for( std::size_t i { }; i < game::g_global_vars.get( )->m_max_clients; ++i ) { 
				hacks::g_visuals->m_dormant_data [ i ].m_origin = { };
				hacks::g_visuals->m_dormant_data [ i ].m_receive_time = 0.f;
				hacks::g_visuals->m_dormant_data [ i ].m_alpha = std::lerp( hacks::g_visuals->m_dormant_data [ i ].m_alpha, 0.f, 8.f * game::g_global_vars.get( )->m_frame_time );
				hacks::g_visuals->m_dormant_data [ i ].m_alpha = std::clamp( hacks::g_visuals->m_dormant_data [ i ].m_alpha, 0.f, 255.f );
				hacks::g_visuals->m_dormant_data[ i ].m_use_shared = false;
				hacks::g_visuals->m_dormant_data[ i ].m_weapon_id = 0;
				hacks::g_visuals->m_dormant_data[ i ].m_weapon_type = -1;
				hacks::g_visuals->m_dormant_data.at( i ).m_last_shared_time = 0.f;
			}
			g_ctx->buy_bot( ) = 2;
			hacks::g_eng_pred->reset_on_spawn( );
			g_ctx->anim_data( ).m_local_data.reset( );

			g_shots->m_elements.clear( );

			game::kill_feed_t* feed = ( game::kill_feed_t* ) game::g_hud->find_element( HASH( "SFHudDeathNoticeAndBotStatus" ) );

			if( feed ) { 
				g_ctx->addresses( ).m_clear_notices( feed );
			}

			hacks::g_local_sync->m_anim_layers = { };
			hacks::g_local_sync->m_old_layers = { };

			hacks::g_local_sync->m_pose_params = { };
			hacks::g_local_sync->m_old_params = { };

			hacks::g_visuals->bullet_trace_info.clear( );
			hacks::g_visuals->m_hit_markers.clear( );

			for( std::size_t i { }; i < 64u; ++i ) { 
				auto& entry = g_lag_comp->entry( i );

				entry.m_stand_not_moved_misses = entry.m_stand_moved_misses = entry.m_last_move_misses =
					entry.m_forwards_misses = entry.m_backwards_misses = entry.m_freestand_misses,
					entry.m_lby_misses = entry.m_just_stopped_misses = entry.m_no_fake_misses =
					entry.m_moving_misses = entry.m_low_lby_misses = 0;

				entry.m_body_data.reset( true );
				entry.m_moving_data.reset( );
			}
		}

		switch( hash_1( event->name( ) ) ) { 
			case 0x2df1832d: { 
				const auto entity = game::g_entity_list->get_entity( game::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );
				if( !entity
					|| entity == g_local_player->self( )
					|| ( ( static_cast < game::cs_player_t* > ( entity ) )->friendly( g_local_player->self( ) ) ) )
					return;

				hacks::g_visuals->m_dormant_data.at( entity->networkable( )->index( ) ).m_receive_time = game::g_global_vars.get( )->m_cur_time;
				hacks::g_visuals->m_dormant_data.at( entity->networkable( )->index( ) ).m_origin = entity->origin( );

			}break;
		}
	}

	float get_absolute_time( )
	{ 
		return ( float )( clock( ) /( float ) 1000.f );
	}

	void c_logs::draw_data( )
	{ 
		for( int i = 0u; i < m_logs.size( ); i++ )
		{ 
			log_data_t* log_data = &m_logs[ i ];
			if( !log_data )
	             continue;

			float_t time_delta = get_absolute_time( ) - log_data->m_creation_time;

			if( time_delta >= 2.65f )
			{ 
				m_logs.erase( m_logs.begin( ) + i );
				continue;
			}

			auto size = g_misc->m_fonts.m_verdana->CalcTextSizeA( 12.f, FLT_MAX, NULL, log_data->m_string.c_str( ) );

			if( time_delta >= 2.5f )
			{ 
				log_data->m_text_alpha = std::lerp( log_data->m_text_alpha, 0.f, 18.f * game::g_global_vars.get( )->m_frame_time );
				log_data->m_spacing = std::lerp( log_data->m_spacing, -size.x * 2.f, 8.f * game::g_global_vars.get( )->m_frame_time );
			}
			else
			{ 
				log_data->m_text_alpha = std::lerp( log_data->m_text_alpha, 1.f, 9.f * game::g_global_vars.get( )->m_frame_time );;
				log_data->m_spacing = std::lerp( log_data->m_spacing, 4.f, 16.f * game::g_global_vars.get( )->m_frame_time );
			}

			constexpr uint8_t white_clr[ 4 ] = { 225, 225, 225, 225 };
			constexpr uint8_t blue_clr[ 4 ]  = { 245, 245, 245, 225 };

			if( !log_data->m_printed )
			{ 
				game::g_cvar->con_print( false, *blue_clr, xor_str("[secret_hack24] ") );
				game::g_cvar->con_print( false, *white_clr, log_data->m_string.c_str( ) );
				log_data->m_printed = true;
			}

			g_render->text( log_data->m_string, sdk::vec2_t( log_data->m_spacing, size.y * i ),
				sdk::col_t( log_data->m_color.r( ), log_data->m_color.g( ), log_data->m_color.b( ),
					( int )( 255.f * log_data->m_text_alpha ) ), hacks::g_misc->m_fonts.m_verdana, false, false, false, false, true );	
		}	
	}

	void c_logs::push_log( std::string string, sdk::col_t color )
	{ 
		log_data_t data;

		data.m_creation_time = get_absolute_time( );
		data.m_spacing = -160.f;
		data.m_text_alpha = 0.f;
		data.m_string = string;
		data.m_color = color;

		m_logs.emplace_back( data );

		while( m_logs.size( ) > 10.f )
			m_logs.pop_back( );
	}

	void c_shot_construct::on_rage_bot( aim_target_t* target, float damage, int bullets, std::shared_ptr < lag_record_t > record, int hitbox, const sdk::vec3_t& shoot_pos, int cmd_number ) { 
		// setup new shot data.
		c_shot_record shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_shot_time = game::g_global_vars.get( )->m_real_time;
		shot.m_damage = damage;
		shot.m_pos = shoot_pos;
		shot.m_hitbox = hitbox;
		shot.m_impacted = false;
		shot.m_confirmed = false;
		shot.m_hurt = false;
		shot.m_cmd_num = cmd_number;
		shot.m_lat = g_ctx->net_info( ).m_latency.m_out;

		if( g_local_player->self( )->weapon( )
			&& g_local_player->self( )->weapon( )->info( ) ) { 
			shot.m_weapon_range = g_local_player->self( )->weapon( )->info( )->m_range;
		}

		if( target 
			&& record ) { 
			auto matrix = record->m_bones;

			shot.m_matrix = matrix;
		}

		// add to tracks.
		m_shots.push_front( shot );

		while( m_shots.size( ) > 128 )
			m_shots.pop_back( );
	}

	void c_shot_construct::on_impact( game::game_event_t* evt )
	{ 
		int        attacker;
		sdk::vec3_t     pos; //dir, start, end;
		// float      time;
		// game::trace_t trace;

		// screw this.
		if( !evt || !g_local_player->self( ) )
			return;

		auto cfg = g_visuals.get( )->cfg( );

		// decode impact coordinates and convert to vec3.
		pos = { 
			evt->get_float( xor_str( "x" ) ),
			evt->get_float( xor_str( "y" ) ),
			evt->get_float( xor_str( "z" ) )
		};

		// get attacker, if its not us, screw it.
		attacker = game::g_engine->index_for_uid( evt->get_int( xor_str( "userid" ) ) );

		if( !attacker )
			return;

		auto entity = static_cast < game::cs_player_t* > ( game::g_entity_list->get_entity( attacker ) );

		if( !entity )
			return;

		if( !entity->networkable( ) )
			return;

		if( entity->networkable( )->dormant( ) )
			return;

		if( !entity->is_player( ) )
			return;

		if( entity == g_local_player->self( ) )
		{ 
			auto& vis_impacts = hacks::g_visuals->m_bullet_impacts;

			if( !vis_impacts.empty( )
				&& vis_impacts.back( ).m_time == game::g_global_vars.get( )->m_cur_time )
				vis_impacts.back( ).m_final = false;

			vis_impacts.emplace_back( 
				game::g_global_vars.get( )->m_cur_time,
				g_ctx->aim_shoot_pos( ),
				pos
			 );

			if( cfg.m_bullet_tracers ) { 
				sdk::col_t local_plr = sdk::col_t( cfg.m_bullet_tracers_clr[ 0 ] * 255.f, cfg.m_bullet_tracers_clr[ 1 ] * 255.f,
					cfg.m_bullet_tracers_clr[ 2 ] * 255.f, cfg.m_bullet_tracers_clr[ 3 ] * 255.f );

				g_visuals.get( )->push_beam_info( { game::g_global_vars.get( )->m_real_time, 
					g_ctx.get( )->shoot_pos( ),
					pos, local_plr,
					entity->networkable( )->index( ), entity->tick_base( ) } );
			}
		}

		if( entity->friendly( g_local_player->self( ) ) )
			return;

		if( !entity->friendly( g_local_player->self( ) ) && cfg.m_enemy_bullet_tracers ) { 
				sdk::col_t enemy_plr = sdk::col_t( cfg.m_enemy_bullet_tracers_clr[ 0 ] * 255.f, cfg.m_enemy_bullet_tracers_clr[ 1 ] * 255.f,
					cfg.m_enemy_bullet_tracers_clr[ 2 ] * 255.f, cfg.m_enemy_bullet_tracers_clr[ 3 ] * 255.f );

				g_visuals.get( )->push_beam_info( { game::g_global_vars.get( )->m_real_time, 
					entity->wpn_shoot_pos( ), 
					pos, enemy_plr,
					entity->networkable( )->index( ), entity->tick_base( ) } );
		}
	}

	void c_shot_construct::on_hurt( game::game_event_t* evt ) { 
		int         attacker, victim, group, hp;
		float       damage;
		std::string name;

		if( !evt || !g_local_player->self( ) )
			return;

		attacker = game::g_engine->index_for_uid( evt->get_int( xor_str( "attacker" ) ) );
		victim = game::g_engine->index_for_uid( evt->get_int( xor_str( "userid" ) ) );

		// skip invalid player indexes.
		// should never happen? world entity could be attacker, or a nade that hits you.
		//if( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 )
		//	return;

		// we were not the attacker or we hurt ourselves.
		if( attacker != g_local_player->self( )->networkable( )->index( )
			|| victim == g_local_player->self( )->networkable( )->index( ) )
			return;

		// get hitgroup.
		// players that get naded( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
		group = evt->get_int( xor_str( "hitgroup" ) );

		if( group == static_cast < int > ( game::e_hitgroup::gear ) )
			return;

		// get the player that was hurt.
		game::cs_player_t* target = static_cast < game::cs_player_t* > ( game::g_entity_list->get_entity( victim ) );
		if( !target )
			return;

		// get player info.
		game::player_info_t info;

		auto find = game::g_engine->get_player_info( victim, &info );

		if( !find )
			return;

		// get player name;
		name = std::string( info.m_name ).substr( 0, 24 );

		// get damage reported by the server.
		damage = ( float ) evt->get_int( xor_str( "dmg_health" ) );

		// get remaining hp.
		hp = evt->get_int( xor_str( "health" ) );

		// print this shit.
		if( hacks::g_misc->cfg( ).m_notification_logs & 1 ) { 
			std::string out = tfm::format( xor_str( "hit %s in the %s for %i damage (%i remain)\n" ), name, m_groups [ group ], ( int ) damage, hp );

			g_logs->push_log( out, sdk::col_t( 255, 255, 255, 255 ) );
		}

		static auto get_hitbox_by_hitgroup = [ ]( int hitgroup ) -> game::e_hitbox
		{ 
			switch( hitgroup )
			{ 
			case 1:
				return game::e_hitbox::head;
			case 2:
				return game::e_hitbox::chest;
			case 3:
				return game::e_hitbox::stomach;
			case 4:
				return game::e_hitbox::left_hand;
			case 5:
				return game::e_hitbox::right_hand;
			case 6:
				return game::e_hitbox::right_calf;
			case 7:
				return game::e_hitbox::left_calf;
			default:
				return game::e_hitbox::pelvis;
			}
		};

		if( group == static_cast < int > ( game::e_hitgroup::generic ) )
			return;

		if( g_shots->m_elements.empty( ) )
			return;

		auto& shot = g_shots->m_elements.back( );

		if( !shot.m_target.m_lag_record.has_value( )
			|| !shot.m_target.m_lag_record.value( ) )
			return;

		c_visuals::hit_marker_data_t hit_marker_data{ };

		hit_marker_data.m_spawn_time = game::g_global_vars.get( )->m_cur_time;
		hit_marker_data.m_pos = target->get_bone_pos( static_cast < int > ( get_hitbox_by_hitgroup( group ) ), shot.m_target.m_lag_record.value( )->m_bones );
		hit_marker_data.m_damage = damage;

		hacks::g_visuals->m_hit_markers.emplace_back( hit_marker_data );
	}

	void c_shot_construct::on_fire( game::game_event_t* evt )
	{ 
		int attacker;

		// screw this.
		if( !evt || !g_local_player->self( ) )
			return;

		// get attacker, if its not us, screw it.
		attacker = game::g_engine->index_for_uid( evt->get_int( xor_str( "userid" ) ) );
		if( attacker != game::g_engine->get_local_player( ) )
			return;
	}

	__forceinline float dist_to( const sdk::vec3_t& from, const sdk::vec3_t& other ) { 
		sdk::vec3_t delta;

		delta.x( ) = from.x( ) - other.x( );
		delta.y( ) = from.y( ) - other.y( );
		delta.z( ) = from.z( ) - other.z( );

		return delta.length( 2u );
	}

	void push_log_in_console( std::string text ) { 
		if( !( hacks::g_misc->cfg( ).m_notification_logs & 4 ) )
			return;

		constexpr uint8_t red_clr [ 4 ] = { 201, 46, 46, 255 };
		text += xor_str( "\n" );

		game::g_cvar->con_print( false, *red_clr, xor_str("[secret_hack24] missed ") );
		game::g_cvar->con_print( false, *red_clr, text.c_str( ) );
	}

	void c_shot_construct::on_render_start( )
	{ 
		if( !game::g_engine->in_game( ) ) { 
			return g_shots->m_elements.clear( );
		}

		for( auto& shot : g_shots->m_elements ) { 
			if( shot.m_processed
				|| game::g_global_vars.get( )->m_real_time > shot.m_process_tick )
				continue;

			if( shot.m_target.m_entry
				&& shot.m_target.m_entry->m_player ) { 
			
				//g_logs->push_log( shot.m_str, sdk::col_t::palette_t::white( ) );

				if( !shot.m_target.m_entry->m_player->alive( ) ) { 
					push_log_in_console( xor_str( "due to player death ( latency )" ) );
				}
				else { 
					lag_backup_t lag_data{ };
					lag_data.setup( shot.m_target.m_entry->m_player );

					if( shot.m_server_info.m_hurt_tick ) { 
					}
					else { 
						shot.m_target.m_lag_record.value( )->adjust( shot.m_target.m_entry->m_player );

						game::trace_t trace{ };

						game::g_engine_trace->clip_ray_to_entity( 
							{ shot.m_src, shot.m_server_info.m_impact_pos },
							CS_MASK_SHOOT_PLAYER, shot.m_target.m_entry->m_player, &trace
						 );

						if( trace.m_entity != shot.m_target.m_entry->m_player ) { 
							if( ( ( shot.m_src - shot.m_target.m_pos ).length( ) - crypt_float( 32.f ) ) > ( shot.m_src - shot.m_server_info.m_impact_pos ).length( ) )
								push_log_in_console( xor_str( "due to occlusion" ) );
							else
								push_log_in_console( xor_str( "due to spread" ) );
						}
						else { 

							std::string solve_method{ };
							{ 
							switch( shot.m_target.m_lag_record.value( )->m_resolver_method ) { 
								case e_solve_methods::no_fake:
									solve_method = "no fake";
									++shot.m_target.m_entry->m_no_fake_misses;
									break;
								case e_solve_methods::lby_delta:
									solve_method = "lby delta";
									++shot.m_target.m_entry->m_lby_misses;
									break;
								case e_solve_methods::fake_walk:
									solve_method = "fake walk";
								break;
								case e_solve_methods::last_move_lby:
									solve_method = "last move";
									++shot.m_target.m_entry->m_last_move_misses;
									break;
								case e_solve_methods::body_flick:
									solve_method = "flick";
									++shot.m_target.m_entry->m_lby_misses;
									break;
								case e_solve_methods::backwards:
									solve_method = "backwards";
									++shot.m_target.m_entry->m_backwards_misses;
									break;
								case e_solve_methods::forwards:
									solve_method = "forwards";
									++shot.m_target.m_entry->m_forwards_misses;
									break;
								case e_solve_methods::freestand:
									solve_method = "anti-freestanding";
									++shot.m_target.m_entry->m_freestand_misses;
									break;
								case e_solve_methods::brute:
									solve_method = "brute";
									++shot.m_target.m_entry->m_stand_moved_misses;
									break;
								case e_solve_methods::brute_not_moved:
									solve_method = "brute(n)";
									++shot.m_target.m_entry->m_stand_not_moved_misses;
									break;
								case e_solve_methods::just_stopped:
									solve_method = "anim lby";
									++shot.m_target.m_entry->m_just_stopped_misses;
									break;
								case e_solve_methods::body_flick_res:
									solve_method = "body flick";
									++shot.m_target.m_entry->m_low_lby_misses;
								break;
								case e_solve_methods::air:
									solve_method = "in air";
									++shot.m_target.m_entry->m_air_misses;
									break;
								case e_solve_methods::move:
									solve_method = "move";
									++shot.m_target.m_entry->m_moving_misses;
									break;
								default:
									solve_method = "unk";
									break;
								}

								std::string out = tfm::format( xor_str( "due to resolver(%s)" ), solve_method );
								
								push_log_in_console( out );

							}
						}
					}
					lag_data.restore( shot.m_target.m_entry->m_player );
				}
			}

			shot.m_processed = true;
		}

		g_shots->m_elements.erase( std::remove_if( 
			g_shots->m_elements.begin( ), g_shots->m_elements.end( ),
			[ & ]( const shot_t& shot ) { 
				if( !shot.m_processed
					&& shot.m_process_tick )
					return false;

				if( std::abs( game::g_global_vars.get( )->m_real_time - shot.m_shot_time ) >= 2.5f )
					return true;

				if( shot.m_processed
					|| shot.m_process_tick
					|| shot.m_target_index == -1
					|| shot.m_cmd_number == -1
					|| std::abs( game::g_client_state.get( )->m_cmd_ack - shot.m_cmd_number ) <= 20 )
					return false;

				const auto delta = std::abs( game::g_global_vars.get( )->m_real_time - shot.m_sent_time );
				if( delta > 2.5f )
					return true;

				if( delta <= shot.m_latency )
					return false;

				return true;
			}
		 ),
			g_shots->m_elements.end( )
				 );
	}
}
