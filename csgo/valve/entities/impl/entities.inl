#pragma once

#include "../entities.hpp"

namespace csgo::valve {
    ALWAYS_INLINE weapon_info_t* cs_weapon_t::info( ) {
        using fn_t = weapon_info_t*( __thiscall* )( sdk::address_t, e_item_index );

        const auto weapon_system = g_ctx->offsets( ).m_weapon_system;

        return ( *weapon_system.as< fn_t** >( ) )[ 2u ]( weapon_system, item_index( ) );
    }

    ALWAYS_INLINE cs_weapon_t* cs_player_t::weapon( ) {
        return static_cast< cs_weapon_t* >( g_entity_list->get_entity( weapon_handle( ) ) );
    }

    ALWAYS_INLINE valve::studio_hdr_t* cs_player_t::mdl_ptr( ) {
        using fn_t = void( __thiscall* )( decltype( this ) );
        if( !studio_hdr( ) )
            reinterpret_cast < fn_t >( g_ctx->offsets( ).m_base_animating.m_lock_std_hdr )( this );

        return studio_hdr( );
    }

    ALWAYS_INLINE std::ptrdiff_t cs_player_t::lookup_seq_act( std::ptrdiff_t seq ) {

        if( !this )
            return -1;

        const auto std_hdr = mdl_ptr( );
        if( !std_hdr ) {
            return -1;
        }

        using fn_t = int( __fastcall* )( studio_hdr_t* const, const int );

        return reinterpret_cast< fn_t >( 
            g_ctx->addresses( ).m_lookup_seq_act
            )( std_hdr, seq );
    }

    ALWAYS_INLINE std::uintptr_t cs_player_t::player_resource( ) {
        for( std::ptrdiff_t i{ 1u }; i < valve::g_entity_list->highest_ent_index( ); ++i ) {
            const auto ent = valve::g_entity_list->get_entity( i );

            if( !ent )
                continue;

            auto cl_class = ent->networkable( )->client_class( );

            if( !cl_class )
                continue;

			if( cl_class->m_class_id == 36 ) {
                return reinterpret_cast < std::uintptr_t >( ent );
			}
        }

        return { };
    }

    ALWAYS_INLINE bool cs_player_t::alive( ) {
        return life_state( ) == e_life_state::alive && health( ) > 0;
    }

    ALWAYS_INLINE std::optional< player_info_t > cs_player_t::info( ) {
        player_info_t info{ };
        if( !g_engine->get_player_info( networkable( )->index( ), &info ) )
            return std::nullopt;

        return info;
    }

    ALWAYS_INLINE bool cs_player_t::friendly( cs_player_t* const with ) {
        if( with == this )
            return true;

#ifndef CSGO2018
        if( g_game_types->game_type( ) == e_game_type::ffa )
            return survival_team( ) == with->survival_team( );
#endif

        if( g_ctx->cvars( ).m_mp_teammates_are_enemies->get_int( ) )
            return false;

        return team( ) == with->team( );
    }
}