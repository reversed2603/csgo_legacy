#pragma once
constexpr auto non_jump_velocity = 140.f;

namespace csgo::hacks { 

	class c_sim_context { 
	public:
		void handle_context( extrapolation_data_t& data );
		void air_move( extrapolation_data_t& data );
		void try_touch_ground_in_quad( extrapolation_data_t& data, const sdk::vec3_t& start, const sdk::vec3_t& end, game::trace_t* trace );
	protected:
		__forceinline void check_velocity( extrapolation_data_t& data ) const;
		__forceinline void trace_player_bbox( 
			extrapolation_data_t& data, sdk::vec3_t& start, sdk::vec3_t& end, game::trace_t* trace ) const;
		__forceinline void friction( extrapolation_data_t& data ) const;
		__forceinline void air_accelerate( extrapolation_data_t& data, sdk::vec3_t& wish_dir, float wish_spd ) const;
		__forceinline void try_player_move( extrapolation_data_t& data ) const;
		__forceinline void walk_move( extrapolation_data_t& data ) const;
		__forceinline void accelerate( extrapolation_data_t& data, sdk::vec3_t& wish_dir, float wish_spd, float accel ) const;
		__forceinline bool categorize_pos( extrapolation_data_t& data );
		__forceinline void try_touch_ground( 
			const sdk::vec3_t& start, const sdk::vec3_t& end, const sdk::vec3_t& mins, const sdk::vec3_t& maxs, game::trace_t* trace );
	};

	const auto g_sim_ctx = std::make_unique < c_sim_context > ( );
}

#include "impl/sim_ctx.inl"