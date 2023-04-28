#pragma once

namespace csgo::hacks {

	struct dormant_data_t {
		float m_alpha{ };
		bool m_was_unseen{ };
		sdk::vec3_t m_origin{ };
		float m_receive_time{ };
		int m_weapon_id{ };
		int m_weapon_type{ -1 };
		bool m_use_shared{ };
		float m_last_shared_time{ };
	};

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
		struct grenade_simulation_t {
			__forceinline grenade_simulation_t( ) = default;

			__forceinline grenade_simulation_t( 
				valve::cs_player_t* const owner, const valve::e_item_index index,
				const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset
			 ) : m_owner{ owner }, m_index{ index } { predict( origin, velocity, throw_time, offset );	}

			void predict( const sdk::vec3_t& origin, const sdk::vec3_t& velocity, const float throw_time, const int offset );

			bool physics_simulate( );

			void physics_trace_entity( 
				const sdk::vec3_t& src, const sdk::vec3_t& dst,
				const std::uint32_t mask, valve::trace_t& trace
			 );

			void physics_push_entity( const sdk::vec3_t& push, valve::trace_t& trace );

			void perform_fly_collision_resolution( valve::trace_t& trace );

			void think( );

			void detonate( const bool bounced );

			void update_path( const bool bounced );

			__forceinline void push_broken_ent( valve::base_entity_t* ent )
			{
				m_broken_ents.emplace_back( ent );
			}

			__forceinline void clear_broken_ents( )
			{
				m_broken_ents.clear( );
			}

			__forceinline bool is_ent_broken( valve::base_entity_t* ent )
			{
				return find( m_broken_ents.begin( ), m_broken_ents.end( ), ent ) != m_broken_ents.end( );
			}

			bool										m_detonated{ };
			valve::cs_player_t* m_owner{ };
			sdk::vec3_t										m_origin{ }, m_velocity{ };
			valve::base_entity_t* m_last_hit_entity{ }, * m_last_breakable{ };
			float										m_detonate_time{ }, m_expire_time{ }, m_source_time{ };
			valve::e_item_index							m_index{ };
			int											m_tick{ }, m_next_think_tick{ },
				m_last_update_tick{ }, m_bounces_count{ }, m_collision_group{ };
			std::vector< std::pair< sdk::vec3_t, bool > >	m_path{ };
			std::vector < valve::base_entity_t* >            m_broken_ents{ };
		};

		__forceinline std::string get_weapon_name( valve::cs_weapon_t* wpn )
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

			valve::e_item_index weapon_index = wpn->item_index( );

			if( !wpn->networkable( )->client_class( ) )
				return "";

			std::string str_result = "";
			switch( weapon_index )
			{
			case valve::e_item_index::glock: str_result = xor_str( "GLOCK-18" ); break;
			case valve::e_item_index::ssg08: str_result = xor_str( "SSG-08" ); break;
			case valve::e_item_index::revolver: str_result = xor_str( "REVOLVER" ); break;
			case valve::e_item_index::scar20: str_result = xor_str( "SCAR-20" ); break;
			case valve::e_item_index::deagle: str_result = xor_str( "DEAGLE" ); break;
			case valve::e_item_index::elite: str_result = xor_str( "DUAL BERETTAS" ); break;
			case valve::e_item_index::five_seven: str_result = xor_str( "FIVE-SEVEN" ); break;
			case valve::e_item_index::frag_grenade: str_result = xor_str( "FRAG" ); break;
			case valve::e_item_index::smoke_grenade: str_result = xor_str( "SMOKE" ); break;
			case valve::e_item_index::decoy: str_result = xor_str( "DECOY" ); break;
			case valve::e_item_index::flashbang: str_result = xor_str( "FLASHBANG" ); break;
			case valve::e_item_index::p2000: str_result = xor_str( "P2000" ); break;
			case valve::e_item_index::inc_grenade: str_result = xor_str( "INCENDIARY" ); break;
			case valve::e_item_index::molotov: str_result = xor_str( "MOLOTOV" ); break;
			case valve::e_item_index::he_grenade: str_result = xor_str( "HE GRENADE" ); break;
			default: str_result = get_clean_name( wpn->networkable( )->client_class( )->m_network_name );
			}

			if( str_result == xor_str( "HKP2000" ) )
				return xor_str( "P2000" );

			std::transform( str_result.begin( ), str_result.end( ), str_result.begin( ), ::toupper );

			return str_result;
		}

		__forceinline std::string get_weapon_icon( valve::cs_weapon_t* wpn )
		{
			if( !wpn )
				return " ";

			if( wpn->is_knife( ) )
				return xor_str( "[" );

			std::string str_result = "";
			switch( wpn->item_index( ) )
			{
			case valve::e_item_index::scar20: str_result = xor_str( "Y" ); break;
			case valve::e_item_index::g3sg1: str_result = xor_str( "X" ); break;
			case valve::e_item_index::awp: str_result = xor_str( "Z" ); break;
			case valve::e_item_index::ssg08: str_result = xor_str( "a" ); break;
			case valve::e_item_index::deagle: str_result = xor_str( "A" ); break;
			case valve::e_item_index::revolver: str_result = xor_str( "J" ); break;
			case valve::e_item_index::p2000: str_result = xor_str( "E" ); break;
			case valve::e_item_index::glock: str_result = xor_str( "D" ); break;
			case valve::e_item_index::usps: str_result = xor_str( "G" ); break;
			case valve::e_item_index::elite: str_result = xor_str( "B" ); break;
			case valve::e_item_index::c4: str_result = xor_str( "o" ); break;
			case valve::e_item_index::p250: str_result = xor_str( "F" ); break;
			case valve::e_item_index::aug: str_result = xor_str( "U" ); break;
			case valve::e_item_index::five_seven: str_result = xor_str( "C" ); break;
			case valve::e_item_index::ak47: str_result = xor_str( "W" ); break;
			case valve::e_item_index::galil: str_result = xor_str( "Q" ); break;
			case valve::e_item_index::cz75a: str_result = xor_str( "I" ); break;
			case valve::e_item_index::famas: str_result = xor_str( "R" ); break;
			case valve::e_item_index::tec9: str_result = xor_str( "H" ); break;
			case valve::e_item_index::bizon: str_result = xor_str( "M" ); break;
			case valve::e_item_index::m249: str_result = xor_str( "g" ); break;
			case valve::e_item_index::negev: str_result = xor_str( "f" ); break;
			case valve::e_item_index::nova: str_result = xor_str( "e" ); break;
			case valve::e_item_index::mag7: str_result = xor_str( "d" ); break;
			case valve::e_item_index::taser: str_result = xor_str( "h" ); break;
			case valve::e_item_index::he_grenade: str_result = xor_str( "j" ); break;
			case valve::e_item_index::smoke_grenade: str_result = xor_str( "k" ); break;
			case valve::e_item_index::frag_grenade: str_result = xor_str( "n" ); break;
			case valve::e_item_index::molotov: str_result = xor_str( "l" ); break;
			case valve::e_item_index::inc_grenade: str_result = xor_str( "n" ); break;
			case valve::e_item_index::sawed_off: str_result = xor_str( "c" ); break;
			case valve::e_item_index::decoy: str_result = xor_str( "m" ); break;
			case valve::e_item_index::flashbang: str_result = xor_str( "i" ); break;
			case valve::e_item_index::m4a4: str_result = xor_str( "S" ); break;
			case valve::e_item_index::m4a1s: str_result = xor_str( "T" ); break;
			case valve::e_item_index::firebomb: str_result = xor_str( "k" ); break;
			case valve::e_item_index::mac10: str_result = xor_str( "K" ); break;
			case valve::e_item_index::ump45: str_result = xor_str( "L" ); break;
			case valve::e_item_index::mp7: str_result = xor_str( "N" ); break;
			case valve::e_item_index::p90: str_result = xor_str( "P" ); break;
			case valve::e_item_index::mp9: str_result = xor_str( "N" ); break;
			case valve::e_item_index::sg553: str_result = xor_str( "V" ); break;
			case valve::e_item_index::xm1014: str_result = xor_str( "e" ); break;
			case valve::e_item_index::ta_grenade: str_result = xor_str( "i" ); break;
			default: break;
			}

			return str_result;
		}

		static bool is_entity_broken( valve::base_entity_t* entity );
		void draw_name( valve::cs_player_t* player, RECT& rect );
		void draw_box( valve::cs_player_t* player, RECT& rect );
		void draw_health( valve::cs_player_t* player, RECT& rect );
		void draw_wpn( valve::cs_player_t* player, RECT& rect );
		void draw_ammo( valve::cs_player_t* player, RECT& rect );
		void draw_flags( valve::cs_player_t* player, RECT& rect );
		void draw_lby_upd( valve::cs_player_t* player, RECT& rect );

		void molotov_timer( valve::base_entity_t* entity );
		void smoke_timer( valve::base_entity_t* entity );
		void grenade_projectiles( valve::base_entity_t* entity );

		struct shot_mdl_t {
			int                         m_player_index { };
			unsigned int                m_hash { };
			float						m_time { }, m_alpha{ 1.f }, m_is_death{ };
			valve::bones_t				m_bones { };
			sdk::mat3x4_t					m_world_matrix { };


			valve::model_render_info_t	m_info { };
			valve::draw_model_state_t	m_state { };
		};
		std::vector< shot_mdl_t >		m_shot_mdls { };
		using throwed_grenades_t = std::unordered_map< valve::ent_handle_t, grenade_simulation_t >;
		struct cfg_t {

			bool m_draw_name{ }, m_draw_health{ }, m_draw_box{ }, m_wpn_icon{ }, m_wpn_text{ }, m_wpn_ammo{ }, m_draw_flags{ },
				m_draw_lby{ }, m_oof_indicator{ }, m_glow{ }, m_bullet_tracers{ }, m_enemy_bullet_tracers{ }, m_bullet_impacts{ },
				 m_shadows_modulation { }, m_tone_map_modulation { }, m_fog { };

			bool m_keybinds_list{ };

			float m_wpn_ammo_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, 
				m_lby_upd_clr[ 4 ] = { 1.f, 1.f, 1.f },
				m_oof_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

			float m_view_model_fov{ 90.f }, m_x_dir { }, m_y_dir { }, m_z_dir { };
			int m_skybox_type{ }, m_bloom { }, m_exposure { }, m_fog_start { }, m_fog_end { }, m_fog_density { }, m_blend_in_scope_val { }, m_player_flags { }, m_removals{ };

			float m_world_modulation[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_props_modulation[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_sky_modulation[ 4 ]{ 1.f, 1.f, 1.f, 1.f }, m_glow_clr[ 4 ]{ 1.f, 1.f, 1.f, 1.f },
				m_bullet_tracers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_enemy_bullet_tracers_clr[ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_fog_clr[ 4 ]{ 1.f, 1.f, 1.f, 1.f },
				m_grenade_trajectory_clr[4]{ 1.f, 1.f, 1.f, 1.f }, m_grenade_proximity_warning_clr[4]{ 1.f, 1.f, 1.f, 1.f };

			bool  m_grenade_trajectory{ }, m_grenade_proximity_warning{ }, m_molotov_timer{ }, m_smoke_timer{ }, m_grenade_projectiles{ }, m_proj_wpn{ }, m_proj_icon{ }, m_manuals_indication { }, m_hit_markers { }, m_blend_in_scope { }, m_show_weapon_in_scope { };
		};

		struct flags_data_t {
			std::string m_name { };
			sdk::col_t m_clr { };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b31u, { } };

	public:
		struct shared_t {
			__forceinline shared_t( ) = default;

			void send_net_data( valve::cs_player_t* const player );
		} m_shared{ };

		RECT get_bbox( valve::cs_player_t* ent, bool is_valid = false );
		void handle_warning_pred( valve::base_entity_t* const entity, const valve::e_class_id class_id );
		void add_trail( const grenade_simulation_t& sim, const bool warning, sdk::col_t clr, float lifetime = 0.025f, float thickness = 0.2f ) const;
		void handle_player_drawings( );
		void handle_world_drawings( );
		void add_shot_mdl( valve::cs_player_t* player, const sdk::mat3x4_t* bones, bool is_death = false );
		void draw_shot_mdl( );
		void draw_glow( );
		void draw_scope_lines( );
		void removals( );
		void change_shadows( valve::base_entity_t* entity );
		void skybox_changer( );
		void draw_auto_peek( );
		void draw_key_binds( );
		void draw_beam( );
		void oof_indicators( valve::cs_player_t* );
		void manuals_indicators( );
		void on_create_move( const valve::user_cmd_t& cmd );
		bool add_grenade_simulation( const grenade_simulation_t& sim, const bool warning ) const;
		void tone_map_modulation( valve::base_entity_t* entity );
		void draw_hitmarkers( );
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
		std::array<dormant_data_t, 65> m_dormant_data { };
		std::array < bool, 65 > m_change_offset_due_to_lby{ };
		std::array < bool, 65 > m_change_offset_due_to_ammo{ };
		throwed_grenades_t				m_throwed_grenades{ };
		grenade_simulation_t			m_grenade_trajectory{ };

		int screen_x{ }, screen_y{ };

		struct inferno_info {
			std::vector< sdk::vec3_t > points;
			sdk::vec3_t entity_origin;
			sdk::vec3_t origin;
			float time_to_die;
			float range;
		};
		std::vector< inferno_info > inferno_information;

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
			float m_flExpTime;
			sdk::vec3_t m_start_pos{ }, m_end_pos{ };
			sdk::col_t col;
			int m_nIndex;
			int tickbase;
			bool ignore;
			bool m_bRing;
		};

		std::vector< bullet_trace_data_t > bullet_trace_info;
		void push_beam_info( bullet_trace_data_t beam_info );

		struct hit_marker_data_t {
			sdk::vec3_t m_pos{ };
			float m_spawn_time{ };
			float m_alpha{ };
		};

		std::vector< bullet_impact_t >	m_bullet_impacts{ };
		std::deque < hit_marker_data_t > m_hit_markers{ };

		bool m_has_death_chams[ 65 ]{ true };
		int m_cur_yaw_dir{ };
	};

	class c_dormant_esp {
	public:
		void start( );

		bool adjust_sound( valve::cs_player_t* player );
		void setup_adjust( valve::cs_player_t* player, valve::snd_info_t& sound );
		bool valid_sound( valve::snd_info_t& sound );

		struct SoundPlayer {
			void reset( bool store_data = false, const sdk::vec3_t& origin = { }, int flags = 0 )
			{
				if( store_data )
				{
					m_iReceiveTime = valve::g_global_vars.get( )->m_real_time;
					m_vecOrigin = origin;
					m_nFlags = flags;
				}
				else
				{
					m_iReceiveTime = 0.0f;
					m_vecOrigin = { };
					m_nFlags = 0;
				}
			}

			void Override( valve::snd_info_t& sound )
			{
				m_iReceiveTime = valve::g_global_vars.get( )->m_real_time;
				m_vecOrigin = *sound.m_pOrigin;
			}

			float m_iReceiveTime = 0.0f;
			sdk::vec3_t m_vecOrigin = { };
			int m_nFlags = { };
		} m_sound_players[ 65 ];

		valve::utl_vec_t <valve::snd_info_t> m_utlvecSoundBuffer;
		valve::utl_vec_t <valve::snd_info_t> m_utlCurSoundList;
	};

	inline std::unique_ptr < c_dormant_esp > g_dormant_esp = std::make_unique < c_dormant_esp >( );

	class c_chams {
	protected:
		valve::c_material* m_reg_mat { };
		valve::c_material* m_flat_mat { };
		valve::c_material* m_glow_mat { };
		valve::c_material* m_glow_overlay_mat { };
		valve::c_material* m_metallic_mat { };

		struct cfg_t {
			bool m_enemy_chams { }, m_local_chams { }, m_arms_chams { }, m_wpn_chams { }, m_shot_chams { }, m_history_chams { };
			bool m_enemy_chams_invisible{ };
			int m_enemy_chams_type { }, m_local_chams_type { }, m_arms_chams_type { }, m_wpn_chams_type { }, m_shot_chams_type { }, m_history_chams_type { }, m_invisible_enemy_chams_type { };
			float m_enemy_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_local_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				  m_arms_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_wpn_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f },
				  m_shot_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_history_clr [ 4 ] = { 1.f, 1.f, 1.f, 1.f }, m_invisible_enemy_clr [ 4 ] { 1.f, 1.f, 1.f, 1.f };
		};

		sdk::cfg_var_t< cfg_t > m_cfg { 0x05562b32u, { } };

	public:
		void init_chams( );
		bool draw_mdl( void* ecx, uintptr_t ctx, const valve::draw_model_state_t& state, const valve::model_render_info_t& info, sdk::mat3x4_t* bone );
		std::optional< valve::bones_t > try_to_lerp_bones( const int index ) const;
		void override_mat( int mat_type, sdk::col_t col, bool ignore_z );
		__forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
	};

	inline const std::unique_ptr < c_chams > g_chams = std::make_unique < c_chams >( );

	inline const std::unique_ptr < c_visuals > g_visuals = std::make_unique < c_visuals >( );

}