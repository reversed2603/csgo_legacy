#pragma once

namespace csgo { 
    class c_menu { 
    private:
        struct { 
           
            struct { 
              
            } m_misc;

            bool m_hidden{ };
            int m_current_tab{ };
        } m_main{ };
    public:
        void render( );
        bool m_is_locked{ };

        __forceinline auto& main( );
    };

    inline const auto g_menu = std::make_unique< c_menu > ( );
}

#include "impl/menu.inl"

inline const char* chams_overlay_type[ ] = {
    "glow",
    "glow overlay",
};

inline const char* bullet_type[ ] = {
    "beam",
    "glow",
    "line"
};

inline const char* chams_type [ ] = {
    "regular",
    "flat",
    "metallic"
};

inline const char* stop_type_type[ ] = {
    "slow-motion",
    "full stop"
};

inline const char* stop_type_dt[] = {
    "default",
    "slow-motion",
    "full stop"
};

inline const char* pitch_type[ ] = {
    "none",
    "down",
    "up"
};

inline const char* force_head_conditions[ ] = {
    "body update",
    "walking",
    "resolved"
};

inline const char* auto_stop_modifiers[] = {
    "early",
    "dynamic"
};

inline const char* body_conditions[ ] = {
    "in air",
    "no move data",
    "unresolved",
    "two shot",
    "double tapping"
};

inline const char* knife_type[ ] = {
    "none",
    "bayonet",
    "flip knife",
    "gut knife",
    "karambit",
    "m9 bayonet",
    "huntsman"
};

inline const char* cfg_slots[ ] = {
    "primary",
    "secondary",
    "additional",
    "additional_2",
    "additional_3",
    "extra"
};

inline const char* tabs[] = {
    "rage",
    "anti-aim",
    "visuals",
    "movement",
    "misc",
    "config"
 };

inline const char* visuals_tabs[] = {
    "esp",
    "chams",
    "world",
 };

inline const char* skins_arr[ ] = {
     "none",
     "groundwater",
     "candy apple",
     "forest ddpat",
     "arctic camo",
     "desert storm",
     "bengal tiger",
     "copperhead",
     "skulls",
     "crimson web",
     "blue streak",
     "red laminate",
     "gunsmoke",
     "jungle tiger",
     "urban ddpat",
     "virus",
     "granite marbleized",
     "contrast spray",
     "forest leaves",
     "lichen dashed",
     "bone mask",
     "anodized navy",
     "snake camo",
     "silver",
     "hot rod",
     "metallic ddpat", // 25 idx
     "ossified",
     "blaze",
     "fade",
     "bulldozer",
     "night",
     "copper",
     "blue steel",
     "stained",
     "case hardened", // 34
     "contractor",
     "colony",
     "dragon tattoo", // 37
     "lighting strike"
 };
 
inline const char* skybox_list [ ] =
{ 
    ( "none" ),
    ( "tibet" ),
    ( "baggage" ),
    ( "italy" ),
    ( "aztec" ),
    ( "vertigo" ),
    ( "daylight" ),
    ( "daylight 2" ),
    ( "clouds" ),
    ( "clouds 2" ),
    ( "gray" ),
    ( "clear" ),
    ( "canals" ),
    ( "cobblestone" ),
    ( "assault" ),
    ( "clouds dark" ),
    ( "night" ),
    ( "night 2" ),
    ( "night flat" ),
    ( "dusty" ),
    ( "rainy" )
};

inline const char* hitboxes [ ] =
{ 
    "head",
    "chest",
    "stomach",
    "arms",
    "legs"
};

inline const char* esp_flags[ ] = { 
    "money",
    "ping",
    "armor",
    "scoped",
    "lc",
    "c4 holder",
    "flashed",
    "pin pulled",
    "lethal",
    "tickbase"
};

inline const char* weapon_selection[ ] = { 
    "text",
    "icon",
};

inline const char* grenade_selection[ ] = { 
    "text",
    "icon",
    "glow"
};

inline const char* removals[ ] = { 
   "scope",
   "second zoom",
   "flash effect",
   "smoke",
   "hands shaking",
   "post processing",
   "view kick",
   "view punch",
   "landing bob"
};

inline const char* fake_lag_triggers[ ] = { 
   "moving",
   "in air",
};

inline const char* hit_markers_option[ ] = {
   "screen",
   "world",
   "damage"
};

inline const char* grenade_traj_options[ ] = { 
   "local grenade trajectory",
   "friendly proximity warning",
   "enemy proximity warning",
   "local proximity warning",
};

inline const char* bullet_tracers_options[ ] = {
   "local",
   "teammate",
   "enemy"
};
 
inline const char* snipers_arr[ ] = {
    "none",
    "auto",
    "awp",
    "ssg 08"
};

inline const char* sounds_arr[ ] = { 
    "neverlose",
    "phonk",
    "skeet",
    "primordial",
    "cock",
    "bepis"
};

inline const char* pistols_arr[ ] = {
    "none",
    "five-seven",
    "tec-9",
    "dual berettas",
    "heavy pistols"
};

inline const char* notification_logs[ ] = {
    "hit",
    "harm",
    "missed shots",
    "rage-bot fire"
};

inline const char* additional_arr[ ] = {
    "grenades",
    "armor",
    "taser",
    "defuser"
};

inline const char* wpns[ ] = { 
    "scar",
    "scout",
    "awp",
    "heavy pistols",
    "pistols",
    "other"
};

inline const char* scan_intensity[] = { 
    "off",
    "low",
    "medium",
    "high",
    "maximum"
};

inline const char* intensity[] = { 
    "off",
    "medium",
    "high"
};

inline const char* wpns_for_skins[ ] = { 
    "scar",
    "g3sg1",
    "deagle",
    "elite",
    "five-seven",
    "glock",
    "ak-47",
    "aug",
    "awp",
    "famas",
    "galil",
    "m249",
    "m4a4",
    "mac-10",
    "p90",
    "ump-45",
    "xm-1014",
    "bizon",
    "mag-7",
    "negev",
    "sawed-off",
    "tec-9",
    "p2000",
    "mp-7",
    "mp-9",
    "nova",
    "p250",
    "sg-553",
    "ssg-08",
    "m4a1-s",
    "usp-s",
    "cz75a",
    "revolver",
    "knife"
};