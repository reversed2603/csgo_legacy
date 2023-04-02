#pragma once
namespace csgo::hacks {
	class c_anti_aim {
	protected:
		bool m_can_choke { };
		int  m_lby_counter{ }, m_lby_on_same_pos{ }, m_lby_counter_updated{ }, m_next_choke_count{ }, m_flicks_count{ };
		struct cfg_t {

			bool m_should_fake_lag{ }, m_anti_aim{ }, m_force_turn{ }, m_shift{ }, m_should_distort{ }, m_swap_distort{ }, m_disable_lag_on_stand{ }, m_fake_move{ }, m_change_flick_dir{ }, m_lby_breaker{ };
			float m_real_yaw{ }, m_jitter_yaw{ }, m_distort_max_time{ }, m_distort_speed{ }, m_distort_factor{ }, m_shift_factor{ }, m_flick_strength{ };
			int  m_ticks_to_choke { 1 }, m_await_shift { };
			int  m_pitch_type { }; 
			s_keybind m_left_manual { }, m_right_manual { }, m_middle_manual { }, m_freestand { }, m_fake_flick { };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b51u, { } };

	public:
		void handle_ctx( valve::user_cmd_t& user_cmd, bool& send_packet, bool in_charge = false );
		float handle_yaw( valve::user_cmd_t& user_cmd );
		void handle_fake_lag( valve::user_cmd_t& user_cmd );
		void handle_pitch( valve::user_cmd_t& user_cmd );
		void handle_distortion( valve::user_cmd_t& user_cmd );
		void fake_move( valve::user_cmd_t& user_cmd );
		bool freestanding( float& yaw );
		bool break_freestand( float& yaw );
		void fake_flick( valve::user_cmd_t& cmd, bool& send_packet );
		ALWAYS_INLINE bool& can_choke( );
		ALWAYS_INLINE bool should_disable( valve::user_cmd_t& user_cmd );
		ALWAYS_INLINE cfg_t& cfg( ) { return m_cfg.value( ); };
		ALWAYS_INLINE float get_manual_rotate( );
		ALWAYS_INLINE const sdk::qang_t& last_anim_ang( );
		bool m_fake_moving{ };
		bool m_switch_dist_dir{ };
		int m_auto_dir_side{ };
		sdk::qang_t m_last_anim_ang{ };
	};

	inline const std::unique_ptr < c_anti_aim > g_anti_aim = std::make_unique < c_anti_aim >( );
}

#include "impl/anti_aim.inl"