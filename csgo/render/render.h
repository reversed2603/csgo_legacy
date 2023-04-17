#pragma once
#include <mutex>
namespace csgo {

	class c_render
	{
	public:

		sdk::vec2_t get_text_size( std::string_view txt, ImFont* font );

		void text( std::string_view txt, sdk::vec2_t pos, const sdk::col_t& clr, ImFont* font, bool should_outline, bool should_center_x, bool should_center_y, bool lower_alpha = false, bool drop_shadow = false );

		void line( const sdk::vec2_t& from, const sdk::vec2_t& to, const sdk::col_t& clr );
		void draw_line( float x1, float y1, float x2, float y2, sdk::col_t clr, float thickness = 1.f );

		void rect( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr, float rounding = 0.f, bool multiplied = false );

		void rect_filled( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr, float rounding = 0.f, bool multiplied = false );

		void rect_filled_multi_clr( const sdk::vec2_t& pos, const sdk::vec2_t& size, const sdk::col_t& clr_upr_left, const sdk::col_t& clr_upr_right, const sdk::col_t& clr_bot_left, const sdk::col_t& clr_bot_right );

		void polygon( const std::vector<sdk::vec2_t>& points, const sdk::col_t& clr );

		void polygon_filled( const std::vector<sdk::vec2_t>& points, const sdk::col_t& clr );

		void triangle( float x1, float y1, float x2, float y2, float x3, float y3, sdk::col_t clr, float thickness );

		void triangle_filled( float x1, float y1, float x2, float y2, float x3, float y3, sdk::col_t clr );

		void render_filled_3d_circle( const sdk::vec3_t& origin, float radius, sdk::col_t color );

		void render_convex_poly_filled( ImVec2* vec, int num_points, sdk::col_t col );

		void render_3d_circle( const sdk::vec3_t& origin, float radius, sdk::col_t color );

		void arc( float x, float y, float radius, float min_angle, float max_angle, sdk::col_t col, float thickness );

		bool world_to_screen( const sdk::vec3_t& in, sdk::vec3_t& out );

		void draw_rect_filled( float x, float y, float w, float h, sdk::col_t clr, float rounding, ImDrawCornerFlags rounding_corners = 15 );

		void draw_rect( float x1, float y1, float x2, float y2, sdk::col_t color );

		__forceinline void add_to_draw_list( ) {
			const auto lock = std::unique_lock<std::mutex>( m_mutex, std::try_to_lock );
			if( lock.owns_lock( ) ) {
				*m_replace_draw_list = *m_data_draw_list;
			}

			*ImGui::GetBackgroundDrawList( ) = *m_replace_draw_list;
		}

		std::mutex m_mutex;

		sdk::vec2_t m_screen_size;
		std::shared_ptr < ImDrawList > m_draw_list { };
		std::shared_ptr < ImDrawList > m_data_draw_list{ };
		std::shared_ptr < ImDrawList > m_replace_draw_list{ };
	};

	inline const std::unique_ptr < c_render > g_render = std::make_unique < c_render >( );

}