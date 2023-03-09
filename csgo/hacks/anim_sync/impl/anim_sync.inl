#pragma once
#include "../anim_sync.h"

namespace csgo::hacks {
	ALWAYS_INLINE float c_resolver::get_away_angle ( lag_record_t* current ) {
		float  delta { std::numeric_limits< float >::max ( ) };
		sdk::vec3_t pos {};
		sdk::qang_t  away {};

		sdk::vec_angs ( g_local_player->self ( )->origin ( ) - current->m_origin, away );
		return away.y ( );
	}

	ALWAYS_INLINE bool c_resolver::is_last_move_valid( cc_def( lag_record_t* ) current, float yaw ) {
		sdk::qang_t away {};
		sdk::vec_angs( g_local_player->self ( )->origin ( ) - current.get ( )->m_origin, away );
		const float delta = std::abs( sdk::norm_yaw( away.y ( ) - yaw ) );
		return delta > crypt_float ( 45.f ) && delta < crypt_float ( 135.f );
	}
}