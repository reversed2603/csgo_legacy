#include "../../../csgo.hpp"
#include <windows.h>
#include <mmsystem.h>
#include "../hit_sounds.h"

#include <playsoundapi.h>
#pragma comment(lib, "Winmm.lib")
const char* translate_hitgroup ( const int index )
{
	switch ( index ) {
		case 0: return ( "generic" ); break;
		case 1: return ( "head" ); break;
		case 2: return ( "chest" ); break;
		case 3: return ( "stomach" ); break;
		case 4: return ( "left arm" ); break;
		case 5: return ( "right arm" ); break;
		case 6: return ( "left leg" ); break;
		case 7: return ( "right leg" ); break;
		case 8: return ( "neck" ); break;
	}

	return ( "generic" );
}
__forceinline constexpr std::uint32_t hash_1 ( const char* str )
{
	auto hash = 0x811c9dc5u;

	char v4 {};
	do {
		v4 = *str++;
		hash = 0x1000193u * ( hash ^ v4 );
	} while ( v4 );

	return hash;
}

namespace csgo::hacks {

	void c_shots::on_new_event ( valve::game_event_t* const event )
	{
		if ( !g_local_player 
			|| !valve::g_engine->in_game ( ) )
			return;

		m_elements.erase( std::remove_if( m_elements.begin( ), m_elements.end( ),
			[ & ]( const shot_t& shot ) {
				if ( std::abs( valve::g_global_vars.get ( )->m_real_time - shot.m_shot_time ) >= 2.5f )
					return true;

				if ( shot.m_processed
					|| shot.m_cmd_number == -1
					|| shot.m_process_tick
					|| std::abs( valve::g_client_state.get ( )->m_cmd_ack - shot.m_cmd_number ) <= 20 )
					return false;

				const auto delta = std::abs( valve::g_global_vars.get( )->m_real_time - shot.m_sent_time );
				return delta > 2.5f || delta > shot.m_latency;
			}
		), m_elements.end( ) );


		switch ( hash_1( event->name ( ) ) ) {
			case 0xf8dba51u/* player_footstep */: {


			} break;
			case 0xbded60d0u/* player_hurt */: {
				if ( valve::g_engine->index_for_uid ( event->get_int ( xor_str ( "attacker" ) ) ) != g_local_player->self ( )->networkable ( )->index ( ) )
					return;
				g_shot_construct->on_hurt ( event );
				const auto victim = valve::g_entity_list->get_entity ( valve::g_engine->index_for_uid ( event->get_int ( xor_str ( "userid" ) ) ) );
				if ( !victim )
					return;

				const auto hitgroup = event->get_int ( xor_str ( "hitgroup" ) );
				if ( hitgroup == 10 )
					return;

				const auto dmg = event->get_int ( xor_str ( "dmg_health" ) );

				if ( g_misc->cfg( ).m_hit_marker_sound ) {
					switch ( g_misc->cfg( ).m_hit_marker_sound_val ) {
					case 0:
						PlaySoundA( reinterpret_cast< char* > ( neverlose_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					case 1:
						PlaySoundA( reinterpret_cast< char* > ( phonk_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					case 2:
						PlaySoundA( reinterpret_cast< char* > ( skeet_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					case 3:
						PlaySoundA( reinterpret_cast< char* > ( primordial_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					case 4:
						PlaySoundA( reinterpret_cast< char* > ( cock_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					case 5:
						PlaySoundA( reinterpret_cast< char* > ( bepis_sound ), NULL, SND_ASYNC | SND_MEMORY );
						break;
					}
				}

				auto damn = valve::g_engine->index_for_uid ( event->get_int ( xor_str ( "userid" ) ) );

				const auto shot = last_unprocessed( );
				if ( !shot
					|| ( shot->m_target.m_entry && shot->m_target_index != victim->networkable ( )->index( ) ) )
					return;

				shot->m_server_info.m_hitgroup = hitgroup;
				shot->m_server_info.m_dmg = event->get_int( xor_str( "dmg_health" ) );
				shot->m_server_info.m_hurt_tick = valve::g_client_state.get ( )->m_server_tick;

			} break;
			case 0xe64effdau/* weapon_fire */: {
				g_shot_construct->on_fire ( event );

				if ( !event || !g_local_player->self( ) )
					return;

				// get attacker, if its not us, screw it.
				auto attacker = valve::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );
				if ( attacker != valve::g_engine->get_local_player( ) )
					return;

				if ( m_elements.empty( ) )
					return;

				const auto shot = std::find_if(
					m_elements.begin( ), m_elements.end( ),
					[ ]( const shot_t& shot ) {
						return shot.m_cmd_number != -1 && !shot.m_server_info.m_fire_tick
							&& std::abs( valve::g_client_state.get ( )->m_cmd_ack - shot.m_cmd_number ) <= 20;
					}
				);

				if ( shot == m_elements.end( ) )
					return;

				shot->m_process_tick = valve::g_global_vars.get ( )->m_real_time + 2.5f;
				shot->m_server_info.m_fire_tick = valve::g_client_state.get( )->m_server_tick;
			} break;
			case 0x9b5f9138u/* bullet_impact */: {
				g_shot_construct->on_impact ( event );
				if ( !event || !g_local_player->self( ) )
					return;

				// get attacker, if its not us, screw it.
				auto attacker = valve::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );
				if ( attacker != valve::g_engine->get_local_player( ) )
					return;

				// decode impact coordinates and convert to vec3.
				sdk::vec3_t pos = {
					event->get_float( xor_str( "x" ) ),
					event->get_float( xor_str( "y" ) ),
					event->get_float( xor_str( "z" ) )
				};

				if ( const auto shot = last_unprocessed( ) )
					shot->m_server_info.m_impact_pos = pos;
			
			} break;	
			case 0xd9dda907u: { // item_equip

				const auto idx = valve::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) );

				hacks::g_visuals->m_dormant_data[ idx ].m_weapon_id = event->get_int( xor_str( "defindex" ) );
				hacks::g_visuals->m_dormant_data[ idx ].m_weapon_type = event->get_int( xor_str( "weptype" ) );
			}break;

			case 0x2df1832d: {
				const auto entity = valve::g_entity_list->get_entity( valve::g_engine->index_for_uid( event->get_int( xor_str( "userid" ) ) ) );
				if ( !entity
					|| entity == g_local_player->self ( )
					|| ( ( static_cast < valve::cs_player_t* > ( entity ) )->friendly( g_local_player->self ( ) ) ) )
					return;
				hacks::g_visuals->m_dormant_data.at( entity->networkable( )->index( ) ).m_receive_time = valve::g_global_vars.get ( )->m_real_time;
				hacks::g_visuals->m_dormant_data.at( entity->networkable( )->index( ) ).m_origin = entity->origin( );

			}break;

			case 0x19180a27u/* round_freeze_end */: /*g_context->freeze_time ( ) = false;*/ break;
			case 0x2301969du/* round_prestart */:
				for ( std::size_t i {}; i < valve::g_global_vars.get ( )->m_max_clients; ++i ) {
					hacks::g_visuals->m_dormant_data [ i ].m_origin = {};
					hacks::g_visuals->m_dormant_data [ i ].m_receive_time = 0.f;
					hacks::g_visuals->m_dormant_data [ i ].m_alpha = 0.f;
					hacks::g_visuals->m_dormant_data [ i ].m_alpha = std::clamp ( hacks::g_visuals->m_dormant_data [ i ].m_alpha, 0.f, 255.f );
					hacks::g_visuals->m_dormant_data[ i ].m_use_shared = false;
					hacks::g_visuals->m_dormant_data[ i ].m_weapon_id = 0;
					hacks::g_visuals->m_dormant_data[ i ].m_weapon_type = -1;
					hacks::g_visuals->m_dormant_data.at( i ).m_last_shared_time = 0.f;
				}
				g_ctx->buy_bot( ) = 2;
				hacks::g_eng_pred->reset_on_spawn ( );
				g_ctx->anim_data( ).m_local_data.reset( );

				g_shots->m_elements.clear( );

				valve::kill_feed_t* feed = ( valve::kill_feed_t* ) valve::g_hud->find_element( HASH( "SFHudDeathNoticeAndBotStatus" ) );

				if ( feed ) {
					g_ctx->addresses( ).m_clear_notices( feed );
				}

				hacks::g_local_sync->m_anim_layers = {};
				hacks::g_local_sync->m_old_layers = {};

				hacks::g_local_sync->m_pose_params = {};
				hacks::g_local_sync->m_old_params = {};

				hacks::g_visuals->m_bullet_tracers.clear( );
				hacks::g_visuals->m_hit_markers.clear( );

				for ( std::size_t i {}; i < 64u; ++i ) {
					auto& entry = g_lag_comp->entry ( i );

					entry.m_air_misses = 0;
					entry.m_lby_misses = 0;
					entry.m_moving_misses = 0;
					entry.m_moved = false;
					entry.m_stand_moved_misses = 0;
					entry.m_stand_not_moved_misses = 0;
				}

				break;
		}
	}

	float get_absolute_time ( )
	{
		return ( float ) ( clock ( ) / ( float ) 1000.f );
	}

	void c_logs::draw_data ( )
	{
		for ( int i = 0u; i < m_logs.size ( ); i++ )
		{
			log_data_t* log_data = &m_logs [ i ];
			if ( !log_data )
				continue;

			float_t time_delta = get_absolute_time ( ) - log_data->m_creation_time;
			if ( time_delta >= 5.0f )
			{
				m_logs.erase ( m_logs.begin ( ) + i );
				continue;
			}

			if ( time_delta >= 4.75f )
			{
				log_data->m_text_alpha = std::clamp ( ( ( 5.0f - time_delta ) / 0.25f ) * 255.0f, 0.0f, 255.0f );
				log_data->m_back_alpha = std::clamp ( ( ( 5.0f - time_delta ) / 0.25f ) * 50.0f, 0.0f, 50.0f );
				log_data->m_spacing = std::clamp ( ( ( 5.0f - time_delta ) / 0.25f ) * 10.0f, 0.0f, 10.0f );
			}
			else if ( time_delta <= 0.25f )
			{
				log_data->m_text_alpha = std::clamp ( ( 1.0f - ( 0.25f - time_delta ) / 0.25f ) * 255.0f, 0.0f, 255.0f );
				log_data->m_back_alpha = std::clamp ( ( 1.0f - ( 0.25f - time_delta ) / 0.25f ) * 50.0f, 0.0f, 50.0f );
				log_data->m_spacing = std::clamp ( ( 1.0f - ( 0.25f - time_delta ) / 0.25f ) * 10.0f, 0.0f, 10.0f );
			}

			float spacing = 0.0f;
			if ( i )
			{
				for ( int i2 = 0; i2 < i; i2++ )
					spacing += m_logs [ i2 ].m_spacing;
			}

			constexpr uint8_t white_clr [ 4 ] = { 255, 255, 255, 255 };
			constexpr uint8_t black_clr [ 4 ] = { 0, 0, 0, 255 };

			if ( !log_data->m_printed )
			{
				if ( i % 2 == 0 ) {
					valve::g_cvar->con_print ( false, *white_clr, "[csgo_project] " );
					valve::g_cvar->con_print ( false, *white_clr, log_data->m_string.c_str( ) );
					valve::g_cvar->con_print ( false, *white_clr, xor_str ( "\n" ) );
				}
				else {
					valve::g_cvar->con_print ( false, *black_clr, log_data->m_string.c_str ( ) );
					valve::g_cvar->con_print ( false, *black_clr, xor_str ( "\n" ) );
				}

				log_data->m_printed = true;
			}

			float_t text_length = 28 + hacks::g_misc->m_fonts.m_segoe_ui_esp->CalcTextSizeA ( 16.0f, FLT_MAX, NULL, log_data->m_string.c_str ( ) ).x;

			g_render->text ( log_data->m_string, sdk::vec2_t ( 4, spacing ), sdk::col_t ( log_data->m_color.r ( ), log_data->m_color.g ( ), log_data->m_color.b ( ), ( int ) ( log_data->m_text_alpha ) ), hacks::g_misc->m_fonts.m_segoe_ui_esp, false, false, false, true, true );
		}
	}

	void c_logs::push_log ( std::string string, std::string icon, sdk::col_t color )
	{
		log_data_t data;

		data.m_creation_time = get_absolute_time ( );
		data.m_spacing = 0.f;
		data.m_text_alpha = 0.f;
		data.m_icon = icon;
		data.m_string = string;
		data.m_color = color;

		m_logs.emplace_back ( data );

		while ( m_logs.size ( ) > 10.f )
			m_logs.pop_back ( );
	}

	void c_shot_construct::on_rage_bot ( aim_target_t* target, float damage, int bullets, std::shared_ptr < lag_record_t > record, int hitbox, const sdk::vec3_t& shoot_pos, int cmd_number ) {
		// setup new shot data.
		c_shot_record shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_shot_time = valve::g_global_vars.get ( )->m_real_time;
		shot.m_damage = damage;
		shot.m_pos = shoot_pos;
		shot.m_hitbox = hitbox;
		shot.m_impacted = false;
		shot.m_confirmed = false;
		shot.m_hurt = false;
		shot.m_cmd_num = cmd_number;
		shot.m_lat = g_ctx->net_info( ).m_latency.m_out;

		if ( g_local_player->self ( )->weapon ( )
			&& g_local_player->self ( )->weapon ( )->info ( ) ) {
			shot.m_weapon_range = g_local_player->self ( )->weapon ( )->info ( )->m_range;
		}

		if ( target 
			&& record ) {
			auto matrix = record->m_bones;

			shot.m_matrix = matrix;
		}

		// add to tracks.
		m_shots.push_front ( shot );

		while ( m_shots.size ( ) > 128 )
			m_shots.pop_back ( );
	}

	void c_shot_construct::on_impact ( valve::game_event_t* evt )
	{
		int        attacker;
		sdk::vec3_t     pos, dir, start, end;
		float      time;
		valve::trace_t trace;

		// screw this.
		if ( !evt || !g_local_player->self ( ) )
			return;

		// get attacker, if its not us, screw it.
		attacker = valve::g_engine->index_for_uid ( evt->get_int ( xor_str ( "userid" ) ) );
		if ( attacker != valve::g_engine->get_local_player( ) ) {
			if ( ( static_cast < valve::cs_player_t* > ( valve::g_entity_list->get_entity( attacker ) ) )->team( ) == g_local_player->self( )->team( ) )
				return;

			pos = {
			evt->get_float( xor_str( "x" ) ),
			evt->get_float( xor_str( "y" ) ),
			evt->get_float( xor_str( "z" ) )
			};

			c_visuals::bullet_trace_data_t enemy_trace_data{};

			enemy_trace_data.m_start_pos = static_cast < valve::cs_player_t* > ( valve::g_entity_list->get_entity( attacker ) )->abs_origin ( ) + static_cast < valve::cs_player_t* > ( valve::g_entity_list->get_entity( attacker ) )->view_offset( );
			enemy_trace_data.m_spawn_time = valve::g_global_vars.get( )->m_cur_time;
			enemy_trace_data.m_end_pos = pos;

			g_visuals->m_enemy_bullet_tracers.emplace_front( enemy_trace_data );

			return;
		}

		// decode impact coordinates and convert to vec3.
		pos = {
			evt->get_float ( xor_str ( "x" ) ),
			evt->get_float ( xor_str ( "y" ) ),
			evt->get_float ( xor_str ( "z" ) )
		};

		// add to visual impacts if we have features that rely on it enabled.
		// todo - dex; need to match shots for this to have proper GetShootPosition, don't really care to do it anymore.
		//if( g_menu.main.visuals.impact_beams.get ( ) )
		//	m_vis_impacts.push_back ( { pos, g_cl.m_local->GetShootPosition ( ), g_cl.m_local->m_nTickBase ( ) } );

		auto& vis_impacts = hacks::g_visuals->m_bullet_impacts;

		c_visuals::bullet_trace_data_t trace_data{};
		trace_data.m_start_pos = g_ctx->aim_shoot_pos( );
		trace_data.m_spawn_time = valve::g_global_vars.get ( )->m_cur_time;
		trace_data.m_end_pos = pos;

		g_visuals->m_bullet_tracers.emplace_front( trace_data );

		if ( !vis_impacts.empty( )
			&& vis_impacts.back( ).m_time == valve::g_global_vars.get ( )->m_cur_time )
			vis_impacts.back( ).m_final = false;

		vis_impacts.emplace_back(
			valve::g_global_vars.get ( )->m_cur_time,
			g_ctx->aim_shoot_pos( ),
			pos
		);
	}

	void c_shot_construct::on_hurt ( valve::game_event_t* evt ) {


		int         attacker, victim, group, hp;
		float       damage;
		std::string name;

		if ( !evt || !g_local_player->self ( ) )
			return;

		attacker = valve::g_engine->index_for_uid ( evt->get_int ( xor_str ( "attacker" ) ) );
		victim = valve::g_engine->index_for_uid ( evt->get_int ( xor_str ( "userid" ) ) );

		// skip invalid player indexes.
		// should never happen? world entity could be attacker, or a nade that hits you.
		//if ( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 )
		//	return;

		// we were not the attacker or we hurt ourselves.
		if ( attacker != g_local_player->self ( )->networkable ( )->index ( )
			|| victim == g_local_player->self ( )->networkable ( )->index ( ) )
			return;

		// get hitgroup.
		// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
		group = evt->get_int ( xor_str ( "hitgroup" ) );

		if ( group == static_cast < int > ( valve::e_hitgroup::gear ) )
			return;

		// get the player that was hurt.
		valve::cs_player_t* target = static_cast < valve::cs_player_t* > ( valve::g_entity_list->get_entity ( victim ) );
		if ( !target )
			return;

		// get player info.
		valve::player_info_t info;

		auto find = valve::g_engine->get_player_info ( victim, &info );

		if ( !find )
			return;
		// get player name;
		name = std::string ( info.m_name ).substr ( 0, 24 );

		// get damage reported by the server.
		damage = ( float ) evt->get_int ( xor_str ( "dmg_health" ) );

		// get remaining hp.
		hp = evt->get_int ( xor_str ( "health" ) );

		// print this shit.
		//if ( c_config::get ( )->b [ "log_damage" ] ) {
			std::string out = tfm::format ( xor_str ( "hurt %s in the %s for %i damage (%i hp remaining)" ), name, m_groups [ group ], ( int ) damage, hp );

			g_logs->push_log ( out, xor_str ( "d" ), sdk::col_t ( 255, 255, 255, 255 ) );
		//}

			static auto get_hitbox_by_hitgroup = [ ]( int hitgroup ) -> valve::e_hitbox
			{
				switch ( hitgroup )
				{
				case 1:
					return valve::e_hitbox::head;
				case 2:
					return valve::e_hitbox::chest;
				case 3:
					return valve::e_hitbox::stomach;
				case 4:
					return valve::e_hitbox::left_hand;
				case 5:
					return valve::e_hitbox::right_hand;
				case 6:
					return valve::e_hitbox::right_calf;
				case 7:
					return valve::e_hitbox::left_calf;
				default:
					return valve::e_hitbox::pelvis;
				}
			};

		if ( group == static_cast < int > ( valve::e_hitgroup::generic ) )
			return;

		auto& entry = hacks::g_lag_comp->entry( target->networkable ( )->index( ) - crypt_int( 1 ) );

		if ( g_shots->m_elements.empty( ) )
			return;

		auto& shot = g_shots->m_elements.back( );

		if ( !shot.m_target.m_lag_record.has_value( )
			|| !shot.m_target.m_lag_record.value( ) )
			return;

		c_visuals::hit_marker_data_t hit_marker_data{};

		hit_marker_data.m_spawn_time = valve::g_global_vars.get( )->m_cur_time;
		hit_marker_data.m_pos = target->who_tf_asked( static_cast < int > ( get_hitbox_by_hitgroup( group ) ), shot.m_target.m_lag_record.value ( )->m_bones );

		hacks::g_visuals->m_hit_markers.emplace_back( hit_marker_data );
	}

	void c_shot_construct::on_fire ( valve::game_event_t* evt )
	{
		int attacker;

		// screw this.
		if ( !evt || !g_local_player->self ( ) )
			return;

		// get attacker, if its not us, screw it.
		attacker = valve::g_engine->index_for_uid ( evt->get_int ( xor_str ( "userid" ) ) );
		if ( attacker != valve::g_engine->get_local_player ( ) )
			return;
	}
	ALWAYS_INLINE float dist_to ( const sdk::vec3_t& from, const sdk::vec3_t& other ) {
		sdk::vec3_t delta;

		delta.x ( ) = from.x ( ) - other.x ( );
		delta.y ( ) = from.y ( ) - other.y ( );
		delta.z ( ) = from.z ( ) - other.z ( );

		return delta.length ( 2u );
	}

	void c_shot_construct::on_render_start( )
	{
		if ( !valve::g_engine->in_game( ) ) {
			return g_shots->m_elements.clear( );
		}

		for ( auto& shot : g_shots->m_elements ) {
			if ( shot.m_processed
				|| valve::g_global_vars.get( )->m_real_time > shot.m_process_tick )
				continue;

			if ( shot.m_target.m_entry
				&& shot.m_target.m_entry->m_player ) {
			
				g_logs->push_log( shot.m_str, "d", sdk::col_t::palette_t::white( ) );

				if ( !shot.m_target.m_entry->m_player->alive( ) ) {
				}
				else {
					lag_backup_t lag_data{};
					lag_data.setup( shot.m_target.m_entry->m_player );

					if ( shot.m_server_info.m_hurt_tick ) {
					}
					else {
						shot.m_target.m_lag_record.value( )->adjust( shot.m_target.m_entry->m_player );

						valve::trace_t trace{};

						valve::g_engine_trace->clip_ray_to_entity(
							{ shot.m_src, shot.m_server_info.m_impact_pos },
							valve::e_mask::shot_player, shot.m_target.m_entry->m_player, &trace
						);

						if ( trace.m_entity != shot.m_target.m_entry->m_player ) {

							if ( ( ( shot.m_src - shot.m_target.m_pos ).length( ) - crypt_float ( 32.f ) ) > ( shot.m_src - shot.m_server_info.m_impact_pos ).length( ) )
								g_logs->push_log( xor_str( "missed shot due to occlusion" ), xor_str( "d" ), sdk::col_t( 224, 145, 27, 255 ) );
							else
								g_logs->push_log( xor_str( "missed shot due to spread" ), xor_str( "d" ), sdk::col_t( 224, 145, 27, 255 ) );
						}
						else {

							std::string solve_method{};

							switch ( shot.m_target.m_lag_record.value( )->m_resolver_method ) {
							case e_solve_methods::no_fake:
								solve_method = "no fake";
								break;
							case e_solve_methods::lby_delta:
								solve_method = "lby delta";
								break;
							case e_solve_methods::last_move:
								solve_method = "last move";
								break;
							case e_solve_methods::body_flick:
								solve_method = "flick";
								break;
							case e_solve_methods::brute:
								solve_method = "brute";
								break;
							case e_solve_methods::just_stopped:
								solve_method = "just stopped";
								break;
							case e_solve_methods::anti_fs:
								solve_method = "anti-fs";
								break;
							case e_solve_methods::brute_not_moved:
								solve_method = "brute [ no move data ]";
								break;
							case e_solve_methods::anti_fs_not_moved:
								solve_method = "anti-fs [ no move data ]";
								break;
							case e_solve_methods::air:
								solve_method = "in air";
								break;
							case e_solve_methods::move:
								solve_method = "move";
								break;
							default:
								solve_method = "unk";
								break;
							}

							{
								std::string out = tfm::format( xor_str( "missed shot due to fake angle [ r_mode: %s ]" ), solve_method );

								g_logs->push_log( out, xor_str( "d" ), sdk::col_t( 207, 91, 28, 255 ) );

							}

							if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::no_fake ) {
								++shot.m_target.m_entry->m_no_fake_misses;
							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::lby_delta
								|| shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::brute
								|| shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::anti_fs
								|| shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::last_move ) {
								++shot.m_target.m_entry->m_stand_moved_misses;

							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::brute_not_moved
								|| shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::anti_fs_not_moved ) {
								++shot.m_target.m_entry->m_stand_not_moved_misses;
							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::body_flick ) {
								++shot.m_target.m_entry->m_lby_misses;
							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::move ) {
								++shot.m_target.m_entry->m_moving_misses;
							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::air ) {
								++shot.m_target.m_entry->m_air_misses;
							}
							else if ( shot.m_target.m_lag_record.value( )->m_resolver_method == e_solve_methods::just_stopped ) {
								++shot.m_target.m_entry->m_just_stopped_misses;
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
				if ( !shot.m_processed
					&& shot.m_process_tick )
					return false;

				if ( std::abs( valve::g_global_vars.get ( )->m_real_time - shot.m_shot_time ) >= 2.5f )
					return true;

				if ( shot.m_processed
					|| shot.m_process_tick
					|| shot.m_target_index == -1
					|| shot.m_cmd_number == -1
					|| std::abs( valve::g_client_state.get ( )->m_cmd_ack - shot.m_cmd_number ) <= 20 )
					return false;

				const auto delta = std::abs( valve::g_global_vars.get( )->m_real_time - shot.m_sent_time );
				if ( delta > 2.5f )
					return true;

				if ( delta <= shot.m_latency )
					return false;

				return true;
			}
		),
			g_shots->m_elements.end( )
				);

		/*if ( !valve::g_engine->in_game( )
			|| m_shots.empty ( ) ) {
			if ( !m_shots.empty ( ) )
				m_shots.clear ( );

			return;
		}

		for ( auto it = m_shots.begin ( ); it != m_shots.end ( );) {
			if ( it->m_shot_time + 2.5f < valve::g_global_vars.get ( )->m_real_time )
				it = m_shots.erase ( it );
			else
				it = next ( it );
		}

		for ( auto it = m_shots.begin ( ); it != m_shots.end ( );) {
			if ( it->m_impacted && !it->m_hurt && it->m_confirmed ) {
				auto& target = it->m_target;
				if ( !target ) {
					it = m_shots.erase ( it );
					continue;
				}

				if ( !target->m_entry ) {
					it = m_shots.erase( it );
					continue;
				}
				if ( !target->m_entry->m_player->alive ( ) ) {
					g_logs->push_log( xor_str( "unregistered shot ( enemy is dead )" ), xor_str( "d" ), sdk::col_t( 224, 145, 27, 255 ) );
					it = m_shots.erase ( it );
					continue;
				}

				auto& entry = target->m_entry;

				valve::trace_t trace{};

				lag_backup_t lag_data {};
				lag_data.setup( target->m_entry->m_player );
				target->m_lag_record.value( )->adjust( target->m_entry->m_player );

				bool intersected{};	

				valve::g_engine_trace->clip_ray_to_entity(
					{ it->m_pos, it->m_server_pos },
					valve::e_mask::shot_player, target->m_entry->m_player, &trace
				);

				if ( trace.m_entity == target->m_entry->m_player )
					intersected = true;

				lag_data.restore( target->m_entry->m_player );

				if ( !intersected ) {
					g_logs->push_log( xor_str( "missed shot due to spread" ), xor_str( "d" ), sdk::col_t( 224, 145, 27, 255 ) );

					it = m_shots.erase( it );

					continue;
				}

				if ( entry->m_solve_method == e_solve_methods::solve_stand_moved ) {
					++entry->m_stand_moved_misses;
				}
				else if ( entry->m_solve_method == e_solve_methods::solve_stand_not_moved ) {
					++entry->m_stand_not_moved_misses;
				}
				else if ( entry->m_solve_method == e_solve_methods::move ) {
					++entry->m_moving_misses;
				}
				else if ( entry->m_solve_method == e_solve_methods::body_flick ) {
					++entry->m_lby_misses;
				}
				else if ( entry->m_solve_method == e_solve_methods::air ) {
					++entry->m_air_misses;
				}
				else if ( entry->m_solve_method == e_solve_methods::solve_stand_no_lby ) {
					++entry->m_no_lby_misses;
				}

				std::string solve_method{};

				switch ( target->m_entry->m_solve_method ) {
				case e_solve_methods::solve_stand_moved:
					solve_method = "stand ( moved )";
					break;

				case e_solve_methods::solve_stand_not_moved:
					solve_method = "stand ( not moved )";
					break;

				case e_solve_methods::move:
					solve_method = "move";
					break;

				case e_solve_methods::body_flick:
					solve_method = "flick";
					break;

				case e_solve_methods::air:
					solve_method = "air";
					break;
				case e_solve_methods::solve_stand_no_lby:
					solve_method = "lby is not broken";
					break;
				default:
					solve_method = "unk";
					break;
				}

				std::string out = tfm::format( xor_str( "missed shot due to fake angle ( r_mode: %s )" ), solve_method );

				g_logs->push_log ( out, xor_str ( "d" ), sdk::col_t ( 207, 91, 28, 255 ) );
				it = m_shots.erase ( it );
			}
			else {
				it = next ( it );
			}
		}*/
	}
}
