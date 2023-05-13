#pragma once
namespace csgo::hacks { 

	class c_chams { 
	protected:
		game::c_material* m_reg_mat { };
		game::c_material* m_flat_mat { };
		game::c_material* m_glow_mat { };
		game::c_material* m_glow_overlay_mat { };
		game::c_material* m_metallic_mat { };
		struct cfg_t { 
			bool m_local_chams { }, m_local_chams_overlay{ }, m_arms_chams { }, m_wpn_chams{ }, m_history_chams { };
			bool m_ragdoll_chams{ };
			bool m_enemy_chams{ }, m_enemy_chams_overlay{ };
			bool m_enemy_chams_invisible{ }, m_enemy_chams_overlay_invisible{ };
			float m_enemy_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_invisible_enemy_clr[ 4 ] { 1.f, 1.f, 1.f, 1.f };
			float m_enemy_clr_overlay[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_invisible_enemy_clr_overlay[ 4 ] { 1.f, 1.f, 1.f, 1.f };

			bool m_shot_chams{ }, m_shot_chams_overlay{ };
			bool m_shot_chams_invisible{ }, m_shot_chams_overlay_invisible{ };
			float m_shot_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_invisible_shot_clr[ 4 ] { 1.f, 1.f, 1.f, 1.f };
			float m_shot_clr_overlay[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_invisible_shot_clr_overlay[ 4 ] { 1.f, 1.f, 1.f, 1.f };

			float m_local_overlay[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			int m_local_overlay_type{ };
			int m_enemy_chams_type{ }, m_invisible_enemy_chams_type{ };
			int m_enemy_chams_overlay_type{ }, m_invisible_enemy_chams_overlay_type{ };

			int m_shot_chams_type{ }, m_invisible_shot_chams_type{ };
			int m_shot_chams_overlay_type{ }, m_invisible_shot_chams_overlay_type{ };

			int m_local_chams_type{ }, m_arms_chams_type { }, m_wpn_chams_type { }, m_history_chams_type { };
			float m_local_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_local_overlay_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				  m_arms_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_wpn_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				  m_history_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b32u, { } };

	public:
		float m_total_distance{ };
		void init_chams( );
		bool draw_mdl( void* ecx, uintptr_t ctx, const game::draw_model_state_t& state, const game::model_render_info_t& info, sdk::mat3x4_t* bone );
		std::optional< game::bones_t > try_to_lerp_bones( player_entry_t entry ) const;
		void override_mat( int mat_type, sdk::col_t col, bool ignore_z, bool is_overlay = false );
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
	};

	inline const std::unique_ptr < c_chams > g_chams = std::make_unique < c_chams > ( );
}