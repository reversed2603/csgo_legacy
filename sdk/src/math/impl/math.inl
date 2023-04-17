#pragma once
#include <DirectXMath.h>
#include "../math.hpp"

namespace sdk {
    template < typename _value_t >
        requires std::is_arithmetic_v< _value_t >
    ALWAYS_INLINE constexpr auto to_deg( const _value_t rad ) {
        using ret_t = detail::enough_float_t< _value_t >;

        return static_cast< ret_t >( rad * k_rad_pi< ret_t > );
    }

    template < typename _value_t >
        requires std::is_arithmetic_v< _value_t >
    ALWAYS_INLINE constexpr auto to_rad( const _value_t deg ) {
        using ret_t = detail::enough_float_t< _value_t >;

        return static_cast< ret_t >( deg * k_deg_pi< ret_t > );
    }

    template < typename _value_t >
        requires is_addable< _value_t, _value_t > && is_multipliable< _value_t, float >
    ALWAYS_INLINE constexpr _value_t lerp( const _value_t& from, const _value_t& to, const float amt ) {
        return from * ( 1.f - amt ) + to * amt;
    }

    template < typename _value_t >
        requires std::is_arithmetic_v< _value_t >
    ALWAYS_INLINE constexpr auto norm_yaw( _value_t ang ) {

            ang = fmod( ang, 360.0f );
            if( ang > 180.0f )
                ang -= 360.0f;
            if( ang < -180.0f )
                ang += 360.0f;

            return ang;
    }

	ALWAYS_INLINE void validate( sdk::vec3_t* const vec )
	{
		if( std::isnan( vec->x( ) )
			|| std::isnan( vec->y( ) )
			|| std::isnan( vec->z( ) ) ) {
			vec->x( ) = 0.f;
			vec->y( ) = 0.f;
			vec->z( ) = 0.f;
		}

		if( std::isinf( vec->x( ) )
			|| std::isinf( vec->y( ) )
			|| std::isinf( vec->z( ) ) ) {
			vec->x( ) = 0.f;
			vec->y( ) = 0.f;
			vec->z( ) = 0.f;
		}
	}

	ALWAYS_INLINE float approach_ang( float target, float val, float speed ) {
		target = ( target * 182.04445f ) * 0.0054931641f;
		val = ( val * 182.04445f ) * 0.0054931641f;

		if( speed < 0 )
			speed = -speed;

		float delta = target - val;
		if( delta < -180.0f )
			delta += 360.0f;
		else if( delta > 180.0f )
			delta -= 360.0f;

		if( delta > speed )
			val += speed;
		else if( delta < -speed )
			val -= speed;
		else
			val = target;

		return val;
	}

		ALWAYS_INLINE vec3_t cross_product( const vec3_t& a, const vec3_t& b )
		{
			return vec3_t( a.y( ) * b.z( ) - a.z( ) * b.y( ), a.z( ) * b.x( ) - a.x( ) * b.z( ), a.x( ) * b.y( ) - a.y( ) * b.x( ) );
		}

		template < typename _value_t >
		requires is_addable< _value_t, _value_t >&& is_multipliable< _value_t, float >
		ALWAYS_INLINE void vec_angs( const vec3_t& forward, vec3_t& up, qang_t& angles )
		{
			vec3_t left = cross_product( up, forward );
			left.normalize( );

			float forward_dist = forward.length( 2 );

			if( forward_dist > 0.001f )
			{
				angles.x( ) = atan2f( -forward.z( ), forward_dist ) * 180.0f / pi_f < float >;
				angles.y( ) = atan2f( forward.y( ), forward.x( ) ) * 180.0f / pi_f < float >;

				float up_z = ( left.y( ) * forward.x( ) ) - ( left.x( ) * forward.y( ) );
				angles.z( ) = atan2f( left.z( ), up_z ) * 180.0f / pi_f < float >;
			}
			else
			{
				angles.x( ) = atan2f( -forward.z( ), forward_dist ) * 180.0f / pi_f < float >;
				angles.y( ) = atan2f( -left.x( ), left.y( ) ) * 180.0f / pi_f < float >;
				angles.z( ) = 0;
			}
		}

		double ALWAYS_INLINE __declspec( naked ) __fastcall sin( double x )
		{
			__asm {
				fld	qword ptr[ esp + 4 ]
				fsin
				ret	8
			}
		}

		double ALWAYS_INLINE __declspec( naked ) __fastcall cos( double x )
		{
			__asm {
				fld	qword ptr[ esp + 4 ]
				fcos
				ret	8
			}
		}

		ALWAYS_INLINE void sin_cos( float rad, float& sin_, float& cos_ )
		{
			sin_ = sin( rad );
			cos_ = cos( rad );
		}

		ALWAYS_INLINE float segment_to_segment( const vec3_t& s1, const vec3_t& s2, const vec3_t& k1, const vec3_t& k2 ) {
			const auto u = s2 - s1, v = k2 - k1, w = s1 - k1;

			const auto a = u.dot( u );
			const auto b = u.dot( v );
			const auto c = v.dot( v );
			const auto d = u.dot( w );
			const auto e = v.dot( w );
			const auto unk = a * c - b * b;

			auto sn = 0.f, tn = 0.f, sd = unk, td = unk;

			if( unk < 0.00000001f ) {
				sn = 0.f;
				sd = 1.f;
				tn = e;
				td = c;
			}
			else {
				sn = b * e - c * d;
				tn = a * e - b * d;

				if( sn < 0.f ) {
					sn = 0.f;
					tn = e;
					td = c;
				}
				else if( sn > sd ) {
					sn = sd;
					tn = e + b;
					td = c;
				}
			}

			if( tn < 0.f ) {
				tn = 0.f;

				if( -d < 0.f ) {
					sn = 0.f;
				}
				else if( -d > a ) {
					sn = sd;
				}
				else {
					sn = -d;
					sd = a;
				}
			}
			else if( tn > td ) {
				tn = td;

				if( -d + b < 0.f ) {
					sn = 0.f;
				}
				else if( -d + b > a ) {
					sn = sd;
				}
				else {
					sn = -d + b;
					sd = a;
				}
			}

			const auto sc = std::abs( sn ) < 0.00000001f ? 0.f : sn / sd;
			const auto tc = std::abs( tn ) < 0.00000001f ? 0.f : tn / td;

			return ( w + u * sc - v * tc ).length( );
		}
		ALWAYS_INLINE bool intersect_bounding_box( vec3_t& src, vec3_t& dir, vec3_t& min, vec3_t& max ) {
			constexpr auto NUMDIM = 3;
			constexpr auto RIGHT = 0;
			constexpr auto LEFT = 1;
			constexpr auto MIDDLE = 2;

			bool inside = true;
			char quadrant[ NUMDIM ];
			int i;

			// Rind candidate planes; this loop can be avoided if
			// rays cast all from the eye( assume perpsective view )
			sdk::vec3_t candidatePlane;
			for( i = 0; i < NUMDIM; i++ ) {
				if( src.at( i ) < min.at( i ) ) {
					quadrant[ i ] = LEFT;
					candidatePlane.at( i ) = min.at( i );
					inside = false;
				}
				else if( src.at( i ) > max.at( i ) ) {
					quadrant[ i ] = RIGHT;
					candidatePlane.at( i ) = max.at( i );
					inside = false;
				}
				else {
					quadrant[ i ] = MIDDLE;
				}
			}

			// Ray origin inside bounding box
			if( inside ) {
				return true;
			}

			// Calculate T distances to candidate planes
			sdk::vec3_t maxT;
			for( i = 0; i < NUMDIM; i++ ) {
				if( quadrant[ i ] != MIDDLE && dir.at( i ) != 0.f )
					maxT.at( i ) = ( candidatePlane.at( i ) - src.at( i ) ) / dir.at( i );
				else
					maxT.at( i ) = -1.f;
			}

			// Get largest of the maxT's for final choice of intersection
			int whichPlane = 0;
			for( i = 1; i < NUMDIM; i++ ) {
				if( maxT.at( whichPlane ) < maxT.at( i ) )
					whichPlane = i;
			}

			// Check final candidate actually inside box
			if( maxT.at( whichPlane ) < 0.f )
				return false;

			for( i = 0; i < NUMDIM; i++ ) {
				if( whichPlane != i ) {
					float temp = src.at( i ) + maxT.at( whichPlane ) * dir.at( i );
					if( temp < min.at( i ) || temp > max.at( i ) ) {
						return false;
					}

				}
			}

			// ray hits box
			return true;
		}

		ALWAYS_INLINE bool intersect_bb( vec3_t& start, vec3_t& delta, vec3_t& min, vec3_t& max )
		{
			float d1, d2, f;
			auto start_solid = true;
			auto t1 = -1.0, t2 = 1.0;

			const float _start [ 3 ] = { start.x( ), start.y( ), start.z( ) };
			const float _delta [ 3 ] = { delta.x( ), delta.y( ), delta.z( ) };
			const float mins [ 3 ] = { min.x( ), min.y( ), min.z( ) };
			const float maxs [ 3 ] = { max.x( ), max.y( ), max.z( ) };

			for( auto i = 0; i < 6; ++i ) {
				if( i >= 3 ) {
					const auto j = ( i - 3 );

					d1 = _start [ j ] - maxs [ j ];
					d2 = d1 + _delta [ j ];
				}
				else {
					d1 = -_start [ i ] + mins [ i ];
					d2 = d1 - _delta [ i ];
				}

				if( d1 > 0 && d2 > 0 ) {
					start_solid = false;
					return false;
				}

				if( d1 <= 0 && d2 <= 0 )
					continue;

				if( d1 > 0 )
					start_solid = false;

				if( d1 > d2 ) {
					f = d1;
					if( f < 0 )
						f = 0;

					f /= d1 - d2;
					if( f > t1 )
						t1 = f;
				}
				else {
					f = d1 /( d1 - d2 );
					if( f < t2 )
						t2 = f;
				}
			}

			return start_solid || ( t1 < t2&& t1 >= 0.0f );
		}

		ALWAYS_INLINE bool intersect( vec3_t start, vec3_t end, vec3_t a, vec3_t b, float radius )
		{
			const auto dist = segment_to_segment( start, end, a, b );
			return ( dist < radius );
		}

		ALWAYS_INLINE void ang_vecs( const qang_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up )
		{
			vec3_t cos, sin;

			sin_cos( to_rad( angles.x( ) ), sin.x( ), cos.x( ) );
			sin_cos( to_rad( angles.y( ) ), sin.y( ), cos.y( ) );
			sin_cos( to_rad( angles.z( ) ), sin.z( ), cos.z( ) );

			if( forward ) {
				forward->x( ) = cos.x( ) * cos.y( );
				forward->y( ) = cos.x( ) * sin.y( );
				forward->z( ) = -sin.x( );
			}

			if( right ) {
				right->x( ) = -sin.z( ) * sin.x( ) * cos.y( ) + -cos.z( ) * -sin.y( );
				right->y( ) = -sin.z( ) * sin.x( ) * sin.y( ) + -cos.z( ) * cos.y( );
				right->z( ) = -sin.z( ) * cos.x( );
			}

			if( up ) {
				up->x( ) = cos.z( ) * sin.x( ) * cos.y( ) + -sin.z( ) * -sin.y( );
				up->y( ) = cos.z( ) * sin.x( ) * sin.y( ) + -sin.z( ) * cos.y( );
				up->z( ) = cos.z( ) * cos.x( );
			}
		}

		ALWAYS_INLINE float calc_fov( const qang_t& view_angles, const vec3_t& src, const vec3_t& dst ) {
			const auto dir = ( dst - src ).normalized( );

			vec3_t fwd{ };

			ang_vecs( view_angles, &fwd, nullptr, nullptr );

			return std::max( to_deg( std::acos( fwd.dot( dir ) ) ), 0.f );
		}

		template <typename t>
		static t lerp_dir( float progress, const t& t1, const t& t2 ) {
			return t1 + ( t2 - t1 ) * progress;
		}

		ALWAYS_INLINE float angle_diff( float f, float w ) {
			float delta;

			delta = fmodf( f - w, 360.0f );
			if( f > w ) {
				if( delta >= 180 )
					delta -= 360;
			}
			else {
				if( delta <= -180 )
					delta += 360;
			}
			return delta;
		}

		ALWAYS_INLINE void concat_transforms( const mat3x4_t& in0, const mat3x4_t& in1, mat3x4_t& out ) {
			out [ 0 ][ 0 ] = in0 [ 0 ][ 0 ] * in1 [ 0 ][ 0 ] + in0 [ 0 ][ 1 ] * in1 [ 1 ][ 0 ] + in0 [ 0 ][ 2 ] * in1 [ 2 ][ 0 ];
			out [ 0 ][ 1 ] = in0 [ 0 ][ 0 ] * in1 [ 0 ][ 1 ] + in0 [ 0 ][ 1 ] * in1 [ 1 ][ 1 ] + in0 [ 0 ][ 2 ] * in1 [ 2 ][ 1 ];
			out [ 0 ][ 2 ] = in0 [ 0 ][ 0 ] * in1 [ 0 ][ 2 ] + in0 [ 0 ][ 1 ] * in1 [ 1 ][ 2 ] + in0 [ 0 ][ 2 ] * in1 [ 2 ][ 2 ];
			out [ 0 ][ 3 ] = in0 [ 0 ][ 0 ] * in1 [ 0 ][ 3 ] + in0 [ 0 ][ 1 ] * in1 [ 1 ][ 3 ] + in0 [ 0 ][ 2 ] * in1 [ 2 ][ 3 ] + in0 [ 0 ][ 3 ];

			out [ 1 ][ 0 ] = in0 [ 1 ][ 0 ] * in1 [ 0 ][ 0 ] + in0 [ 1 ][ 1 ] * in1 [ 1 ][ 0 ] + in0 [ 1 ][ 2 ] * in1 [ 2 ][ 0 ];
			out [ 1 ][ 1 ] = in0 [ 1 ][ 0 ] * in1 [ 0 ][ 1 ] + in0 [ 1 ][ 1 ] * in1 [ 1 ][ 1 ] + in0 [ 1 ][ 2 ] * in1 [ 2 ][ 1 ];
			out [ 1 ][ 2 ] = in0 [ 1 ][ 0 ] * in1 [ 0 ][ 2 ] + in0 [ 1 ][ 1 ] * in1 [ 1 ][ 2 ] + in0 [ 1 ][ 2 ] * in1 [ 2 ][ 2 ];
			out [ 1 ][ 3 ] = in0 [ 1 ][ 0 ] * in1 [ 0 ][ 3 ] + in0 [ 1 ][ 1 ] * in1 [ 1 ][ 3 ] + in0 [ 1 ][ 2 ] * in1 [ 2 ][ 3 ] + in0 [ 1 ][ 3 ];

			out [ 2 ][ 0 ] = in0 [ 2 ][ 0 ] * in1 [ 0 ][ 0 ] + in0 [ 2 ][ 1 ] * in1 [ 1 ][ 0 ] + in0 [ 2 ][ 2 ] * in1 [ 2 ][ 0 ];
			out [ 2 ][ 1 ] = in0 [ 2 ][ 0 ] * in1 [ 0 ][ 1 ] + in0 [ 2 ][ 1 ] * in1 [ 1 ][ 1 ] + in0 [ 2 ][ 2 ] * in1 [ 2 ][ 1 ];
			out [ 2 ][ 2 ] = in0 [ 2 ][ 0 ] * in1 [ 0 ][ 2 ] + in0 [ 2 ][ 1 ] * in1 [ 1 ][ 2 ] + in0 [ 2 ][ 2 ] * in1 [ 2 ][ 2 ];
			out [ 2 ][ 3 ] = in0 [ 2 ][ 0 ] * in1 [ 0 ][ 3 ] + in0 [ 2 ][ 1 ] * in1 [ 1 ][ 3 ] + in0 [ 2 ][ 2 ] * in1 [ 2 ][ 3 ] + in0 [ 2 ][ 3 ];
		}
		ALWAYS_INLINE float dot( sdk::vec3_t from, const sdk::vec3_t& v ) {
			return ( from.x( ) * v.x( ) + from.y( ) * v.y( ) + from.z( ) * v.z( ) );
		}

		ALWAYS_INLINE float dot( const sdk::vec3_t from, const float* v ) {
			return ( from.x( ) * v [ 0 ] + from.y( ) * v [ 1 ] + from.z( ) * v [ 2 ] );
		}
		static const __m128 signmask = _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ) );

		static const __declspec( align( 16 ) ) float null [ 4 ] = { 0.f, 0.f, 0.f, 0.f };
		static const __declspec( align( 16 ) ) float _pi2 [ 4 ] = { 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f };
		static const __declspec( align( 16 ) ) float _pi [ 4 ] = { 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f };

		typedef __declspec( align( 16 ) ) union {
			float f [ 4 ];
			__m128 v;
		} m128;

		ALWAYS_INLINE __m128 sqrt_ps( const __m128 squared )
		{
			return _mm_sqrt_ps( squared );
		}

		ALWAYS_INLINE __m128 cos_52s_ps( const __m128 x )
		{
			const auto c1 = _mm_set1_ps( 0.9999932946f );
			const auto c2 = _mm_set1_ps( -0.4999124376f );
			const auto c3 = _mm_set1_ps( 0.0414877472f );
			const auto c4 = _mm_set1_ps( -0.0012712095f );
			const auto x2 = _mm_mul_ps( x, x );
			return _mm_add_ps( c1, _mm_mul_ps( x2, _mm_add_ps( c2, _mm_mul_ps( x2, _mm_add_ps( c3, _mm_mul_ps( c4, x2 ) ) ) ) ) );
		}
		static const float invtwopi = 0.1591549f;
		static const float twopi = 6.283185f;
		static const float threehalfpi = 4.7123889f;
		static const float pi = 3.14159265358979323846;
		static const float halfpi = 1.570796f;
		ALWAYS_INLINE __m128 cos_ps( __m128 angle )
		{
			angle = _mm_andnot_ps( signmask, angle );
			angle = _mm_sub_ps( angle, _mm_mul_ps( _mm_cvtepi32_ps( _mm_cvttps_epi32( _mm_mul_ps( angle, _mm_set1_ps( invtwopi ) ) ) ), _mm_set1_ps( twopi ) ) );

			auto cosangle = angle;
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( halfpi ) ), _mm_xor_ps( cosangle, _mm_sub_ps( _mm_set1_ps( pi ), angle ) ) ) );
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( pi ) ), signmask ) );
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( threehalfpi ) ), _mm_xor_ps( cosangle, _mm_sub_ps( _mm_set1_ps( twopi ), angle ) ) ) );

			auto result = cos_52s_ps( cosangle );
			result = _mm_xor_ps( result, _mm_and_ps( _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( halfpi ) ), _mm_cmplt_ps( angle, _mm_set1_ps( threehalfpi ) ) ), signmask ) );
			return result;
		}

		ALWAYS_INLINE __m128 sin_ps( const __m128 angle )
		{
			return cos_ps( _mm_sub_ps( _mm_set1_ps( halfpi ), angle ) );
		}

		ALWAYS_INLINE void sincos_ps( __m128 angle, __m128* sin, __m128* cos )
		{
			const auto anglesign = _mm_or_ps( _mm_set1_ps( 1.f ), _mm_and_ps( signmask, angle ) );
			angle = _mm_andnot_ps( signmask, angle );
			angle = _mm_sub_ps( angle, _mm_mul_ps( _mm_cvtepi32_ps( _mm_cvttps_epi32( _mm_mul_ps( angle, _mm_set1_ps( invtwopi ) ) ) ), _mm_set1_ps( twopi ) ) );

			auto cosangle = angle;
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( halfpi ) ), _mm_xor_ps( cosangle, _mm_sub_ps( _mm_set1_ps( pi ), angle ) ) ) );
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( pi ) ), signmask ) );
			cosangle = _mm_xor_ps( cosangle, _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( threehalfpi ) ), _mm_xor_ps( cosangle, _mm_sub_ps( _mm_set1_ps( twopi ), angle ) ) ) );

			auto result = cos_52s_ps( cosangle );
			result = _mm_xor_ps( result, _mm_and_ps( _mm_and_ps( _mm_cmpge_ps( angle, _mm_set1_ps( halfpi ) ), _mm_cmplt_ps( angle, _mm_set1_ps( threehalfpi ) ) ), signmask ) );
			*cos = result;

			const auto sinmultiplier = _mm_mul_ps( anglesign, _mm_or_ps( _mm_set1_ps( 1.f ), _mm_and_ps( _mm_cmpgt_ps( angle, _mm_set1_ps( pi ) ), signmask ) ) );
			*sin = _mm_mul_ps( sinmultiplier, sqrt_ps( _mm_sub_ps( _mm_set1_ps( 1.f ), _mm_mul_ps( result, result ) ) ) );
		}


		ALWAYS_INLINE mat3x4_t angle_matrix( const sdk::qang_t& angles )
		{
			mat3x4_t result;

			m128 angle, sin, cos;
			angle.f [ 0 ] = sdk::to_rad( angles.x( ) );
			angle.f [ 1 ] = sdk::to_rad( angles.y( ) );
			angle.f [ 2 ] = sdk::to_rad( angles.z( ) );
			sincos_ps( angle.v, &sin.v, &cos.v );

			result [ 0 ][ 0 ] = cos.f [ 0 ] * cos.f [ 1 ];
			result [ 1 ][ 0 ] = cos.f [ 0 ] * sin.f [ 1 ];
			result [ 2 ][ 0 ] = -sin.f [ 0 ];

			const auto crcy = cos.f [ 2 ] * cos.f [ 1 ];
			const auto crsy = cos.f [ 2 ] * sin.f [ 1 ];
			const auto srcy = sin.f [ 2 ] * cos.f [ 1 ];
			const auto srsy = sin.f [ 2 ] * sin.f [ 1 ];

			result [ 0 ][ 1 ] = sin.f [ 0 ] * srcy - crsy;
			result [ 1 ][ 1 ] = sin.f [ 0 ] * srsy + crcy;
			result [ 2 ][ 1 ] = sin.f [ 2 ] * cos.f [ 0 ];

			result [ 0 ][ 2 ] = sin.f [ 0 ] * crcy + srsy;
			result [ 1 ][ 2 ] = sin.f [ 0 ] * crsy - srcy;
			result [ 2 ][ 2 ] = cos.f [ 2 ] * cos.f [ 0 ];

			return result;
		}
		ALWAYS_INLINE vec3_t calc_ang( const vec3_t& src, const vec3_t& dst ) {
			constexpr auto k_pi = 3.14159265358979323846f;
			constexpr auto k_rad_pi = 180.f / k_pi;

			vec3_t angles;

			vec3_t delta = src - dst;
			float hyp = delta.length( 2u );

			angles.y( ) = std::atanf( delta.y( ) / delta.x( ) ) * k_rad_pi;
			angles.x( ) = std::atanf( -delta.z( ) / hyp ) * -k_rad_pi;
			angles.z( ) = 0.0f;

			if( delta.x( ) >= 0.0f )
				angles.y( ) += 180.0f;

			return angles;
		}

		ALWAYS_INLINE vec3_t vector_rotate( const vec3_t& in1, const mat3x4_t& in2 )
		{
			return vec3_t( dot( in1, in2 [ 0 ] ), dot( in1, in2 [ 1 ] ), dot( in1, in2 [ 2 ] ) );
		}
		ALWAYS_INLINE vec3_t vector_rotate( const vec3_t& in1, const sdk::qang_t& in2 )
		{
			const auto matrix = angle_matrix( in2 );
			return vector_rotate( in1, matrix );
		}

		ALWAYS_INLINE void vec_transform( const sdk::vec3_t& in, const sdk::mat3x4_t& matrix, sdk::vec3_t& out ) {
			out = {
				dot( in, sdk::vec3_t( matrix [ 0 ][ 0 ], matrix [ 0 ][ 1 ], matrix [ 0 ][ 2 ] ) ) + matrix [ 0 ][ 3 ],
				dot( in, sdk::vec3_t( matrix [ 1 ][ 0 ], matrix [ 1 ][ 1 ], matrix [ 1 ][ 2 ] ) ) + matrix [ 1 ][ 3 ],
				dot( in,  sdk::vec3_t( matrix [ 2 ][ 0 ], matrix [ 2 ][ 1 ], matrix [ 2 ][ 2 ] ) ) + matrix [ 2 ][ 3 ]
			};
		}
		ALWAYS_INLINE void vector_transform( const vec3_t& in, const mat3x4_t& matrix, vec3_t& out ) {
			out = {
				dot( in, { matrix [ 0 ][ 0 ], matrix [ 0 ][ 1 ], matrix [ 0 ][ 2 ] } ) + matrix [ 0 ][ 3 ],
				dot( in, { matrix [ 1 ][ 0 ], matrix [ 1 ][ 1 ], matrix [ 1 ][ 2 ] } ) + matrix [ 1 ][ 3 ],
				dot( in, { matrix [ 2 ][ 0 ], matrix [ 2 ][ 1 ], matrix [ 2 ][ 2 ] } ) + matrix [ 2 ][ 3 ]
			};
		}

		ALWAYS_INLINE void vec_angs( const vec3_t forward, qang_t& angles )
		{
			float temp, yaw, pitch;

			if( forward.y( ) == 0 && forward.x( ) == 0 )
			{
				yaw = 0;

				pitch = 90;
				if( forward.z( ) > 0 )
					pitch = 270;
			}
			else
			{
				yaw = ( atan2( forward.y( ), forward.x( ) ) * 180 / DirectX::XM_PI );
				if( yaw < 0 )
					yaw += 360;

				temp = sqrt( forward.x( ) * forward.x( ) + forward.y( ) * forward.y( ) );
				pitch = ( atan2( -forward.z( ), temp ) * 180 / DirectX::XM_PI );
				if( pitch < 0 )
					pitch += 360;
			}

			angles.x( ) = pitch;
			angles.y( ) = yaw;
			angles.z( ) = 0;
		}
}