#pragma once

namespace csgo::hacks { 
	enum e_solve_modes { 
		solve_stand,
		solve_move,
		solve_air
	};

	enum e_solve_methods { 
		low_lby,
		body_flick,
		lby_delta,
		brute,
		fake_walk,
		brute_not_moved,
		air,
		body_flick_res,
		just_stopped,
		backwards,
		forwards,
		last_move_lby,
		freestand,
		move
	};

	std::string resolver_mode( int solve_method );

	struct lag_backup_t { 
		sdk::vec3_t m_mins{ };
		sdk::vec3_t m_maxs{ };
		float m_foot_yaw{ };
		sdk::vec3_t m_old_origin{ }, m_origin{ }, m_abs_origin{ };
		std::array < sdk::mat3x4_t, game::k_max_bones > m_bones{ };
		std::ptrdiff_t m_bones_cnt{ }, m_readable_bones{ }, m_writable_bones{ };
		sdk::ulong_t m_mdl_bone_count{ };
		bool m_has_valid_bones{ false };

		__forceinline void setup( game::cs_player_t* player ) { 

			if( !player || !player->alive( ) || player->networkable( )->dormant( ) )
				return;

			m_readable_bones = player->bone_accessor( ).m_readable_bones;
			m_writable_bones = player->bone_accessor( ).m_writable_bones;

			m_has_valid_bones = false;
			m_bones_cnt = player->bone_cache( ).m_size;

			if( player->bone_cache( ).m_mem.m_ptr ) { 
				std::memcpy( 
					m_bones.data( ),
					player->bone_cache( ).m_mem.m_ptr,
					m_bones_cnt * sizeof( sdk::mat3x4_t )
				 );

				m_has_valid_bones = true;
			}

			m_mdl_bone_count = player->mdl_bone_cnt( );
			m_origin = player->origin( );
			m_old_origin = player->old_origin( );
			m_mins = player->obb_min( );
			m_maxs = player->obb_max( );
			m_abs_origin = player->abs_origin( );

			if( const game::anim_state_t* anim_state = player->anim_state( ); anim_state != nullptr )
				m_foot_yaw = anim_state->m_foot_yaw;
		}

		__forceinline void restore( game::cs_player_t* player ) { 

			if( !player || !player->alive( ) || player->networkable( )->dormant( ) )
				return;

			player->bone_accessor( ).m_readable_bones = m_readable_bones;
			player->bone_accessor( ).m_writable_bones = m_writable_bones;

			if( m_has_valid_bones ) { 
				std::memcpy( 
					player->bone_cache( ).m_mem.m_ptr,
					m_bones.data( ), m_bones_cnt * sizeof( sdk::mat3x4_t )
				 );
			}

			player->mdl_bone_cnt( ) = m_mdl_bone_count;

			player->origin( ) = m_origin;
			player->old_origin( ) = m_old_origin;
			player->set_collision_bounds( m_mins, m_maxs );
			player->set_abs_ang( { 0.f, m_foot_yaw, 0.f } );
			player->set_abs_origin( m_abs_origin );
		}  
	};

	struct player_entry_t;
	struct previous_lag_data_t;

	struct lag_record_t { 
		std::array < sdk::mat3x4_t, game::k_max_bones > m_bones { };
		std::array < sdk::mat3x4_t, game::k_max_bones > m_extrapolated_bones{ }; // rendering only
		bool m_extrapolated{ };
		game::anim_layers_t m_anim_layers { };
		game::pose_params_t m_pose_params { };
		game::cs_player_t* m_player { };
		game::cs_weapon_t* m_wpn { };
		float m_third_person_recoil{ };
		float m_last_shot_time{ };
		float m_anim_time{ };
		int m_choked_cmds { };
		int m_lag_ticks { };
		bool m_dormant{ };
		bool m_broke_lc { };
		bool m_shifting{ };
		int m_velocity_step{ };
		float m_sim_time { };
		float m_received_sim_time{ };
		float m_old_sim_time { };
		bool m_fake_walking { };
		e_solve_methods m_resolver_method{ };
		game::e_ent_flags m_flags { };
		sdk::vec3_t m_origin { }, m_old_origin { };
		float m_interp_time{ };
		bool m_delta_resolver_invoked{ };
		sdk::vec3_t m_abs_origin { };
		int m_bones_count{ };
		std::optional < bool >          m_on_ground{ };
		float                           m_act_time{ };
		sdk::vec3_t m_mins { };
		float m_foot_yaw{ };
		float m_away_angle{ };
		bool m_broke_lby{ };
		float m_move_yaw { };
		float m_move_yaw_cur_to_ideal { };
		float m_move_yaw_ideal { };
		bool m_just_stopped{ };
		float m_move_weight_smoothed { };
		sdk::vec3_t m_maxs { };
		bool m_has_fake{ };
		int m_receive_tick { };
		sdk::qang_t m_eye_angles { };
		bool m_fake_flick{ };
		sdk::qang_t m_abs_angles { };
		float m_duck_amt { };
		float m_lby { };
		float m_old_lby { };
		sdk::vec3_t m_anim_velocity { };
		e_solve_modes m_mode { };
		bool m_resolved{ };
		bool m_valid_move{ };
		bool m_shot { };
		bool m_has_valid_bones{ false };

		__forceinline void store( game::cs_player_t* player );

		__forceinline lag_record_t( ) { 
			m_broke_lby = m_broke_lc = m_shifting = m_resolved
				= m_fake_walking = m_shot = m_valid_move = false;

			m_choked_cmds = m_lag_ticks = 0;
			m_old_lby = 0.f;
			m_interp_time = 0;
			m_has_valid_bones = false;
		}

		__forceinline lag_record_t( game::cs_player_t* player ) { 
			m_broke_lby = m_broke_lc = m_shifting = m_resolved
				= m_fake_walking = m_shot = m_valid_move = false;

			m_choked_cmds = m_lag_ticks = 0;
			m_old_lby = 0.f;
			m_mins = { };
			m_maxs = { };
			m_interp_time = 0;
			m_has_valid_bones = false;

			store( player );
		}

		__forceinline void adjust( game::cs_player_t* player ) 
		{ 
			if( !player || !player->alive( ) || player->networkable( )->dormant( ) )
				return;

			if( m_has_valid_bones && player->bone_cache( ).m_mem.m_ptr != nullptr ) { // extra safety

				std::memcpy( 
					player->bone_cache( ).m_mem.m_ptr,
					m_bones.data( ), player->bone_cache( ).m_size * sizeof( sdk::mat3x4_t ) ) ;

				player->mdl_bone_cnt( ) = **reinterpret_cast< unsigned long** > ( 
					g_ctx->addresses( ).m_invalidate_bone_cache + 0xau
					 );
			}

			player->origin( ) = m_origin;
			player->old_origin( ) = m_old_origin;
			player->set_abs_origin( m_origin );
			player->set_collision_bounds( m_mins, m_maxs );
			player->set_abs_ang( { 0.f, m_foot_yaw, 0.f } );
		}

		__forceinline bool valid( );
		__forceinline void simulate( cc_def( previous_lag_data_t* ) previous, player_entry_t& entry );
	};

	struct previous_lag_data_t { 
		__forceinline previous_lag_data_t( lag_record_t* lag_record ) { 
			m_foot_yaw = lag_record->m_foot_yaw;
			m_move_yaw = lag_record->m_move_yaw;
			m_move_yaw_cur_to_ideal = lag_record->m_move_yaw_cur_to_ideal;
			m_move_yaw_ideal = lag_record->m_move_yaw_ideal;
			m_move_weight_smoothed = lag_record->m_move_weight_smoothed;

			m_wpn = lag_record->m_wpn;
			m_eye_angles = lag_record->m_eye_angles;
			m_origin = lag_record->m_origin;
			m_old_origin = lag_record->m_old_origin;
			m_anim_velocity = lag_record->m_anim_velocity;

			m_resolved = lag_record->m_resolved;

			m_flags = lag_record->m_flags;
			m_lby = lag_record->m_lby;
			m_duck_amt = lag_record->m_duck_amt;

			m_anim_layers = lag_record->m_anim_layers;
			m_pose_params = lag_record->m_pose_params;
			m_sim_time = lag_record->m_sim_time;

			m_broke_lby = lag_record->m_broke_lby;

			m_delta_resolver_invoked = lag_record->m_delta_resolver_invoked;
			m_just_stopped = lag_record->m_just_stopped;
			m_dormant = lag_record->m_dormant;
			m_fake_flick = lag_record->m_fake_flick;
			m_anim_time = lag_record->m_anim_time;
			m_has_valid_bones = lag_record->m_has_valid_bones;
			m_shifting = lag_record->m_shifting;
		}

		float	m_foot_yaw{ }, m_move_yaw{ };
		float m_move_yaw_cur_to_ideal{ };
		float m_move_yaw_ideal{ };
		float m_move_weight_smoothed{ };
		bool  m_broke_lby{ }, m_fake_flick{ }, m_shifting{ }, m_delta_resolver_invoked{ }, m_just_stopped{ }, m_dormant{ }, m_resolved{ };

		bool m_has_valid_bones{ false };
		game::cs_weapon_t* m_wpn{ };

		sdk::qang_t						m_eye_angles{ };
		sdk::vec3_t						m_old_origin{ }, m_origin{ }, m_anim_velocity{ };

		game::e_ent_flags				m_flags{ };
		float							m_lby{ }, m_duck_amt{ };

		game::anim_layers_t			m_anim_layers{ };
		game::pose_params_t         m_pose_params{ };
		float                           m_sim_time{ }, m_anim_time{ };
	};

	struct moving_data_t { 

		sdk::vec3_t     m_origin{ };
		float			m_lby{ }, m_time{ };
		bool            m_moved{ };

		__forceinline void reset( ) { 
			m_origin = sdk::vec3_t( 0, 0, 0 );
			m_lby	 = m_moved = 0;
			m_time	 = FLT_MAX;
		}
	};

	struct body_update_data_t { 
		bool  m_has_updated{ false };
		float m_realign_timer{ FLT_MAX };

		__forceinline void reset( bool reset_update = false ) { 

			if( reset_update )
				m_has_updated = false;

			m_realign_timer = FLT_MAX;
		}
	};

	struct player_entry_t { 
		__forceinline void reset( );

		body_update_data_t m_body_data;
		moving_data_t      m_moving_data;

		game::cs_player_t* m_player;
		float m_spawn_time { }, m_receive_time { },	m_alive_loop_cycle{ }, m_alive_loop_rate{ }, m_lby{ }, m_old_lby{ };

		bool  m_body_proxy_updated{ };

		std::array < sdk::mat3x4_t, game::k_max_bones >			    m_bones { };
		std::array < sdk::vec3_t, game::k_max_bones >                 m_bone_origins { };

		std::ptrdiff_t                                  m_air_misses { };
		sdk::vec3_t                                     m_render_origin { };

		std::deque< std::shared_ptr< lag_record_t > >	m_lag_records { };
		std::ptrdiff_t                                  m_stand_not_moved_misses{ }, m_stand_moved_misses{ }, m_last_move_misses{ },
			m_forwards_misses{ }, m_backwards_misses{ }, m_freestand_misses{ }
		, m_lby_misses{ }, m_just_stopped_misses{ },
			m_moving_misses{ }, m_low_lby_misses{ };

		float											m_valid_pitch{ };
		bool                                            m_delta_resolver_invoked { };
		float                                           m_left_dmg{ }, m_right_dmg{ }, m_left_frac{ }, m_right_frac{ }, m_unmoved_lby{ };
		bool                                            m_predicting_lby{ }, m_had_last_move { };
		std::optional < previous_lag_data_t >           m_previous_record{ };
		bool                                            m_has_fake_flick{ }, m_has_freestand{ };
		float                                           m_last_dist_lby{ }, m_pre_last_dist_lby{ }, m_pre_pre_last_dist_lby{ };
		float											m_freestand_angle{ };
	};

	class c_lag_comp { 
	private:
		std::array < player_entry_t, 64u > m_entries { };
	public:
		void handle_net_update( );
		__forceinline player_entry_t& entry( const std::size_t i );
		std::shared_ptr< lag_record_t > get_first_valid_record( game::cs_player_t* player, bool& success );
	};

	inline const auto g_lag_comp = std::make_unique < c_lag_comp > ( );
}

#include "impl/lag_comp.inl"