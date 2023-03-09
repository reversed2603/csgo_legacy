#pragma once

namespace sdk {
    template < typename _ret_t >
        requires std::is_floating_point_v< _ret_t >
    inline constexpr auto k_pi = static_cast< _ret_t >( 3.141592653589793 );

    template < typename _ret_t >
        requires std::is_floating_point_v< _ret_t >
    inline constexpr auto k_pi2 = static_cast< _ret_t >( k_pi< double > * 2.0 );

    template < typename _ret_t >
        requires std::is_floating_point_v< _ret_t >
    inline constexpr auto k_rad_pi = static_cast< _ret_t >( 180.0 / k_pi< double > );

    template < typename _ret_t >
        requires std::is_floating_point_v< _ret_t >
    inline constexpr auto k_deg_pi = static_cast< _ret_t >( k_pi< double > / 180.0 );

    template < typename _ret_t >
        requires std::is_floating_point_v< _ret_t >
    inline constexpr auto pi_f = static_cast< float >( k_pi< double > );

    template < typename _lhs_t, typename _rhs_t >
    concept is_addable = requires ( const _lhs_t& lhs, const _rhs_t& rhs ) { lhs + rhs; };

    template < typename _lhs_t, typename _rhs_t >
    concept is_substractable = requires ( const _lhs_t& lhs, const _rhs_t& rhs ) { lhs - rhs; };

    template < typename _lhs_t, typename _rhs_t >
    concept is_multipliable = requires ( const _lhs_t& lhs, const _rhs_t& rhs ) { lhs * rhs; };

    template < typename _lhs_t, typename _rhs_t >
    concept is_dividable = requires ( const _lhs_t& lhs, const _rhs_t& rhs ) { lhs / rhs; };

    template < typename _value_t >
        requires std::is_arithmetic_v< _value_t >
    ALWAYS_INLINE constexpr auto to_deg( const _value_t rad );

    template < typename _value_t >
        requires std::is_arithmetic_v< _value_t >
    ALWAYS_INLINE constexpr auto to_rad( const _value_t deg );

    template < typename _value_t >
        requires is_addable< _value_t, _value_t > && is_multipliable< _value_t, float >
    ALWAYS_INLINE constexpr _value_t lerp( const _value_t& from, const _value_t& to, const float amt );
}

#include "detail/detail.hpp"

namespace sdk {
    using vec2_t = detail::base_vec_t< float, 2u >;

    using ivec2_t = detail::base_vec_t< int, 2u >;

    using s16vec2_t = detail::base_vec_t< short, 2u >;

    using vec3_t = detail::base_vec_t< float, 3u >;

    using qang_t = detail::base_qang_t< float >;

    using mat3x4_t = detail::base_mat_t< float, 3u, 4u >;

    using mat4x4_t = detail::base_mat_t< float, 4u, 4u >;

    using rect_t = detail::base_rect_t< float >;

    using argb_t = detail::base_argb_t<>;

    using ahsv_t = detail::base_ahsv_t<>;

	struct vec3_t_ {
		vec3_t_ ( ) = default;
		vec3_t_ ( float ix, float iy, float iz )
		{
			x = ix;
			y = iy;
			z = iz;
		}

		/* yooooooooooooooooooooooooooooooooo < > x = forward_move < > y = side_move < > z = up_move */

		float x = 0.f, y = 0.f, z = 0.f;

		__forceinline vec3_t_ operator+( const vec3_t_& value ) const { return vec3_t_ ( x + value.x, y + value.y, z + value.z ); }

		__forceinline vec3_t_ operator-( const vec3_t_& value ) const { return vec3_t_ ( x - value.x, y - value.y, z - value.z ); }

		__forceinline vec3_t_ operator-( float value ) const { return vec3_t_ ( x - value, y - value, z - value ); }

		__forceinline vec3_t_ operator+( float value ) const { return vec3_t_ ( x + value, y + value, z + value ); }

		__forceinline vec3_t_ operator/( float value ) const { return vec3_t_ ( x / value, y / value, z / value ); }

		__forceinline vec3_t_ operator*( float value ) const { return vec3_t_ ( x * value, y * value, z * value ); }

		__forceinline vec3_t_ operator-( ) const { return vec3_t_ ( -x, -y, -z ); }

		__forceinline vec3_t_& operator-=( const vec3_t_& value )
		{
			x -= value.x;
			y -= value.y;
			z -= value.z;

			return *this;
		}

		__forceinline vec3_t_& operator+=( const vec3_t_& value )
		{
			x += value.x;
			y += value.y;
			z += value.z;

			return *this;
		}

		__forceinline vec3_t_& operator/=( const vec3_t_& value )
		{
			x /= value.x;
			y /= value.y;
			z /= value.z;

			return *this;
		}

		__forceinline vec3_t_& operator*=( const vec3_t_& value )
		{
			x *= value.x;
			y *= value.y;
			z *= value.z;

			return *this;
		}

		__forceinline vec3_t_& operator/=( float value )
		{
			x /= value;
			y /= value;
			z /= value;

			return *this;
		}

		__forceinline vec3_t_& operator*=( float value )
		{
			x *= value;
			y *= value;
			z *= value;

			return *this;
		}

		__forceinline vec3_t_& normalize ( )
		{
			if ( const auto l = length ( ) ) {
				x /= l;
				y /= l;
				z /= l;
			}

			return *this;
		}

		__forceinline vec3_t_ normalized ( ) const
		{
			auto ret = *this;

			return ret.normalize ( );
		}

		float normalize_ ( ) const
		{
			vec3_t_ res = *this;
			float l = res.length ( );
			if ( l != 0.0f )
			{
				res /= l;
			}
			else
			{
				res.x = res.y = res.z = 0.0f;
			}
			return l;
		}

		__forceinline bool operator==( const vec3_t_& value ) const { return x == value.x && y == value.y && z == value.z; }

		__forceinline bool operator!=( const vec3_t_& value ) const { return !( operator==( value ) ); }

		__forceinline float& operator[]( int i ) { return reinterpret_cast< float* >( this )[ i ]; }

		__forceinline const float operator[]( int i ) const { return reinterpret_cast< const float* >( this )[ i ]; }

		__forceinline float length_sqr ( ) const { return x * x + y * y + z * z; }

		__forceinline float length ( ) const { return std::sqrt ( length_sqr ( ) ); }

		__forceinline float length_2d ( ) const { return std::sqrt ( x * x + y * y ); }

		__forceinline float dot_product ( const vec3_t_& value ) const { return x * value.x + y * value.y + z * value.z; }

		__forceinline float dot_product ( const float* value ) const { return x * value[ 0 ] + y * value[ 1 ] + z * value[ 2 ]; }

		__forceinline vec3_t_ cross_product ( const vec3_t_& value ) const { return vec3_t_ ( y * value.z - z * value.y, z * value.x - x * value.z, x * value.y - y * value.x ); }

		__forceinline float dist_to ( const vec3_t_& value ) const { return ( *this - value ).length ( ); }

		__forceinline float dist_to_2d ( const vec3_t_& value ) const { return ( *this - value ).length_2d ( ); }

		__forceinline bool is_valid ( ) const { return std::isfinite ( x ) && std::isfinite ( y ) && std::isfinite ( z ); }

		__forceinline float distance_squared ( const vec3_t_& v ) const
		{
			return ( ( *this - v ).length_sqr ( ) );
		}

		__forceinline bool is_zero ( )
		{
			return ( x > -0.01f && x < 0.01f &&
				y > -0.01f && y < 0.01f &&
				z > -0.01f && z < 0.01f );
		}

		__forceinline void normalize_in_place ( )
		{
			vec3_t_& v = *this;

			float iradius = 1.0f / ( this->length ( ) + FLT_EPSILON );

			v.x *= iradius;
			v.y *= iradius;
			v.z *= iradius;
		}


		__forceinline bool empty ( ) const { return x == 0.f && y == 0.f && z == 0.f; }
	};

	struct vec4_t : public vec3_t_ {
		vec4_t ( ) = default;
		vec4_t ( float ix, float iy, float iz, float iw )
		{
			x = ix;
			y = iy;
			z = iz;
			w = iw;
		}
		vec4_t ( const vec3_t_& value )
		{
			x = value.x;
			y = value.y;
			z = value.z;
			w = 0.f;
		}
		vec4_t ( const vec2_t& value0, const vec2_t& value1 )
		{
			x = value0.x ( );
			y = value0.y ( );
			z = value1.x ( );
			w = value1.y ( );
		}

		__forceinline vec4_t& operator=( const vec3_t_& value )
		{
			x = value.x;
			y = value.y;
			z = value.z;
			w = 0.f;

			return *this;
		}

		__forceinline vec4_t& operator=( const vec4_t& value )
		{
			x = value.x;
			y = value.y;
			z = value.z;
			w = value.w;

			return *this;
		}

		float w = 0.f;
	};

	struct v_matrix {
		__forceinline vec4_t& operator[]( int i ) { return m_value.at ( i ); }

		__forceinline const vec4_t& operator[]( int i ) const { return m_value.at ( i ); }

		std::array<vec4_t, 4u> m_value = {};
	};


	struct col_t {
		col_t ( ) = default;
		col_t ( int r, int g, int b ) { set ( r, g, b, 255 ); }
		col_t ( int r, int g, int b, int a ) { set ( r, g, b, a ); }
		col_t ( const col_t& col, int a ) { set ( col.r ( ), col.g ( ), col.b ( ), a ); }

		std::array<uint8_t, 4> m_value = {};

		__forceinline void set ( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) { m_value = { r, g, b, a }; }

		__forceinline int r ( ) const { return m_value.at ( 0u ); }

		__forceinline int g ( ) const { return m_value.at ( 1u ); }

		__forceinline int b ( ) const { return m_value.at ( 2u ); }

		__forceinline int a ( ) const { return m_value.at ( 3u ); }

		__forceinline uint32_t hex ( bool rgba = false ) const
		{
			return rgba
				? ( ( r ( ) & 0xFF ) << 24 ) + ( ( g ( ) & 0xFF ) << 16 ) + ( ( b ( ) & 0xFF ) << 8 ) + ( a ( ) & 0xFF )
				: ( ( a ( ) & 0xFF ) << 24 ) + ( ( b ( ) & 0xFF ) << 16 ) + ( ( g ( ) & 0xFF ) << 8 ) + ( r ( ) & 0xFF );
		}

		__forceinline col_t alpha ( int value ) const { return col_t ( r ( ), g ( ), b ( ), value ); }

		__forceinline col_t& operator=( const col_t& value )
		{
			set ( value.r ( ), value.g ( ), value.b ( ), value.a ( ) );

			return *this;
		}

		__forceinline col_t& operator-=( uint8_t value )
		{
			set ( r ( ) - value, g ( ) - value, b ( ) - value, a ( ) - value );

			return *this;
		}

		__forceinline col_t& operator+=( uint8_t value )
		{
			set ( r ( ) + value, g ( ) + value, b ( ) + value, a ( ) + value );

			return *this;
		}

		__forceinline col_t& operator/=( uint8_t value )
		{
			set ( r ( ) / value, g ( ) / value, b ( ) / value, a ( ) / value );

			return *this;
		}

		__forceinline col_t& operator*=( uint8_t value )
		{
			set ( r ( ) * value, g ( ) * value, b ( ) * value, a ( ) * value );

			return *this;
		}

		__forceinline col_t& operator-=( const col_t& value )
		{
			set ( r ( ) - value.r ( ), g ( ) - value.g ( ), b ( ) - value.b ( ), a ( ) - value.a ( ) );

			return *this;
		}

		__forceinline col_t& operator+=( const col_t& value )
		{
			set ( r ( ) + value.r ( ), g ( ) + value.g ( ), b ( ) + value.b ( ), a ( ) + value.a ( ) );

			return *this;
		}

		__forceinline col_t operator-( uint8_t value ) const { return col_t ( r ( ) - value, g ( ) - value, b ( ) - value, a ( ) - value ); }

		__forceinline col_t operator+( uint8_t value ) const { return col_t ( r ( ) + value, g ( ) + value, b ( ) + value, a ( ) + value ); }

		__forceinline col_t operator/( uint8_t value ) const { return col_t ( r ( ) / value, g ( ) / value, b ( ) / value, a ( ) / value ); }

		__forceinline col_t operator*( uint8_t value ) const { return col_t ( r ( ) * value, g ( ) * value, b ( ) * value, a ( ) * value ); }

		__forceinline col_t operator-( const col_t& value ) const { return col_t ( r ( ) - value.r ( ), g ( ) - value.g ( ), b ( ) - value.b ( ), a ( ) - value.a ( ) ); }

		__forceinline col_t operator+( const col_t& value ) const { return col_t ( r ( ) + value.r ( ), g ( ) + value.g ( ), b ( ) + value.b ( ), a ( ) + value.a ( ) ); }

		__forceinline bool operator==( const col_t& value ) const { return *const_cast< col_t* >( this ) == *const_cast< col_t* >( &value ); }

		__forceinline bool operator!=( const col_t& value ) const { return !( operator==( value ) ); }

		__forceinline float hue ( ) const
		{
			const auto red = r ( ) / 255.f;
			const auto green = g ( ) / 255.f;
			const auto blue = b ( ) / 255.f;

			const auto max = std::max<float> ( { red, green, blue } );
			const auto min = std::min<float> ( { red, green, blue } );

			if ( max == min )
				return 0.f;

			const auto delta = max - min;

			auto hue = 0.f;

			if ( max == red ) {
				hue = ( green - blue ) / delta;
			}
			else if ( max == green ) {
				hue = 2.f + ( blue - red ) / delta;
			}
			else {
				hue = 4.f + ( red - green ) / delta;
			}

			hue *= 60.f;

			if ( hue < 0.f ) {
				hue += 360.f;
			}

			return hue / 360.f;
		}

		__forceinline float saturation ( ) const
		{
			const auto red = r ( ) / 255.f;
			const auto green = g ( ) / 255.f;
			const auto blue = b ( ) / 255.f;

			const auto max = std::max<float> ( { red, green, blue } );
			const auto min = std::min<float> ( { red, green, blue } );

			const auto delta = max - min;

			if ( max == 0.f )
				return delta;

			return delta / max;
		}

		__forceinline float brightness ( ) const { return std::max<float> ( { r ( ) / 255.f, g ( ) / 255.f, b ( ) / 255.f } ); }

		__forceinline static col_t from_hsb ( float hue, float saturation, float brightness )
		{
			const auto h = hue == 1.f ? 0 : hue * 6.f;
			const auto f = h - static_cast< int >( h );
			const auto p = brightness * ( 1.f - saturation );
			const auto q = brightness * ( 1.f - saturation * f );
			const auto t = brightness * ( 1.f - ( saturation * ( 1.f - f ) ) );

			if ( h < 1.f ) {
				return col_t (
					static_cast< uint8_t >( brightness * 255.f ),
					static_cast< uint8_t >( t * 255.f ),
					static_cast< uint8_t >( p * 255.f )
				);
			}

			if ( h < 2.f ) {
				return col_t (
					static_cast< uint8_t >( q * 255 ),
					static_cast< uint8_t >( brightness * 255 ),
					static_cast< uint8_t >( p * 255 )
				);
			}

			if ( h < 3.f ) {
				return col_t (
					static_cast< uint8_t >( p * 255.f ),
					static_cast< uint8_t >( brightness * 255.f ),
					static_cast< uint8_t >( t * 255.f )
				);
			}

			if ( h < 4.f ) {
				return col_t (
					static_cast< uint8_t >( p * 255.f ),
					static_cast< uint8_t >( q * 255.f ),
					static_cast< uint8_t >( brightness * 255.f )
				);
			}

			if ( h < 5.f ) {
				return col_t (
					static_cast< uint8_t >( t * 255.f ),
					static_cast< uint8_t >( p * 255.f ),
					static_cast< uint8_t >( brightness * 255.f )
				);
			}

			return col_t (
				static_cast< uint8_t >( brightness * 255.f ),
				static_cast< uint8_t >( p * 255.f ),
				static_cast< uint8_t >( q * 255.f )
			);
		}

		struct palette_t {
			static col_t red ( int alpha = 255 ) { return col_t ( 255, 0, 0, alpha ); }
			static col_t green ( int alpha = 255 ) { return col_t ( 0, 255, 0, alpha ); }
			static col_t blue ( int alpha = 255 ) { return col_t ( 0, 0, 255, alpha ); }

			static col_t white ( int alpha = 255 ) { return col_t ( 255, 255, 255, alpha ); }
			static col_t black ( int alpha = 255 ) { return col_t ( 13, 13, 13, alpha ); }
			static col_t grey ( int alpha = 255 ) { return col_t ( 60, 60, 60, alpha ); }
			static col_t orange ( int alpha = 255 ) { return col_t ( 245, 171, 53, alpha ); }

			static col_t light_black ( int alpha = 255 ) { return col_t ( 32, 32, 32, alpha ); }
			static col_t light_grey ( int alpha = 255 ) { return col_t ( 77, 77, 77, alpha ); }
			static col_t light_green ( int alpha = 255 ) { return col_t ( 127, 255, 0, alpha ); }
			static col_t light_red ( int alpha = 255 ) { return col_t ( 236, 98, 95, alpha ); }
			static col_t light_blue ( int alpha = 255 ) { return col_t ( 42, 87, 233, alpha ); }
			static col_t light_violet ( int alpha = 255 ) { return col_t ( 153, 153, 255, alpha ); }
			static col_t light_purple ( int alpha = 255 ) { return col_t ( 127, 73, 208, alpha ); }

			static col_t dark_grey ( int alpha = 255 ) { return col_t ( 44, 44, 44, alpha ); }
			static col_t dark_blue ( int alpha = 255 ) { return col_t ( 14, 18, 134, alpha ); }

			static col_t purple ( int alpha = 255 ) { return col_t ( 220, 0, 220, alpha ); }
			static col_t pink ( int alpha = 255 ) { return col_t ( 255, 105, 180, alpha ); }
		};
	};

}

#include "impl/math.inl"