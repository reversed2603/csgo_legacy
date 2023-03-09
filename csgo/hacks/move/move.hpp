#pragma once
namespace csgo::hacks {
    class c_move {
    private:
        float					m_max_player_speed {}, m_max_weapon_speed {};
        bool                    m_allow_predictive_autostop{};
        struct cfg_t {
            s_keybind m_slow_walk {}, m_auto_peek_key {};
            
            bool m_bhop {}, m_auto_strafe {}, m_fast_stop {}, m_infinity_duck {};
            int m_auto_stop_type_scar{}, m_auto_stop_type_scout {}, m_auto_stop_type_awp {}, m_auto_stop_type_heavy_pistol {}, m_auto_stop_type_pistol {}, m_auto_stop_type_other {};
        };

        sdk::cfg_var_t< cfg_t > m_cfg { 0x05562e58u, {} };

    public:
        void handle( valve::user_cmd_t& cmd, sdk::qang_t nigga_who );

        void rotate( valve::user_cmd_t& cmd, const sdk::qang_t& wish_angles, const valve::e_ent_flags flags, const valve::e_move_type move_type ) const;

        void bunny_hop ( valve::user_cmd_t& cmd ) const;

        void auto_strafe ( valve::user_cmd_t& cmd ) const;

        void fast_stop ( valve::user_cmd_t& cmd ) const;

        void slow_walk ( valve::user_cmd_t& cmd ) const;

        void unlock_crouch_cooldown ( valve::user_cmd_t& cmd ) const;

        void auto_stop ( valve::user_cmd_t& user_cmd, sdk::qang_t& who, float target_speed = 35.f );

        void auto_peek ( sdk::qang_t& wish_ang, valve::user_cmd_t& user_cmd );

        void accelerate (
            const valve::user_cmd_t& user_cmd, const sdk::vec3_t& wishdir,
            const float wishspeed, sdk::vec3_t& velocity, float acceleration
        ) const;

        void walk_move (
            const valve::user_cmd_t& user_cmd, sdk::vec3_t& move,
            sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
        ) const;

        void full_walk_move (
            const valve::user_cmd_t& user_cmd, sdk::vec3_t& move,
            sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
        ) const;

        void modify_move ( valve::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const;

        void predict_move ( const valve::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const;

        __forceinline cfg_t& cfg ( ) { return m_cfg.value ( ); };

        __forceinline bool& allow_early_stop( ) { return m_allow_predictive_autostop; };
    };

    inline const auto g_move = std::make_unique< c_move >( );
}