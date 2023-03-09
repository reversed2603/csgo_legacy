#pragma once

namespace csgo::hacks {
	struct pen_data_t {
		valve::cs_player_t* m_hit_player{};
		int					m_dmg{}, m_hitbox{}, m_hitgroup{}, m_remaining_pen{};
	};
	struct auto_wall_data_t {
		int m_dmg {};
		int m_hitbox {};
		int m_hitgroup{}, m_remaining_pen{};
		auto_wall_data_t ( ) { }

		auto_wall_data_t ( int dmg, int hitbox, int hitgroup, int remaining_pen ) {
			m_dmg = dmg;
			m_hitbox = hitbox;
			m_hitgroup = hitgroup;
			m_remaining_pen = remaining_pen;
		}
	};

	enum contents_t {
		CONTENTS_EMPTY = 0,
		CONTENTS_SOLID = 0x1,
		CONTENTS_WINDOW = 0x2,
		CONTENTS_AUX = 0x4,
		CONTENTS_GRATE = 0x8,
		CONTENTS_SLIME = 0x10,
		CONTENTS_WATER = 0x20,
		CONTENTS_BLOCKLOS = 0x40,
		CONTENTS_OPAQUE = 0x80,
		CONTENTS_TESTFOGVOLUME = 0x100,
		CONTENTS_UNUSED = 0x200,
		CONTENTS_BLOCKLIGHT = 0x400,
		CONTENTS_TEAM1 = 0x800,
		CONTENTS_TEAM2 = 0x1000,
		CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
		CONTENTS_MOVEABLE = 0x4000,
		CONTENTS_AREAPORTAL = 0x8000,
		CONTENTS_PLAYERCLIP = 0x10000,
		CONTENTS_MONSTERCLIP = 0x20000,
		CONTENTS_CURRENT_0 = 0x40000,
		CONTENTS_CURRENT_90 = 0x80000,
		CONTENTS_CURRENT_180 = 0x100000,
		CONTENTS_CURRENT_270 = 0x200000,
		CONTENTS_CURRENT_UP = 0x400000,
		CONTENTS_CURRENT_DOWN = 0x800000,
		CONTENTS_ORIGIN = 0x1000000,
		CONTENTS_MONSTER = 0x2000000,
		CONTENTS_DEBRIS = 0x4000000,
		CONTENTS_DETAIL = 0x8000000,
		CONTENTS_TRANSLUCENT = 0x10000000,
		CONTENTS_LADDER = 0x20000000,
		CONTENTS_HITBOX = 0x40000000,
	};

	enum masks_t {
		MASK_ALL = 0xFFFFFFFF,
		MASK_SOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
		MASK_PLAYERSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
		MASK_NPCSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
		MASK_NPCFLUID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
		MASK_WATER = CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME,
		MASK_OPAQUE = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE,
		MASK_OPAQUE_AND_NPCS = MASK_OPAQUE | CONTENTS_MONSTER,
		MASK_BLOCKLOS = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS,
		MASK_BLOCKLOS_AND_NPCS = MASK_BLOCKLOS | CONTENTS_MONSTER,
		MASK_VISIBLE = MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE,
		MASK_VISIBLE_AND_NPCS = MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE,
		MASK_SHOT = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE | CONTENTS_HITBOX,
		MASK_SHOT_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_DEBRIS,
		MASK_SHOT_HULL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE,
		MASK_SHOT_PORTAL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER,
		MASK_SOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE,
		MASK_PLAYERSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE,
		MASK_NPCSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
		MASK_NPCWORLDSTATIC = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
		MASK_NPCWORLDSTATIC_FLUID = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP,
		MASK_SPLITAREPORTAL = CONTENTS_WATER | CONTENTS_SLIME,
		MASK_CURRENT = CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN,
		MASK_DEADSOLID = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE,
	};

	class c_auto_wall {
	public:
		__forceinline bool is_breakable ( valve::base_entity_t* entity );
		void scale_dmg ( valve::cs_player_t* player, valve::trace_t& trace, valve::weapon_info_t* wpn_info, float& cur_dmg, const int hit_group );
		bool trace_to_exit ( const sdk::vec3_t& src, const sdk::vec3_t& dir,
			const valve::trace_t& enter_trace, valve::trace_t& exit_trace );
		bool handle_bullet_penetration ( 
			valve::weapon_info_t* wpn_data, valve::trace_t& enter_trace, sdk::vec3_t& eye_pos, const sdk::vec3_t& direction, int& possible_hits_remain,
			float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration 
		);
		bool fire_bullet ( valve::cs_weapon_t* wpn, sdk::vec3_t& direction, bool& visible, float& cur_dmg, int& remaining_pen, int& hit_group,
			int& hitbox, valve::base_entity_t* e = nullptr, float length = 0.f, const sdk::vec3_t& pos = { 0.f,0.f,0.f } );
		auto_wall_data_t wall_penetration ( sdk::vec3_t& eye_pos, sdk::vec3_t& point, valve::cs_player_t* e );

		pen_data_t fire_emulated(
			valve::cs_player_t* const shooter, valve::cs_player_t* const target, sdk::vec3_t src, const sdk::vec3_t& dst
		);
	};

	inline const std::unique_ptr < c_auto_wall > g_auto_wall = std::make_unique < c_auto_wall > ( );
}

#include "impl/auto_wall.inl"