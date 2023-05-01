#pragma once

#include "../entities.hpp"

namespace csgo::game { 
    __forceinline game::weapon_info_t* cs_weapon_t::info( ) { 
        using fn_t = weapon_info_t* ( __thiscall* )( sdk::address_t, e_item_index );

        const auto weapon_system = g_ctx->offsets( ).m_weapon_system;

        return ( *weapon_system.as< fn_t** >( ) )[ 2u ]( weapon_system, item_index( ) );
    }

    __forceinline game::cs_weapon_t* cs_player_t::weapon( ) { 
        return static_cast< cs_weapon_t* >( g_entity_list->get_entity( weapon_handle( ) ) );
    }

    __forceinline game::studio_hdr_t* cs_player_t::mdl_ptr( ) { 
        using fn_t = void( __thiscall* )( decltype( this ) );
        if( !studio_hdr( ) )
            reinterpret_cast < fn_t >( g_ctx->offsets( ).m_base_animating.m_lock_std_hdr )( this );

        return studio_hdr( );
    }

    __forceinline std::ptrdiff_t cs_player_t::lookup_seq_act( std::ptrdiff_t seq ) { 


        const auto std_hdr = mdl_ptr( );
        if( !std_hdr ) { 
            return -1;
        }

        using fn_t = int( __fastcall* )( studio_hdr_t* const, const int );

        return reinterpret_cast< fn_t >( 
            g_ctx->addresses( ).m_lookup_seq_act
            )( std_hdr, seq );
    }

    __forceinline std::uintptr_t cs_player_t::player_resource( ) { 
        for( std::ptrdiff_t i{ 1u }; i < game::g_entity_list->highest_ent_index( ); ++i ) { 
            const auto ent = game::g_entity_list->get_entity( i );

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

    __forceinline bool cs_player_t::alive( ) { 
        if( !this )
            return false;

        return life_state( ) == e_life_state::alive && health( ) > 0;
    }

    __forceinline std::optional< player_info_t > cs_player_t::info( ) { 
        player_info_t info{ };
        if( !g_engine->get_player_info( networkable( )->index( ), &info ) )
            return std::nullopt;

        return info;
    }

    __forceinline bool cs_player_t::friendly( cs_player_t* const with, bool swap_teams ) { 
        if( g_ctx->cvars( ).m_mp_teammates_are_enemies->get_int( ) )
            return false;

        if( !with || !with->networkable( ) || !this || !this->networkable( ) )
            return false;

        cs_player_t* ent = with;

        if( swap_teams ) { 
            if( with->networkable( )->index( ) == game::g_engine->get_local_player( ) ) { 
                if( !with->alive( ) ) { 
                    cs_player_t* spec = reinterpret_cast< cs_player_t* >( game::g_entity_list->get_entity( with->observer_target_handle( ) ) );
                    if( spec->is_valid_ptr( ) ) { 
                        if( with->observer_mode( ) == 4 || with->observer_mode( ) == 5 )
                            ent = spec;
                    }
                }
            }
        }
        
        return team( ) == ent->team( );
    }
}