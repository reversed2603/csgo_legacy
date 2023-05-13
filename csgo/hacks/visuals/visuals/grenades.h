#pragma once

namespace csgo::hacks {
	class c_grenades {
	public:
		struct grenade_simulation_t { 
			__forceinline grenade_simulation_t( ) = default;

			__forceinline grenade_simulation_t( 
				game::cs_player_t* const owner, const game::e_item_index index,
				const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset
			 ) : m_owner{ owner }, m_index{ index } { predict( origin, velocity, throw_time, offset );	}

			void predict( const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset );

			bool physics_simulate( );

			void physics_trace_entity( 
				const sdk::vec3_t& src, const sdk::vec3_t& dst,
				const std::uint32_t mask, game::trace_t& trace
			 );

			void physics_push_entity( const sdk::vec3_t& push, game::trace_t& trace );

			void physics_clip_velocity( const sdk::vec3_t& in, const sdk::vec3_t& normal, sdk::vec3_t& out, float overbounce );

			void perform_fly_collision_resolution( game::trace_t& trace );

			void think( );

			void detonate( const bool bounced );

			void update_path( const bool bounced );

			__forceinline void push_broken_ent( game::base_entity_t* ent )
			{ 
				m_broken_ents.emplace_back( ent );
			}

			__forceinline void clear_broken_ents( )
			{ 
				m_broken_ents.clear( );
			}

			__forceinline bool is_ent_broken( game::base_entity_t* ent )
			{ 
				return find( m_broken_ents.begin( ), m_broken_ents.end( ), ent ) != m_broken_ents.end( );
			}

			bool										m_detonated{ };
			game::cs_player_t* m_owner{ };
			sdk::vec3_t										m_origin{ }, m_velocity{ };
			game::base_entity_t* m_last_hit_entity{ }, * m_last_breakable{ };
			float										m_detonate_time{ }, m_expire_time{ }, m_source_time{ };
			game::e_item_index							m_index{ };
			int											m_tick{ }, m_next_think_tick{ },
				m_last_update_tick{ }, m_bounces_count{ }, m_collision_group{ };
			std::vector< std::pair< sdk::vec3_t, bool > >	m_path{ };
			std::vector < game::base_entity_t* >            m_broken_ents{ };
		};

	private:
		using throwed_grenades_t = std::unordered_map< game::ent_handle_t, grenade_simulation_t >;

		void handle_warning_pred( game::base_entity_t* const entity, const game::e_class_id class_id );
		void add_trail( const grenade_simulation_t& sim, sdk::col_t clr, float lifetime = 0.025f, float thickness = 0.2f ) const;
		void on_create_move( const game::user_cmd_t& cmd );
		bool add_grenade_simulation( const grenade_simulation_t& sim, const bool warning ) const;
		throwed_grenades_t				m_throwed_grenades{ };
		grenade_simulation_t			m_grenade_trajectory{ };
	};
}