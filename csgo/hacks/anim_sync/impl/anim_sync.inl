#pragma once
#include "../anim_sync.h"

namespace csgo::hacks {

	class adaptive_angle {
	public:
		float m_yaw;
		float m_dist;

	public:
		// ctor.
		__forceinline adaptive_angle( float yaw, float penalty = 0.f ) {
			// set yaw.
			m_yaw = sdk::norm_yaw( yaw );

			// init distance.
			m_dist = 0.f;

			// remove penalty.
			m_dist -= penalty;
		}
	};

	__forceinline float c_resolver::get_away_angle( lag_record_t* current ) {
		sdk::qang_t  away { };
		sdk::vec_angs( g_local_player->self( )->origin( ) - current->m_origin, away );
		return away.y( );
	}

	__forceinline bool c_resolver::is_sideways( cc_def( lag_record_t* ) current, float yaw, bool high_delta ) {
		sdk::qang_t away { };
		sdk::vec_angs( g_local_player->self( )->origin( ) - current.get( )->m_origin, away );
		const float delta = std::abs( sdk::angle_diff( away.y( ), yaw ) );

		// note: 180 - 40 != 150
		return high_delta ? ( delta > crypt_float( 40.f ) && delta < crypt_float( 140.f ) ) : ( delta > crypt_float( 25.f ) && delta < crypt_float( 165.f ) );
	}
}