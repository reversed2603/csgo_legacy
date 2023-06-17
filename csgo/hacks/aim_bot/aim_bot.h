﻿#pragma once

namespace csgo::hacks { 

	enum e_hit_scan_mode { 
		normal,
		prefer
	};

	struct hit_box_data_t { 
		game::e_hitbox         m_index;
		e_hit_scan_mode m_mode;

		__forceinline bool operator== ( const hit_box_data_t& c ) const { 
			return m_index == c.m_index && m_mode == c.m_mode;
		}
	};

	struct point_t { 
		__forceinline point_t( ) = default;

		__forceinline point_t( sdk::vec3_t& pos, game::e_hitbox index, bool center ) { 
			m_pos = pos;
			m_index = index;
			m_center = center;

			m_valid = false;
			m_remaining_pen = m_hitgroup = m_hitbox = m_dmg = -1;
		}

		bool m_valid{ false };
		sdk::vec3_t m_pos{ 0, 0, 0};
		game::e_hitbox m_index{ game::e_hitbox::head };
		bool m_center{ false };
		int  m_dmg{ -1 }, m_hitbox{ -1 }, m_hitgroup{ -1 }, m_remaining_pen{ 4 };
	};

	struct aim_target_t { 
		__forceinline aim_target_t( ) = default;

		__forceinline aim_target_t( player_entry_t* entry, std::optional < std::shared_ptr < lag_record_t > > record ) { 
			m_entry = entry;
			m_lag_record = record;
		}

		std::vector < sdk::vec3_t > m_temp_points{ };
		player_entry_t* m_entry { };
		std::optional < std::shared_ptr < lag_record_t > > m_lag_record{ };
		point_t* m_best_point{ };
		point_t* m_best_body_point{ };
		std::vector < point_t > m_points{ };
		sdk::vec3_t m_pos{ };

		lag_backup_t m_backup_record{ };
	};

	class c_hitbox { 
	public:
		int m_hitbox_id { };
		bool m_is_oob { };
		sdk::vec3_t m_mins { };
		sdk::vec3_t m_maxs { };
		sdk::vec3_t m_start_scaled { };
		float m_radius { };
		game::studio_bbox_t* m_hitbox { };
		int m_bone { };
		int m_hitgroup { };
	};

	struct scan_data_t { 
		__forceinline scan_data_t( ) = default;

		sdk::vec3_t m_pos { };
		float m_dmg { };
		lag_record_t* m_record { };
		game::e_hitbox m_hit_box { };
	};

	struct extrapolation_data_t { 
		__forceinline constexpr extrapolation_data_t( ) = default;

		__forceinline extrapolation_data_t( 
			game::cs_player_t* const player, const std::shared_ptr < lag_record_t > lag_record
		 ) : m_player{ player }, m_sim_time{ lag_record->m_sim_time }, m_flags{ lag_record->m_flags },
			m_was_in_air{ !( lag_record->m_flags & game::e_ent_flags::on_ground ) }, m_origin{ lag_record->m_origin },
			m_velocity{ lag_record->m_anim_velocity }, m_obb_min{ lag_record->m_mins }, m_obb_max{ lag_record->m_maxs } { }

		game::cs_player_t* m_player{ };

		float				m_sim_time{ };

		game::e_ent_flags	m_flags{ };
		bool				m_was_in_air{ };

		float               m_change{ }, m_dir{ };

		sdk::vec3_t				m_origin{ }, m_velocity{ },
			m_obb_min{ }, m_obb_max{ };
	};

	class c_aim_bot { 
	protected:
		struct cfg_t { 
			bool m_rage_bot{ false };
			int m_stop_modifiers{ 0 };

			int m_auto_stop_type_scar{ 0 },
				m_auto_stop_type_scout{ 0 },
				m_auto_stop_type_awp{ 0 },
				m_auto_stop_type_heavy_pistol{ 0 },
				m_auto_stop_type_pistol{ 0 },
				m_auto_stop_type_other{ 0 };

			int m_scar_hitboxes{ 0 }, 
				m_scout_hitboxes{ 0 },
				m_awp_hitboxes{ 0 },
				m_heavy_pistol_hitboxes{ 0 },
				m_pistol_hitboxes{ 0 },
				m_other_hitboxes{ 0 };

			int m_min_dmg_scar{ 20 }, 
				m_min_dmg_scout{ 20 }, 
				m_min_dmg_awp{ 20 }, 
				m_min_dmg_heavy_pistol{ 20 },
				m_min_dmg_pistol{ 20 }, 
				m_min_dmg_other{ 20 };

			float m_hit_chance_scar{ 50.f },
				m_hit_chance_scout{ 50.f },
				m_hit_chance_awp{ 50.f }, 
				m_hit_chance_heavy_pistol{ 50.f }, 
				m_hit_chance_pistol{ 50.f }, 
				m_hit_chance_other{ 50.f };

			float m_dt_hit_chance_scar{ 0.f }, 
				m_dt_hit_chance_scout{ 0.f }, 
				m_dt_hit_chance_awp{ 0.f }, 
				m_dt_hit_chance_heavy_pistol{ 0.f }, 
				m_dt_hit_chance_pistol{ 0.f }, 
				m_dt_hit_chance_other{ 0.f };

			int m_scar_point_scale{ 90 }, 
				m_scout_point_scale{ 90 }, 
				m_awp_point_scale{ 90 }, 
				m_heavy_pistol_point_scale{ 90 }, 
				m_pistol_point_scale{ 90 }, 
				m_other_point_scale{ 90 };

			s_keybind m_min_scar_dmg_key{ }, 
				m_min_scout_dmg_key{ }, 
				m_min_awp_dmg_key{ }, 
				m_min_heavy_pistol_dmg_key{ },
				m_min_pistol_dmg_key{ }, 
				m_min_other_dmg_key{ };

			int m_scar_min_dmg_on_key{ 0 }, 
				m_awp_min_dmg_on_key{ 0 },
				m_heavy_pistol_min_dmg_on_key{ 0 },
				m_pistol_min_dmg_on_key{ 0 },
				m_other_min_dmg_on_key{ 0 },
				m_scout_min_dmg_on_key{ 0 };

			int m_force_head_conditions_scar{ 0 }, 
				m_force_head_conditions_scout{ 0 },
				m_force_head_conditions_awp{ 0 },
				m_force_head_conditions_heavy_pistol{ 0 },
				m_force_head_conditions_pistol{ 0 },
				m_force_head_conditions_other{ 0 };

			int m_force_body_conditions_scar{ 0 }, 
				m_force_body_conditions_scout{ 0 },
				m_force_body_conditions_awp{ 0 },
				m_force_body_conditions_heavy_pistol{ 0 },
				m_force_body_conditions_pistol{ 0 },
				m_force_body_conditions_other{ 0 };

			int m_auto_stop_type_dt_scar{ 0 }, 
				m_auto_stop_type_dt_scout{ 0 },
				m_auto_stop_type_dt_awp{ 0 },
				m_auto_stop_type_dt_heavy_pistol{ 0 },
				m_auto_stop_type_dt_pistol{ 0 },
				m_auto_stop_type_dt_other{ 0 };

			bool m_dynamic_limit{ false }, m_auto_scope{ false };
			s_keybind m_baim_key{ };

			int m_max_targets{ 6 };
			int m_backtrack_intensity{ 0 };
			
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b71u, { } };

		void add_targets( );
		void select( game::user_cmd_t& user_cmd, bool& send_packet );

		sdk::qang_t m_angle { }; // AXAXAXAXAx
		int m_should_stop { };

	public:
		std::vector < aim_target_t > m_targets{ };
		void scan_center_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, sdk::vec3_t shoot_pos, std::vector < point_t >& points ) const;
		void setup_threading( );
		void handle_ctx( game::user_cmd_t& user_cmd, bool& send_packet );
		static void setup_hitboxes( std::vector < hit_box_data_t >& hitboxes );
		static void setup_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, game::e_hitbox index, e_hit_scan_mode mode
		 );
		std::vector < hit_box_data_t > m_hit_boxes{ };
		std::optional < aim_target_t > select_ideal_record( const player_entry_t& entry ) const;
		std::optional < aim_target_t > get_latest_record( const player_entry_t& entry ) const;

		void player_move( extrapolation_data_t& lag_record ) const;
		int get_min_dmg_override_key( );
		int get_min_dmg_override( );
		int get_force_head_conditions( );
		int get_force_body_conditions( );
		bool get_min_dmg_override_state( );
		void get_hitbox_data( c_hitbox* rtn, game::cs_player_t* ent, int ihitbox, const game::bones_t& matrix );
		int  get_min_dmg_to_set_up( );
		int get_dt_stop_type( );
		int get_autostop_type( );
		int get_hitboxes_setup( );
		float get_pointscale( );
		float get_hit_chance( );
		int calc_hit_chance( 
			game::cs_player_t* player, std::shared_ptr < lag_record_t > record, const sdk::qang_t& angle, const std::ptrdiff_t hit_box
		 );
		void add_aim_point(aim_target_t& target, sdk::vec3_t& point, game::e_hitbox index, bool is_center);
		void calc_capsule_points ( aim_target_t& target, const game::studio_bbox_t* hit_box, const std::ptrdiff_t index,
		const sdk::mat3x4_t matrix, float scale );
		static void scan_point( player_entry_t* entry, 
			point_t& point, sdk::vec3_t& shoot_pos = g_ctx->shoot_pos( ) );
		
		aim_target_t* select_target( );
		bool scan_points( cc_def( aim_target_t* ) target, std::vector < point_t >& points, bool additional = false ) const;

		point_t* select_point( cc_def( aim_target_t* ) target, const int cmd_num );

    	bool can_hit( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box );
		bool can_shoot( 
			bool skip_r8, const int shift_amount, const bool what = false
		 ) const;

		std::optional < aim_target_t > extrapolate( const player_entry_t& entry ) const;

		bool m_silent_aim;
		bool m_setupped_threading{ false };

		__forceinline int& stop_type( ) { return m_should_stop; };
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
		__forceinline c_aim_bot( ) = default;
	};

	class c_knife_bot
	{ 
	public:
		void handle_knife_bot( game::user_cmd_t& cmd );
		bool select_target( );

	protected:

		__forceinline int get_min_dmg( )
		{ 
			if( !g_local_player->self( ) 
				|| !g_local_player->self( )->weapon( ) )
				return -1;

			if( game::to_time( g_local_player->self( )->tick_base( ) ) > ( g_local_player->self( )->weapon( )->next_primary_attack( ) + 0.4f ) )
				return 34;

			return 21;
		}

		__forceinline sdk::vec3_t get_hitbox_pos( int hitbox_id, game::cs_player_t* player )
		{ 

			auto hdr = player->mdl_ptr( );

			if( !hdr )
				return sdk::vec3_t( );

			auto hitbox_set = hdr->m_studio->get_hitbox_set( player->hitbox_set_index( ) );

			if( !hitbox_set )
				return sdk::vec3_t( );

			auto hitbox = hitbox_set->get_bbox( hitbox_id );

			if( !hitbox )
				return sdk::vec3_t( );

			sdk::vec3_t min { }, max { };

			sdk::vector_transform( hitbox->m_mins, player->bone_cache( ).m_elements[ hitbox->m_bone ], min );
			sdk::vector_transform( hitbox->m_maxs, player->bone_cache( ).m_elements[ hitbox->m_bone ], max );

			return ( min + max ) * 0.5f;
		}

		__forceinline bool is_visible( const sdk::vec3_t& start, const sdk::vec3_t& end, game::cs_player_t* player, game::cs_player_t* local )
		{ 
			game::trace_t trace;

			game::ray_t ray{ start, end };

			game::trace_filter_simple_t filter{ player, 0 };
			filter.m_ignore_entity = local;

			game::g_engine_trace->trace_ray( ray, CS_MASK_SHOOT_PLAYER, reinterpret_cast< game::base_trace_filter_t* > ( &filter ), &trace );

			return trace.m_entity == player || trace.m_frac == 1.0f;
		}

		game::cs_player_t* m_best_player;
		int m_best_distance;
		int m_best_index;

	};

	inline const std::unique_ptr < c_knife_bot > g_knife_bot = std::make_unique < c_knife_bot > ( );
	inline const std::unique_ptr < c_aim_bot > g_aim_bot = std::make_unique < c_aim_bot > ( );
}