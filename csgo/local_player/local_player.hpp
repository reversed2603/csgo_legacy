#pragma once

namespace csgo {
    class c_local_player {
    private:
        struct cfg_t {
            bool m_shitty_mrx_servers{}, m_force_crosshair {};
        };

        void update_prediction( ) const;

        sdk::cfg_var_t< cfg_t > m_cfg{ 0x02566b33u, {} };

        valve::cs_weapon_t*    m_weapon{};
        valve::weapon_info_t*           m_weapon_info{};
        std::deque < valve::vfyd_user_cmd_t* > m_vfyd_user_cmds{};
    public:
        void create_move( bool& send_packet,
            valve::user_cmd_t& cmd, valve::vfyd_user_cmd_t& vfyd_cmd
        );

        bool will_shoot( valve::cs_weapon_t* const weapon, const valve::user_cmd_t& user_cmd ) const;

        ALWAYS_INLINE valve::cs_player_t* self( ) const;

        ALWAYS_INLINE valve::cs_weapon_t* weapon( ) const;

        ALWAYS_INLINE valve::weapon_info_t* weapon_info( ) const;

        ALWAYS_INLINE std::deque < valve::vfyd_user_cmd_t* > vfyd_user_cmds( ) { return m_vfyd_user_cmds; }

        ALWAYS_INLINE cfg_t& cfg( ) { return m_cfg.value( ); };

    };

    inline const auto g_local_player = std::make_unique< c_local_player >( );
}

#include "impl/local_player.inl"