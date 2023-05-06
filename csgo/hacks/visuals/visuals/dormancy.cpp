#include "../../../csgo.hpp"

namespace csgo::hacks {
	void c_dormant_esp::start( )
	{ 
		m_utlCurSoundList.remove_all( );
		game::g_engine_sound->get_act_sounds( m_utlCurSoundList );

		if( !m_utlCurSoundList.m_size )
			return;

		for( auto i = 0; i < m_utlCurSoundList.m_size; i++ )
		{ 
			auto& sound = m_utlCurSoundList.at( i );

			if( sound.m_nSoundSource < 1 || sound.m_nSoundSource > 64 )
				continue;

			if( sound.m_pOrigin->x( ) == 0.f && sound.m_pOrigin->y( ) == 0.f && sound.m_pOrigin->z( ) == 0.f )
				continue;

			if( !valid_sound( sound ) )
				continue;

			auto player = static_cast< game::cs_player_t* > ( game::g_entity_list->get_entity( sound.m_nSoundSource ) );

			if( !player || !player->alive( ) || player->friendly( g_local_player->self( ) ) ||
				player == g_local_player->self( ) )
				continue;

			setup_adjust( player, sound );
			m_sound_players[ sound.m_nSoundSource ].Override( sound );
		}

		m_utlvecSoundBuffer = m_utlCurSoundList;
	}

	void c_dormant_esp::setup_adjust( game::cs_player_t* player, game::snd_info_t& sound )
	{ 
		sdk::vec3_t src3D, dst3D;
		game::trace_t tr;
		game::trace_filter_simple_t filter;

		src3D = *sound.m_pOrigin + sdk::vec3_t( 0.0f, 0.0f, 1.0f );
		dst3D = src3D - sdk::vec3_t( 0.0f, 0.0f, 100.0f );

		filter.m_ignore_entity = player;
		game::ray_t ray = { src3D, dst3D };

		game::g_engine_trace->trace_ray( ray, MASK_PLAYERSOLID, reinterpret_cast < game::base_trace_filter_t* > ( &filter ), &tr );

		if( tr.m_all_solid )
			m_sound_players[ sound.m_nSoundSource ].m_iReceiveTime = -1;

		*sound.m_pOrigin = tr.m_frac <= 0.97f ? tr.m_end : *sound.m_pOrigin;
		m_sound_players[ sound.m_nSoundSource ].m_nFlags = static_cast < int > ( player->flags( ) );
		m_sound_players[ sound.m_nSoundSource ].m_nFlags |= ( tr.m_frac < 0.50f ? game::e_ent_flags::ducking : static_cast < game::e_ent_flags > ( 0 ) ) |( tr.m_frac < 1.0f ? game::e_ent_flags::on_ground : static_cast < game::e_ent_flags > ( 0 ) );
		m_sound_players[ sound.m_nSoundSource ].m_nFlags &= ( tr.m_frac >= 0.50f ? ~game::e_ent_flags::ducking : 0 ) |( tr.m_frac >= 1.0f ? ~game::e_ent_flags::on_ground : 0 );
	}

	bool c_dormant_esp::adjust_sound( game::cs_player_t* entity )
	{ 
		auto i = entity->networkable( )->index( );
		auto sound_player = m_sound_players[ i ];

		//entity->spotted( ) = true;
		entity->flags( ) = ( game::e_ent_flags ) sound_player.m_nFlags;
		entity->set_abs_origin( sound_player.m_vecOrigin );

		return ( fabs( game::g_global_vars.get( )->m_real_time - sound_player.m_iReceiveTime ) < 2.5f );
	}

	bool c_dormant_esp::valid_sound( game::snd_info_t& sound )
	{ 
		for( auto i = 0; i < m_utlvecSoundBuffer.m_size; i++ )
			if( m_utlvecSoundBuffer.at( i ).m_nGuid == sound.m_nGuid )
				return false;

		return true;
	}

	void c_visuals::shared_t::send_net_data( game::cs_player_t* const player ) { 
		game::player_info_t info{ };

		const bool exists = game::g_engine->get_player_info( player->networkable( )->index( ), &info );

		if( !exists )
			return;

		if( info.m_fake_player )
			return;

		game::cclc_msg_data_legacy_t client_msg{ };

		memset( &client_msg, 0, sizeof( client_msg ) );

		const auto func = ( std::uint32_t( __fastcall* )( void*, void* ) )g_ctx->addresses( ).m_voice_msg_ctor;

		func( ( void* ) &client_msg, nullptr );

		cheat_data_t* ptr = ( cheat_data_t* ) &client_msg.m_xuid_low;

		ptr->m_unique_key = 228;
		ptr->m_player_idx = static_cast < std::uint8_t > ( player->networkable( )->index( ) );
		ptr->m_x = static_cast < std::int16_t > ( player->origin( ).x( ) );
		ptr->m_y = static_cast < std::int16_t > ( player->origin( ).y( ) );
		ptr->m_z = static_cast < std::int16_t > ( player->origin( ).z( ) );

		client_msg.m_flags = 63;
		client_msg.m_format = 0;

		game::g_client_state.get( )->m_net_chan->send_net_msg( &client_msg );
	}
}