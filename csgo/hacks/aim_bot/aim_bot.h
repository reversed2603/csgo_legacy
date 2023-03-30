#pragma once

namespace csgo::hacks {

	enum e_hit_scan_mode {
		normal,
		has_lethality,
		prefer
	};

	struct hit_box_data_t {
		valve::e_hitbox         m_index;
		e_hit_scan_mode m_mode;

		__forceinline bool operator== ( const hit_box_data_t& c ) const {
			return m_index == c.m_index && m_mode == c.m_mode;
		}
	};

	struct point_t {
		__forceinline point_t( ) = default;

		__forceinline point_t( sdk::vec3_t& pos, valve::e_hitbox index, bool center ) {
			m_pos = pos;
			m_index = index;
			m_center = center;
		}

		auto_wall_data_t m_pen_data{ };
		bool m_valid{ };
		sdk::vec3_t m_pos{ };
		valve::e_hitbox m_index{ };
		bool m_center{ };
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
	};

	class c_hitbox {
	public:
		int m_hitbox_id { };
		bool m_is_oob { };
		sdk::vec3_t m_mins { };
		sdk::vec3_t m_maxs { };
		sdk::vec3_t m_start_scaled { };
		float m_radius { };
		valve::studio_bbox_t* m_hitbox { };
		int m_bone { };
		int m_hitgroup { };
	};

	struct scan_data_t {
		__forceinline scan_data_t( ) = default;

		sdk::vec3_t m_pos { };
		float m_dmg { };
		lag_record_t* m_record { };
		valve::e_hitbox m_hit_box { };
	};

	struct extrapolation_data_t {
		__forceinline constexpr extrapolation_data_t( ) = default;

		__forceinline extrapolation_data_t( 
			valve::cs_player_t* const player, const std::shared_ptr < lag_record_t > const lag_record
		 ) : m_player{ player }, m_sim_time{ lag_record->m_sim_time }, m_flags{ lag_record->m_flags },
			m_was_in_air{ !( lag_record->m_flags & valve::e_ent_flags::on_ground ) }, m_origin{ lag_record->m_origin },
			m_velocity{ lag_record->m_anim_velocity }, m_obb_min{ lag_record->m_mins }, m_obb_max{ lag_record->m_maxs } { }

		valve::cs_player_t* m_player{ };

		float				m_sim_time{ };

		valve::e_ent_flags	m_flags{ };
		bool				m_was_in_air{ };

		float               m_change{ }, m_dir{ };

		sdk::vec3_t				m_origin{ }, m_velocity{ },
			m_obb_min{ }, m_obb_max{ };
	};

	class c_aim_bot {
	protected:
		struct cfg_t {
			bool m_rage_bot{ }, m_threading{ }, m_early_autostop{ };/*, m_prefer_body{ };
			int m_hitboxes { };
			float m_head_scale { 0.f }, m_body_scale { 0.f }, m_hit_chance { 1.f };*/
			int m_scar_hitboxes{ }, m_scout_hitboxes{ }, m_awp_hitboxes{ }, m_heavy_pistol_hitboxes{ }, m_pistol_hitboxes{ }, m_other_hitboxes{ };
			float m_min_dmg_scar{ 1.f }, m_min_dmg_scout{ 1.f }, m_min_dmg_awp{ 1.f }, m_min_dmg_heavy_pistol{ 1.f }, m_min_dmg_pistol{ 1.f }, m_min_dmg_other{ 1.f };
			float m_hit_chance_scar{ }, m_hit_chance_scout{ }, m_hit_chance_awp{ }, m_hit_chance_heavy_pistol{ }, m_hit_chance_pistol{ }, m_hit_chance_other{ };
			float m_scar_head_scale{ }, m_scout_head_scale{ }, m_awp_head_scale{ }, m_heavy_pistol_head_scale{ }, m_pistol_head_scale{ }, m_other_head_scale{ };
			bool m_scar_prefer_body{ }, m_scout_prefer_body{ }, m_awp_prefer_body{ }, m_heavy_pistol_prefer_body{ }, m_pistol_prefer_body{ }, m_other_prefer_body{ };
			int m_scar_min_dmg_on_key{ }, m_awp_min_dmg_on_key{ }, m_heavy_pistol_min_dmg_on_key{ }, m_pistol_min_dmg_on_key{ }, m_other_min_dmg_on_key{ };
			int m_syka_min_dmg_on_key{ };
			int m_force_head_conditions_scar{ }, m_force_head_conditions_scout{ }, m_force_head_conditions_awp{ }, m_force_head_conditions_heavy_pistol{ }, m_force_head_conditions_pistol{ }, m_force_head_conditions_other{ };
			bool m_auto_scope{ };
			s_keybind m_baim_key{ };
			s_keybind m_min_scar_dmg_key{ }, m_min_scout_dmg_key{ }, m_min_awp_dmg_key{ }, m_min_heavy_pistol_dmg_key{ }, m_min_pistol_dmg_key{ }, m_min_other_dmg_key{ };
			int m_force_body_conditions_scar{ }, m_force_body_conditions_scout{ }, m_force_body_conditions_awp{ }, m_force_body_conditions_heavy_pistol{ }, m_force_body_conditions_pistol{ }, m_force_body_conditions_other{ };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b71u, { } };

		void add_targets( );
		void select( valve::user_cmd_t& user_cmd, bool& send_packet );
		void select_target( );

		sdk::qang_t m_angle { }; // AXAXAXAXAx
		int m_should_stop { };

	public:
		std::vector < aim_target_t > m_targets{ };
		void scan_center_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, sdk::vec3_t shoot_pos, std::vector < point_t >& points ) const;
		void handle_ctx( valve::user_cmd_t& user_cmd, bool& send_packet );
		static void setup_hitboxes( std::vector < hit_box_data_t >& hitboxes );
		static void setup_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, valve::e_hitbox index, e_hit_scan_mode mode
		 );
		std::vector < hit_box_data_t > m_hit_boxes{ };
		std::optional < aim_target_t > select_ideal_record( const player_entry_t& entry ) const;
		std::optional < aim_target_t > get_latest_record( const player_entry_t& entry ) const;

		void player_move( extrapolation_data_t& lag_record ) const;
		void get_hitbox_data( c_hitbox* rtn, valve::cs_player_t* ent, int ihitbox, const valve::bones_t& matrix );
		bool calc_hit_chance(
			valve::cs_player_t* player, const sdk::qang_t& angle
		 );

		static void scan_point( player_entry_t* entry, 
			point_t& point, float min_dmg_key, bool min_dmg_key_pressed, sdk::vec3_t& shoot_pos = g_ctx->shoot_pos( ) );

		bool scan_points( cc_def( aim_target_t* ) target, std::vector < point_t >& points, bool additional_scan = false ) const;

		point_t* select_point( cc_def( aim_target_t* ) target, const int cmd_num );

    	bool can_hit( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box );
		bool can_shoot( 
			bool skip_r8, const int shift_amount, const bool what
		 ) const;

		std::optional < aim_target_t > extrapolate( const player_entry_t& entry ) const;

		std::vector < point_t > m_nigga_hack { };

		sdk::vec3_t m_cur_body_point{ }, m_cur_head_point{ };

		__forceinline int& stop_type( ) { return m_should_stop; };
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
		__forceinline c_aim_bot( ) = default;
	};

	class c_knife_bot
	{
	public:
		void handle_knife_bot( valve::user_cmd_t& cmd );
		bool select_target( );

	protected:

		__forceinline int get_min_dmg( )
		{
			if( !g_local_player->self( ) || !g_local_player->self( )->weapon( ) )
				return -1;

			if( valve::to_time( g_local_player->self( )->tick_base( ) ) >( g_local_player->self( )->weapon( )->next_primary_attack( ) + 0.4f ) )
				return 34;

			return 21;
		}

		__forceinline sdk::vec3_t get_hitbox_pos( int hitbox_id, valve::cs_player_t* player )
		{
			if( !this )
				return  sdk::vec3_t( );

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


		__forceinline bool is_visible( const sdk::vec3_t& start, const sdk::vec3_t& end, valve::cs_player_t* player, valve::cs_player_t* local )
		{
			valve::trace_t trace;

			valve::ray_t ray{ start, end };

			valve::trace_filter_simple_t filter{ player, 0 };
			filter.m_ignore_entity = local;

			valve::g_engine_trace->trace_ray( ray, valve::e_mask::shot_player, reinterpret_cast< valve::base_trace_filter_t* >( &filter ), &trace );

			return trace.m_entity == player || trace.m_frac == 1.0f;
		}

		valve::cs_player_t* m_best_player;
		int m_best_distance;
		int m_best_index;
	};

	inline const std::unique_ptr < c_knife_bot > g_knife_bot = std::make_unique < c_knife_bot >( );
	inline const std::unique_ptr < c_aim_bot > g_aim_bot = std::make_unique < c_aim_bot >( );
}