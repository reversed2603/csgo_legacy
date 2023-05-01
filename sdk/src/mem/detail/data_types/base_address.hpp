#pragma once

namespace sdk::detail { 
    template < typename _addr_t >
       requires std::is_unsigned_v< _addr_t >
    struct base_address_t { 
    protected:
        _addr_t m_addr{ };
    public:
        __forceinline constexpr base_address_t( ) = default;

        __forceinline constexpr base_address_t( const _addr_t addr ) : m_addr{ addr } { }

        template < typename _ptr_t >
            requires std::is_pointer_v< _ptr_t >
        __forceinline base_address_t( const _ptr_t ptr ) : m_addr{ reinterpret_cast< _addr_t >( ptr ) } { }

        __forceinline constexpr operator _addr_t( ) const { return m_addr; }

        template < typename _ret_t >
            requires std::is_pointer_v< _ret_t >
        __forceinline _ret_t as( ) const { return reinterpret_cast< _ret_t >( m_addr ); }

        __forceinline constexpr base_address_t< _addr_t >& self_offset( const std::ptrdiff_t offset ) { 
            m_addr += offset;

            return *this;
        }

        __forceinline base_address_t< _addr_t >& self_deref( std::size_t count ) { 
            for( ; m_addr && count; --count )
                m_addr = *as< _addr_t* >( );

            return *this;
        }

        __forceinline base_address_t< _addr_t >& self_rel( const std::ptrdiff_t offset, const bool is_long ) { 
            m_addr +=
                is_long
                ? offset + sizeof( std::uint32_t ) + *reinterpret_cast< std::int32_t* >( m_addr + offset )
                : offset + sizeof( std::uint8_t ) + *reinterpret_cast< std::int8_t* >( m_addr + offset );

            return *this;
        }

        __forceinline base_address_t< _addr_t >& self_find_byte( 
            const std::uint8_t byte, const std::size_t max_region, const bool up
        ) { 
            for( auto i = m_addr; i && ( i - m_addr ) < max_region; up ? --i : ++i ) { 
                if( *reinterpret_cast< std::uint8_t* >( i ) != byte )
                    continue;

                m_addr = i;

                break;
            }

            return *this;
        }

        __forceinline constexpr base_address_t< _addr_t > offset( const std::ptrdiff_t offset ) const { 
            auto ret = *this;

            return ret.self_offset( offset );
        }

        __forceinline base_address_t< _addr_t > deref( const std::size_t count ) const { 
            auto ret = *this;

            return ret.self_deref( count );
        }

        __forceinline base_address_t< _addr_t > rel( const std::ptrdiff_t offset, const bool is_long ) const { 
            auto ret = *this;

            return ret.self_rel( offset, is_long );
        }

        __forceinline base_address_t< _addr_t > find_byte( 
            const std::uint8_t byte, const std::size_t max_region, const bool up
        ) const { 
            auto ret = *this;

            return ret.self_find_byte( byte, max_region, up );
        }
    };
}