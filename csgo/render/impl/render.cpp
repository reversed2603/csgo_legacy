#include "../../csgo.hpp"
#define IM_FLOOR( _VAL )( ( float )( int )( _VAL ) ) 
namespace csgo 
{ 
	bool c_render::world_to_screen( const sdk::vec3_t& origin, sdk::vec3_t& screen )
	{ 
		const auto screen_transform = [ &origin, &screen ]( ) -> bool
		{ 
			static std::uintptr_t view_matrix;
			if( !view_matrix )
			{ 
				view_matrix = g_ctx->addresses( ).m_v_matrix;
				view_matrix += 3;
				view_matrix = *reinterpret_cast< std::uintptr_t* >( view_matrix );
				view_matrix += 176;
			}

			const sdk::v_matrix& w2s_matrix = *reinterpret_cast< sdk::v_matrix* >( view_matrix );
			screen.x( ) = w2s_matrix.m_value[ 0 ][ 0 ] * origin.x( ) + w2s_matrix.m_value[ 0 ][ 1 ] * origin.y( ) + w2s_matrix.m_value[ 0 ][ 2 ] * origin.z( ) + w2s_matrix.m_value[ 0 ][ 3 ];
			screen.y( ) = w2s_matrix.m_value[ 1 ][ 0 ] * origin.x( ) + w2s_matrix.m_value[ 1 ][ 1 ] * origin.y( ) + w2s_matrix.m_value[ 1 ][ 2 ] * origin.z( ) + w2s_matrix.m_value[ 1 ][ 3 ];

			float w = w2s_matrix.m_value[ 3 ][ 0 ] * origin.x( ) + w2s_matrix.m_value[ 3 ][ 1 ] * origin.y( ) + w2s_matrix.m_value[ 3 ][ 2 ] * origin.z( ) + w2s_matrix.m_value[ 3 ][ 3 ];

			if( w < 0.001f )
			{ 
				screen.x( ) *= 100000;
				screen.y( ) *= 100000;
				return true;
			}

			float invw = 1.f / w;
			screen.x( ) *= invw;
			screen.y( ) *= invw;

			return false;
		};

		if( !screen_transform( ) )
		{ 
			int screen_width, screen_height;
			game::g_engine->get_screen_size( screen_width, screen_height );

			screen.x( ) = ( screen_width * 0.5f ) + ( screen.x( ) * screen_width ) * 0.5f;
			screen.y( ) = ( screen_height * 0.5f ) - ( screen.y( ) * screen_height ) * 0.5f;

			return true;
		}

		return false;
	}

	sdk::vec2_t c_render::get_text_size( std::string_view txt, ImFont* font )
	{ 
		if( !font
			|| txt.empty( )
			|| !font->IsLoaded( ) )
			return sdk::vec2_t( );

		const auto size = font->CalcTextSizeA( font->FontSize, std::numeric_limits<float>::max( ), 0.f, txt.data( ) );

		return sdk::vec2_t( IM_FLOOR( size.x + 0.95f ), size.y );
	}

	void c_render::triangle( float x1, float y1, float x2, float y2, float x3, float y3, sdk::col_t clr, float thickness )
	{ 
		m_draw_list->AddTriangle( ImVec2( x1, y1 ), ImVec2( x2, y2 ), ImVec2( x3, y3 ), clr.hex( ), thickness );
	}

	void c_render::triangle_filled( float x1, float y1, float x2, float y2, float x3, float y3, sdk::col_t clr )
	{ 
		m_draw_list->AddTriangleFilled( ImVec2( x1, y1 ), ImVec2( x2, y2 ), ImVec2( x3, y3 ), clr.hex( ) );
	}

	void c_render::text( std::string_view txt, sdk::vec2_t pos, const sdk::col_t& clr, ImFont* font, bool should_outline, bool should_center_x, bool should_center_y, bool lower_alpha, bool drop_shadow )
	{ 
		if( !font
			|| txt.empty( )
			|| clr.a( ) <= 0
			|| !font->IsLoaded( ) )
			return;

		const auto centered_x = should_center_x;
		const auto centered_y = should_center_y;

		if( centered_x
			|| centered_y ) { 
			const auto text_size = get_text_size( txt, font );

			if( centered_x ) { 
				pos.x( ) -= text_size.x( ) / 2.f;
			}

			if( centered_y ) { 
				pos.y( ) -= text_size.y( ) / 2.f;
			}
		}
			
		m_draw_list->PushTextureID( font->ContainerAtlas->TexID );

		auto outline_alpha = std::move( clr.a( ) / 2.f );

		if( lower_alpha )
			outline_alpha = std::move( clr.a( ) / 2.5f );

		if( should_outline ) { 
			m_draw_list->AddTextOutline( font, font->FontSize, *reinterpret_cast< ImVec2* >( &pos ), clr.hex( ), txt.data( ), 0, outline_alpha );
		}
		else { 
			if( drop_shadow ) { 
				m_draw_list->AddText( font, font->FontSize, ImVec2( pos.x( ) + 1, pos.y( ) + 1 ), sdk::col_t( 0.f, 0.f, 0.f, outline_alpha ).hex( ), txt.data( ) );
			}
			m_draw_list->AddText( font, font->FontSize, *reinterpret_cast< ImVec2* >( &pos ), clr.hex( ), txt.data( ) );
		}

		m_draw_list->PopTextureID( );
	}

	void c_render::line( const sdk::vec2_t& from, const sdk::vec2_t& to, const sdk::col_t& clr )
	{ 
		m_draw_list->AddLine( *reinterpret_cast< const ImVec2* >( &from ), *reinterpret_cast< const ImVec2* >( &to ), clr.hex( ) );
	}

	void c_render::draw_line( float x1, float y1, float x2, float y2, sdk::col_t clr, float thickness )
	{ 
		m_draw_list->AddLine( ImVec2( x1, y1 ), ImVec2( x2, y2 ), clr.hex( ), thickness );
	}

	void c_render::rect( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr, float rounding, bool multiplied )
	{ 
		m_draw_list->AddRect( ImVec2( pos.x( ), pos.y( ) ), multiplied ? ImVec2( pos.x( ) + size.x( ), pos.y( ) + size.y( ) ) : ImVec2( size.x( ), size.y( ) ), clr.hex( ), rounding, 15, 0.3f );
	}

	void c_render::rect_filled( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr, float rounding, bool multiplied )
	{ 
		m_draw_list->AddRectFilled( ImVec2( pos.x( ), pos.y( ) ), multiplied ? ImVec2( pos.x( ) + size.x( ), pos.y( ) + size.y( ) ) : ImVec2( size.x( ), size.y( ) ), clr.hex( ), rounding );
	}

	void c_render::rect_filled_multi_clr( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr_upr_left, const sdk::col_t& clr_upr_right, const sdk::col_t& clr_bot_left, const sdk::col_t& clr_bot_right )
	{ 
		m_draw_list->AddRectFilledMultiColor( *reinterpret_cast< const ImVec2* >( &pos ), ImVec2( pos.x( ) + size.x( ), pos.y( ) + size.y( ) ), clr_upr_left.hex( ), clr_upr_right.hex( ), clr_bot_right.hex( ), clr_bot_left.hex( ) );
	}

	void c_render::draw_rect_filled( float x, float y, float w, float h, sdk::col_t clr, float rounding, ImDrawCornerFlags rounding_corners )
	{ 
		m_draw_list->AddRectFilled( ImVec2( x, y ), ImVec2( x + w, y + h ), clr.hex( ), rounding, rounding_corners );
	}

	void c_render::draw_rect( float x1, float y1, float x2, float y2, sdk::col_t color )
	{ 
		m_draw_list->AddRect( ImVec2( x1, y1 ), ImVec2( x2, y2 ), color.hex( ), 0.0f );
	}

	void c_render::polygon( const std::vector<sdk::vec2_t>& points, const sdk::col_t& clr )
	{ 
		if( clr.a( ) <= 0 )
			return;

		m_draw_list->_Path.reserve( m_draw_list->_Path.Size + points.size( ) + 1 );

		for( auto& point : points ) { 
			m_draw_list->_Path.push_back( *reinterpret_cast< const ImVec2* >( &point ) );
		}

		m_draw_list->PathStroke( clr.hex( ), true, 1.f );
	}

	void c_render::polygon_filled( const std::vector<sdk::vec2_t>& points, const sdk::col_t& clr )
	{ 
		if( clr.a( ) <= 0 )
			return;

		m_draw_list->_Path.reserve( m_draw_list->_Path.Size + points.size( ) + 1 );

		for( auto& point : points ) { 
			m_draw_list->_Path.push_back( *reinterpret_cast< const ImVec2* >( &point ) );
		}

		m_draw_list->PathFillConvex( clr.hex( ) );
	}

	void c_render::render_filled_3d_circle( const sdk::vec3_t& origin, float radius, sdk::col_t color )
	{ 
		static auto prev_screen_pos = sdk::vec3_t( );
		static auto step = sdk::pi * 2.0f / 72.0f;

		auto screen_pos = sdk::vec3_t( );
		auto screen = sdk::vec3_t( );

		if( !world_to_screen( origin, screen ) )
			return;

		for( auto rotation = 0.0f; rotation <= sdk::pi * 2.0f; rotation += step )
		{ 
			sdk::vec3_t pos( radius * cos( rotation ) + origin.x( ), radius * sin( rotation ) + origin.y( ), origin.z( ) );

			if( world_to_screen( pos, screen_pos ) )
			{ 
				if( !prev_screen_pos.is_zero( ) && prev_screen_pos.is_valid( ) && screen_pos.is_valid( ) && prev_screen_pos != screen_pos )
				{ 
					line( sdk::vec2_t( prev_screen_pos.x( ), prev_screen_pos.y( ) ), sdk::vec2_t( screen_pos.x( ), screen_pos.y( ) ), color );
					triangle( screen.x( ), screen.y( ), screen_pos.x( ), screen_pos.y( ), prev_screen_pos.x( ), prev_screen_pos.y( ), sdk::col_t( color.r( ), color.g( ), color.b( ), color.a( ) / 2 ), 1.f );
				}

				prev_screen_pos = screen_pos;
			}
		}
	}

	void c_render::render_3d_circle( const sdk::vec3_t& origin, float radius, sdk::col_t color )
	{ 
		static sdk::vec3_t previous_screen_pos = sdk::vec3_t( 0, 0, 0 );
		static float_t step = sdk::pi * 2.0f / 72.0f;

		sdk::vec3_t screen_position = sdk::vec3_t( 0, 0, 0 );
		if( !world_to_screen( origin, screen_position ) )
			return;

		for( float_t rotation = 0.0f; rotation <= sdk::pi * 2.0f; rotation += step )
		{ 
			sdk::vec3_t world_position = sdk::vec3_t( radius * cos( rotation ) + origin.x( ), radius * sin( rotation ) + origin.y( ), origin.z( ) );
			if( !world_to_screen( world_position, screen_position ) )
				continue;

			draw_line( previous_screen_pos.x( ), previous_screen_pos.y( ), screen_position.x( ), screen_position.y( ), color, 1.0f );
			triangle
			( 
				screen_position.x( ), screen_position.y( ),
				screen_position.x( ), screen_position.y( ),
				previous_screen_pos.x( ), previous_screen_pos.y( ),
				sdk::col_t( color.r( ), color.g( ), color.b( ), color.a( ) / 2 ), 1.f
			 );

			previous_screen_pos = screen_position;
		}
	}

	void c_render::arc( float x, float y, float radius, float min_angle, float max_angle, sdk::col_t col, float thickness )
	{ 
		m_draw_list->PathArcTo( ImVec2( x, y ), radius, sdk::to_rad( min_angle ), sdk::to_rad( max_angle ), 32 );
		m_draw_list->PathStroke( col.hex( ), false, thickness );
	}

	void c_render::render_convex_poly_filled( ImVec2* vec, int num_points, sdk::col_t col )
	{ 
		m_draw_list->AddConvexPolyFilled( vec, num_points, col.hex( ) );
	}

}
