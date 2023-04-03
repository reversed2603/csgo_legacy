#include "../../csgo.hpp"

const char* enemy_chams_type [ ] = {
    "regular",
    "flat",
    "glow",
    "glow overlay",
    "metallic",
};

const char* stop_type_type[ ] = {
    "minimal",
    "full"
};

 const char* pitch_type[ ] = {
    "none",
    "down",
    "up"
};

 const char* force_head_conditions[ ] = {
     "body update",
     "walking",
     "resolved"
 };

 const char* body_conditions[ ] = {
     "in air",
     "no move data",
     "unresolved",
     "two shot",
     "double tapping"
 };

 const char* knife_type[ ] = {
     "none",
     "bayonet",
     "flip knife",
     "gut knife",
     "karambit",
     "m9 bayonet",
     "huntsman"
 };

 const char* cfg_slots[ ] = {
     "primary",
     "secondary",
     "additional",
     "additional_2",
     "additional_3",
     "extra"
 };

 const char* tabs[] = {
    "rage",
    "anti-aim",
    "misc",
    "config"
 };


 const char* skins_arr[ ] = {
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
 
const char* skybox_list [ ] =
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
    "fake duck",
    "r method",
    "is resolved",
    "lc"
};

const char* snipers_arr[ ] = {
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

const char* pistols_arr[ ] = {
    "none",
    "dual berettas",
    "heavy pistol"
};

const char* additional_arr[ ] = {
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

int cur_weapon { };
int cur_wpn_for_skins{ };

void draw_top( ImVec2 pos, ImDrawList* draw, int alpha ) {
    draw->AddRectFilled( ImVec2( pos.x + 0, pos.y ), ImVec2( pos.x + 940, pos.y + 57 ), ImColor( 15, 15, 15, alpha ) );  //data
   // draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 70, pos.y + 108 ), ImVec2( pos.x + 439, pos.y + 110 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
   // draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 513, pos.y + 108 ), ImVec2( pos.x + 880, pos.y + 110 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );

 // first rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 26 ), ImVec2( pos.x + 40, pos.y + 30 ), ImColor( 255, 255, 255, alpha - 115 ) );
    // second rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 34 ), ImVec2( pos.x + 40, pos.y + 38 ), ImColor( 255, 255, 255, alpha - 115 ) );
    // third rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 42 ), ImVec2( pos.x + 40, pos.y + 46 ), ImColor( 255, 255, 255, alpha - 115 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 57 ), ImVec2( pos.x + 940, pos.y + 57 ), ImColor( 35, 35, 35, alpha - 125 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 1 ), ImVec2( pos.x + 940, pos.y + 1 ), ImColor( 160, 160, 160, static_cast < int >( alpha ) - 55 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 687 ), ImVec2( pos.x + 940, pos.y + 687 ), ImColor( 40, 40, 40, static_cast < int >( alpha ) - 55 ) );
}

void extra_elements( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;

    if( cur_subtab == 0 ) {
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int >( alpha ) ), "Main" );
        draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int >( alpha ) ), "Extra" );
        ImGui::PopFont( );
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
        static int nahhh_bro{ 64 };
        static int i_fuck_who_nah_brooo{ -70 };
        static int nahhh_bro_{ 64 };
        static int i_fuck_who_nah_brooo_{ -70 };
        static int idontgiveafuck_{ 64 };
        static int idontgiveafuckliterally{ -70 };
        ImGui::SetCursorPos( ImVec2( 33.f, 110 ) );
        ImGui::Combo( xor_str( "knife" ), &csgo::hacks::g_skins->cfg( ).m_knife_type, knife_type, IM_ARRAYSIZE( knife_type ) );
        ImGui::SetCursorPos( ImVec2( 33.f, 140 ) );
        ImGui::Combo( xor_str( "weapon" ), &cur_wpn_for_skins, wpns_for_skins, IM_ARRAYSIZE( wpns_for_skins ) );

        ImGui::SetCursorPos( ImVec2( 71, 165 ) );

        /* skins via cur_wpn_for_skins idx */

        switch( cur_wpn_for_skins ) {
        case 0:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_scar20, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_scar20, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 1:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_g3sg1, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_g3sg1, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 2:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_deagle, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_deagle, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 3:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_elite, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_elite, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 4:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_five_seven, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_five_seven, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 5:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_glock, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_glock, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 6:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ak47, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ak47, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 7:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_aug, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_aug, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 8:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_awp, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_awp, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 9:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_famas, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_famas, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 10:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_galil, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_galil, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 11:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m249, 1, 600 );
           //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m249, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 12:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4a4, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4a4, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 13:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mac10, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mac10, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 14:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p90, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p90, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 15:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ump45, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ump45, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 16:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_xm1014, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_xm1014, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 17:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_bizon, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_bizon, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 18:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mag7, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mag7, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 19:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_negev, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_negev, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 20:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sawedoff, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sawedoff, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 21:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_tec9, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_tec9, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 22:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p2000, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p2000, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 23:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp7, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp7, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 24:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp9, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_mp9, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 25:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_nova, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_nova, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 26:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p250, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_p250, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 27:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sg553, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_sg553, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 28:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ssg08, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_ssg08, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 29:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4_s, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_m4_s, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 30:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_usp_s, 1, 600 );
          //  ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_usp_s, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 31:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_cz75a, 1, 600 );
          // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_cz75a, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 32:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_revolver, 1, 600 );
           // ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_revolver, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        case 33:
            ImGui::SliderInt( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_knifes, 1, 600 );
            //ImGui::Combo( xor_str( "skin" ), &csgo::hacks::g_skins->cfg( ).m_cur_skin_knifes, nahhh_bro_, i_fuck_who_nah_brooo_, skins_arr, IM_ARRAYSIZE( skins_arr ) );
            break;
        }
        ImGui::PopFont( );
    }

    ImGui::GetStyle( ).Alpha = backup_alpha;
}

/*
void anti_aim_elements( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;

    if( cur_subtab == 0 ) {

        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int >( alpha ) ), "Main" );
        draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int >( alpha ) ), "Extra" );
        ImGui::PopFont( );
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
        ImGui::SetCursorPos( ImVec2( 33, 90 ) );

        ImGui::Checkbox( xor_str( "anti aim" ), &csgo::hacks::g_anti_aim->cfg( ).m_anti_aim );

        ImGui::SetCursorPos( ImVec2( 33, 135 ) );

        ImGui::Combo( xor_str( "pitch" ), &csgo::hacks::g_anti_aim->cfg( ).m_pitch_type,pitch_type, IM_ARRAYSIZE( pitch_type ) );

        ImGui::SetCursorPos( ImVec2( 71, 155 ) );

        ImGui::SliderFloat( xor_str( "yaw" ), &csgo::hacks::g_anti_aim->cfg( ).m_real_yaw, -180.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 71, 185 ) );

        ImGui::SliderFloat( xor_str( "break delta" ), &csgo::hacks::g_anti_aim->cfg( ).m_flick_strength, 0.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 30, 210 ) );

        ImGui::Checkbox( xor_str( "breaker" ), &csgo::hacks::g_anti_aim->cfg( ).m_lby_breaker );

        ImGui::SetCursorPos( ImVec2( 30, 235 ) );

        ImGui::Checkbox( xor_str( "safe break" ), &csgo::hacks::g_anti_aim->cfg( ).m_change_flick_dir );

        ImGui::SetCursorPos( ImVec2( 73, 280 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual left" ), &csgo::hacks::g_anti_aim->cfg( ).m_left_manual, 33, 335, true );

        ImGui::SetCursorPos( ImVec2( 73, 320 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual right" ), &csgo::hacks::g_anti_aim->cfg( ).m_right_manual, 33, 420, true );

        ImGui::SetCursorPos( ImVec2( 73, 360 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual middle" ), &csgo::hacks::g_anti_aim->cfg( ).m_middle_manual, 33, 505, true );

        ImGui::SetCursorPos( ImVec2( 33, 400 ) );

        g_key_binds->KeybindNelfo( xor_str( "freestanding" ), &csgo::hacks::g_anti_aim->cfg( ).m_freestand, 33, 430, false );

        ImGui::SetCursorPos( ImVec2( 71, 425 ) );

        ImGui::SliderFloat( xor_str( "jitter" ), &csgo::hacks::g_anti_aim->cfg( ).m_jitter_yaw, -180.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 511, 130 ) );

        ImGui::SliderFloat( xor_str( "distortion speed" ), &csgo::hacks::g_anti_aim->cfg( ).m_distort_speed, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 511, 165 ) );

        ImGui::SliderFloat( xor_str( "distortion factor" ), &csgo::hacks::g_anti_aim->cfg( ).m_distort_factor, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509, 190 ) );

        ImGui::Checkbox( xor_str( "force turn" ), &csgo::hacks::g_anti_aim->cfg( ).m_force_turn );

        ImGui::SetCursorPos( ImVec2( 509, 215 ) );

        ImGui::Checkbox( xor_str( "shift" ), &csgo::hacks::g_anti_aim->cfg( ).m_shift );

        ImGui::SetCursorPos( ImVec2( 511, 250 ) );

        ImGui::SliderInt( xor_str( "await" ), &csgo::hacks::g_anti_aim->cfg( ).m_await_shift, 0, 10 );

        ImGui::SetCursorPos( ImVec2( 511, 285 ) );

        ImGui::SliderFloat( xor_str( "shift factor" ), &csgo::hacks::g_anti_aim->cfg( ).m_shift_factor, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509, 90 ) );

        ImGui::Checkbox( xor_str( "distortion" ), &csgo::hacks::g_anti_aim->cfg( ).m_should_distort );

        ImGui::SetCursorPos( ImVec2( 470, 330 ) );

        g_key_binds->KeybindNelfo( xor_str( "fake flick" ), &csgo::hacks::g_anti_aim->cfg( ).m_fake_flick, 510, 355, false, 770 );

        ImGui::PopFont( );
    } else {

        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int >( alpha ) ), "Main" );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int >( alpha - 230 ) ) );
        ImGui::PopFont( );

        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );

        ImGui::SetCursorPos( ImVec2( 120, 90 ) );

        ImGui::Checkbox( xor_str( "fake lag" ), &csgo::hacks::g_anti_aim->cfg( ).m_should_fake_lag );

        ImGui::SetCursorPos( ImVec2( 71, 130 ) );

        ImGui::SliderInt( xor_str( "lag ticks" ), &csgo::hacks::g_anti_aim->cfg( ).m_ticks_to_choke, 1, 14 );

        ImGui::SetCursorPos( ImVec2( 120, 155 ) );

        ImGui::Checkbox( xor_str( "disable while standing" ), &csgo::hacks::g_anti_aim->cfg( ).m_disable_lag_on_stand );

        ImGui::PopFont( );
    }

    ImGui::GetStyle( ).Alpha = backup_alpha;
}

*/



void draw_misc( ) {


    auto& misc_cfg = csgo::hacks::g_misc->cfg( );
    auto& move_cfg = csgo::hacks::g_move->cfg( );

    // movement
    ImGui::Checkbox( "bunny hop##misc", &move_cfg.m_bhop );
    ImGui::Checkbox( "automatic strafe##misc", &move_cfg.m_auto_strafe );
    ImGui::Checkbox( "standalone quick stop##misc", &move_cfg.m_fast_stop );
    ImGui::Checkbox( "infinite duck##misc", &move_cfg.m_infinity_duck );
    g_key_binds->Keybind( "slow motion##misc", &move_cfg.m_slow_walk, false, 140.f );


    // miscellaneous
    ImGui::SliderInt( "fov amount", &misc_cfg.m_camera_distance, 45, 130 );
    ImGui::Checkbox( "remove zoom on scope##misc", &misc_cfg.m_remove_zoom_on_second_scope );
    ImGui::Checkbox( "clantag spammer##misc", &misc_cfg.m_clan_tag );
    g_key_binds->Keybind( xor_str( "ping spike##misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_ping_spike_key, false, 140 );
    ImGui::SliderFloat( xor_str( "##fake_latency_value_misc" ), &csgo::hacks::g_ping_spike->cfg( ).m_to_spike, 50.f, 600.f, "%.1f" );
    g_key_binds->Keybind( "third person##misc", &misc_cfg.m_third_person_key, false, 140.f );
    ImGui::SliderFloat( "##third_person_dist_misc", &misc_cfg.m_third_person_dist, 45.f, 150.f );
    ImGui::Checkbox( "force thirdperson when spectating##misc", &misc_cfg.m_force_thirdperson_dead );

    ImGui::Checkbox( "aspect ratio##misc", &misc_cfg.m_aspect_ratio );

    if( misc_cfg.m_aspect_ratio )
        ImGui::SliderFloat( "##aspect_ratio_amt_misc", &misc_cfg.m_aspect_ratio_value, 0.f, 2.f );

    ImGui::Checkbox( "hitsound", &misc_cfg.m_hit_marker_sound );

    if( misc_cfg.m_hit_marker_sound )
        ImGui::Combo( "##hitsound_value_misc", &misc_cfg.m_hit_marker_sound_val, sounds_arr, IM_ARRAYSIZE( sounds_arr ) );

    ImGui::Checkbox( "spectator list", &misc_cfg.m_spectators );
    ImGui::Checkbox( "keybind list", &misc_cfg.m_key_binds );
}

#pragma region rage
void rage_hitbox( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_auto" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_scar_hitboxes &( 1 << i );
                ImGui::Selectable( hitboxes[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_scar_hitboxes |= ( 1 << i );
                else
                    cfg.m_scar_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_scout_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_scout_hitboxes |= ( 1 << i );
                else
                    cfg.m_scout_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_awp_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_awp_hitboxes |= ( 1 << i );
                else
                    cfg.m_awp_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "target hitboxes##rage_heavy_pistols" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_heavy_pistol_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_heavy_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_heavy_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "hitboxes#rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_pistol_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_pistol_hitboxes |= ( 1 << i );
                else
                    cfg.m_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "hitboxes#rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_other_hitboxes &( 1 << i );

                ImGui::Selectable( 
                    hitboxes[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_other_hitboxes |= ( 1 << i );
                else
                    cfg.m_other_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    default:
        break;
    }
}
void rage_pointscale( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_scar_point_scale, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_scout_point_scale, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_awp_point_scale, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_heavy_pistol_point_scale, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_pistol_point_scale, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderInt( xor_str("point scale"), &cfg.m_other_point_scale, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }
}
void rage_damage_ovr( ) {



    auto& cfg = csgo::hacks::g_aim_bot->cfg();


    switch (cur_weapon) {
    case 0:
        g_key_binds->Keybind("damage override", &cfg.m_min_scar_dmg_key, 140.f);
        break;
    case 1:
        g_key_binds->Keybind("damage override", &cfg.m_min_scout_dmg_key, 140.f);
        break;
    case 2:
        g_key_binds->Keybind("damage override", &cfg.m_min_awp_dmg_key, 140.f);
        break;
    case 3:
        g_key_binds->Keybind("damage override", &cfg.m_min_heavy_pistol_dmg_key, 140.f);
        break;
    case 4:
        g_key_binds->Keybind("damage override", &cfg.m_min_pistol_dmg_key, 140.f);
        break;
    case 5:
        g_key_binds->Keybind("damage override", &cfg.m_min_other_dmg_key, 140.f);
        break;
    default:
        break;
    }

    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_scar" ), &cfg.m_scar_min_dmg_on_key, 1, 126 );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_scout" ), &cfg.m_syka_min_dmg_on_key, 1, 126 );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_awp" ), &cfg.m_awp_min_dmg_on_key, 1, 126 );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_heavy_pistol" ), &cfg.m_heavy_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_pistol" ), &cfg.m_pistol_min_dmg_on_key, 1, 126 );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "##rage_dmg_override_other" ), &cfg.m_other_min_dmg_on_key, 1, 126 );
        break;
    default:
        break;
    }

}
void rage_autostop( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch (cur_weapon) {
    case 0:
        ImGui::Combo( xor_str( "auto stop##rage_scar" ), &cfg.m_auto_stop_type_scar, stop_type_type, IM_ARRAYSIZE( stop_type_type), -1);
        break;
    case 1:
        ImGui::Combo( xor_str( "auto stop##rage_scout" ), &cfg.m_auto_stop_type_scout, stop_type_type, IM_ARRAYSIZE( stop_type_type), -1);
        break;
    case 2:
        ImGui::Combo( xor_str( "auto stop##rage_awp" ), &cfg.m_auto_stop_type_awp, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1);
        break;
    case 3:
        ImGui::Combo( xor_str( "auto stop##rage_heavy_pistol" ), &cfg.m_auto_stop_type_heavy_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1);
        break;
    case 4:
        ImGui::Combo( xor_str( "auto stop##rage_pistol" ), &cfg.m_auto_stop_type_pistol, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1);
        break;
    case 5:
        ImGui::Combo( xor_str( "auto stop##rage_other" ), &cfg.m_auto_stop_type_other, stop_type_type, IM_ARRAYSIZE( stop_type_type ), -1);
        break;
    default:
        break;
    }

    ImGui::Checkbox( xor_str( "early stop" ), &cfg.m_early_autostop );
    ImGui::Checkbox( xor_str( "dynamic stop##rage" ), &cfg.m_between_shots_stop );

}
void rage_hitchance( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg();
    switch (cur_weapon) {
    case 0:
        ImGui::SliderFloat( xor_str( "hit chance##rage_scar" ), &cfg.m_hit_chance_scar, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "hit chance##rage_scout" ), &cfg.m_hit_chance_scout, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "hit chance##rage_awp" ), &cfg.m_hit_chance_awp, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "hit chance##rage_heavy_pistol" ), &cfg.m_hit_chance_heavy_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "hit chance##rage_pistol" ), &cfg.m_hit_chance_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "hit chance##rage_other" ), &cfg.m_hit_chance_other, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }
}
void rage_body_aim( ) {

    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str("prefer body-aim##rage_scar" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scar & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo();
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_scout & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_scout &= ~ ( 1 << i );
            }

            ImGui::EndCombo();
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[i] = cfg.m_force_body_conditions_awp & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if(hitgroups_vars[i])
                    cfg.m_force_body_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo();
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_heavy_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_heavy_pistol & ( 1 << i );
                ImGui::Selectable( body_conditions[i], &hitgroups_vars[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_heavy_pistol |= (1 << i);
                else
                    cfg.m_force_body_conditions_heavy_pistol &= ~(1 << i);
            }

            ImGui::EndCombo();
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_pistol & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo();
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "prefer body-aim##rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( body_conditions ) ]{ };

            for ( std::size_t i{ }; i < IM_ARRAYSIZE( body_conditions ); ++i ) {

                hitgroups_vars[ i ] = cfg.m_force_body_conditions_other & ( 1 << i );
                ImGui::Selectable( body_conditions[ i ], &hitgroups_vars[ i ], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_body_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_body_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo();
        }
        break;
    default:
        break;
    }

    g_key_binds->Keybind( xor_str( "force body-aim##rage" ), &cfg.m_baim_key, false, 160.f );
}

void rage_head_aim() {
    
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_scar" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scar &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scar |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 1:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_scout" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_scout &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_scout |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_scout &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 2:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_awp" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_awp &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_awp |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 3:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_heavy_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_heavy_pistol &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ], &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_heavy_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_heavy_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 4:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_pistol" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_pistol &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_pistol |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    case 5:
        if( ImGui::BeginCombo( xor_str( "prefer head-aim##rage_other" ), "" ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{ };

            for( std::size_t i{ }; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = cfg.m_force_head_conditions_other &( 1 << i );

                ImGui::Selectable( 
                    force_head_conditions[ i ],  &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if( hitgroups_vars[ i ] )
                    cfg.m_force_head_conditions_other |= ( 1 << i );
                else
                    cfg.m_force_head_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        break;
    default:
        break;
    }

}
void rage_damage( ) {
    
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );
    switch( cur_weapon ) {
    case 0:
        ImGui::SliderInt( xor_str( "minimum damage##rage_scar" ), &cfg.m_min_dmg_scar, 1.f, 126.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "minimum damage##rage_scout" ), &cfg.m_min_dmg_scout, 1.f, 126.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "minimum damage##rage_awp" ), &cfg.m_min_dmg_awp, 1.f, 126.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "minimum damage##rage_heavy_pistol" ), &cfg.m_min_dmg_heavy_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "minimum damage##rage_pistol" ), &cfg.m_min_dmg_pistol, 1.f, 126.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "minimum damage##rage_other" ), &cfg.m_min_dmg_other, 1.f, 126.f, "%.1f" );
        break;
    default:
        break;
    }
}

void draw_rage( ) {
    auto& cfg = csgo::hacks::g_aim_bot->cfg( );

    ImGui::Checkbox( xor_str( "enabled##rage" ), &cfg.m_rage_bot );
    ImGui::Combo( xor_str( "current weapon##rage" ), &cur_weapon, wpns, IM_ARRAYSIZE( wpns ) );
    ImGui::Checkbox( xor_str( "threading##rage" ), &cfg.m_threading );
    ImGui::Checkbox( xor_str( "fast record selection ( unsafe )##rage" ), &cfg.m_unsafe_record );
    ImGui::Checkbox( xor_str( "auto scope##rage" ), &cfg.m_auto_scope );

    rage_hitbox( );
    rage_pointscale( );
    rage_hitchance( ); 
    rage_damage( );
    rage_damage_ovr( );
    rage_body_aim( );
    rage_head_aim( );
    rage_autostop( );

    g_key_binds->Keybind( xor_str( "double tap##rage" ), &csgo::hacks::g_exploits->cfg( ).m_dt_key, false, 140.f );

}
#pragma endregion

void draw_config( ) {

    int& slot = csgo::g_ctx->cur_cfg_slot( );
    ImGui::Combo( xor_str( "cfg slot" ), &slot, cfg_slots, IM_ARRAYSIZE( cfg_slots ) );

    if( ImGui::Button( "load" ) )
        sdk::g_cfg->load( cfg_slots[slot] );

    if( ImGui::Button( "save" ) )
        sdk::g_cfg->save( cfg_slots[slot] );
}

namespace csgo {

    void c_menu::render( ) {

        if( m_main.m_hidden )
            return;

        ImGui::Begin( "hehe", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse );
        ImGui::StyleColorsClassic( );

        ImGui::Combo( "current tab", &m_main.m_current_tab, tabs, IM_ARRAYSIZE( tabs ) );


        switch ( m_main.m_current_tab ) {
        case 0:
            draw_rage( );
            break;
        case 1:
            break;
        case 2:
            draw_misc( );
            break;
        case 3:
            draw_config( );
            break;
        default:
            break;
        }

        ImGui::End( );
    }
}