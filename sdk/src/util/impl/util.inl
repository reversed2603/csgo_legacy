#include "../util.hpp"

#include "..\..\..\..\thirdparty\security\Dependencies\Hash.hpp"

namespace sdk { 
    __forceinline std::string to_multi_byte( const std::wstring_view wstr ) { 
        if( wstr.empty( ) )
            return { };

        std::size_t len{ };
        std::mbstate_t state{ };

        auto src = wstr.data( );

        if( wcsrtombs_s( &len, nullptr, 0u, &src, wstr.size( ), &state ) )
            return { };

        std::string str{ };

        str.resize( len - 1u );

        if( wcsrtombs_s( &len, str.data( ), len, &src, wstr.size( ), &state ) )
            return { };

        return str;
    }

    __forceinline std::wstring to_wide_char( const std::string_view str ) { 
        if( str.empty( ) )
            return { };

        std::size_t len{ };
        std::mbstate_t state{ };

        auto src = str.data( );

        if( mbsrtowcs_s( &len, nullptr, 0u, &src, str.size( ), &state ) )
            return { };

        std::wstring wstr{ };

        wstr.resize( len - 1u );

        if( mbsrtowcs_s( &len, wstr.data( ), len, &src, str.size( ), &state ) )
            return { };

        return wstr;
    }

    template < typename _char_t >
        requires is_char_v< _char_t >
    __forceinline constexpr hash_t hash( const _char_t* const str, const std::size_t len ) { 
#if defined( _M_IX86 ) || defined( __i386__ ) || defined( _M_ARM ) || defined( __arm__ )
        constexpr auto k_basis = 0x45C3370D;
        constexpr auto k_prime = 0x1000193u;
#else
        constexpr auto k_basis = 0xcbf29ce484222325u;
        constexpr auto k_prime = 0x100000001b3u;
#endif

        auto hash = k_basis;

        for( std::size_t i{ }; i < len; ++i ) { 
            hash ^= str[ i ];
            hash *= k_prime;
        }

        return ~hash;
    }

    template < typename _char_t >
        requires is_char_v< _char_t >
    __forceinline constexpr hash_t hash( const _char_t* const str ) { 
        std::size_t len{ };
        while( str[ ++len ] != '\0' )
            ;

        return hash( str, len );
    }

    template < typename _enum_t >
        requires std::is_enum_v< _enum_t >
    __forceinline constexpr std::underlying_type_t< _enum_t > to_underlying( const _enum_t value ) { 
        return static_cast< std::underlying_type_t< _enum_t > > ( value );
    }
}