#pragma once

#include "../ctx.hpp"

namespace csgo { 
    __forceinline const auto& c_ctx::offsets( ) const { return m_offsets; }

    __forceinline const auto& c_ctx::cvars( ) const { return m_cvars; }

    __forceinline auto& c_ctx::addresses( ) { return m_addresses; }

    __forceinline auto& c_ctx::anim_data( ) { return m_anim_data; }

    __forceinline auto& c_ctx::ticks_data( ) { return m_ticks_data; }

    __forceinline auto& c_ctx::cur_cfg_slot( ) { return m_cur_cfg_slot; }

    __forceinline auto& c_ctx::frame_droped( ) { return m_frame_droped; }

    __forceinline auto& c_ctx::is_out_of_epsilon( ) { return m_is_out_of_epsilon; }

    __forceinline auto& c_ctx::send_packet( ) { return m_send_packet; }

    __forceinline auto& c_ctx::can_shoot( ) { return m_can_shoot; }

    __forceinline auto& c_ctx::was_shooting( ) { return m_was_shooting; }

    __forceinline auto& c_ctx::buy_bot( ) { return m_buy_bot; }

    __forceinline auto& c_ctx::left_create_move( ) { return m_left_create_move; }

    __forceinline c_ctx::net_info_t& c_ctx::net_info( ) { 
        return m_net_info;
    }

    __forceinline auto& c_ctx::shoot_pos( ) { return m_shoot_pos; }

    __forceinline auto& c_ctx::aim_shoot_pos( ) { return m_aim_shoot_pos; }

    __forceinline std::deque < c_ctx::out_cmds_data_t >& c_ctx::get_out_cmds( ) { 
        return m_out_cmds_data;
    }

    __forceinline auto& c_ctx::defensive_allowed( ) { 
        return m_defensive_allowed;
    }

    __forceinline c_ctx::s_auto_peek_data& c_ctx::get_auto_peek_info( ) { 
        return m_apeek_data;
    }
}