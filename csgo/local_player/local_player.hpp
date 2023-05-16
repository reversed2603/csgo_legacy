#pragma once

namespace csgo { 
    class c_local_player { 
    private:
        struct cfg_t { 
            bool m_shitty_mrx_servers{ }, m_force_crosshair { };
        };

        sdk::cfg_var_t< cfg_t > m_cfg{ 0x02566b33u, { } };

        game::cs_weapon_t*    m_weapon{ };
        game::weapon_info_t*           m_weapon_info{ };
        std::deque < game::vfyd_user_cmd_t* > m_vfyd_user_cmds{ };
    public:
        void create_move( bool& send_packet,
            game::user_cmd_t& cmd, game::vfyd_user_cmd_t& vfyd_cmd
        );

        __forceinline game::cs_player_t* self( ) const;

        __forceinline game::cs_weapon_t* weapon( ) const;

        __forceinline game::weapon_info_t* weapon_info( ) const;

        __forceinline std::deque < game::vfyd_user_cmd_t* > vfyd_user_cmds( ) { return m_vfyd_user_cmds; }

        __forceinline cfg_t& cfg( ) { return m_cfg.value( ); };

        int                    m_defensive_cmd;
    };

    inline const auto g_local_player = std::make_unique< c_local_player > ( );
}

#include "impl/local_player.inl"