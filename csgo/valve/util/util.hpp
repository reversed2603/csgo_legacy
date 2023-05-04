#pragma once

namespace csgo::game { 
    /* should be in interfaces.hpp tbh */
    class c_mem_alloc { 
    public:
        VFUNC( sdk::address_t( __thiscall* )( decltype( this ), std::size_t ), alloc( std::size_t size ), 1u, size );

        VFUNC( sdk::address_t( __thiscall* )( decltype( this ), void*, std::size_t ),
           realloc( sdk::address_t addr, std::size_t size ), 3u, addr.as< void* > ( ), size );

        VFUNC( void( __thiscall* )( decltype( this ), void* ), free( sdk::address_t addr ), 5u, addr.as< void* > ( ) );
    } inline* g_mem_alloc{ };

    template < typename _value_t, typename _index_t >
    struct utl_mem_t { 
    public:
        _value_t*   m_ptr{ };
        int         m_alloc_count{ };
        int         m_grow_size{ };
    public:
        __forceinline _value_t& at( const _index_t i );

        __forceinline const _value_t& at( const _index_t i ) const;

        __forceinline void clear( );

        __forceinline void grow( const int count );

        __forceinline int alloc_count( ) const;
    };

    template < typename _value_t >
    struct utl_vec_t { 
    public:    
        utl_mem_t< _value_t, int >  m_mem{ };
        int                         m_size{ };
        _value_t*                   m_elements{ };
    public:
        inline void destruct( _value_t* memory )
        { 
            memory->~_value_t( );
        }

        __forceinline void remove_all( ) { 
            for( int i = m_size; --i >= 0; ) { 
                destruct( &at( i ) );
            }

            m_size = 0;
        }
        __forceinline constexpr utl_vec_t( ) = default;

        __forceinline _value_t& at( const int i );

        __forceinline const _value_t& at( const int i ) const;

        __forceinline void clear( );

        __forceinline void reserve( const int size );

        __forceinline int size( ) const;

        __forceinline utl_vec_t< _value_t >& operator = ( const utl_vec_t< _value_t >& other );
    };

    struct key_values_t { 
        using get_symbol_proc_t = bool( __cdecl* )( const char* );

        std::uint32_t       m_key_name : 24u,
                            m_key_name_case_sensitive1 : 8u;

        char*               m_str{ };
        wchar_t*            m_wstr{ };

        union { 
            int             m_int;
            float           m_float;
            sdk::address_t  m_ptr;
            sdk::argb_t     m_clr{ };
        };

        char                m_data_type{ },
                            m_has_esc_sequences{ };
        std::uint16_t       m_key_name_case_sensitive2{ };

        key_values_t*       m_peer{ }, *m_sub{ }, *m_chain{ };

        get_symbol_proc_t   m_get_symbol_proc_fn{ };
    };
}

#include "impl/util.inl"