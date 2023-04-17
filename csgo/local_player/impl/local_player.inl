#pragma once

#include "../local_player.hpp"

namespace csgo {
    __forceinline valve::cs_player_t* c_local_player::self( ) const {
        return *g_ctx->offsets( ).m_local_player.as< valve::cs_player_t** >( );
    }

    __forceinline valve::cs_weapon_t* c_local_player::weapon( ) const { return m_weapon; }

    __forceinline valve::weapon_info_t* c_local_player::weapon_info( ) const { return m_weapon_info; }

}