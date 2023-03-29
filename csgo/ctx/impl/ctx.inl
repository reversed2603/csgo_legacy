#pragma once

#include "../ctx.hpp"

namespace csgo {
    ALWAYS_INLINE const auto& c_ctx::offsets( ) const { return m_offsets; }

    ALWAYS_INLINE const auto& c_ctx::cvars( ) const { return m_cvars; }

    ALWAYS_INLINE auto& c_ctx::addresses( ) { return m_addresses; }

    ALWAYS_INLINE auto& c_ctx::anim_data( ) { return m_anim_data; }

    ALWAYS_INLINE auto& c_ctx::ticks_data( ) { return m_ticks_data; }

    ALWAYS_INLINE auto& c_ctx::cur_cfg_slot( ) { return m_cur_cfg_slot; }

    ALWAYS_INLINE bool& c_ctx::in_charge( ) { return m_in_charge; }

    ALWAYS_INLINE auto& c_ctx::frame_droped( ) { return m_frame_droped; }

    ALWAYS_INLINE auto& c_ctx::is_out_of_epsilon( ) { return m_is_out_of_epsilon; }

    ALWAYS_INLINE auto& c_ctx::send_packet( ) { return m_send_packet; }

    ALWAYS_INLINE auto& c_ctx::can_shoot( ) { return m_can_shoot; }

    ALWAYS_INLINE auto& c_ctx::was_shooting( ) { return m_was_shooting; }

    ALWAYS_INLINE auto& c_ctx::buy_bot( ) { return m_buy_bot; }

    ALWAYS_INLINE auto& c_ctx::left_create_move( ) { return m_left_create_move; }

    ALWAYS_INLINE c_ctx::net_info_t& c_ctx::net_info( ) {
        return m_net_info;
    }

    ALWAYS_INLINE bool& c_ctx::allow_defensive( ) { return m_allow_defensive; }

    ALWAYS_INLINE auto& c_ctx::shoot_pos( ) { return m_shoot_pos; }

    ALWAYS_INLINE auto& c_ctx::aim_shoot_pos( ) { return m_aim_shoot_pos; }

    ALWAYS_INLINE std::deque < c_ctx::out_cmds_data_t >& c_ctx::get_out_cmds( ) {
        return m_out_cmds_data;
    }

    ALWAYS_INLINE auto& c_ctx::defensive_allowed( ) {
        return m_defensive_allowed;
    }

    ALWAYS_INLINE c_ctx::s_auto_peek_data& c_ctx::get_auto_peek_info( ) {
        return m_apeek_data;
    }
}