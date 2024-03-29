#pragma once

namespace csgo::hacks { 
	struct player_entry_t;

	enum collision_group_t_ { 
		_COLLISION_GROUP_NONE = 0,
		_COLLISION_GROUP_DEBRIS,
		_COLLISION_GROUP_DEBRIS_TRIGGER,
		_COLLISION_GROUP_INTERACTIVE_DEBRIS,
		_COLLISION_GROUP_INTERACTIVE,
		_COLLISION_GROUP_PLAYER,
		_COLLISION_GROUP_BREAKABLE_GLASS,
		_COLLISION_GROUP_VEHICLE,
		_COLLISION_GROUP_PLAYER_MOVEMENT,
		_COLLISION_GROUP_NPC,
		_COLLISION_GROUP_IN_VEHICLE,
		_COLLISION_GROUP_WEAPON,
		_COLLISION_GROUP_VEHICLE_CLIP,
		_COLLISION_GROUP_PROJECTILE,
		_COLLISION_GROUP_DOOR_BLOCKER,
		_COLLISION_GROUP_PASSABLE_DOOR,
		_COLLISION_GROUP_DISSOLVING,
		_COLLISION_GROUP_PUSHAWAY,
		_COLLISION_GROUP_NPC_ACTOR,
		_COLLISION_GROUP_NPC_SCRIPTED,
		_COLLISION_GROUP_PZ_CLIP,
		_COLLISION_GROUP_DEBRIS_BLOCK_PROJECTILE,
		_LAST_SHARED_COLLISION_GROUP
	};

	enum contents_t_ { 
		_CONTENTS_EMPTY = 0,
		_CONTENTS_SOLID = 0x1,
		_CONTENTS_WINDOW = 0x2,
		_CONTENTS_AUX = 0x4,
		_CONTENTS_GRATE = 0x8,
		_CONTENTS_SLIME = 0x10,
		_CONTENTS_WATER = 0x20,
		_CONTENTS_BLOCKLOS = 0x40,
		_CONTENTS_OPAQUE = 0x80,
		_CONTENTS_TESTFOGVOLUME = 0x100,
		_CONTENTS_UNUSED = 0x200,
		_CONTENTS_BLOCKLIGHT = 0x400,
		_CONTENTS_TEAM_ct = 0x800,
		_CONTENTS_TEAM_t = 0x1000,
		_CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
		_CONTENTS_MOVEABLE = 0x4000,
		_CONTENTS_AREAPORTAL = 0x8000,
		_CONTENTS_PLAYERCLIP = 0x10000,
		_CONTENTS_MONSTERCLIP = 0x20000,
		_CONTENTS_CURRENT_0 = 0x40000,
		_CONTENTS_CURRENT_90 = 0x80000,
		_CONTENTS_CURRENT_180 = 0x100000,
		_CONTENTS_CURRENT_270 = 0x200000,
		_CONTENTS_CURRENT_UP = 0x400000,
		_CONTENTS_CURRENT_DOWN = 0x800000,
		_CONTENTS_ORIGIN = 0x1000000,
		_CONTENTS_MONSTER = 0x2000000,
		_CONTENTS_DEBRIS = 0x4000000,
		_CONTENTS_DETAIL = 0x8000000,
		_CONTENTS_TRANSLUCENT = 0x10000000,
		_CONTENTS_LADDER = 0x20000000,
		_CONTENTS_HITBOX = 0x40000000,
	};

	enum masks_t_ { 
		_MASK_ALL = 0xFFFFFFFF,
		_MASK_SOLID = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_MONSTER | _CONTENTS_GRATE,
		_MASK_PLAYERSOLID = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_PLAYERCLIP | _CONTENTS_WINDOW | _CONTENTS_MONSTER | _CONTENTS_GRATE,
		_MASK_NPCSOLID = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_MONSTERCLIP | _CONTENTS_WINDOW | _CONTENTS_MONSTER | _CONTENTS_GRATE,
		_MASK_NPCFLUID = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_MONSTERCLIP | _CONTENTS_WINDOW | _CONTENTS_MONSTER | _CONTENTS_GRATE,
		_MASK_WATER = _CONTENTS_WATER | _CONTENTS_MOVEABLE | _CONTENTS_SLIME,
		_MASK_OPAQUE = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_OPAQUE,
		_MASK_OPAQUE_AND_NPCS = _MASK_OPAQUE | _CONTENTS_MONSTER,
		_MASK_BLOCKLOS = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_BLOCKLOS,
		_MASK_BLOCKLOS_AND_NPCS = _MASK_BLOCKLOS | _CONTENTS_MONSTER,
		_MASK_VISIBLE = _MASK_OPAQUE | _CONTENTS_IGNORE_NODRAW_OPAQUE,
		_MASK_VISIBLE_AND_NPCS = _MASK_OPAQUE_AND_NPCS | _CONTENTS_IGNORE_NODRAW_OPAQUE,
		_MASK_SHOT = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_MONSTER | _CONTENTS_WINDOW | _CONTENTS_DEBRIS | _CONTENTS_GRATE | _CONTENTS_HITBOX,
		_MASK_SHOT_BRUSHONLY = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_DEBRIS,
		_MASK_SHOT_HULL = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_MONSTER | _CONTENTS_WINDOW | _CONTENTS_DEBRIS | _CONTENTS_GRATE,
		_MASK_SHOT_PORTAL = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_MONSTER,
		_MASK_SOLID_BRUSHONLY = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_GRATE,
		_MASK_PLAYERSOLID_BRUSHONLY = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_PLAYERCLIP | _CONTENTS_GRATE,
		_MASK_NPCSOLID_BRUSHONLY = _CONTENTS_SOLID | _CONTENTS_MOVEABLE | _CONTENTS_WINDOW | _CONTENTS_MONSTERCLIP | _CONTENTS_GRATE,
		_MASK_NPCWORLDSTATIC = _CONTENTS_SOLID | _CONTENTS_WINDOW | _CONTENTS_MONSTERCLIP | _CONTENTS_GRATE,
		_MASK_NPCWORLDSTATIC_FLUID = _CONTENTS_SOLID | _CONTENTS_WINDOW | _CONTENTS_MONSTERCLIP,
		_MASK_SPLITAREPORTAL = _CONTENTS_WATER | _CONTENTS_SLIME,
		_MASK_CURRENT = _CONTENTS_CURRENT_0 | _CONTENTS_CURRENT_90 | _CONTENTS_CURRENT_180 | _CONTENTS_CURRENT_270 | _CONTENTS_CURRENT_UP | _CONTENTS_CURRENT_DOWN,
		_MASK_DEADSOLID = _CONTENTS_SOLID | _CONTENTS_PLAYERCLIP | _CONTENTS_WINDOW | _CONTENTS_GRATE,
	};

	struct key_data_t { 
		__forceinline key_data_t( ) = default;
		float m_alpha{ };
		std::string m_status{ };
	};

	class c_visuals
	{ 
	protected:
		float lby_array[ 64 ]{ 0.f };
		float ammo_array[ 64 ]{ 0.f };
		float hp_array[ 64 ]{ };

		__forceinline std::string get_weapon_name( game::cs_weapon_t* wpn )
		{ 
			auto get_clean_name = [ ]( const char* name ) -> const char* { 
				if( name [ 0 ] == 'C' )
					name++;

				auto start = strstr( name, xor_str( "Weapon" ) );
				if( start != nullptr )
					name = start + 6;

				return name;
			};

			if( !wpn )
				return "";

			game::e_item_index weapon_index = wpn->item_index( );

			if( !wpn->networkable( )->client_class( ) )
				return "";

			std::string str_result = "";
			switch( weapon_index )
			{ 
			case game::e_item_index::glock: str_result = xor_str( "glock-18" ); break;
			case game::e_item_index::ssg08: str_result = xor_str( "ssg-08" ); break;
			case game::e_item_index::revolver: str_result = xor_str( "revolver" ); break;
			case game::e_item_index::scar20: str_result = xor_str( "scar20" ); break;
			case game::e_item_index::deagle: str_result = xor_str( "deagle" ); break;
			case game::e_item_index::elite: str_result = xor_str( "dual berettas" ); break;
			case game::e_item_index::five_seven: str_result = xor_str( "five-seven" ); break;
			case game::e_item_index::frag_grenade: str_result = xor_str( "frag" ); break;
			case game::e_item_index::smoke_grenade: str_result = xor_str( "smoke" ); break;
			case game::e_item_index::decoy: str_result = xor_str( "decoy" ); break;
			case game::e_item_index::flashbang: str_result = xor_str( "flashbang" ); break;
			case game::e_item_index::p2000: str_result = xor_str( "p2000" ); break;
			case game::e_item_index::inc_grenade: str_result = xor_str( "incendiary" ); break;
			case game::e_item_index::molotov: str_result = xor_str( "molotov" ); break;
			case game::e_item_index::he_grenade: str_result = xor_str( "he grenade" ); break;
			default: str_result = get_clean_name( wpn->networkable( )->client_class( )->m_network_name );
			}

			if( str_result == xor_str( "HKP2000" ) )
				return xor_str( "p2000" );

			std::transform( str_result.begin( ), str_result.end( ), str_result.begin( ), ::tolower );

			return str_result;
		}

		__forceinline std::string get_weapon_icon( game::cs_weapon_t* wpn )
		{ 
			if( !wpn )
				return " ";

			if( wpn->is_knife( ) )
				return xor_str( "[" );

			std::string str_result = "";
			switch( wpn->item_index( ) )
			{ 
			case game::e_item_index::scar20: str_result = xor_str( "Y" ); break;
			case game::e_item_index::g3sg1: str_result = xor_str( "X" ); break;
			case game::e_item_index::awp: str_result = xor_str( "Z" ); break;
			case game::e_item_index::ssg08: str_result = xor_str( "a" ); break;
			case game::e_item_index::deagle: str_result = xor_str( "A" ); break;
			case game::e_item_index::revolver: str_result = xor_str( "J" ); break;
			case game::e_item_index::p2000: str_result = xor_str( "E" ); break;
			case game::e_item_index::glock: str_result = xor_str( "D" ); break;
			case game::e_item_index::usps: str_result = xor_str( "G" ); break;
			case game::e_item_index::elite: str_result = xor_str( "B" ); break;
			case game::e_item_index::c4: str_result = xor_str( "o" ); break;
			case game::e_item_index::p250: str_result = xor_str( "F" ); break;
			case game::e_item_index::aug: str_result = xor_str( "U" ); break;
			case game::e_item_index::five_seven: str_result = xor_str( "C" ); break;
			case game::e_item_index::ak47: str_result = xor_str( "W" ); break;
			case game::e_item_index::galil: str_result = xor_str( "Q" ); break;
			case game::e_item_index::cz75a: str_result = xor_str( "I" ); break;
			case game::e_item_index::famas: str_result = xor_str( "R" ); break;
			case game::e_item_index::tec9: str_result = xor_str( "H" ); break;
			case game::e_item_index::bizon: str_result = xor_str( "M" ); break;
			case game::e_item_index::m249: str_result = xor_str( "g" ); break;
			case game::e_item_index::negev: str_result = xor_str( "f" ); break;
			case game::e_item_index::nova: str_result = xor_str( "e" ); break;
			case game::e_item_index::mag7: str_result = xor_str( "d" ); break;
			case game::e_item_index::taser: str_result = xor_str( "h" ); break;
			case game::e_item_index::he_grenade: str_result = xor_str( "j" ); break;
			case game::e_item_index::smoke_grenade: str_result = xor_str( "k" ); break;
			case game::e_item_index::frag_grenade: str_result = xor_str( "n" ); break;
			case game::e_item_index::molotov: str_result = xor_str( "l" ); break;
			case game::e_item_index::inc_grenade: str_result = xor_str( "n" ); break;
			case game::e_item_index::sawed_off: str_result = xor_str( "c" ); break;
			case game::e_item_index::decoy: str_result = xor_str( "m" ); break;
			case game::e_item_index::flashbang: str_result = xor_str( "i" ); break;
			case game::e_item_index::m4a4: str_result = xor_str( "S" ); break;
			case game::e_item_index::m4a1s: str_result = xor_str( "T" ); break;
			case game::e_item_index::firebomb: str_result = xor_str( "k" ); break;
			case game::e_item_index::mac10: str_result = xor_str( "K" ); break;
			case game::e_item_index::ump45: str_result = xor_str( "L" ); break;
			case game::e_item_index::mp7: str_result = xor_str( "N" ); break;
			case game::e_item_index::p90: str_result = xor_str( "P" ); break;
			case game::e_item_index::mp9: str_result = xor_str( "N" ); break;
			case game::e_item_index::sg553: str_result = xor_str( "V" ); break;
			case game::e_item_index::xm1014: str_result = xor_str( "e" ); break;
			case game::e_item_index::ta_grenade: str_result = xor_str( "i" ); break;
			default: break;
			}

			return str_result;
		}

		void draw_name( game::cs_player_t* player, RECT& rect );
		void draw_box( game::cs_player_t* player, RECT& rect );
		void draw_health( game::cs_player_t* player, RECT& rect );
		void draw_wpn( game::cs_player_t* player, RECT& rect );
		void draw_ammo( game::cs_player_t* player, RECT& rect );
		void draw_flags( game::cs_player_t* player, RECT& rect );
		void draw_lby_upd( game::cs_player_t* player, RECT& rect );

		void draw_skeleton( game::cs_player_t* player );

		void draw_c4( game::base_entity_t* entity );
		void molotov_timer( game::base_entity_t* entity );
		void smoke_timer( game::base_entity_t* entity );
		void grenade_projectiles( game::base_entity_t* entity );

		struct shot_mdl_t { 
			int                         m_player_index { };
			unsigned int                m_hash { };
			float						m_time { }, m_alpha{ 1.f }, m_is_death{ };
			game::bones_t				m_bones { };
			sdk::mat3x4_t				m_world_matrix { };

			game::cs_player_t*			m_player{ };

			game::model_render_info_t	m_info { };
			game::draw_model_state_t	m_state { };
		};
		std::vector< shot_mdl_t >		m_shot_mdls { };
		struct cfg_t { 

			bool m_draw_name{ }, m_draw_health{ }, m_custom_healthbar{ }, m_draw_box{ }, m_fill_box{ }, m_gradient_fill_box{ }, m_wpn_ammo{ }, m_draw_flags{ },
				m_draw_lby{ }, m_oof_indicator{ }, m_glow{ }, m_bullet_impacts{ },
				 m_shadows_modulation { }, m_tone_map_modulation { }, m_fog { };

			bool m_keybinds_list{ };
			bool m_gradient{ }, m_gradient_lby{ }, m_gradient_ammo{ };

			float m_wpn_ammo_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, 
				m_lby_upd_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_oof_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_wpn_icon_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_draw_box_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_custom_healthbar_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_draw_name_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_dt_flag_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_lc_flag_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_broken_lc_flag_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_health_clr_bottom[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_health_clr_right[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_health_clr_left[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_lby_clr_bottom[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_lby_clr_right[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_lby_clr_left[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_wpn_ammo_clr_up[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_wpn_ammo_clr_bottom[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_wpn_text_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_wpn_ammo_clr_left[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_fill_box_clr_up_left[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_fill_box_clr_up_right[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_fill_box_clr_right[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_fill_box_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			int m_oof_radius{ }, m_oof_size{ };

			float m_view_model_fov{ 60.f }, m_x_dir { }, m_y_dir { }, m_z_dir { };
			int m_skybox_type{ }, m_bloom { }, m_exposure { }, m_fog_start { }, m_fog_end { },
				m_blend_in_scope_val { }, m_player_flags { }, m_removals{ },
				m_grenade_trajectory_options{ },
				m_draw_bomb_options{ },
				m_weapon_selection{ },
				m_grenade_selection{ },
				m_dropped_weapon_selection{ },
				m_bullet_tracer_selection{ },
				m_hit_markers_selection{ },
				m_team_bullet_type{ },
				m_enemy_bullet_type{ },
				m_bullet_type{ };

			float m_world_modulation[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_props_modulation[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_sky_modulation[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_glow_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_bullet_tracers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_team_bullet_tracers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_enemy_bullet_tracers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_fog_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_grenade_trajectory_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_grenade_proximity_warning_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_friendly_grenade_proximity_warning_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_damage_markers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_hit_markers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_screen_hit_markers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_bullet_impacts_server_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_bullet_impacts_client_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_manuals_indication_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, 
				m_draw_grenade_glow_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_draw_weapon_glow_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_molotov_range[ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				m_molotov_timer_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			float m_foot_step_esp_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };
			bool m_foot_step_esp{ };

			float m_skeleton_esp_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };
			bool m_skeleton_esp{ };

			bool m_rain{ };
			
			float m_proj_icon_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_proj_wpn_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			float m_grenade_projectiles_icon_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_grenade_projectiles_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			bool m_shared_esp{ }, m_engine_radar{ };
			float m_draw_c4_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_draw_c4_icon_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_draw_c4_glow_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };
			float m_draw_planted_c4_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_draw_planted_c4_icon_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_draw_planted_c4_glow_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			bool m_molotov_timer{ }, m_smoke_timer{ },
				m_manuals_indication { }, m_blend_in_scope { }, m_show_weapon_in_scope { };
		};

		struct flags_data_t { 
			std::string m_name { };
			sdk::col_t m_clr { };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b31u, { } };

	public:
		RECT get_bbox( game::cs_player_t* ent, bool is_valid = false );
		void handle_player_drawings( );
		void handle_world_drawings( );
		void add_shot_mdl( game::cs_player_t* player, const sdk::mat3x4_t* bones, bool is_death = false );
		void draw_shot_mdl( );
		void draw_glow( );
		void draw_scope_lines( );
		void removals( );
		void change_shadows( game::base_entity_t* entity );
		void skybox_changer( );
		void draw_auto_peek( );
		void draw_key_binds( );
		void draw_beam( );
		void oof_indicators( game::cs_player_t* );
		void manuals_indicators( );

		void tone_map_modulation( game::base_entity_t* entity );
		void draw_hitmarkers( );
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
		std::array < bool, 65 > m_change_offset_due_to_lby{ };
		std::array < bool, 65 > m_change_offset_due_to_ammo{ };

		int screen_x{ }, screen_y{ };

		struct inferno_info { 
			sdk::vec3_t origin;
			float range;
		};
		std::vector< inferno_info > inferno_information{ };

		struct bullet_impact_t { 
			__forceinline bullet_impact_t( ) = default;

			__forceinline bullet_impact_t( 
				const float time, const sdk::vec3_t& from, const sdk::vec3_t& pos
			 ) : m_time{ time }, m_from{ from }, m_pos{ pos } { }

			float	m_time{ };
			bool	m_final{ true };

			sdk::vec3_t	m_from{ }, m_pos{ };
		};

		struct bullet_trace_data_t
		{ 
			float m_exp_time{ };
			sdk::vec3_t m_start_pos{ }, m_end_pos{ };
			sdk::col_t col{ };
			int m_player_index{ };
			int m_tickbase{ };
			bool m_ignore{ };
			bool m_ring{ };
			int m_type{ };
		};

		std::vector< bullet_trace_data_t > bullet_trace_info;
		void push_beam_info( bullet_trace_data_t beam_info );

		struct hit_marker_data_t { 
			sdk::vec3_t m_pos{ };
			float m_spawn_time{ };
			float m_alpha{ };
			int   m_damage{ };
		};

		std::vector< bullet_impact_t >	m_bullet_impacts{ };
		std::deque < hit_marker_data_t > m_hit_markers{ };

		struct bomb_holder_t { 
			game::cs_player_t* bomb_holder{ };
		};

		std::vector< bomb_holder_t >	m_bomb_holder{ };
		int m_cur_yaw_dir{ };
	};

	inline const std::unique_ptr < c_visuals > g_visuals = std::make_unique < c_visuals > ( );
}