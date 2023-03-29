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
     "lby update",
     "running",
     "resolved",
     "1 bullet remain"
 };

 const char* force_body_conditions[ ] = {
     "lethal",
     "in air",
     "no move data",
     "fake"
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

int cur_wpn_blyat {};
int cur_wpn_for_skins{};

void draw_top( ImVec2 pos, ImDrawList* draw, int alpha ) {
    draw->AddRectFilled( ImVec2( pos.x + 0, pos.y ), ImVec2( pos.x + 940, pos.y + 57 ), ImColor( 15, 15, 15, alpha ) );  //data
   // draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 70, pos.y + 108 ), ImVec2( pos.x + 439, pos.y + 110 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
   // draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 513, pos.y + 108 ), ImVec2( pos.x + 880, pos.y + 110 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );

 // first rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 26 ), ImVec2( pos.x + 40, pos.y + 30 ), ImColor( 255, 255, 255, alpha - 115 ) );
    // second rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 34 ), ImVec2( pos.x + 40, pos.y + 38 ), ImColor( 255, 255, 255, alpha - 115 ) );
    // third rect for tabs
    draw->AddRectFilled( ImVec2( pos.x + 15, pos.y + 42 ), ImVec2( pos.x + 40, pos.y + 46 ), ImColor( 255, 255, 255, alpha - 115 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 57 ), ImVec2( pos.x + 940, pos.y + 57 ), ImColor( 35, 35, 35, alpha - 125 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 1 ), ImVec2( pos.x + 940, pos.y + 1 ), ImColor( 160, 160, 160, static_cast < int > ( alpha ) - 55 ) );

    draw->AddLine( ImVec2( pos.x, pos.y + 687 ), ImVec2( pos.x + 940, pos.y + 687 ), ImColor( 40, 40, 40, static_cast < int > ( alpha ) - 55 ) );
}

void extra_elements( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;

    if ( cur_subtab == 0 ) {
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
        draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Extra" );
        ImGui::PopFont( );
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
        static int nahhh_bro{ 64 };
        static int i_fuck_who_nah_brooo{ -70 };
        static int nahhh_bro_{ 64 };
        static int i_fuck_who_nah_brooo_{ -70 };
        static int idontgiveafuck_{ 64 };
        static int idontgiveafuckliterally{ -70 };
        ImGui::SetCursorPos( ImVec2( 33.f, 110 ) );
        ImGui::Combo( xor_str( "knife" ), &csgo::hacks::g_skins->cfg( ).m_knife_type, nahhh_bro, i_fuck_who_nah_brooo, knife_type, IM_ARRAYSIZE( knife_type ) );
        ImGui::SetCursorPos( ImVec2( 33.f, 140 ) );
        ImGui::Combo( xor_str( "weapon" ), &cur_wpn_for_skins, idontgiveafuck_, idontgiveafuckliterally, wpns_for_skins, IM_ARRAYSIZE( wpns_for_skins ) );

        ImGui::SetCursorPos( ImVec2( 71, 165 ) );

        /* skins via cur_wpn_for_skins idx */

        switch ( cur_wpn_for_skins ) {
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

void anti_aim_elements( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;

    if ( cur_subtab == 0 ) {

        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
        draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Extra" );
        ImGui::PopFont( );
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
        ImGui::SetCursorPos( ImVec2( 33, 90 ) );

        ImGui::Checkbox( xor_str( "anti aim" ), &csgo::hacks::g_anti_aim->cfg( ).m_anti_aim );

        static int nahhh_bro{ 64 };
        static int i_fuck_who_nah_brooo{ -70 };
        static int nahhh_bro_{ 64 };
        static int i_fuck_who_nah_brooo_{ -70 };
        static int nahhh_bro__{ 64 };
        static int i_fuck_who_nah_brooo__{ -70 };
        static int nahhh_bro___{ 64 };
        static int i_fuck_who_nah_brooo___{ -70 };
        static int nahhh_bro____{ 64 };
        static int i_fuck_who_nah_brooo____{ -70 };
        static int iloveallah{ 64 };
        static int wholoveallah{ -70 };
        ImGui::SetCursorPos( ImVec2( 33, 135 ) );

        ImGui::Combo( xor_str( "pitch" ), &csgo::hacks::g_anti_aim->cfg( ).m_pitch_type, nahhh_bro, i_fuck_who_nah_brooo, pitch_type, IM_ARRAYSIZE( pitch_type ) );

        ImGui::SetCursorPos( ImVec2( 71, 155 ) );

        ImGui::SliderFloat( xor_str( "yaw" ), &csgo::hacks::g_anti_aim->cfg( ).m_real_yaw, -180.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 71, 185 ) );

        ImGui::SliderFloat( xor_str( "break delta" ), &csgo::hacks::g_anti_aim->cfg( ).m_flick_strength, 0.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 30, 210 ) );

        ImGui::Checkbox( xor_str( "breaker" ), &csgo::hacks::g_anti_aim->cfg( ).m_lby_breaker );

        ImGui::SetCursorPos( ImVec2( 30, 235 ) );

        ImGui::Checkbox( xor_str( "safe break" ), &csgo::hacks::g_anti_aim->cfg( ).m_change_flick_dir );

        ImGui::SetCursorPos( ImVec2( 73, 280 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual left" ), &csgo::hacks::g_anti_aim->cfg( ).m_left_manual, nahhh_bro_, 73, 335, i_fuck_who_nah_brooo_, true );

        ImGui::SetCursorPos( ImVec2( 73, 320 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual right" ), &csgo::hacks::g_anti_aim->cfg( ).m_right_manual, nahhh_bro__, 73, 420, i_fuck_who_nah_brooo__, true );

        ImGui::SetCursorPos( ImVec2( 73, 360 ) );

        g_key_binds->KeybindNelfo( xor_str( "manual middle" ), &csgo::hacks::g_anti_aim->cfg( ).m_middle_manual, nahhh_bro___, 73, 505, i_fuck_who_nah_brooo___, true );

        ImGui::SetCursorPos( ImVec2( 33, 400 ) );

        g_key_binds->KeybindNelfo( xor_str( "freestanding" ), &csgo::hacks::g_anti_aim->cfg( ).m_freestand, nahhh_bro____, 73, 430, i_fuck_who_nah_brooo____, false );

        ImGui::SetCursorPos( ImVec2( 71, 425 ) );

        ImGui::SliderFloat( xor_str( "jitter" ), &csgo::hacks::g_anti_aim->cfg( ).m_jitter_yaw, -180.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 511, 130 ) );

        ImGui::SliderFloat( xor_str( "distortion speed" ), &csgo::hacks::g_anti_aim->cfg( ).m_distort_speed, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 511, 165 ) );

        ImGui::SliderFloat( xor_str( "distortion factor" ), &csgo::hacks::g_anti_aim->cfg( ).m_distort_factor, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509, 190 ) );

        ImGui::Checkbox( xor_str( "force turn" ), &csgo::hacks::g_anti_aim->cfg( ).m_force_turn, 510.f );

        ImGui::SetCursorPos( ImVec2( 509, 215 ) );

        ImGui::Checkbox( xor_str( "shift" ), &csgo::hacks::g_anti_aim->cfg( ).m_shift, 510.f );

        ImGui::SetCursorPos( ImVec2( 511, 250 ) );

        ImGui::SliderInt( xor_str( "await" ), &csgo::hacks::g_anti_aim->cfg( ).m_await_shift, 0, 10 );

        ImGui::SetCursorPos( ImVec2( 511, 285 ) );

        ImGui::SliderFloat( xor_str( "shift factor" ), &csgo::hacks::g_anti_aim->cfg( ).m_shift_factor, 0.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509, 90 ) );

        ImGui::Checkbox( xor_str( "distortion" ), &csgo::hacks::g_anti_aim->cfg( ).m_should_distort, 510.f );

        ImGui::SetCursorPos( ImVec2( 470, 330 ) );

        g_key_binds->KeybindNelfo( xor_str( "fake flick" ), &csgo::hacks::g_anti_aim->cfg( ).m_fake_flick, iloveallah, 510, 355, wholoveallah, false, 770 );

        ImGui::PopFont( );
    } else {

        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
        draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
        draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
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

void rage_bot_elements( ImVec2 pos, float alpha, ImDrawList* draw_list ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;

    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
    draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
    draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Extra" );
    ImGui::PopFont( );
    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    ImGui::SetCursorPos( ImVec2( 33, 110 ) );
    static int nahhh_bro{ 64 };
    static int i_fuck_who_nah_brooo{ -70 };
    ImGui::Combo( "current weapon", &cur_wpn_blyat, nahhh_bro, i_fuck_who_nah_brooo, wpns, IM_ARRAYSIZE( wpns ) );
    ImGui::SetCursorPos( ImVec2( 70, 120 ) );
    ImGui::Checkbox( xor_str( "rage bot" ), &csgo::hacks::g_aim_bot->cfg( ).m_rage_bot );

    ImGui::SetCursorPos( ImVec2( 70, 145 ) );
    ImGui::Checkbox( xor_str( "threading" ), &csgo::hacks::g_aim_bot->cfg( ).m_threading );

    ImGui::SetCursorPos( ImVec2( 71, 180 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_scar_head_scale, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_scout_head_scale, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_awp_head_scale, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_head_scale, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_pistol_head_scale, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "point scale" ), &csgo::hacks::g_aim_bot->cfg( ).m_other_head_scale, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 71, 220 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_scar, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_scout, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_awp, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_heavy_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "min dmg" ), &csgo::hacks::g_aim_bot->cfg( ).m_min_dmg_other, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }
    static int nahhh_bro____________{ 64 };
    static int i_fuck_who_nah_brooo________________{ -70 };

    ImGui::SetCursorPos( ImVec2( 33.5f, 280 ) );

     switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_scar, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    case 1:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_scout, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    case 2:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_awp, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    case 3:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_heavy_pistol, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    case 4:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_pistol, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    case 5:
        ImGui::Combo( xor_str( "stop type" ), &csgo::hacks::g_move->cfg( ).m_auto_stop_type_other, nahhh_bro____________, i_fuck_who_nah_brooo________________, stop_type_type, IM_ARRAYSIZE( stop_type_type ) );
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 33.5f, 310 ) );

    static int i_fuck_kids{ -70 };

    i_fuck_kids += 2;

    if ( i_fuck_kids > 0 )
        i_fuck_kids = 0;


    static int i_want_to_kys{ 64 };

    switch ( cur_wpn_blyat ) {
    case 0:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_scar_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_scar_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_scar_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    case 1:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_scout_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_scout_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_scout_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    case 2:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_awp_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_awp_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_awp_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    case 3:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    case 4:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_pistol_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_pistol_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_pistol_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    case 5:
        if ( ImGui::BeginCombo( xor_str( "hitboxes" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( hitboxes ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( hitboxes ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_other_hitboxes & ( 1 << i );

                ImGui::Selectable(
                    hitboxes[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_other_hitboxes |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_other_hitboxes &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 71, 330 ) );
    switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_scar, 1.f, 100.f, "%.1f" );
        break;
    case 1:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_scout, 1.f, 100.f, "%.1f" );
        break;
    case 2:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_awp, 1.f, 100.f, "%.1f" );
        break;
    case 3:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_heavy_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 4:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_pistol, 1.f, 100.f, "%.1f" );
        break;
    case 5:
        ImGui::SliderFloat( xor_str( "hit chance" ), &csgo::hacks::g_aim_bot->cfg( ).m_hit_chance_other, 1.f, 100.f, "%.1f" );
        break;
    default:
        break;
    }


    ImGui::SetCursorPos( ImVec2( 70, 350 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_scar_prefer_body );
        break;
    case 1:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_scout_prefer_body );
        break;
    case 2:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_awp_prefer_body );
        break;
    case 3:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_prefer_body );
        break;
    case 4:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_pistol_prefer_body );
        break;
    case 5:
        ImGui::Checkbox( xor_str( "prefer body" ), &csgo::hacks::g_aim_bot->cfg( ).m_other_prefer_body );
        break;
    default:
        break;
    }

    /* keybinds */

    ImGui::SetCursorPos( ImVec2( 33, 390 ) );

    static int nahhh_bro__________{ 64 };
    static int i_fuck_who_nah_brooo_______________{ -70 };

    switch ( cur_wpn_blyat ) {
    case 0:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_scar_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    case 1:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_scout_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    case 2:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_awp_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    case 3:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_heavy_pistol_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    case 4:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_pistol_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    case 5:
        g_key_binds->KeybindNelfo( "dmg override", &csgo::hacks::g_aim_bot->cfg( ).m_min_other_dmg_key, nahhh_bro__________, 73, 420, i_fuck_who_nah_brooo_______________ );
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 71, 415 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_scar_min_dmg_on_key, 1, 100 );
        break;
    case 1:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_syka_min_dmg_on_key, 1, 100 );
        break;
    case 2:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_awp_min_dmg_on_key, 1, 100 );
        break;
    case 3:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_heavy_pistol_min_dmg_on_key, 1, 100 );
        break;
    case 4:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_pistol_min_dmg_on_key, 1, 100 );
        break;
    case 5:
        ImGui::SliderInt( xor_str( "min dmg on key" ), &csgo::hacks::g_aim_bot->cfg( ).m_other_min_dmg_on_key, 1, 100 );
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 33, 465 ) );

    static int dont_care{ 64 };
    static int i_fuck_who{ -70 };

    g_key_binds->KeybindNelfo( xor_str( "double tap" ), &csgo::hacks::g_exploits->cfg( ).m_dt_key, dont_care, 74, 495, i_fuck_who );

    ImGui::SetCursorPos( ImVec2( 470, 110 ) );

    static int dont_care___{ 64 };
    static int i_fuck_who___{ -70 };

    g_key_binds->KeybindNelfo( xor_str( "force body" ), &csgo::hacks::g_aim_bot->cfg( ).m_baim_key, dont_care___, 510, 140, i_fuck_who___, false, 770 );

    ImGui::SetCursorPos( ImVec2( 510.f, 125 ) );

    ImGui::Checkbox( xor_str( "auto scope" ), &csgo::hacks::g_aim_bot->cfg( ).m_auto_scope, 510.f );

    static int i_want_to_rape_niggas{ 64 };
    static int i_hate_all_these_monkey_niggas_black_idiots{ -70 };
    i_hate_all_these_monkey_niggas_black_idiots += 2;

    if ( i_hate_all_these_monkey_niggas_black_idiots > 0 )
        i_hate_all_these_monkey_niggas_black_idiots = 0;

    static int i_want_to_rape_niggas__{ 64 };
    static int i_hate_all_these_monkey_niggas_black_idiots__{ -70 };
    i_hate_all_these_monkey_niggas_black_idiots__ += 2;

    if ( i_hate_all_these_monkey_niggas_black_idiots__ > 0 )
        i_hate_all_these_monkey_niggas_black_idiots__ = 0;

    ImGui::SetCursorPos( ImVec2( 473, 170 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scar & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scar |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    case 1:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scout & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scout |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_scout &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    case 2:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_awp & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_awp |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    case 3:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_heavy_pistol & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_heavy_pistol |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_heavy_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    case 4:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_pistol & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_pistol |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    case 5:
        if ( ImGui::BeginCombo( xor_str( "force head conditions" ), "", i_want_to_rape_niggas ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_head_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_head_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_other & ( 1 << i );

                ImGui::Selectable(
                    force_head_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_other |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_head_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots = -70;
        break;
    default:
        break;
    }

    ImGui::SetCursorPos( ImVec2( 510, 182 ) );

    ImGui::Checkbox( xor_str( "early autostop" ), &csgo::hacks::g_aim_bot->cfg( ).m_early_autostop, 510 );

    ImGui::SetCursorPos( ImVec2( 473, 230 ) );

    switch ( cur_wpn_blyat ) {
    case 0:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scar & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scar |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scar &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    case 1:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scout & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scout |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_scout &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    case 2:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_awp & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_awp |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_awp &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    case 3:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_heavy_pistol & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_heavy_pistol |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_heavy_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    case 4:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_pistol & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_pistol |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_pistol &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    case 5:
        if ( ImGui::BeginCombo( xor_str( "force body conditions" ), "", i_want_to_rape_niggas__ ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( force_body_conditions ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( force_body_conditions ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_other & ( 1 << i );

                ImGui::Selectable(
                    force_body_conditions[ i ], i_hate_all_these_monkey_niggas_black_idiots__, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_other |= ( 1 << i );
                else
                    csgo::hacks::g_aim_bot->cfg( ).m_force_body_conditions_other &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_hate_all_these_monkey_niggas_black_idiots__ = -70;
        break;
    default:
        break;
    }

    ImGui::PopFont( );

    ImGui::GetStyle( ).Alpha = backup_alpha;
}

void visuals_tab( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;
    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
    draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
    draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Extra" );
    ImGui::PopFont( );
    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    if ( cur_subtab == 0 ) {
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular ); 

        ImGui::SetCursorPos( ImVec2( 120, 90 ) );
        ImGui::Checkbox( xor_str( "draw name" ), &csgo::hacks::g_visuals->cfg( ).m_draw_name );

        ImGui::SetCursorPos( ImVec2( 120, 115 ) );
        ImGui::Checkbox( xor_str( "draw health" ), &csgo::hacks::g_visuals->cfg( ).m_draw_health );

        ImGui::SetCursorPos( ImVec2( 120, 140 ) );
        ImGui::Checkbox( xor_str( "draw box" ), &csgo::hacks::g_visuals->cfg( ).m_draw_box );

        ImGui::SetCursorPos( ImVec2( 120, 165 ) );
        ImGui::Checkbox( xor_str( "draw wpn icon" ), &csgo::hacks::g_visuals->cfg( ).m_wpn_icon );

        ImGui::SetCursorPos( ImVec2( 120, 190 ) );
        ImGui::Checkbox( xor_str( "draw wpn txt" ), &csgo::hacks::g_visuals->cfg( ).m_wpn_text );

        ImGui::SetCursorPos( ImVec2( 120, 215 ) );
        ImGui::Checkbox( xor_str( "draw ammo" ), &csgo::hacks::g_visuals->cfg( ).m_wpn_ammo );

        ImGui::SetCursorPos( ImVec2( 120, 240 ) );
        ImGui::Checkbox( xor_str( "draw flags" ), &csgo::hacks::g_visuals->cfg( ).m_draw_flags );

        ImGui::SetCursorPos( ImVec2( 33, 281 ) );
        static int i_fuck_kids{ -70 };

        i_fuck_kids += 2;

        if ( i_fuck_kids > 0 )
            i_fuck_kids = 0;


        static int i_want_to_kys{ 64 };
        if ( ImGui::BeginCombo( xor_str( "flags" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( esp_flags ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( esp_flags ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_visuals->cfg( ).m_player_flags & ( 1 << i );

                ImGui::Selectable(
                    esp_flags[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_visuals->cfg( ).m_player_flags |= ( 1 << i );
                else
                    csgo::hacks::g_visuals->cfg( ).m_player_flags &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;

        ImGui::SetCursorPos( ImVec2( 120, 290 ) );
        ImGui::Checkbox( xor_str( "draw oof" ), &csgo::hacks::g_visuals->cfg( ).m_oof_indicator );

        ImGui::SetCursorPos( ImVec2( 120, 315 ) );
        ImGui::Checkbox( xor_str( "draw lby timer" ), &csgo::hacks::g_visuals->cfg( ).m_draw_lby );

        ImGui::SetCursorPos( ImVec2( 120, 340 ) );
        ImGui::Checkbox( xor_str( "glow" ), &csgo::hacks::g_visuals->cfg( ).m_glow );

        ImGui::SetCursorPos( ImVec2( 72, 385 ) );
        ImGui::ColorEdit4( xor_str( "glow clr" ), csgo::hacks::g_visuals->cfg( ).m_glow_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509, 90 ) );

        ImGui::Checkbox( xor_str( "enemy chams" ), &csgo::hacks::g_chams->cfg( ).m_enemy_chams, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 130 ) );

        static int nahhh_bro{ 64 };
        static int i_fuck_who_nah_brooo{ -70 };
        ImGui::Combo( xor_str( "chams type" ), &csgo::hacks::g_chams->cfg( ).m_enemy_chams_type, nahhh_bro, i_fuck_who_nah_brooo, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 513, 160 ) );

        ImGui::ColorEdit4( xor_str( "enemy chams clr" ), csgo::hacks::g_chams->cfg( ).m_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        static int I_HATE_NIGGAS{ 64 };
        static int wtffffff_umad{ -70 };


        ImGui::SetCursorPos( ImVec2( 472, 190 ) );

        ImGui::Combo( xor_str( "invisible type" ), &csgo::hacks::g_chams->cfg( ).m_invisible_enemy_chams_type, I_HATE_NIGGAS, wtffffff_umad, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 511, 220 ) );

        ImGui::ColorEdit4( xor_str( "invisible chams clr" ), csgo::hacks::g_chams->cfg( ).m_invisible_enemy_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );


        ImGui::SetCursorPos( ImVec2( 509, 230 ) );

        ImGui::Checkbox( xor_str( "arms chams" ), &csgo::hacks::g_chams->cfg( ).m_arms_chams, 509.f );
        static int idk{ 64 };
        static int noonecares{ -70 };
        ImGui::SetCursorPos( ImVec2( 472, 270 ) );
        ImGui::Combo( xor_str( "arms type" ), &csgo::hacks::g_chams->cfg( ).m_arms_chams_type, idk, noonecares, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 512, 298 ) );

        ImGui::ColorEdit4( xor_str( "arms chams clr" ), csgo::hacks::g_chams->cfg( ).m_arms_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509.f, 310 ) );

        ImGui::Checkbox( xor_str( "weapon chams" ), &csgo::hacks::g_chams->cfg( ).m_wpn_chams, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 350 ) );
        static int allah{ 64 };
        static int akbar{ -70 };
        ImGui::Combo( xor_str( "weapon type" ), &csgo::hacks::g_chams->cfg( ).m_wpn_chams_type, allah, akbar, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 511, 378 ) );

        ImGui::ColorEdit4( xor_str( "weapon chams clr" ), csgo::hacks::g_chams->cfg( ).m_wpn_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509.f, 390 ) );

        ImGui::Checkbox( xor_str( "shot chams" ), &csgo::hacks::g_chams->cfg( ).m_shot_chams, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 430 ) );

        static int nigga{ 64 };
        static int i_hate_them{ -70 };

        ImGui::Combo( xor_str( "shot type" ), &csgo::hacks::g_chams->cfg( ).m_shot_chams_type, nigga, i_hate_them, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 511, 458 ) );

        ImGui::ColorEdit4( xor_str( "shot chams clr" ), csgo::hacks::g_chams->cfg( ).m_shot_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509.f, 470 ) );

        ImGui::Checkbox( xor_str( "history chams" ), &csgo::hacks::g_chams->cfg( ).m_history_chams, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 510 ) );

        static int i_dont_care_who_tf_{ 64 };
        static int are_u{ -70 };

        ImGui::Combo( xor_str( "history type" ), &csgo::hacks::g_chams->cfg( ).m_history_chams_type, i_dont_care_who_tf_, are_u, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 511, 538 ) );

        ImGui::ColorEdit4( xor_str( "history chams clr" ), csgo::hacks::g_chams->cfg( ).m_history_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        static int wtfff{ 64 };
        static int i_fuck_no_fkin_way{ -70 };

        ImGui::SetCursorPos( ImVec2( 509.f, 550 ) );

        ImGui::Checkbox( xor_str( "local chams" ), &csgo::hacks::g_chams->cfg( ).m_local_chams, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 590 ) );

        ImGui::Combo( xor_str( "local type" ), &csgo::hacks::g_chams->cfg( ).m_local_chams_type, wtfff, i_fuck_no_fkin_way, enemy_chams_type, IM_ARRAYSIZE( enemy_chams_type ) );

        ImGui::SetCursorPos( ImVec2( 511, 618 ) );

        ImGui::ColorEdit4( xor_str( "local chams clr" ), csgo::hacks::g_chams->cfg( ).m_local_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::PopFont( );
    }
    else if ( cur_subtab == 1 ) {
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        ImGui::SetCursorPos( ImVec2( 120, 90 ) );
        ImGui::Checkbox( xor_str( "remove scope" ), &csgo::hacks::g_visuals->cfg( ).m_remove_scope );
        ImGui::SetCursorPos( ImVec2( 120, 115 ) );
        ImGui::Checkbox( xor_str( "remove flash" ), &csgo::hacks::g_visuals->cfg( ).m_remove_flash );
        ImGui::SetCursorPos( ImVec2( 120, 140 ) );
        ImGui::Checkbox( xor_str( "remove smoke" ), &csgo::hacks::g_visuals->cfg( ).m_remove_smoke );
        ImGui::SetCursorPos( ImVec2( 120, 165 ) );
        ImGui::Checkbox( xor_str( "remove hands shaking" ), &csgo::hacks::g_visuals->cfg( ).m_remove_hands_shaking );
        ImGui::SetCursorPos( ImVec2( 120, 190 ) );
        ImGui::Checkbox( xor_str( "remove post processing" ), &csgo::hacks::g_visuals->cfg( ).m_remove_post_processing );
        ImGui::SetCursorPos( ImVec2( 120, 215 ) );
        ImGui::Checkbox( xor_str( "remove view kick" ), &csgo::hacks::g_visuals->cfg( ).m_remove_view_kick );
        ImGui::SetCursorPos( ImVec2( 120, 240 ) );
        ImGui::Checkbox( xor_str( "remove view punch" ), &csgo::hacks::g_visuals->cfg( ).m_remove_view_punch );
        ImGui::SetCursorPos( ImVec2( 120, 265 ) );
        ImGui::Checkbox( xor_str( "remove landing bob" ), &csgo::hacks::g_visuals->cfg( ).m_land_bob );
        ImGui::SetCursorPos( ImVec2( 33, 305 ) );
        static int i_dont_care_who_tf_{ 64 };
        static int are_u{ -70 };
        ImGui::Combo( xor_str( "sky box" ), &csgo::hacks::g_visuals->cfg( ).m_skybox_type, i_dont_care_who_tf_, are_u, skybox_list, IM_ARRAYSIZE( skybox_list ) );
        draw_list->AddText( ImVec2( pos.x + 101, pos.y + 339 ), ImColor ( 255, 255, 255, 255 ), "world modulation" );
        draw_list->AddText( ImVec2( pos.x + 101, pos.y + 369 ), ImColor( 255, 255, 255, 255 ), "props modulation" );
        draw_list->AddText( ImVec2( pos.x + 101, pos.y + 399 ), ImColor( 255, 255, 255, 255 ), "sky modulation" );
        ImGui::SetCursorPos( ImVec2( 71, 335 ) );
        ImGui::ColorEdit4( xor_str( "world modulation" ), csgo::hacks::g_visuals->cfg( ).m_world_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::SetCursorPos( ImVec2( 71, 365 ) );
        ImGui::ColorEdit4( xor_str( "props modulation" ), csgo::hacks::g_visuals->cfg( ).m_props_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::SetCursorPos( ImVec2( 71, 395 ) );
        ImGui::ColorEdit4( xor_str( "sky modulation" ), csgo::hacks::g_visuals->cfg( ).m_sky_modulation, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 120, 405 ) );

        ImGui::Checkbox( xor_str( "manuals indication" ), &csgo::hacks::g_visuals->cfg( ).m_manuals_indication );

        ImGui::SetCursorPos( ImVec2( 120, 430 ) );

        ImGui::Checkbox( xor_str( "tone map modulation" ), &csgo::hacks::g_visuals->cfg( ).m_tone_map_modulation );

        ImGui::SetCursorPos( ImVec2( 71, 465 ) );

        ImGui::SliderInt( xor_str( "bloom" ), &csgo::hacks::g_visuals->cfg( ).m_bloom, 0, 750 );

        ImGui::SetCursorPos( ImVec2( 71, 495 ) );

        ImGui::SliderInt( xor_str( "exposure" ), &csgo::hacks::g_visuals->cfg( ).m_exposure, 0, 2000 );

        ImGui::SetCursorPos( ImVec2( 120, 520 ) );

        ImGui::Checkbox( xor_str( "killfeed" ), &csgo::hacks::g_misc->cfg( ).m_kill_feed );

        ImGui::SetCursorPos( ImVec2( 120, 545 ) );

        ImGui::Checkbox( xor_str( "hits marker" ), &csgo::hacks::g_visuals->cfg( ).m_hit_markers );

        ImGui::SetCursorPos( ImVec2( 120, 570 ) );

        ImGui::Checkbox( xor_str( "show weapon in scope" ), &csgo::hacks::g_visuals->cfg( ).m_show_weapon_in_scope );

        ImGui::SetCursorPos( ImVec2( 120.f, 595 ) );

        ImGui::Checkbox( xor_str( "enemy bullet tracers" ), &csgo::hacks::g_visuals->cfg( ).m_enemy_bullet_tracers );

        ImGui::SetCursorPos( ImVec2( 71, 635 ) );
        ImGui::ColorEdit4( xor_str( "enemy trace clr" ), csgo::hacks::g_visuals->cfg( ).m_enemy_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509.f, 90 ) );

        ImGui::Checkbox( xor_str( "molotov timer" ), &csgo::hacks::g_visuals->cfg( ).m_molotov_timer, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 115 ) );

        ImGui::Checkbox( xor_str( "smoke timer" ), &csgo::hacks::g_visuals->cfg( ).m_smoke_timer, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 140 ) );

        ImGui::Checkbox( xor_str( "projectiles" ), &csgo::hacks::g_visuals->cfg( ).m_grenade_projectiles, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 165 ) );

        ImGui::Checkbox( xor_str( "draw wpn proj" ), &csgo::hacks::g_visuals->cfg( ).m_proj_wpn, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 190 ) );

        ImGui::Checkbox( xor_str( "draw wpn proj ( icon )" ), &csgo::hacks::g_visuals->cfg( ).m_proj_icon, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 215 ) );

        ImGui::Checkbox( xor_str( "bullet tracers" ), &csgo::hacks::g_visuals->cfg( ).m_bullet_tracers, 509.f );

        ImGui::SetCursorPos( ImVec2( 511.f, 255 ) );
        ImGui::ColorEdit4( xor_str( "trace clr" ), csgo::hacks::g_visuals->cfg( ).m_bullet_tracers_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );

        ImGui::SetCursorPos( ImVec2( 509.f, 265 ) );

        ImGui::Checkbox( xor_str( "impacts" ), &csgo::hacks::g_visuals->cfg( ).m_bullet_impacts, 509.f );

        ImGui::SetCursorPos( ImVec2( 509.f, 290 ) );

        ImGui::Checkbox( xor_str( "modulate shadows" ), &csgo::hacks::g_visuals->cfg( ).m_shadows_modulation, 509.f );

        ImGui::SetCursorPos( ImVec2( 511, 330 ) );
        ImGui::SliderFloat( "x dir", &csgo::hacks::g_visuals->cfg( ).m_x_dir, -100.f, 100.f, "%.1f" );
        ImGui::SetCursorPos( ImVec2( 511, 365 ) );
        ImGui::SliderFloat( "y dir", &csgo::hacks::g_visuals->cfg( ).m_y_dir, -100.f, 100.f, "%.1f" );
        ImGui::SetCursorPos( ImVec2( 511, 400 ) );
        ImGui::SliderFloat( "z dir", &csgo::hacks::g_visuals->cfg( ).m_z_dir, -100.f, 100.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509.f, 425 ) );

        ImGui::Checkbox( xor_str( "fog" ), &csgo::hacks::g_visuals->cfg( ).m_fog, 509.f );

        ImGui::SetCursorPos( ImVec2( 511, 465 ) );
        ImGui::ColorEdit4( xor_str( "fog clr" ), csgo::hacks::g_visuals->cfg( ).m_fog_clr, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar );
        ImGui::SetCursorPos( ImVec2( 511, 485 ) );
        ImGui::SliderInt( "fog start", &csgo::hacks::g_visuals->cfg( ).m_fog_start, 0, 1000 );
        ImGui::SetCursorPos( ImVec2( 511, 515 ) );
        ImGui::SliderInt( "fog end", &csgo::hacks::g_visuals->cfg( ).m_fog_end, 100, 1100 );
        ImGui::SetCursorPos( ImVec2( 511, 545 ) );
        ImGui::SliderInt( "fog density", &csgo::hacks::g_visuals->cfg( ).m_fog_density, 0, 100 );

        ImGui::SetCursorPos( ImVec2( 511, 570 ) );
        ImGui::Checkbox( xor_str( "blend in scope" ), &csgo::hacks::g_visuals->cfg( ).m_blend_in_scope, 509.f );
        ImGui::SetCursorPos( ImVec2( 511, 605 ) );
        ImGui::SliderInt( "blend value", &csgo::hacks::g_visuals->cfg( ).m_blend_in_scope_val, 0, 100 );

        ImGui::PopFont( );
    };

    ImGui::GetStyle( ).Alpha = backup_alpha;
}

void misc_tab( ImVec2 pos, float alpha, ImDrawList* draw_list, int cur_subtab ) {
    auto backup_alpha = ImGui::GetStyle( ).Alpha;
    ImGui::GetStyle( ).Alpha = alpha / 255;
    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_verdana_main );
    draw_list->AddText( ImVec2( pos.x + 68, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Main" );
    draw_list->AddText( ImVec2( pos.x + 507, pos.y + 80 ), ImColor( 255, 255, 255, static_cast < int > ( alpha ) ), "Extra" );
    ImGui::PopFont( );
    ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 110, pos.y + 88 ), ImVec2( pos.x + 479, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    draw_list->AddRectFilledMultiColor( ImVec2( pos.x + 553, pos.y + 88 ), ImVec2( pos.x + 920, pos.y + 90 ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 55 ) ), ImColor( 255, 255, 255, static_cast < int > ( alpha - 230 ) ) );
    if ( cur_subtab == 0 ) {
        ImGui::PushFont( csgo::hacks::g_misc->m_fonts.m_muli_regular );
        ImGui::SetCursorPos( ImVec2( 120, 90 ) );
        ImGui::Checkbox( "bhop", &csgo::hacks::g_move->cfg( ).m_bhop );
        ImGui::SetCursorPos( ImVec2( 120, 115 ) );
        ImGui::Checkbox( "strafe", &csgo::hacks::g_move->cfg( ).m_auto_strafe );
        ImGui::SetCursorPos( ImVec2( 120, 140 ) );
        ImGui::Checkbox( "fast stop", &csgo::hacks::g_move->cfg( ).m_fast_stop );
        ImGui::SetCursorPos( ImVec2( 120, 165 ) );
        ImGui::Checkbox( "infinity duck", &csgo::hacks::g_move->cfg( ).m_infinity_duck );
        ImGui::SetCursorPos( ImVec2( 120, 190 ) );
        ImGui::Checkbox( "clan tag", &csgo::hacks::g_misc->cfg( ).m_clan_tag );

        static int dont_care{ 64 };
        static int i_fuck_who{ -70 };

        static int dont_care_{ 64 };
        static int i_fuck_who_{ -70 };
        ImGui::SetCursorPos( ImVec2( 33.f, 235 ) );
        g_key_binds->KeybindNelfo( xor_str( "fake walk" ), &csgo::hacks::g_move->cfg( ).m_slow_walk, dont_care, 33, 265, i_fuck_who );

        ImGui::SetCursorPos( ImVec2( 33.f, 270 ) );

        g_key_binds->KeybindNelfo( xor_str( "third person" ), &csgo::hacks::g_misc->cfg( ).m_third_person_key, dont_care_, 33, 300, i_fuck_who_ );

        ImGui::SetCursorPos( ImVec2( 71, 310 ) );

        ImGui::SliderFloat( "third person distance", &csgo::hacks::g_misc->cfg( ).m_third_person_dist, 0.f, 180.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 71, 335 ) );

        ImGui::Checkbox( "third person while dead", &csgo::hacks::g_misc->cfg( ).m_force_thirdperson_dead );

        ImGui::SetCursorPos( ImVec2( 71, 370 ) );

        ImGui::SliderInt( xor_str( "camera distance" ), &csgo::hacks::g_misc->cfg( ).m_camera_distance, 60, 140 );

        ImGui::SetCursorPos( ImVec2( 30, 390 ) );

        ImGui::Checkbox( xor_str( "override in scope" ), &csgo::hacks::g_misc->cfg( ).m_remove_zoom_on_second_scope );

        ImGui::SetCursorPos( ImVec2( 30, 415 ) );

        ImGui::Checkbox( xor_str( "aspect ratio" ), &csgo::hacks::g_misc->cfg( ).m_aspect_ratio );

        ImGui::SetCursorPos( ImVec2( 71, 450 ) );

        ImGui::SliderFloat( xor_str( "aspect ratio value" ), &csgo::hacks::g_misc->cfg( ).m_aspect_ratio_value, 0.f, 3.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 30, 470 ) );

        ImGui::Checkbox( xor_str( "spectators" ), &csgo::hacks::g_misc->cfg( ).m_spectators );

        ImGui::SetCursorPos( ImVec2( 30, 495 ) );

        ImGui::Checkbox( xor_str( "keybinds" ), &csgo::hacks::g_visuals->cfg( ).m_keybinds_list );

        ImGui::SetCursorPos( ImVec2( 30, 520 ) );

        ImGui::Checkbox( xor_str( "hit sound" ), &csgo::hacks::g_misc->cfg( ).m_hit_marker_sound );

        ImGui::SetCursorPos( ImVec2( 33, 562 ) );

        static int i_dont_care_who_tf_123131{ 64 };
        static int are_u222{ -70 };
        ImGui::Combo( xor_str( "sounds list" ), &csgo::hacks::g_misc->cfg( ).m_hit_marker_sound_val, i_dont_care_who_tf_123131, are_u222, sounds_arr, IM_ARRAYSIZE( sounds_arr ) );

        ImGui::SetCursorPos( ImVec2( 30, 570 ) );

        ImGui::Checkbox( xor_str( "force crosshair" ), &csgo::g_local_player->cfg( ).m_force_crosshair );

        ImGui::SetCursorPos( ImVec2( 470, 110 ) );
        static int dont_care__{ 64 };
        static int i_fuck_who__{ -70 };
        g_key_binds->KeybindNelfo( xor_str( "auto peek" ), &csgo::hacks::g_move->cfg( ).m_auto_peek_key, dont_care__, 510, 140, i_fuck_who__, false, 770 );

        ImGui::SetCursorPos( ImVec2( 470, 145 ) );

        static int dont_care___{ 64 };
        static int i_fuck_who___{ -70 };

        g_key_binds->KeybindNelfo( xor_str( "ping spike" ), &csgo::hacks::g_ping_spike->cfg( ).m_ping_spike_key, dont_care___, 510, 175, i_fuck_who___, false, 770 );

        ImGui::SetCursorPos( ImVec2( 511, 170 ) );

        ImGui::SliderFloat( xor_str( "ping spike value" ), &csgo::hacks::g_ping_spike->cfg( ).m_to_spike, 200.f, 1000.f, "%.1f" );

        ImGui::SetCursorPos( ImVec2( 509.f, 195 ) );

        ImGui::Checkbox( "buy bot", &csgo::hacks::g_misc->cfg( ).m_buy_bot, 509.f );

        ImGui::SetCursorPos( ImVec2( 472, 240 ) );

        static int i_dont_care_who_tf_{ 64 };
        static int are_u{ -70 };
        ImGui::Combo( xor_str( "snipers" ), &csgo::hacks::g_misc->cfg( ).m_buy_bot_snipers, i_dont_care_who_tf_, are_u, snipers_arr, IM_ARRAYSIZE( snipers_arr ) );

        ImGui::SetCursorPos( ImVec2( 472, 270 ) );

        static int i_dont_care_who_tf__{ 64 };
        static int are_u_{ -70 };
        ImGui::Combo( xor_str( "pistols" ), &csgo::hacks::g_misc->cfg( ).m_buy_bot_pistols, i_dont_care_who_tf__, are_u_, pistols_arr, IM_ARRAYSIZE( pistols_arr ) );

        static int i_fuck_kids{ -70 };

        i_fuck_kids += 2;

        if ( i_fuck_kids > 0 )
            i_fuck_kids = 0;

        ImGui::SetCursorPos( ImVec2( 473, 300 ) );

        static int i_want_to_kys{ 64 };

        if ( ImGui::BeginCombo( xor_str( "additionals" ), "", i_want_to_kys ) ) {
            static bool hitgroups_vars[ IM_ARRAYSIZE( additional_arr ) ]{};

            for ( std::size_t i{}; i < IM_ARRAYSIZE( additional_arr ); ++i ) {
                hitgroups_vars[ i ] = csgo::hacks::g_misc->cfg( ).m_buy_bot_additional & ( 1 << i );

                ImGui::Selectable(
                    additional_arr[ i ], i_fuck_kids, &hitgroups_vars[ i ],
                    ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
                );

                if ( hitgroups_vars[ i ] )
                    csgo::hacks::g_misc->cfg( ).m_buy_bot_additional |= ( 1 << i );
                else
                    csgo::hacks::g_misc->cfg( ).m_buy_bot_additional &= ~( 1 << i );
            }

            ImGui::EndCombo( );
        }
        else
            i_fuck_kids = -70;

        ImGui::SetCursorPos( ImVec2( 509.f, 310 ) );

        ImGui::Checkbox( "mrx", &csgo::g_local_player->cfg( ).m_shitty_mrx_servers, 509.f );

        ImGui::SetCursorPos( ImVec2( 473, 354 ) );

        static int nahhh_browtff{ 64 };
        static int i_fuck_who_nah_broooadasda{ -70 };
        ImGui::Combo( xor_str( "cfg slot" ), &csgo::g_ctx->cur_cfg_slot ( ), nahhh_browtff, i_fuck_who_nah_broooadasda, cfg_slots, IM_ARRAYSIZE( cfg_slots ) );

        ImGui::SetCursorPos( ImVec2( 511, 384 ) );

        if ( ImGui::Button( "load" ) )
            sdk::g_cfg->load( cfg_slots[ csgo::g_ctx->cur_cfg_slot( ) ] );

        ImGui::SetCursorPos( ImVec2( 511, 414 ) );

        if ( ImGui::Button( "save" ) )
            sdk::g_cfg->save( cfg_slots[ csgo::g_ctx->cur_cfg_slot( ) ] );

        ImGui::PopFont( );
    }

    ImGui::GetStyle( ).Alpha = backup_alpha;
}

namespace csgo {

    void c_menu::render( ) {

        static float menus_alpha{ 255.f };

        if ( m_main.m_hidden ) {
            menus_alpha -= 10;
        }
        else {
            menus_alpha += 10;
        }

        menus_alpha = std::clamp( menus_alpha, 0.f, 255.f );

        bool swap_alpha{};

        if ( menus_alpha > 0.f
            && menus_alpha < 255.f )
            swap_alpha = true;

        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, menus_alpha );

        if ( menus_alpha < 0.00002f ) {
            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 1.f );
            return;
        }

        ImGui::StyleColorsDark( );
        static float f = 0.0f;
        static int counter = 0;
        auto get_mouse_pos = ImGui::GetMousePos( );
        static int next_pos{ 70 };
        static bool niggas_die{};
        static int rage_tab_alphas{};
        static int anti_aim_alphas{};
        static int visuals_alphas{};
        static int misc_alphas{};
        static int extra_alphas{};

        ImGui::Begin( "Hello, world!", 64, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar );
        {
            m_is_locked = false;
            static int tabs{};
            ImVec2 pos;
            ImDrawList* draw;
            pos = ImGui::GetWindowPos( );
            draw = ImGui::GetWindowDrawList( );
            static bool opened{ false };
            static bool rage_tab{ true };
            static bool anti_aim{};
            static bool visuals{};
            static bool misc{};
            static bool cfg{};
            static int cur_tab{};
            static int last_tab{};
            static int anti_aim_subtab{};
            static int anti_aim_subtab_main_alpha{ 170 };
            static int anti_aim_subtab_fakelags_alpha{ 170 };
            static int visuals_subtab{};
            static int visuals_subtab_player_alpha{ 170 };
            static int visuals_subtab_world_alpha{ 170 };
            bool clicked{ ImGui::GetIO( ).MouseClicked[ 0 ] };
            auto is_valid_cursor = [ ]( int max_x, int max_y, int min_x, int min_y ) {
                auto mouse_pos = ImGui::GetMousePos( );
                auto win_pos = ImGui::GetWindowPos( );
                if ( mouse_pos.x <= win_pos.x + max_x
                    && mouse_pos.x > win_pos.x + min_x
                    && mouse_pos.y <= win_pos.y + max_y
                    && mouse_pos.y > win_pos.y + min_y )
                    return true;

                return false;
            };

            bool move{};
            static int letters_alpha{};

            int x_max{ 60 };

            if ( opened )
                x_max += 45;

            if ( is_valid_cursor( x_max, 900, 0, 0 ) ) {
                opened = true;
            }
            else {
                opened = false;
            }

            if ( opened ) {
                if ( next_pos > 100 ) {
                    if ( next_pos <= 130 )
                        next_pos += 4;
                }
                else {
                    if ( next_pos <= 130 )
                        next_pos += 5;
                }
                m_is_locked = true;
                move = true;
            }

            if ( !move ) {
                if ( next_pos > 50 )
                    next_pos -= 5;
            }

            next_pos = std::clamp( next_pos, 50, 130 );

            if ( opened 
                && next_pos > 100 ) {
                letters_alpha += 5;
            }
            else {
                letters_alpha -= 180;
            }

            static int rage_alpha{ 200 };
            static int anti_aim_alpha{ 200 };
            static int visuals_alpha{ 200 };
            static int misc_alpha{ 200 };
            static int cfg_alpha{ 200 };

            last_tab = cur_tab;

            if ( is_valid_cursor( next_pos - 70, 110, 0, 80 )
                && clicked ) {
                cur_tab = 0;
                rage_tab = !rage_tab;
                anti_aim = false;
                visuals = false;
                misc = false;
                cfg = false;
            }

            if ( is_valid_cursor( next_pos - 70, 150, 0, 120 )
                && clicked ) {
                cur_tab = 1;
                anti_aim = !anti_aim;
                rage_tab = false;
                visuals = false;
                misc = false;
                cfg = false;
            }
            if ( is_valid_cursor( next_pos - 70, 190, 0, 160 )
                && clicked ) {
                cur_tab = 2;
                visuals = !visuals;
                anti_aim = false;
                rage_tab = false;
                misc = false;
                cfg = false;
            }

            if ( is_valid_cursor( next_pos - 70, 230, 0, 200 )
                && clicked ) {
                cur_tab = 3;
                misc = !misc;
                anti_aim = false;
                rage_tab = false;
                visuals = false;
                cfg = false;
            }

            if ( is_valid_cursor( next_pos - 70, 270, 0, 240 )
                && clicked ) {
                cur_tab = 4;
                cfg = !cfg;
                anti_aim = false;
                rage_tab = false;
                visuals = false;
                misc = false;
            }


            if ( cur_tab == 0 ) {
                rage_alpha += 15;
            }
            else {
                rage_alpha -= 5;
            }

            if ( cur_tab == 1 ) {
                anti_aim_alpha += 15;
            }
            else {
                anti_aim_alpha -= 5;
            }

            if ( cur_tab == 2 ) {
                visuals_alpha += 15;
            }
            else {
                visuals_alpha -= 5;
            }

            if ( cur_tab == 3 ) {
                misc_alpha += 15;
            }
            else {
                misc_alpha -= 5;
            }

            if ( cur_tab == 4 ) {
                cfg_alpha += 15;
            }
            else {
                cfg_alpha -= 5;
            }
            rage_alpha = std::clamp( rage_alpha, 200, 255 );
            anti_aim_alpha = std::clamp( anti_aim_alpha, 200, 255 );
            visuals_alpha = std::clamp( visuals_alpha, 200, 255 );
            misc_alpha = std::clamp( misc_alpha, 200, 255 );
            cfg_alpha = std::clamp( cfg_alpha, 200, 255 );

            if ( swap_alpha ) {
                rage_alpha = anti_aim_alpha = visuals_alpha = misc_alpha = cfg_alpha = menus_alpha;
            }

            if ( swap_alpha
                && m_main.m_hidden )
                letters_alpha -= 20;

            letters_alpha = std::clamp( letters_alpha, 0, 255 );

            bool can_draw_text{ next_pos > 40 };

            ImGui::SetWindowSize( ImVec2( 940, 710 ) );

            draw->AddRectFilled( ImVec2( pos.x + 0, pos.y + 0 ), ImVec2( pos.x + 940, pos.y + 710 ), ImColor( 11, 11, 11, static_cast < int > ( menus_alpha ) ), 10.f );  //BG    

            /* rage tabs, everything related to rage should be drawn here */
            if ( cur_tab == 0 ) {
                if ( rage_tab_alphas < 255 )
                    rage_tab_alphas += 13;

                if ( swap_alpha )
                    rage_tab_alphas = menus_alpha;
                    
                draw->AddRectFilled( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 13, 13, 13, rage_tab_alphas ) );
                draw->AddRectFilled( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 13, 13, 13, rage_tab_alphas ) );
            }
            else {
                if ( rage_tab_alphas > 0 )
                    rage_tab_alphas -= 17;
            }

            /* anti aim tabs, everything related to rage should be drawn here */
            if ( cur_tab == 1 ) {
                if ( anti_aim_alphas < 255 )
                    anti_aim_alphas += 13;

                if ( swap_alpha )
                    anti_aim_alphas = menus_alpha;

                draw->AddRectFilled( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 13, 13, 13, anti_aim_alphas ) );
                draw->AddRectFilled( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 13, 13, 13, anti_aim_alphas ) );

            }
            else {
                if ( anti_aim_alphas > 0 )
                    anti_aim_alphas -= 17;
            }

            if ( cur_tab == 2 ) {
                if ( visuals_alphas < 255 )
                    visuals_alphas += 13;

                if ( swap_alpha )
                    visuals_alphas = menus_alpha;

                draw->AddRectFilled( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 13, 13, 13, visuals_alphas ) );
                draw->AddRectFilled( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 13, 13, 13, visuals_alphas ) );
            }
            else {
                if ( visuals_alphas > 0 )
                    visuals_alphas -= 17;
            }

            if ( cur_tab == 3 ) {
                if ( misc_alphas < 255 )
                    misc_alphas += 13;

                if ( swap_alpha )
                    misc_alphas = menus_alpha;

                draw->AddRectFilled( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 13, 13, 13, misc_alphas ) );
                draw->AddRectFilled( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 13, 13, 13, misc_alphas ) );
            }
            else {
                if ( misc_alphas > 0 )
                    misc_alphas -= 17;
            }

            if ( cur_tab == 4 ) {
                if ( extra_alphas < 255 )
                    extra_alphas += 13;

                if ( swap_alpha )
                    extra_alphas = menus_alpha;

                draw->AddRectFilled( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 13, 13, 13, extra_alphas ) );
                draw->AddRectFilled( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 13, 13, 13, extra_alphas ) );
            }
            else {
                if ( extra_alphas > 0 )
                    extra_alphas -= 17;
            }
            
            draw->AddLine( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 60, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 60, pos.y + 680 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 60, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 70 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 480, pos.y + 70 ), ImVec2( pos.x + 480, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );

            draw->AddLine( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 500, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 500, pos.y + 680 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 500, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 70 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );
            draw->AddLine( ImVec2( pos.x + 920, pos.y + 70 ), ImVec2( pos.x + 920, pos.y + 680 ), ImColor( 35, 35, 35, static_cast < int > ( menus_alpha ) - 125 ) );

            /* EVERYTHING THIS SHOULD BE DRAWN AT THE END SO OTHER ELEMENTS DON'T OVERLAP THEM */

            static float target_y_pos{};
            static float last_target_y_pos{ };

            if ( last_tab != cur_tab ) { // update last targeted y pos
                last_target_y_pos = target_y_pos;
            }

            switch ( cur_tab ) {
            case 0:
                target_y_pos = 80.f;
                break;
            case 1:
                target_y_pos = 120.f;
                break;
            case 2:
                target_y_pos = 160.f;
                break;
            case 3:
                target_y_pos = 200.f;
                break;
            case 4:
                target_y_pos = 240.f;
                break;
            default:
                break;
            }

            static float target_delta{};
            if ( target_y_pos ) {
                target_delta = target_y_pos - last_target_y_pos;
            }

            if ( last_target_y_pos != target_y_pos )
                last_target_y_pos += target_delta / 4;

            if ( cur_tab == 0 )
                rage_bot_elements( pos, rage_tab_alphas, /*muli_regular_*/ draw );
            else if ( cur_tab == 1 )
                anti_aim_elements( pos, anti_aim_alphas, draw, anti_aim_subtab );
            else if ( cur_tab == 2 ) {
                visuals_tab( pos, visuals_alphas, draw, visuals_subtab );
            }
            else if ( cur_tab == 3 )
                misc_tab( pos, misc_alphas, draw, 0 );
            else if ( cur_tab == 4 )
                extra_elements( pos, extra_alphas, draw, 0 );

            draw_top( pos, draw, menus_alpha );
            //draw->AddImage( hacks::g_misc->m_fonts.m_misc_texture, ImVec2( pos.x + 450, pos.y + 5 ), ImVec2( pos.x + 475, pos.y + 30 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( menus_alpha ) ) );
            ImGui::PushFont( hacks::g_misc->m_fonts.m_xiaomi );
            draw->AddText( ImVec2( pos.x + 10, pos.y + 691 ), ImColor( 255, 255, 255, static_cast < int > ( menus_alpha ) ), "user: " );
            draw->AddText( ImVec2( pos.x + 40, pos.y + 691 ), ImColor( 255, 255, 255, static_cast < int > ( menus_alpha ) ), std::string ( "admin" ).c_str ( ) );
            draw->AddText( ImVec2( pos.x + 810, pos.y + 691 ), ImColor( 255, 255, 255, static_cast < int > ( menus_alpha ) ), "hehe_hack" );
            ImGui::PopFont( );

            if ( cur_tab == 1 ) {
                if ( is_valid_cursor( 130, 50, 80, 20 )
                    && clicked ) {
                    anti_aim_subtab = 0;
                }

                if ( is_valid_cursor( 150, 50, 110, 20 )
                    && clicked ) {
                    anti_aim_subtab = 1;
                }

                if ( anti_aim_subtab == 0 ) {
                    anti_aim_subtab_main_alpha += 5;
                    anti_aim_subtab_fakelags_alpha -= 5;
                }
                else if ( anti_aim_subtab == 1 ) {
                    anti_aim_subtab_main_alpha -= 5;
                    anti_aim_subtab_fakelags_alpha += 5;
                }

                anti_aim_subtab_main_alpha = std::clamp( anti_aim_subtab_main_alpha, 170, 255 );
                anti_aim_subtab_fakelags_alpha = std::clamp( anti_aim_subtab_fakelags_alpha, 170, 255 );

                if ( swap_alpha ) {
                    anti_aim_subtab_main_alpha = menus_alpha;
                    anti_aim_subtab_fakelags_alpha = menus_alpha;
                }

                ImGui::PushFont( hacks::g_misc->m_fonts.m_muli_regular );        

                draw->AddText( ImVec2( pos.x + 81, pos.y + 19 ), ImColor( 255, 255, 255, anti_aim_subtab_main_alpha ), "Main" );
                draw->AddCircleFilled( ImVec2( pos.x + 95, pos.y + 40 ), 3.f, ImColor( 255, 255, 255, anti_aim_subtab_main_alpha ), 30.f );
                draw->AddCircleFilled( ImVec2( pos.x + 146, pos.y + 40 ), 3.f, ImColor( 255, 255, 255, anti_aim_subtab_fakelags_alpha ), 30.f );
                draw->AddText( ImVec2( pos.x + 120, pos.y + 19 ), ImColor( 255, 255, 255, anti_aim_subtab_fakelags_alpha ), "Fake lags" );
                ImGui::PopFont( );
            }

            else if ( cur_tab == 2 ) {
                if ( is_valid_cursor( 130, 50, 80, 20 )
                    && clicked ) {
                    visuals_subtab = 0;
                }

                if ( is_valid_cursor( 150, 50, 110, 20 )
                    && clicked ) {
                    visuals_subtab = 1;
                }

                if ( visuals_subtab == 0 ) {
                    visuals_subtab_player_alpha += 5;
                    visuals_subtab_world_alpha -= 5;
                }
                else if ( visuals_subtab == 1 ) {
                    visuals_subtab_player_alpha -= 5;
                    visuals_subtab_world_alpha += 5;
                }

                visuals_subtab_player_alpha = std::clamp( visuals_subtab_player_alpha, 170, 255 );
                visuals_subtab_world_alpha = std::clamp( visuals_subtab_world_alpha, 170, 255 );


                if ( swap_alpha ) {
                    visuals_subtab_world_alpha = menus_alpha;
                    visuals_subtab_player_alpha = menus_alpha;
                }

                ImGui::PushFont( hacks::g_misc->m_fonts.m_muli_regular );

                draw->AddText( ImVec2( pos.x + 82, pos.y + 19 ), ImColor( 255, 255, 255, visuals_subtab_player_alpha ), "Players" );
                draw->AddCircleFilled( ImVec2( pos.x + 100, pos.y + 40 ), 3.f, ImColor( 255, 255, 255, visuals_subtab_player_alpha ), 30.f );
                draw->AddCircleFilled( ImVec2( pos.x + 147, pos.y + 40 ), 3.f, ImColor( 255, 255, 255, visuals_subtab_world_alpha ), 30.f );
                draw->AddText( ImVec2( pos.x + 132, pos.y + 19 ), ImColor( 255, 255, 255, visuals_subtab_world_alpha ), "World" );
                ImGui::PopFont( );
            }

            draw->AddRectFilled( ImVec2( pos.x + 0, pos.y ), ImVec2( pos.x + next_pos, pos.y + 680 ), ImColor( 13, 13, 13, static_cast < int > ( menus_alpha ) ) ); // tabs

            ImGui::PushFont( hacks::g_misc->m_fonts.m_museo_sans_name );
            if ( can_draw_text ) {
                //draw->AddRectFilled( ImVec2( pos.x + 4, pos.y + last_target_y_pos ), ImVec2( pos.x + next_pos, pos.y + last_target_y_pos + 40 ), ImColor( 36, 37, 41, static_cast < int > ( menus_alpha ) ) );
            }
            if ( can_draw_text ) {
                draw->AddText( ImVec2( pos.x + next_pos - 95, pos.y + 90 ), ImColor( 255, 255, 255, rage_alpha ), "Rage" );
                draw->AddText( ImVec2( pos.x + next_pos - 95, pos.y + 130 ), ImColor( 255, 255, 255, anti_aim_alpha ), "AA" );
                draw->AddText( ImVec2( pos.x + next_pos - 95, pos.y + 170 ), ImColor( 255, 255, 255, visuals_alpha ), "Visuals" );
                draw->AddText( ImVec2( pos.x + next_pos - 95, pos.y + 210 ), ImColor( 255, 255, 255, misc_alpha ), "Misc" );
                draw->AddText( ImVec2( pos.x + next_pos - 95, pos.y + 250 ), ImColor( 255, 255, 255, cfg_alpha ), "Extra" );
            }

            static int additional_alpha{ 0 };

            if ( opened )
                additional_alpha += 6;
            else
                additional_alpha -= 6;

            additional_alpha = std::clamp( additional_alpha, 0, 100 );

            draw->AddLine( ImVec2( pos.x, pos.y + 80 ), ImVec2( pos.x + next_pos, pos.y + 80 ), ImColor( 200, 200, 200, static_cast < int > ( menus_alpha ) - 100 + additional_alpha ) );
            draw->AddLine( ImVec2( pos.x, pos.y + 120 ), ImVec2( pos.x + next_pos, pos.y + 120 ), ImColor( 40, 40, 40, static_cast < int > ( menus_alpha ) ) );
            draw->AddLine( ImVec2( pos.x, pos.y + 160 ), ImVec2( pos.x + next_pos, pos.y + 160 ), ImColor( 40, 40, 40, static_cast < int > ( menus_alpha ) ) );
            draw->AddLine( ImVec2( pos.x, pos.y + 200 ), ImVec2( pos.x + next_pos, pos.y + 200 ), ImColor( 40, 40, 40, static_cast < int > ( menus_alpha ) ) );
            draw->AddLine( ImVec2( pos.x, pos.y + 240 ), ImVec2( pos.x + next_pos, pos.y + 240 ), ImColor( 40, 40, 40, static_cast < int > ( menus_alpha ) ) );
            draw->AddLine( ImVec2( pos.x, pos.y + 280 ), ImVec2( pos.x + next_pos, pos.y + 280 ), ImColor( 40, 40, 40, static_cast < int > ( menus_alpha ) ) );


            draw->AddImage( hacks::g_misc->m_fonts.m_rage_texture, ImVec2( pos.x + next_pos - 32, pos.y + 93 ), ImVec2( pos.x + next_pos - 12, pos.y + 113 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( rage_alpha ) ) );
            draw->AddImage( hacks::g_misc->m_fonts.m_anti_aim_texture, ImVec2( pos.x + next_pos - 32, pos.y + 130 ), ImVec2( pos.x + next_pos - 12, pos.y + 150 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( anti_aim_alpha ) ) );
            draw->AddImage( hacks::g_misc->m_fonts.m_visuals_texture, ImVec2( pos.x + next_pos - 32, pos.y + 169 ), ImVec2( pos.x + next_pos - 12, pos.y + 189 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( visuals_alpha ) ) );
            draw->AddImage( hacks::g_misc->m_fonts.m_misc_texture, ImVec2( pos.x + next_pos - 32, pos.y + 210 ), ImVec2( pos.x + next_pos - 12, pos.y + 230 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( misc_alpha ) ) );
            draw->AddImage( hacks::g_misc->m_fonts.m_extra_texture, ImVec2( pos.x + next_pos - 32, pos.y + 250 ), ImVec2( pos.x + next_pos - 12, pos.y + 270 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, static_cast < int > ( cfg_alpha ) ) );
            ImGui::PopFont( );
        }
        ImGui::End( );
        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 1.f );
        /*ImGui::Begin( "wok sdk" );

        ImGui::Checkbox( "bhop", &hacks::g_move->cfg ( ).m_bhop );

        ImGui::Checkbox ( "strafe", &hacks::g_move->cfg ( ).m_auto_strafe );

        ImGui::Checkbox ( "fast stop", &hacks::g_move->cfg ( ).m_fast_stop );

        ImGui::Checkbox ( "infinity duck", &hacks::g_move->cfg ( ).m_infinity_duck );

        ImGui::Checkbox ( "clan tag", &hacks::g_misc->cfg ( ).m_clan_tag );

        g_key_binds->KeybindNelfo ( xor_str ( "slow walk" ), &hacks::g_move->cfg ( ).m_slow_walk );

        g_key_binds->KeybindNelfo ( xor_str ( "third person" ), &hacks::g_misc->cfg ( ).m_third_person_key );

        ImGui::SliderFloat ( "third person distance", &hacks::g_misc->cfg ( ).m_third_person_dist, 0.f, 180.f );

        ImGui::Checkbox ( "third person while dead", &hacks::g_misc->cfg ( ).m_force_thirdperson_dead );

        ImGui::SliderInt ( xor_str ( "camera distance" ), &hacks::g_misc->cfg ( ).m_camera_distance, 60.f, 140.f );

        ImGui::Checkbox ( xor_str ( "override in scope" ), &hacks::g_misc->cfg ( ).m_remove_zoom_on_second_scope );

        ImGui::Checkbox ( xor_str ( "aspect ratio" ), &hacks::g_misc->cfg ( ).m_aspect_ratio );

        ImGui::SliderFloat ( xor_str ( "aspect ratio value" ), &hacks::g_misc->cfg ( ).m_aspect_ratio_value, 0.f, 3.f );

        ImGui::Checkbox ( xor_str ( "spectators" ), &hacks::g_misc->cfg ( ).m_spectators );

        ImGui::Checkbox ( xor_str ( "hit sound" ), &hacks::g_misc->cfg ( ).m_hit_marker_sound );

        ImGui::Checkbox ( xor_str ( "draw name" ), &hacks::g_visuals->cfg ( ).m_draw_name );

        ImGui::Checkbox ( xor_str ( "draw health" ), &hacks::g_visuals->cfg ( ).m_draw_health );

        ImGui::Checkbox ( xor_str ( "draw box" ), &hacks::g_visuals->cfg ( ).m_draw_box );

        ImGui::Checkbox ( xor_str ( "draw wpn icon" ), &hacks::g_visuals->cfg ( ).m_wpn_icon );

        ImGui::Checkbox ( xor_str ( "draw wpn txt" ), &hacks::g_visuals->cfg ( ).m_wpn_text );

        ImGui::Checkbox ( xor_str ( "draw ammo" ), &hacks::g_visuals->cfg ( ).m_wpn_ammo );

        ImGui::Checkbox ( xor_str ( "draw flags" ), &hacks::g_visuals->cfg ( ).m_draw_flags );

        ImGui::Checkbox ( xor_str ( "enemy chams" ), &hacks::g_chams->cfg ( ).m_enemy_chams );

        ImGui::Combo ( xor_str ( "chams type" ), &hacks::g_chams->cfg ( ).m_enemy_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "enemy chams clr" ), hacks::g_chams->cfg ( ).m_enemy_clr );

        ImGui::Checkbox ( xor_str ( "local chams" ), &hacks::g_chams->cfg ( ).m_local_chams );

        ImGui::Combo ( xor_str ( "local type" ), &hacks::g_chams->cfg ( ).m_local_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "local chams clr" ), hacks::g_chams->cfg ( ).m_local_clr );

        ImGui::Checkbox ( xor_str ( "arms chams" ), &hacks::g_chams->cfg ( ).m_arms_chams );

        ImGui::Combo ( xor_str ( "arms type" ), &hacks::g_chams->cfg ( ).m_arms_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "arms chams clr" ), hacks::g_chams->cfg ( ).m_arms_clr );

        ImGui::Checkbox ( xor_str ( "weapon chams" ), &hacks::g_chams->cfg ( ).m_wpn_chams );

        ImGui::Combo ( xor_str ( "weapon type" ), &hacks::g_chams->cfg ( ).m_wpn_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "weapon chams clr" ), hacks::g_chams->cfg ( ).m_wpn_clr );

        ImGui::Checkbox ( xor_str ( "shot chams" ), &hacks::g_chams->cfg ( ).m_shot_chams );

        ImGui::Combo ( xor_str ( "shot type" ), &hacks::g_chams->cfg ( ).m_shot_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "shot chams clr" ), hacks::g_chams->cfg ( ).m_shot_clr );

        ImGui::Checkbox ( xor_str ( "history chams" ), &hacks::g_chams->cfg ( ).m_history_chams );

        ImGui::Combo ( xor_str ( "history type" ), &hacks::g_chams->cfg ( ).m_history_chams_type, enemy_chams_type, IM_ARRAYSIZE ( enemy_chams_type ) );

        ImGui::ColorEdit4 ( xor_str ( "history chams clr" ), hacks::g_chams->cfg ( ).m_history_clr );

        ImGui::Checkbox ( xor_str ( "remove scope" ), &hacks::g_visuals->cfg ( ).m_remove_scope );

        ImGui::Checkbox ( xor_str ( "remove flash" ), &hacks::g_visuals->cfg ( ).m_remove_flash );

        ImGui::Checkbox ( xor_str ( "remove smoke" ), &hacks::g_visuals->cfg ( ).m_remove_smoke );

        ImGui::Checkbox ( xor_str ( "remove hands shaking" ), &hacks::g_visuals->cfg ( ).m_remove_hands_shaking );

        ImGui::Checkbox ( xor_str ( "remove post processing" ), &hacks::g_visuals->cfg ( ).m_remove_post_processing );

        ImGui::Checkbox ( xor_str ( "remove view kick" ), &hacks::g_visuals->cfg ( ).m_remove_view_kick );

        ImGui::Checkbox ( xor_str ( "remove view punch" ), &hacks::g_visuals->cfg ( ).m_remove_view_punch );

        ImGui::Checkbox ( xor_str ( "remove landing bob" ), &hacks::g_visuals->cfg ( ).m_land_bob );

        ImGui::Combo ( xor_str ( "sky box" ), &hacks::g_visuals->cfg ( ).m_skybox_type, skybox_list, IM_ARRAYSIZE ( skybox_list ) );

        ImGui::ColorEdit4 ( xor_str ( "world modulation" ), hacks::g_visuals->cfg ( ).m_world_modulation );

        ImGui::ColorEdit4 ( xor_str ( "props modulation" ), hacks::g_visuals->cfg ( ).m_props_modulation );

        ImGui::ColorEdit4 ( xor_str ( "sky modulation" ), hacks::g_visuals->cfg ( ).m_sky_modulation );

        ImGui::Checkbox ( xor_str ( "fake lag" ), &hacks::g_anti_aim->cfg ( ).m_should_fake_lag );

        ImGui::SliderInt ( xor_str ( "lag ticks" ), &hacks::g_anti_aim->cfg ( ).m_ticks_to_choke, 1, 14 );

        ImGui::Checkbox ( xor_str ( "disable while standing" ), &hacks::g_anti_aim->cfg ( ).m_disable_lag_on_stand );

        ImGui::Combo ( xor_str ( "pitch" ), &hacks::g_anti_aim->cfg ( ).m_pitch_type, pitch_type, IM_ARRAYSIZE ( pitch_type ) );

        ImGui::Checkbox ( xor_str ( "anti aim" ), &hacks::g_anti_aim->cfg ( ).m_anti_aim );

        ImGui::SliderFloat ( xor_str ( "yaw" ), &hacks::g_anti_aim->cfg ( ).m_real_yaw, -180.f, 180.f );

        ImGui::SliderFloat( xor_str( "flick strength" ), &hacks::g_anti_aim->cfg( ).m_flick_strength, 50.f, 180.f );

        ImGui::Checkbox( xor_str( "change flick dir" ), &hacks::g_anti_aim->cfg( ).m_change_flick_dir );

        g_key_binds->KeybindNelfo ( xor_str ( "manual left" ), &hacks::g_anti_aim->cfg ( ).m_left_manual );

        g_key_binds->KeybindNelfo ( xor_str ( "manual right" ), &hacks::g_anti_aim->cfg ( ).m_right_manual );

        g_key_binds->KeybindNelfo ( xor_str ( "manual middle" ), &hacks::g_anti_aim->cfg ( ).m_middle_manual );

        g_key_binds->KeybindNelfo( xor_str( "freestanding" ), &hacks::g_anti_aim->cfg( ).m_freestand );

        ImGui::SliderFloat ( xor_str ( "jitter" ), &hacks::g_anti_aim->cfg ( ).m_jitter_yaw, -180.f, 180.f );

        ImGui::Checkbox ( xor_str ( "distortion" ), &hacks::g_anti_aim->cfg ( ).m_should_distort );

        ImGui::SliderFloat ( xor_str ( "distortion speed" ), &hacks::g_anti_aim->cfg ( ).m_distort_speed, 0.f, 100.f );

        ImGui::SliderFloat ( xor_str ( "distortion range" ), &hacks::g_anti_aim->cfg ( ).m_distort_range, -180.f, 180.f );

        ImGui::Checkbox ( xor_str ( "rage bot" ), &hacks::g_aim_bot->cfg ( ).m_rage_bot );

        ImGui::SliderFloat ( xor_str ( "head scale" ), &hacks::g_aim_bot->cfg ( ).m_head_scale, 1.f, 100.f );

        ImGui::SliderFloat ( xor_str ( "body scale" ), &hacks::g_aim_bot->cfg ( ).m_body_scale, 1.f, 100.f );

        ImGui::SliderInt ( xor_str ( "min dmg" ), &hacks::g_aim_bot->cfg ( ).m_min_dmg, 1, 100 );

        g_key_binds->KeybindNelfo( xor_str( "min dmg key" ), &hacks::g_aim_bot->cfg( ).m_min_dmg_key );

        ImGui::SliderInt( xor_str( "min dmg on key" ), &hacks::g_aim_bot->cfg( ).m_min_dmg_on_key, 1, 100 );

        if ( ImGui::BeginCombo ( xor_str ( "hitboxes" ), "" ) ) {


            								static bool hitgroups_vars[ IM_ARRAYSIZE ( hitboxes ) ] {};

            								for ( std::size_t i {}; i < IM_ARRAYSIZE ( hitboxes ); ++i ) {
            									hitgroups_vars[ i ] = hacks::g_aim_bot->cfg ( ).m_hitboxes & ( 1 << i );

            									ImGui::Selectable (
                                                    hitboxes [ i ], &hitgroups_vars[ i ],
            										ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups
            									);

            									if ( hitgroups_vars[ i ] )
                                                    hacks::g_aim_bot->cfg ( ).m_hitboxes |= ( 1 << i );
            									else
                                                    hacks::g_aim_bot->cfg ( ).m_hitboxes &= ~( 1 << i );
            								}

            								ImGui::EndCombo ( );
            							}

        ImGui::SliderFloat ( xor_str ( "hit chance" ), &hacks::g_aim_bot->cfg ( ).m_hit_chance, 1.f, 100.f );
        ImGui::Checkbox ( xor_str ( "prefer body" ), &hacks::g_aim_bot->cfg ( ).m_prefer_body );

        g_key_binds->KeybindNelfo ( xor_str ( "ping spike" ), &hacks::g_ping_spike->cfg ( ).m_ping_spike_key );

        ImGui::SliderFloat ( xor_str ( "ping spike value" ), &hacks::g_ping_spike->cfg ( ).m_to_spike, 200.f, 1000.f );

        g_key_binds->KeybindNelfo ( xor_str ( "double tap" ), &hacks::g_exploits->cfg ( ).m_dt_key );

        g_key_binds->KeybindNelfo ( xor_str ( "auto peek" ), &hacks::g_move->cfg ( ).m_auto_peek_key );

        ImGui::Checkbox ( xor_str ( "molotov timer" ), &hacks::g_visuals->cfg ( ).m_molotov_timer );

        ImGui::Checkbox ( xor_str ( "smoke timer" ), &hacks::g_visuals->cfg ( ).m_smoke_timer );

        ImGui::Checkbox ( xor_str ( "projectiles" ), &hacks::g_visuals->cfg ( ).m_grenade_projectiles );

        ImGui::Checkbox ( xor_str ( "draw wpn proj" ), &hacks::g_visuals->cfg ( ).m_proj_wpn );

        ImGui::Checkbox ( xor_str ( "draw wpn proj ( icon )" ), &hacks::g_visuals->cfg ( ).m_proj_icon );

        if ( ImGui::Button( "load" ) )
            sdk::g_cfg->load( "example" );

        if ( ImGui::Button( "save" ) )
            sdk::g_cfg->save( "example" );

        ImGui::End( );*/
    }
}