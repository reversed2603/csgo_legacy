#pragma once
namespace csgo::hacks {
    class c_move {
    private:
        float					m_max_player_speed { }, m_max_weapon_speed { };
        bool                    m_allow_predictive_autostop{ };
        struct cfg_t {
            s_keybind m_slow_walk { }, m_auto_peek_key { };

            float m_auto_peek_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };
            
            bool m_bhop { }, m_auto_strafe { }, m_fast_stop { }, m_infinity_duck { };
        };

        sdk::cfg_var_t< cfg_t > m_cfg { 0x05562e58u, { } };

    public:
        void handle( game::user_cmd_t& cmd );

        void rotate( game::user_cmd_t& cmd, const sdk::qang_t& wish_angles, const game::e_ent_flags flags, const game::e_move_type move_type ) const;

        void bunny_hop( game::user_cmd_t& cmd ) const;

        void auto_strafe( game::user_cmd_t& cmd ) const;

        void fast_stop( game::user_cmd_t& cmd ) const;

        void slow_walk( game::user_cmd_t& cmd ) const;

        void unlock_crouch_cooldown( game::user_cmd_t& cmd ) const;

        void auto_stop( game::user_cmd_t& user_cmd, float target_speed = 35.f );

        void auto_peek( sdk::qang_t& wish_ang, game::user_cmd_t& user_cmd );

        void accelerate( 
            const game::user_cmd_t& user_cmd, const sdk::vec3_t& wishdir,
            const float wishspeed, sdk::vec3_t& velocity, float acceleration
        ) const;

        void walk_move( 
            const game::user_cmd_t& user_cmd, sdk::vec3_t& move,
            sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
        ) const;

        void full_walk_move( 
            const game::user_cmd_t& user_cmd, sdk::vec3_t& move,
            sdk::vec3_t& fwd, sdk::vec3_t& right, sdk::vec3_t& velocity
        ) const;

        void modify_move( game::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const;

        void predict_move( const game::user_cmd_t& user_cmd, sdk::vec3_t& velocity ) const;

        __forceinline cfg_t& cfg( ) { return m_cfg.value( ); };

        __forceinline bool& allow_early_stop( ) { return m_allow_predictive_autostop; };
    };

    inline const auto g_move = std::make_unique< c_move >( );
}