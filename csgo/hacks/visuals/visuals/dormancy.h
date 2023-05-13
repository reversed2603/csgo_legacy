#pragma once

namespace csgo::hacks {
	class c_dormancy { 
	public:
		struct shared_t { 
			__forceinline shared_t( ) = default;

			void send_net_data( game::cs_player_t* const player );
		} m_shared{ };

		struct dormant_data_t { 
			float m_alpha{ };
			bool m_was_unseen{ };
			sdk::vec3_t m_origin{ };
			float m_receive_time{ };
			int m_weapon_id{ };
			int m_weapon_type{ -1 };
			bool m_use_shared{ };
			float m_last_shared_time{ };
		};

		std::array< dormant_data_t, 65 > m_data { };
		
		class c_dormant_esp { 
		public:
			void start( );

			bool adjust_sound( game::cs_player_t* player );
			void setup_adjust( game::cs_player_t* player, game::snd_info_t& sound );
			bool valid_sound( game::snd_info_t& sound );

			struct snd_player { 
				void reset( bool store_data = false, const sdk::vec3_t& origin = { }, int flags = 0 )
				{ 
					if( store_data )
					{ 
						m_receive_time = game::g_global_vars.get( )->m_cur_time;
						m_origin = origin;
						m_flags = flags;
					}
					else
					{ 
						m_receive_time = 0.0f;
						m_origin = { };
						m_flags = 0;
					}
				}

				void override( game::snd_info_t& sound )
				{ 
					m_receive_time = game::g_global_vars.get( )->m_cur_time;
					m_origin = *sound.m_p_origin;
				}

				float m_receive_time = 0.0f;
				sdk::vec3_t m_origin = { };
				int m_flags = { };
			} m_sound_players[ 65 ];

			game::utl_vec_t< game::snd_info_t > m_sound_buffer;
			game::utl_vec_t< game::snd_info_t > m_sound_list;
		};

		std::unique_ptr < c_dormant_esp > g_dormant_esp = std::make_unique < c_dormant_esp > ( );
	};
	inline const std::unique_ptr < c_dormancy > g_dormancy = std::make_unique < c_dormancy > ( );
}