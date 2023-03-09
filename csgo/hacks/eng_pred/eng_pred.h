#pragma once
namespace csgo::hacks {
	class c_eng_pred {
	private:
		struct {
			float m_cur_time {}, m_frame_time {};
		}								m_backup {};
		struct {
			int m_view_model_index, m_animation_parity;
			float m_view_cycle, m_anim_time;
			int m_view_sequence;

		} m_view_model {};

		struct r8_t {
			ALWAYS_INLINE r8_t( ) = default;

			ALWAYS_INLINE r8_t( const int tick, const bool in_attack, const bool can_shoot ) {
				m_tick = tick;
				m_in_attack = in_attack;
				m_can_shoot = can_shoot;
			}

			int m_tick{};
			bool m_in_attack{};
			bool m_can_shoot{};
		};

		struct net_vars_t {
			__forceinline void restore ( const int cmd_number ) const;

			__forceinline void store ( const int cmd_number );

			int			m_cmd_number {};

			float		m_view_offset_z {}, m_velocity_modifier {}, m_fall_velocity {}, m_duck_speed {};

			sdk::qang_t	m_aim_punch {}, m_aim_punch_vel {}, m_view_punch {};
			sdk::vec3_t      m_velocity {}, m_origin {};
			r8_t        m_r8{};
		};
	public:
		struct local_data_t {
			__forceinline void init ( const valve::user_cmd_t& user_cmd );

			float m_spawn_time {}, m_abs_yaw {};
			int	m_tick_base {},
				m_adjusted_tick_base {}, m_fake_amount {};
			bool m_override_tick_base {}, m_restore_tick_base {};

			sdk::vec3_t	m_move {};
			bool m_repredicted{};
			valve::user_cmd_t m_user_cmd {};
		};
	private:

		bool m_r8_can_shoot {};
		float m_spread {}, m_inaccuracy {},
			m_min_inaccuracy {},
			m_postpone_fire_ready_time {},
			m_recoil_index {}, m_accuracy_penalty {},
			m_velocity_modifier {}, m_net_velocity_modifier { 1.f };

		bool                                m_is_out_of_epsilon {};

		std::array< net_vars_t, 150u >		m_net_vars {};
		std::array< local_data_t, 150u >	m_local_data {};

		valve::move_data_t					m_move_data {};
		valve::e_frame_stage				m_last_frame_stage {};
		sdk::qang_t                         m_aim_punch_vel {}, m_view_punch {}, m_aim_punch {};
		sdk::vec3_t                         m_velocity {}, m_origin {}, m_view_offset {}, m_base_velocity {}, m_move {};
		int                                 m_tick_base {};
		valve::e_ent_flags                  m_flags {};
		valve::e_move_type                  m_move_type {};
		float                               m_duck_amt {}, m_duck_speed {}, m_fall_velocity {}, m_velocity_modifier_data {};
		float                               m_processed_cmd{};
		valve::e_buttons                    m_buttons{};

	public:
		void prepare ( );

		void predict ( valve::user_cmd_t* const user_cmd, bool init_data, bool force_repredict );

		void process ( valve::user_cmd_t* const user_cmd, bool init_data, bool force_repredict = false );

		void restore ( );

		void on_packet_update ( std::uintptr_t cl_state );

		__forceinline void recompute_velocity_modifier ( const std::ptrdiff_t cmd_num, const bool predicted ) const;

		void update_shoot_pos ( const valve::user_cmd_t& user_cmd ) const;

		__forceinline valve::e_frame_stage& last_frame_stage ( );

		__forceinline std::array< net_vars_t, 150u >& net_vars ( );

		__forceinline std::array< local_data_t, 150u >& local_data ( );

		__forceinline void reset_on_spawn ( );

		__forceinline float& spread ( );

		__forceinline bool& is_out_of_epsilon ( );

		__forceinline float& inaccuracy ( );

		__forceinline void velocity_modifier_to_data_map ( );

		__forceinline float& min_inaccuracy ( );

		__forceinline void store_data_map ( );

		__forceinline void restore_data_map ( );

		__forceinline float& recoil_index ( );

		__forceinline void save_view_model ( );

		__forceinline void adjust_view_model ( );
		__forceinline float& accuracy_penalty ( );

		__forceinline float& velocity_modifier ( );

		__forceinline float& net_velocity_modifier ( );

		__forceinline bool& r8_can_shoot ( );

		__forceinline float& postpone_fire_ready_time ( );
	};

	inline const auto g_eng_pred = std::make_unique< c_eng_pred > ( );
}

#include "impl/eng_pred.inl"