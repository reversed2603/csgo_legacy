#pragma once

namespace csgo::valve {
    ALWAYS_INLINE void user_cmd_t::sanitize( ) {
        m_view_angles.normalize( );

        constexpr auto k_max_roll = 50.f;

        const auto& cvars = g_ctx->cvars( );

        m_view_angles.x( ) = std::clamp( m_view_angles.x( ), -cvars.m_cl_pitchup->get_float( ), cvars.m_cl_pitchdown->get_float( ) );
        m_view_angles.z( ) = std::clamp( m_view_angles.z( ), -k_max_roll, k_max_roll );
    }

    ALWAYS_INLINE const char* game_event_t::name( )
    {
        using fn_t = const char* ( __thiscall* )( decltype( this ) );

        return ( *reinterpret_cast< fn_t** >( this ) )[ 1u ]( this );
    }

    ALWAYS_INLINE float game_event_t::get_float( const char* name )
    {
        using fn_t = float( __thiscall* )( decltype( this ), const char*, const float );

        return ( *reinterpret_cast< fn_t** >( this ) )[ 8u ]( this, name, 0.f );
    }

    ALWAYS_INLINE int game_event_t::get_int( const char* name )
    {
        using fn_t = int( __thiscall* )( decltype( this ), const char*, const int );

        return ( *reinterpret_cast< fn_t** >( this ) )[ 6u ]( this, name, 0 );
    }

    ALWAYS_INLINE const char* game_event_t::get_str( const char* name, const char* def_val )
    {
        using fn_t = const char* ( __thiscall* )( decltype( this ), const char*, const char* );

        return ( *reinterpret_cast< fn_t** >( this ) )[ 9u ]( this, name, def_val );
    }


    ALWAYS_INLINE const char* studio_hitbox_set_t::name( ) const {
        return !m_name_index ? nullptr
            : sdk::address_t{ this }.self_offset( m_name_index ).as< const char* >( );
    }

    ALWAYS_INLINE studio_bbox_t* studio_hitbox_set_t::get_bbox( const int i ) const {
        return i > m_hitboxes_count ? nullptr
            : sdk::address_t{ this }.self_offset( m_hitbox_index ).as< studio_bbox_t* >( ) + i;
    }

    ALWAYS_INLINE studio_bone_t* studio_hdr_t::studio_t::get_bone( const int i ) const {
        return i > m_bones_count ? nullptr
            : sdk::address_t{ this }.self_offset( m_bone_index ).as< studio_bone_t* >( ) + i;
    }

    ALWAYS_INLINE studio_hitbox_set_t* studio_hdr_t::studio_t::get_hitbox_set( const int i ) const {
        return i > m_hitbox_sets_count ? nullptr
            : sdk::address_t{ this }.self_offset( m_hitbox_set_index ).as< studio_hitbox_set_t* >( ) + i;
    }

    ALWAYS_INLINE c_net_vars::entry_t& c_net_vars::entry( const std::string& name )
    {
        return m_entries.at( name );
    }

    ALWAYS_INLINE bool trace_t::hit( ) const {
        return m_frac < 1.f || m_all_solid || m_start_solid;
    }
}