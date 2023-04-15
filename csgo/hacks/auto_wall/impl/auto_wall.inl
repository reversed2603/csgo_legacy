#pragma once
#include "../auto_wall.h"

namespace csgo::hacks {
	ALWAYS_INLINE bool c_auto_wall::is_breakable( valve::base_entity_t* entity ) {
		if( !entity )
			return false;

		/* 	if( !entity )
			return false;
		using fn_t = bool( __thiscall* )( decltype( entity ) );

		static uintptr_t take_dmg = *( uintptr_t* )( ( uintptr_t ) g_ctx->addresses( ).m_is_breakable + 38 );
		const uintptr_t take_dmg_backup = *( uint8_t* )( ( uintptr_t ) entity + take_dmg );

		const auto client_class = entity->networkable( )->client_class( );
		if( client_class ) {
			const char* name = client_class->m_network_name;

			if( name[ 1 ] != 'F'
				|| name[ 4 ] != 'c'
				|| name[ 5 ] != 'B'
				|| name[ 9 ] != 'h' ) {
				*( uint8_t* )( ( uintptr_t ) entity + take_dmg ) = 2;
			}
		}

		const bool result = reinterpret_cast< fn_t >( g_ctx->addresses( ).m_is_breakable )( entity );
		*( uint8_t* )( ( uintptr_t ) entity + take_dmg ) = take_dmg_backup;

		return result; */

		using fn_t = bool( __thiscall* )( decltype( entity ) );

		if( reinterpret_cast< fn_t >( g_ctx->addresses( ).m_is_breakable )( entity ) )
			return true;

		const auto cc = entity->networkable( )->client_class( );
		if( !cc )
			return false;

		return ( *reinterpret_cast< const std::uint32_t* >( cc->m_network_name ) == 'erBC'
			&& *reinterpret_cast< const std::uint32_t* >( cc->m_network_name + 7 ) == 'Selb' )
			|| ( *reinterpret_cast< const std::uint32_t* >( cc->m_network_name ) == 'saBC'
				&& *reinterpret_cast< const std::uint32_t* >( cc->m_network_name + 7 ) == 'ytit' );
	}
}