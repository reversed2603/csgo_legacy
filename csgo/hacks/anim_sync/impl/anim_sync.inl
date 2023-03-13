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
}