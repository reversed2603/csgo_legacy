#pragma once

namespace csgo::hacks {
	class c_weather { 
	public:
		void reset( );
		void update( );
		bool m_has_created_rain{ };
	};

	inline const std::unique_ptr < c_weather > g_weather = std::make_unique < c_weather > ( );
}