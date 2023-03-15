#include "../../csgo.hpp"
#include "../../../thirdparty/include/imgui/imgui_freetype.h"

int __stdcall EXP_Init( _In_ HINSTANCE hInstance, _In_ LPVOID lpReserved )
{
    return 1;
}

int __stdcall DllMain( _In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved ) {
    if ( reason != DLL_PROCESS_ATTACH )
        return 0;

    std::jthread{ [ ] ( ) {
        csgo::g_ctx->init( );
    } }.detach( );

    return 1;
}

#ifdef _DEBUG
#define THROW_IF_DBG( exception ) throw std::runtime_error{ exception }
#else
#define THROW_IF_DBG( exception ) throw std::runtime_error{ exception }
#endif

#define HOOK( target, hook, original ) \
    if ( MH_CreateHook( sdk::address_t{ target }.as< LPVOID >( ), \
        reinterpret_cast< LPVOID >( &hook ), reinterpret_cast< LPVOID* >( &original ) ) != MH_OK ) \
        THROW_IF_DBG( "can't hook " #hook "." ) \

#define HOOK_VFUNC( vft, index, hook, original ) \
    if ( MH_CreateHook( ( *sdk::address_t{ vft }.as< LPVOID** >( ) )[ index ], \
        reinterpret_cast< LPVOID >( &hook ), reinterpret_cast< LPVOID* >( &original ) ) != MH_OK ) \
        THROW_IF_DBG( "can't hook " #hook "." ) \

#define CSGO2018 
struct code_section_t {
    ALWAYS_INLINE constexpr code_section_t( ) = default;

    /*ALWAYS_INLINE*/ code_section_t( const sdk::x86_pe_image_t* const image ) {
        if ( image->m_dos_hdr.e_magic != sdk::k_dos_hdr_magic )
            THROW_IF_DBG( "invalid dos hdr." );

        const auto nt_hdrs = image->nt_hdrs( );
        if ( nt_hdrs->m_sig != sdk::k_nt_hdrs_magic )
            THROW_IF_DBG( "invalid nt hdrs." );

        m_start = image;
        m_start.self_offset( nt_hdrs->m_opt_hdr.m_code_base );

        m_end = m_start.offset( nt_hdrs->m_opt_hdr.m_code_size );
    }

    sdk::address_t m_start{}, m_end{};
};
#include "../../../sdk/font.hpp"
namespace csgo {
    bool c_ctx::wait_for_all_modules( modules_t& modules ) const {
        sdk::peb( )->for_each_ldr_data_table_entry( [ & ] ( sdk::ldr_data_table_entry_t* const entry ) {
            modules.insert_or_assign(
                sdk::hash( entry->m_base_dll_name.m_buffer, entry->m_base_dll_name.m_len / sizeof( wchar_t ) ),
                entry->m_dll_base.as< sdk::x86_pe_image_t* >( )
            );

            return false;
        }, sdk::e_ldr_data_table::in_load_order );

        return modules.find( HASH( "serverbrowser.dll" ) ) == modules.end( );
    }

    void c_ctx::init_imgui( const modules_t& modules ) const {
        const code_section_t shaderapidx9{ modules.at( HASH( "shaderapidx9.dll" ) ) };

        const auto device = **BYTESEQ( "A1 ? ? ? ? 50 8B 08 FF 51 0C" ).search(
            shaderapidx9.m_start, shaderapidx9.m_end, false
        ).self_offset( 0x1 ).as< IDirect3DDevice9*** >( );

        D3DDEVICE_CREATION_PARAMETERS params{};
        if ( device->GetCreationParameters( &params ) != D3D_OK )
            THROW_IF_DBG( "can't get creation params." );

        ImGui::CreateContext( );

        ImGui_ImplWin32_Init ( params.hFocusWindow );
        ImGui_ImplDX9_Init ( device );

        g_render->m_draw_list = std::make_shared<ImDrawList>( ImGui::GetDrawListSharedData( ) );
        g_render->m_data_draw_list = std::make_shared<ImDrawList>( ImGui::GetDrawListSharedData( ) );
        g_render->m_replace_draw_list = std::make_shared<ImDrawList>( ImGui::GetDrawListSharedData( ) );

        ImGui::StyleColorsClassic( );

        ImGui::GetStyle( ).WindowMinSize = { 450, 350 };

        auto& io = ImGui::GetIO( );

        io.IniFilename = io.LogFilename = nullptr;
        static const ImWchar ranges[ ] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B
            0xE000, 0xE226, // icons
            0,
        };

        ImFontConfig menu_elements_cfg;
        menu_elements_cfg.RasterizerFlags = ImGuiFreeType::ForceAutoHint;
        menu_elements_cfg.OversampleH = menu_elements_cfg.OversampleV = 3;
        menu_elements_cfg.PixelSnapH = false;
        menu_elements_cfg.RasterizerMultiply = 1.3f;

        ImFontConfig menu_tabs_cfg;
        menu_tabs_cfg.RasterizerFlags = ImGuiFreeType::ForceAutoHint;
        menu_tabs_cfg.OversampleH = menu_tabs_cfg.OversampleV = 3;  
        menu_tabs_cfg.PixelSnapH = false;
        menu_tabs_cfg.RasterizerMultiply = 1.2f;

        ImFontConfig nicknametop_tabs_cfg;
        menu_tabs_cfg.RasterizerFlags = ImGuiFreeType::NoHinting;
        menu_tabs_cfg.OversampleH = menu_tabs_cfg.OversampleV = 3;
        menu_tabs_cfg.PixelSnapH = false;
        menu_tabs_cfg.RasterizerMultiply = 1.3f;

        ImFontConfig for_esp_shit;
        for_esp_shit.RasterizerFlags = ImGuiFreeType::ForceAutoHint;
        for_esp_shit.OversampleH = menu_tabs_cfg.OversampleV = 3;
        for_esp_shit.PixelSnapH = false;
        for_esp_shit.RasterizerMultiply = 1.2f;

        ImFontConfig esp_cfg;
        esp_cfg.RasterizerFlags = ImGuiFreeType::Monochrome | ImGuiFreeType::MonoHinting;
        esp_cfg.OversampleH = esp_cfg.OversampleV = 5;
        esp_cfg.PixelSnapH = false;
        esp_cfg.RasterizerMultiply = 1.2f;

        ImFontConfig tahoma14;
        // set-up all fonts here...
        hacks::g_misc->m_fonts.m_tahoma14 = io.Fonts->AddFontFromFileTTF ( xor_str ( "C:\\Windows\\Fonts\\Tahoma.ttf" ), 13.f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_verdana = io.Fonts->AddFontFromFileTTF ( xor_str ( "C:\\Windows\\Fonts\\Verdana.ttf" ), 13.f, &menu_tabs_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_icon_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF ( WeaponIcons_compressed_data_base85, 12.f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
        hacks::g_misc->m_fonts.m_warning_icon_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF ( WeaponIcons_compressed_data_base85, 24.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_log_font = io.Fonts->AddFontFromFileTTF ( xor_str ( "C:/windows/fonts/seguisb.ttf" ), 14.0f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_log_icons = io.Fonts->AddFontFromMemoryCompressedTTF ( Icons_compressed_data123, Icons_compressed_size123, 15.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_museo_sans_name = io.Fonts->AddFontFromMemoryTTF ( xiaomi, sizeof ( xiaomi ), 18.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_segoe_ui_esp = io.Fonts->AddFontFromFileTTF ( xor_str ( "C:/windows/fonts/seguisb.ttf" ), 13.0f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_xxxtentacion_icons = io.Fonts->AddFontFromMemoryTTF ( xxx_tentacion_icons, sizeof ( xxx_tentacion_icons ), 12.f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_skeet_font_esp = io.Fonts->AddFontFromMemoryTTF ( _04B03, sizeof ( _04B03 ), 8.f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic ( ) );
        hacks::g_misc->m_fonts.m_middle_skeet_indicator = io.Fonts->AddFontFromMemoryTTF( _04B03, sizeof( _04B03 ), 9.f, &menu_elements_cfg, io.Fonts->GetGlyphRangesCyrillic( ) );
        hacks::g_misc->m_fonts.m_roboto = io.Fonts->AddFontFromMemoryTTF( roboto_font, sizeof( roboto_font ), 16.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_muli_regular = io.Fonts->AddFontFromMemoryTTF( muli_regular, sizeof( muli_regular ), 13.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_xiaomi = io.Fonts->AddFontFromMemoryTTF( xiaomi, sizeof( xiaomi ), 15.f, &menu_elements_cfg );
        hacks::g_misc->m_fonts.m_museo_700 = io.Fonts->AddFontFromMemoryTTF( museo_700, sizeof( museo_700 ), 13.f, &nicknametop_tabs_cfg );
        hacks::g_misc->m_fonts.m_museo_700_35 = io.Fonts->AddFontFromMemoryTTF( museo_700, sizeof( museo_700 ), 32.f, &nicknametop_tabs_cfg );
        hacks::g_misc->m_fonts.m_final_sans = io.Fonts->AddFontFromMemoryTTF( final_sans, sizeof( final_sans ), 15.f, &menu_elements_cfg ); // fira sans not final........
        hacks::g_misc->m_fonts.m_josefin_sans = io.Fonts->AddFontFromMemoryTTF( josefin_sans, sizeof( josefin_sans ), 17.f, &menu_tabs_cfg );
        hacks::g_misc->m_fonts.m_esp.m_04b = io.Fonts->AddFontFromMemoryTTF( small_fonts, sizeof( small_fonts ), 9.f, &tahoma14 );
        hacks::g_misc->m_fonts.m_esp.m_idk = io.Fonts->AddFontFromMemoryTTF( _04B03, sizeof( _04B03 ), 8.f, &esp_cfg );
        hacks::g_misc->m_fonts.m_esp.m_visitor_tt2_brk = io.Fonts->AddFontFromMemoryTTF( _visitor_tt2_brk, sizeof( _visitor_tt2_brk ), 9.f, &esp_cfg );
        hacks::g_misc->m_fonts.m_esp.m_icon_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF( WeaponIcons_compressed_data_base85, 9.f, &esp_cfg );
        hacks::g_misc->m_fonts.m_esp.m_verdana = io.Fonts->AddFontFromFileTTF( xor_str( "C:\\Windows\\Fonts\\Verdana.ttf" ), 13.f, &esp_cfg );
        hacks::g_misc->m_fonts.m_font_for_fkin_name = io.Fonts->AddFontFromMemoryTTF( xiaomi, sizeof( xiaomi ), 14.f, &nicknametop_tabs_cfg );

        /* _smallest_pixel */
        hacks::g_misc->m_fonts.m_smallest_pixel = io.Fonts->AddFontFromMemoryTTF( _smallest_pixel, sizeof( _smallest_pixel ), 9.f, &esp_cfg );

        /* Nickname at the top */
        hacks::g_misc->m_fonts.m_verdana_main = io.Fonts->AddFontFromMemoryTTF( verdana, sizeof( verdana ), 15.f, &nicknametop_tabs_cfg );
        ImGuiFreeType::BuildFontAtlas( io.Fonts );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_advance, sizeof( png_icon_advance ), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_robot_texture );

        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_misc, sizeof( png_icon_misc ), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_misc_texture );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_rage, sizeof( png_icon_rage ), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_rage_texture );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_anti_aim, sizeof( png_icon_anti_aim ), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_anti_aim_texture );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_extra, sizeof( png_icon_extra ), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_extra_texture );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_visuals, sizeof( png_icon_visuals ), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_visuals_texture );
        D3DXCreateTextureFromFileInMemoryEx( device, &png_icon_expand, sizeof( png_icon_expand ), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &hacks::g_misc->m_fonts.m_expand_texture );
        const code_section_t inputsystem{ modules.at( HASH( "inputsystem.dll" ) ) };

        HOOK( BYTESEQ( "55 8B EC 83 EC ? 80 3D" ).search(
            inputsystem.m_start, inputsystem.m_end, false
        ), hooks::wnd_proc, hooks::o_wnd_proc );

        HOOK_VFUNC( device, 16u, hooks::dx9_reset, hooks::o_dx9_reset );
        HOOK_VFUNC( device, 17u, hooks::dx9_present, hooks::o_dx9_present );
    }

    void c_ctx::parse_interfaces( sdk::x86_pe_image_t* const image, interfaces_t& interfaces ) const {
        sdk::address_t list{};

        image->for_each_export( image, [ & ] ( const char* name, const sdk::address_t addr ) {
            if ( sdk::hash( name ) != HASH( "CreateInterface" ) )
                return false;

            list = addr;

            return true;
        } );

        if ( !list )
            THROW_IF_DBG( "can't find CreateInterface export." );

        if ( *list.offset( 0x4 ).as< std::uint8_t* >( ) == 0xe9u
            && *list.self_rel( 0x5, true ).offset( 0x5 ).as< std::uint8_t* >( ) == 0x35u )
            list.self_offset( 0x6 ).self_deref( 2u );
        else if ( *list.offset( 0x2 ).as< std::uint8_t* >( ) == 0x35 )
            list.self_offset( 0x3 ).self_deref( 2u );
        else
            THROW_IF_DBG( "can't find interfaces list." );

        struct interface_entry_t {
            using create_t = std::uintptr_t* ( __cdecl* )( );

            create_t            m_create_fn{};
            const char*         m_name{};
            interface_entry_t*  m_next{};
        };

        for ( auto entry = list.as< interface_entry_t* >( ); entry; entry = entry->m_next )
            if ( entry->m_name )
                interfaces.insert_or_assign( sdk::hash( entry->m_name ), entry->m_create_fn( ) );
    }

    void c_ctx::init_interfaces( const modules_t& modules ) const {
        const code_section_t client{ modules.at( HASH( "client.dll" ) ) };

        interfaces_t interfaces{};

        std::array < unsigned int, 9 > modules_arr = { HASH ( "client.dll" ), HASH ( "engine.dll" ), 
            HASH ( "vstdlib.dll" ), HASH ( "vphysics.dll" ), 
            HASH ( "matchmaking.dll" ), HASH ( "vgui2.dll" ),
            HASH ( "materialsystem.dll" ), HASH ( "studiorender.dll" ),
            HASH ( "datacache.dll" ) };
        
        for ( size_t i = 0lu; i < modules_arr.size ( ); ++i )
        {
            parse_interfaces ( modules.at ( modules_arr.at ( i ) ), interfaces );
        }

        if ( interfaces.empty( ) )
            THROW_IF_DBG( "can't find interfaces." );

        {
            const auto tier0 = modules.at( HASH( "tier0.dll" ) );

            tier0->for_each_export( tier0, [ & ] ( const char* name, const sdk::address_t addr ) {
                if ( sdk::hash( name ) != HASH( "g_pMemAlloc" ) )
                    return false;

                valve::g_mem_alloc = *addr.as< valve::c_mem_alloc** >( );

                return true;
            } );
        }

        valve::g_client = interfaces.at( HASH( "VClient018" ) ).as< valve::c_client* >( );
        valve::g_engine = interfaces.at( HASH( "VEngineClient014" ) ).as< valve::c_engine* >( );
        valve::g_entity_list = interfaces.at( HASH( "VClientEntityList003" ) ).as< valve::c_entity_list* >( );
        valve::g_panel = interfaces.at ( HASH ( "VGUI_Panel009" ) ).as < valve::c_panel* > ( );
        valve::g_mdl_render = interfaces.at ( HASH ( "VEngineModel016" ) ).as < valve::c_mdl_render* > ( );
        valve::g_mat_sys = interfaces.at ( HASH ( "VMaterialSystem080" ) ).as < valve::c_mat_sys* > ( );
        valve::g_studio_render = interfaces.at ( HASH ( "VStudioRender026" ) ).as < valve::c_studio_render_ctx* > ( );
        valve::g_mdl_cache = interfaces.at ( HASH ( "MDLCache004" ) ).as < valve::c_mdl_cache* > ( );
        valve::g_model_info = interfaces.at( HASH( "VModelInfoClient004" ) ).as < valve::c_model_info* >( );
        
        valve::g_view_render = *BYTESEQ( "8B 0D ? ? ? ? 8B 01 FF 50 4C 8B 06" ).search(
            client.m_start, client.m_end, false
        ).self_offset( 0x2u ).as < valve::view_render_t** >( );

        valve::g_global_vars = **reinterpret_cast< valve::global_vars_base_t*** >(
            ( *reinterpret_cast< std::uintptr_t** >( valve::g_client ) )[ 11u ] + 0xau
        );
        valve::g_client_state = **reinterpret_cast< valve::client_state_t*** >(
            ( *reinterpret_cast< std::uintptr_t** >( valve::g_engine ) )[ 12u ] + 0x10u
        );

        valve::g_input = *BYTESEQ( "B9 ? ? ? ? 8B 40 38 FF D0 84 C0 0F 85" ).search(
            client.m_start, client.m_end, false
        ).self_offset( 0x1 ).as< valve::input_t** >( );

        valve::g_game_event_mgr = interfaces.at ( HASH ( "GAMEEVENTSMANAGER002" ) ).as < valve::c_game_event_mgr* > ( );
        valve::g_cvar = interfaces.at( HASH( "VEngineCvar007" ) ).as< valve::c_cvar* >( );

        valve::g_move_helper = **BYTESEQ( "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01" ).search(
            client.m_start, client.m_end, false
        ).self_offset( 0x2 ).as< valve::c_move_helper*** >( );

        valve::g_prediction = interfaces.at( HASH( "VClientPrediction001" ) ).as< valve::prediction_t* >( );
        valve::g_movement = interfaces.at( HASH( "GameMovement001" ) ).as< valve::c_movement* >( );

        valve::g_mdl_info = interfaces.at ( HASH ( "VModelInfoClient004" ) ).as < valve::c_mdl_info* > ( );

        valve::g_engine_trace = interfaces.at( HASH( "EngineTraceClient004" ) ).as< valve::c_engine_trace* >( );
        valve::g_surface_data = interfaces.at( HASH( "VPhysicsSurfaceProps001" ) ).as< valve::c_surface_data* >( );

        valve::g_game_rules = *BYTESEQ( "A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 74 7A" ).search(
            client.m_start, client.m_end, false
        ).self_offset( 0x1 ).as< valve::game_rules_t*** >( );
        valve::g_game_types = interfaces.at( HASH( "VENGINE_GAMETYPES_VERSION002" ) ).as< valve::c_game_types* >( );

        valve::g_render_view = interfaces.at( HASH( "VEngineRenderView014" ) ).as < valve::render_view_t* >( );

        valve::g_engine_sound = interfaces.at( HASH( "IEngineSoundClient003" ) ).as < valve::engine_sound_t* >( );

        valve::g_debug_overlay = interfaces.at( HASH( "VDebugOverlay004" ) ).as < valve::debug_overlay_t* >( );
 
        valve::fn_get_glow_obj_mngr = *BYTESEQ( "A1 ? ? ? ? A8 01 75 4B" ).search(
            client.m_start, client.m_end, false ).as < valve::get_glow_obj_mngr_t >( );
        valve::g_glow = valve::fn_get_glow_obj_mngr( );

        valve::g_hud = *BYTESEQ( "B9 ? ? ? ? 0F 94 C0 0F B6 C0 50 68" ).search(
            client.m_start, client.m_end, false ).self_offset( 0x1u ).as < valve::hud_t** >( );

        valve::g_beams = *BYTESEQ( "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9" ).search(
            client.m_start, client.m_end, false ).self_offset( 0x1u ).as < valve::beams_t** >( );
    }

    bool c_ctx::parse_ent_offsets( ent_offsets_t& offsets, const modules_t& modules ) const {
        offsets.reserve( 41000u );

        std::string concated{};

        concated.reserve( 128u );

        const auto parse_recv_table = [ & ] ( const auto& self, const char* name,
            valve::recv_table_t* const table, const std::uint32_t offset = 0u ) -> void {
            for ( int i{}; i < table->m_props_count; ++i ) {
                const auto prop = &table->m_props[ i ];

                const auto child = prop->m_data_table;
                if ( child
                    && child->m_props_count > 0 )
                    self( self, name, child, prop->m_offset + offset );

                concated = name;
                concated += "->";
                concated += prop->m_var_name;

                offsets.insert_or_assign(
                    sdk::hash( concated.data( ), concated.size( ) ),
                    ent_offset_t{ prop, prop->m_offset + offset }
                );
            }
        };

        for ( auto client_class = valve::g_client->all_classes( ); client_class; client_class = client_class->m_next )
            if ( client_class->m_recv_table )
                parse_recv_table( parse_recv_table, client_class->m_network_name, client_class->m_recv_table );

        const auto mov_data_map = BYTESEQ( "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3 CC" );

        const code_section_t client{ modules.at( HASH( "client.dll" ) ) };
        for ( auto start = client.m_start; ; start.self_offset( 0x1 ) ) {
            start = mov_data_map.search( start, client.m_end );
            if ( start == client.m_end )
                break;

            const auto data_map = start.offset( 0x2 ).deref( 1u ).offset( -0x4 ).as< valve::data_map_t* >( );
            if ( !data_map
                || !data_map->m_name
                || !data_map->m_descriptions
                || data_map->m_size <= 0
                || data_map->m_size >= 200 )
                continue;

            for ( int i{}; i < data_map->m_size; ++i ) {
                const auto& desc = data_map->m_descriptions[ i ];
                if ( !desc.m_name )
                    continue;

                concated = data_map->m_name;
                concated.erase( std::remove( concated.begin( ), concated.end( ), '_' ), concated.end( ) );

                concated += "->";
                concated += desc.m_name;

                offsets.insert_or_assign(
                    sdk::hash( concated.data( ), concated.size( ) ),
                    ent_offset_t{ nullptr, desc.m_offset }
                );
            }
        }

        return !offsets.empty( );
    }

    void c_ctx::init_offsets( const modules_t& modules ) {
        const code_section_t client{ modules.at( HASH( "client.dll" ) ) };

        m_offsets.m_local_player = BYTESEQ( "8B 0D ? ? ? ? 83 FF FF 74 07" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).self_deref( 1u );

        m_offsets.m_weapon_system = BYTESEQ( "8B 35 ? ? ? ? FF 10 0F B7 C0" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).self_deref( 1u );

        m_offsets.m_user_cmd_checksum = BYTESEQ( "53 8B D9 83 C8" ).search( client.m_start, client.m_end );

        m_offsets.m_anim_state.m_reset = BYTESEQ( "56 6A 01 68 ? ? ? ? 8B F1" ).search(
            client.m_start, client.m_end
        );
        m_offsets.m_anim_state.m_update = BYTESEQ( "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ).search(
            client.m_start, client.m_end
        );

        m_offsets.m_renderable.m_bone_cache = *BYTESEQ( "FF B7 ? ? ? ? 52" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).as< std::uint32_t* >( );
        m_offsets.m_renderable.m_mdl_bone_cnt = *BYTESEQ( "EB 05 F3 0F 10 45 ? 8B 87 ? ? ? ?" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x9 ).as< std::uint32_t* >( );

        ent_offsets_t offsets{};
        if ( !parse_ent_offsets( offsets, modules ) )
            THROW_IF_DBG( "can't find ent offsets." );

        m_offsets.m_base_entity.m_health = offsets.at( HASH( "CBaseEntity->m_iHealth" ) ).m_offset;
        m_offsets.m_base_entity.m_model_idx = offsets.at( HASH( "CBaseEntity->m_nModelIndex" ) ).m_offset;
        m_offsets.m_base_entity.m_team_num = offsets.at( HASH( "CBaseEntity->m_iTeamNum" ) ).m_offset;
        m_offsets.m_base_entity.m_sim_time = offsets.at( HASH( "CBaseEntity->m_flSimulationTime" ) ).m_offset;
        m_offsets.m_base_entity.m_flags = offsets.at( HASH( "CBaseEntity->m_fFlags" ) ).m_offset;
        m_offsets.m_base_entity.m_origin = offsets.at( HASH( "CBaseEntity->m_vecOrigin" ) ).m_offset;
        m_offsets.m_base_entity.m_anim_time = offsets.at ( HASH ( "CBaseEntity->m_flAnimTime" ) ).m_offset;
        m_offsets.m_base_entity.m_velocity = offsets.at( HASH( "CCSPlayer->m_vecVelocity[0]" ) ).m_offset;
        m_offsets.m_base_entity.m_abs_origin = offsets.at( HASH( "CBaseEntity->m_vecAbsOrigin" ) ).m_offset;
        m_offsets.m_base_entity.m_abs_velocity = offsets.at( HASH( "CBaseEntity->m_vecAbsVelocity" ) ).m_offset;
        m_offsets.m_base_entity.m_effects = offsets.at ( HASH ( "CBaseEntity->m_fEffects" ) ).m_offset;
        m_offsets.m_base_entity.m_abs_rotation = offsets.at( HASH( "CBaseEntity->m_angAbsRotation" ) ).m_offset;
        m_offsets.m_base_entity.m_move_type = offsets.at( HASH( "CBaseEntity->m_MoveType" ) ).m_offset;
        m_offsets.m_base_entity.m_rgfl = offsets.at ( HASH ( "CBaseEntity->m_CollisionGroup" ) ).m_offset;
        m_offsets.m_base_entity.m_mins = offsets.at( HASH( "CBaseEntity->m_vecMins" ) ).m_offset;
        m_offsets.m_base_entity.m_maxs = offsets.at( HASH( "CBaseEntity->m_vecMaxs" ) ).m_offset;
        m_offsets.m_player_resource.m_ping = offsets.at( HASH( "CCSPlayerResource->m_iPing" ) ).m_offset;
        m_offsets.m_cascade_light.m_shadow_dir = offsets.at( HASH( "CCascadeLight->m_envLightShadowDirection" ) ).m_offset;

        m_offsets.m_tone_map.m_use_custom_bloom_scale = offsets.at( HASH( "CEnvTonemapController->m_bUseCustomBloomScale" ) ).m_offset;
        m_offsets.m_tone_map.m_user_custom_auto_exposure_min = offsets.at( HASH( "CEnvTonemapController->m_bUseCustomAutoExposureMin" ) ).m_offset;
        m_offsets.m_tone_map.m_user_custom_auto_exposure_max = offsets.at( HASH( "CEnvTonemapController->m_bUseCustomAutoExposureMax" ) ).m_offset;
        m_offsets.m_tone_map.m_custom_bloom_scale = offsets.at( HASH( "CEnvTonemapController->m_flCustomBloomScale" ) ).m_offset;
        m_offsets.m_tone_map.m_auto_custom_exposure_min = offsets.at( HASH( "CEnvTonemapController->m_flCustomAutoExposureMin" ) ).m_offset;
        m_offsets.m_tone_map.m_auto_custom_exposure_max = offsets.at( HASH( "CEnvTonemapController->m_flCustomAutoExposureMax" ) ).m_offset;

        m_offsets.m_base_animating.m_cycle = offsets.at( HASH( "CBaseAnimating->m_flCycle" ) ).m_offset;
        m_offsets.m_base_animating.m_sequence = offsets.at ( HASH ( "CBaseAnimating->m_nSequence" ) ).m_offset;
        m_offsets.m_base_animating.m_force_bone = offsets.at ( HASH ( "CBaseAnimating->m_nForceBone" ) ).m_offset;
        m_offsets.m_base_animating.m_hitbox_set_index = offsets.at( HASH( "CBaseAnimating->m_nHitboxSet" ) ).m_offset;
        m_offsets.m_base_animating.m_b_cl_side_anim = offsets.at ( HASH ( "CBaseAnimating->m_bClientSideAnimation" ) ).m_offset;
        m_offsets.m_base_animating.m_thrower_handle = offsets.at ( HASH ( "CBaseGrenade->m_hThrower" ) ).m_offset;
       /* m_offsets.m_base_animating.m_studio_hdr = *BYTESEQ( "8B 86 ? ? ? ? 89 44 24 10 85 C0" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).as< std::uint32_t* >( );*/

        m_offsets.m_base_animating.m_studio_hdr = *BYTESEQ ( "8B B7 ? ? ? ? 89 74 24 20" ).search (
            client.m_start, client.m_end
        ).self_offset ( 0x2 ).as< std::uint32_t* > ( ) + 0x4;

        m_offsets.m_base_animating.m_lock_std_hdr = *BYTESEQ ( "55 8B EC 51 53 8B D9 56 57 8D B3" ).search (
            client.m_start, client.m_end
        ).as< std::uint32_t* > ( );

        m_offsets.m_base_animating.m_pose_params = offsets.at( HASH( "CBaseAnimating->m_flPoseParameter" ) ).m_offset;
        m_offsets.m_base_animating.m_anim_layers = *BYTESEQ( "8B 80 ? ? ? ? 8D 34 C8" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).as< std::uint32_t* >( );

        m_offsets.m_base_animating.m_bone_accessor = *BYTESEQ ( "8D 81 ? ? ? ? 50 8D 84 24" ).search (
            client.m_start, client.m_end
        ).self_offset ( 0x2 ).as< std::uint32_t* > ( );

        auto invalidate_bone_cache = BYTESEQ ( "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" ).search (
            client.m_start, client.m_end
        );

        m_offsets.m_base_animating.m_last_bone_setup_time = *invalidate_bone_cache.self_offset ( 0x11 ).as < std::uint32_t* > ( );
        m_offsets.m_base_animating.m_most_recent_model_bone_counter = *invalidate_bone_cache.self_offset ( 0x1b ).as < std::uint32_t* > ( );

        m_offsets.m_base_grenade.m_pin_pulled = offsets.at( HASH( "CBaseCSGrenade->m_bPinPulled" ) ).m_offset;
        m_offsets.m_base_grenade.m_throw_time = offsets.at( HASH( "CBaseCSGrenade->m_fThrowTime" ) ).m_offset;
        m_offsets.m_base_grenade.m_throw_strength = offsets.at( HASH( "CBaseCSGrenade->m_flThrowStrength" ) ).m_offset;
        m_offsets.m_base_grenade.m_did_smoke_effect = offsets.at ( HASH ( "CSmokeGrenadeProjectile->m_bDidSmokeEffect" ) ).m_offset;
        m_offsets.m_base_grenade.m_smoke_effect_tick_begin = offsets.at ( HASH ( "CSmokeGrenadeProjectile->m_nSmokeEffectTickBegin" ) ).m_offset;

        m_offsets.m_base_attributable_item.m_item_index = offsets.at( HASH( "CBaseAttributableItem->m_iItemDefinitionIndex" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_orig_owner_xuid_low = offsets.at( HASH( "CBaseAttributableItem->m_OriginalOwnerXuidLow" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_item_id_high = offsets.at( HASH( "CBaseAttributableItem->m_iItemIDHigh" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_entity_quality = offsets.at( HASH( "CBaseAttributableItem->m_iEntityQuality" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_account_id = offsets.at( HASH( "CBaseAttributableItem->m_iAccountID" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_fallback_paint_kit = offsets.at( HASH( "CBaseAttributableItem->m_nFallbackPaintKit" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_fallback_seed = offsets.at( HASH( "CBaseAttributableItem->m_nFallbackSeed" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_fallback_wear = offsets.at( HASH( "CBaseAttributableItem->m_flFallbackWear" ) ).m_offset;
        m_offsets.m_base_attributable_item.m_fallback_starttrak = offsets.at( HASH( "CBaseAttributableItem->m_nFallbackStatTrak" ) ).m_offset;

        m_offsets.m_base_weapon.m_clip1 = offsets.at( HASH( "CBaseCombatWeapon->m_iClip1" ) ).m_offset;
        m_offsets.m_base_weapon.m_item = offsets.at( HASH( "CBaseCombatWeapon->m_Item" ) ).m_offset;
        m_offsets.m_base_weapon.m_primary_reserve_ammo_count = offsets.at( HASH( "CBaseCombatWeapon->m_iPrimaryReserveAmmoCount" ) ).m_offset;
        m_offsets.m_base_weapon.m_world_dropped_mdl_idx = offsets.at( HASH( "CBaseCombatWeapon->m_iWorldDroppedModelIndex" ) ).m_offset;
        m_offsets.m_base_weapon.m_world_mdl_idx = offsets.at( HASH( "CBaseCombatWeapon->m_iWorldModelIndex" ) ).m_offset;
        m_offsets.m_base_weapon.m_view_mdl_idx = offsets.at( HASH( "CBaseCombatWeapon->m_iViewModelIndex" ) ).m_offset;
        m_offsets.m_base_weapon.m_next_primary_attack = offsets.at( HASH( "CBaseCombatWeapon->m_flNextPrimaryAttack" ) ).m_offset;
        m_offsets.m_base_weapon.m_next_secondary_attack = offsets.at( HASH( "CBaseCombatWeapon->m_flNextSecondaryAttack" ) ).m_offset;

        m_offsets.m_weapon_cs_base.m_burst_mode = offsets.at( HASH( "CWeaponCSBase->m_bBurstMode" ) ).m_offset;
        m_offsets.m_weapon_cs_base.m_last_shot_time = offsets.at( HASH( "CWeaponCSBase->m_fLastShotTime" ) ).m_offset;
        m_offsets.m_weapon_cs_base.m_recoil_index = offsets.at( HASH( "CWeaponCSBase->m_flRecoilIndex" ) ).m_offset;
        m_offsets.m_weapon_cs_base.m_accuracy_penalty = offsets.at ( HASH ( "CWeaponCSBase->m_fAccuracyPenalty" ) ).m_offset;
        m_offsets.m_weapon_cs_base.m_wpn_world_mdl_handle = offsets.at ( HASH ( "CBaseCombatWeapon->m_hWeaponWorldModel" ) ).m_offset;
        m_offsets.m_weapon_cs_base.m_postpone_fire_ready_time = offsets.at( HASH( "CWeaponCSBase->m_flPostponeFireReadyTime" ) ).m_offset;

        m_offsets.m_weapon_cs_base_gun.m_zoom_lvl = offsets.at( HASH( "CWeaponCSBaseGun->m_zoomLevel" ) ).m_offset;
        m_offsets.m_weapon_cs_base_gun.m_burst_shots_remaining = offsets.at( HASH( "CWeaponCSBaseGun->m_iBurstShotsRemaining" ) ).m_offset;
        m_offsets.m_weapon_cs_base_gun.m_next_burst_shot = offsets.at( HASH( "CWeaponCSBaseGun->m_fNextBurstShot" ) ).m_offset;

        m_offsets.m_base_combat_character.m_weapon_handle = offsets.at( HASH( "CBaseCombatCharacter->m_hActiveWeapon" ) ).m_offset;
        m_offsets.m_base_combat_character.m_next_attack = offsets.at( HASH( "CBaseCombatCharacter->m_flNextAttack" ) ).m_offset;

        m_offsets.m_c_base_view_model.m_sequence = offsets.at ( HASH ( "CBaseViewModel->m_nSequence" ) ).m_offset;
        m_offsets.m_c_base_view_model.m_animation_parity = offsets.at ( HASH ( "CBaseViewModel->m_nAnimationParity" ) ).m_offset;
        m_offsets.m_base_player.m_tick_base = offsets.at( HASH( "CBasePlayer->m_nTickBase" ) ).m_offset;
        m_offsets.m_base_player.m_max_player_speed = offsets.at( HASH( "CBasePlayer->m_flMaxspeed" ) ).m_offset;
        m_offsets.m_base_player.m_money = offsets.at ( HASH ( "CCSPlayer->m_iAccount" ) ).m_offset;
        m_offsets.m_base_player.m_has_helmet = offsets.at ( HASH ( "CCSPlayer->m_bHasHelmet" ) ).m_offset;
        m_offsets.m_base_player.m_has_heavy_armor = offsets.at ( HASH ( "CCSPlayer->m_bHasHeavyArmor" ) ).m_offset;
        m_offsets.m_predicted_view_model.m_weapon_handle = offsets.at( HASH( "CPredictedViewModel->m_hWeapon" ) ).m_offset;
        m_offsets.m_base_player.m_armor_val = offsets.at ( HASH ( "CCSPlayer->m_ArmorValue" ) ).m_offset;
        m_offsets.m_base_player.m_observer_mode = offsets.at ( HASH ( "CBasePlayer->m_iObserverMode" ) ).m_offset;
        m_offsets.m_base_player.m_observer_target = offsets.at ( HASH ( "CBasePlayer->m_hObserverTarget" ) ).m_offset;
        m_offsets.m_base_player.m_life_state = offsets.at( HASH( "CBasePlayer->m_lifeState" ) ).m_offset;
        m_offsets.m_base_player.m_duck_amt = offsets.at( HASH( "CBasePlayer->m_flDuckAmount" ) ).m_offset;
        m_offsets.m_base_player.m_ground_entity = offsets.at ( HASH ( "CBasePlayer->m_hGroundEntity" ) ).m_offset;
        m_offsets.m_base_player.m_duck_speed = offsets.at( HASH( "CBasePlayer->m_flDuckSpeed" ) ).m_offset;
        m_offsets.m_base_player.m_surface_friction = offsets.at ( HASH ( "CBasePlayer->m_surfaceFriction" ) ).m_offset;
        m_offsets.m_base_player.m_fall_velocity = offsets.at ( HASH ( "CBasePlayer->m_flFallVelocity" ) ).m_offset;
        m_offsets.m_base_player.m_base_velocity = offsets.at ( HASH ( "CBasePlayer->m_vecBaseVelocity" ) ).m_offset;
        m_offsets.m_base_player.m_model_idx = offsets.at( HASH( "CBasePlayer->m_nModelIndex" ) ).m_offset;
        m_offsets.m_base_player.m_next_attack = offsets.at ( HASH ( "CBasePlayer->m_flNextAttack" ) ).m_offset;
        m_offsets.m_base_player.m_spawn_time = *BYTESEQ( "89 86 ? ? ? ? E8 ? ? ? ? 80 BE" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).as< std::uint32_t* >( );
        m_offsets.m_base_player.m_aim_punch = offsets.at( HASH( "CBasePlayer->m_aimPunchAngle" ) ).m_offset;
        m_offsets.m_base_player.m_view_punch = offsets.at( HASH( "CBasePlayer->m_viewPunchAngle" ) ).m_offset;
        m_offsets.m_base_player.m_view_offset = offsets.at ( HASH ( "CBasePlayer->m_vecViewOffset[0]" ) ).m_offset;
        m_offsets.m_base_player.m_aim_punch_vel = offsets.at ( HASH ( "CBasePlayer->m_aimPunchAngleVel" ) ).m_offset;
        m_offsets.m_base_player.m_skin = offsets.at ( HASH ( "CBasePlayer->m_nSkin" ) ).m_offset;
        m_offsets.m_base_player.m_body = offsets.at ( HASH ( "CBasePlayer->m_nBody" ) ).m_offset;

        m_offsets.m_cs_player.m_lby = offsets.at( HASH( "CCSPlayer->m_flLowerBodyYawTarget" ) ).m_offset;
        m_offsets.m_cs_player.m_scoped = offsets.at ( HASH ( "CCSPlayer->m_bIsScoped" ) ).m_offset;
        m_offsets.m_cs_player.m_view_model = offsets.at ( HASH ( "CBasePlayer->m_hViewModel[0]" ) ).m_offset;
        m_offsets.m_cs_player.m_shots_fired = offsets.at( HASH( "CCSPlayer->m_iShotsFired" ) ).m_offset;
        m_offsets.m_cs_player.m_velocity_modifier = offsets.at ( HASH ( "CCSPlayer->m_flVelocityModifier" ) ).m_offset;
        m_offsets.m_cs_player.m_third_person_recoil = offsets.at ( HASH ( "CCSPlayer->m_flThirdpersonRecoil" ) ).m_offset;
        m_offsets.m_cs_player.m_walking = offsets.at ( HASH ( "CCSPlayer->m_bIsWalking" ) ).m_offset;
        m_offsets.m_cs_player.m_strafing = offsets.at ( HASH ( "CCSPlayer->m_bStrafing" ) ).m_offset;
        m_offsets.m_cs_player.m_wait_for_no_attack = offsets.at ( HASH ( "CCSPlayer->m_bWaitForNoAttack" ) ).m_offset;
        m_offsets.m_cs_player.m_move_state = offsets.at ( HASH ( "CCSPlayer->m_iMoveState" ) ).m_offset;
        m_offsets.m_cs_player.m_collision_state = offsets.at ( HASH ( "CBasePlayer->m_vphysicsCollisionState" ) ).m_offset;
        m_offsets.m_cs_player.m_eye_angles = offsets.at( HASH( "CCSPlayer->m_angEyeAngles" ) ).m_offset;
        m_offsets.m_base_animating.m_flash_dur = offsets.at ( HASH ( "CCSPlayer->m_flFlashDuration" ) ).m_offset;

#ifndef CSGO2018
        m_offsets.m_cs_player.m_survival_team = offsets.at( HASH( "CCSPlayer->m_nSurvivalTeam" ) ).m_offset;
#endif
        m_offsets.m_cs_player.m_anim_state = *BYTESEQ( "8B 8E ? ? ? ? 85 C9 74 3E" ).search(
            client.m_start, client.m_end
        ).self_offset( 0x2 ).as< std::uint32_t* >( );
        m_offsets.m_cs_player.m_defusing = offsets.at( HASH( "CCSPlayer->m_bIsDefusing" ) ).m_offset;
        //m_offsets.m_cs_player.m_is_scoped = offsets.at ( HASH ( "CCSPlayer->m_bIsScoped" ) ).m_offset;

        m_offsets.m_game_rules.m_warmup_period = offsets.at( HASH( "CCSGameRulesProxy->m_bWarmupPeriod" ) ).m_offset;
        m_offsets.m_game_rules.m_freeze_period = offsets.at( HASH( "CCSGameRulesProxy->m_bFreezePeriod" ) ).m_offset;
        m_offsets.m_game_rules.m_valve_ds = offsets.at( HASH( "CCSGameRulesProxy->m_bIsValveDS" ) ).m_offset;
        m_offsets.m_game_rules.m_bomb_planted = offsets.at( HASH( "CCSGameRulesProxy->m_bBombPlanted" ) ).m_offset;
    }

    void c_ctx::init_addresses ( const modules_t& modules )
    {
        const code_section_t client { modules.at ( HASH ( "client.dll" ) ) };
        const code_section_t engine { modules.at ( HASH ( "engine.dll" ) ) };
        const auto vstdlib_dll = GetModuleHandle ( xor_str ( "vstdlib.dll" ) );

        m_addresses.m_v_matrix = BYTESEQ ( "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9" ).search (
            client.m_start, client.m_end );

        m_addresses.m_sky_box = BYTESEQ ( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ).search ( engine.m_start, engine.m_end );

        m_addresses.m_lookup_seq_act = BYTESEQ ( "53 56 8B F1 8B DA 85 F6 74 55" ).search ( client.m_start, client.m_end );
        
        m_addresses.m_invalidate_bone_cache = BYTESEQ ( "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_cull_check = BYTESEQ( "85 C0 0F 84 ? ? ? ? 8B 0D ? ? ? ? 8D 54 24 6C" ).search(
            client.m_start, client.m_end
        );

        m_addresses.m_voice_msg_ctor = BYTESEQ( "56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7" ).search(
            engine.m_start, engine.m_end
        );

        m_addresses.m_trace_filter_simple_vtable = *reinterpret_cast < std::uintptr_t* > ( BYTESEQ ( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ).search ( 
            client.m_start, client.m_end ) + 0x3du );

        m_addresses.m_trace_filter_skip_two_entities_vtable = *reinterpret_cast< std::uintptr_t* >(
            BYTESEQ ( "C7 45 ? ? ? ? ? 89 45 E4 8B 01" ).search(
                client.m_start, client.m_end ) + 0x3u );

        m_addresses.m_pred_seed = *reinterpret_cast< int** >(
            BYTESEQ ( "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4" ).search ( client.m_start, client.m_end ) + 2u
            );

        m_addresses.m_ret_to_check_receiving_list = BYTESEQ (
            "8B 1D ? ? ? ? 85 C0 74 16 FF B6" ).search (
            engine.m_start, engine.m_end ).as < std::uintptr_t* >( );

        m_addresses.m_ret_to_read_sub_channel_data_return = BYTESEQ (
            "85 C0 74 12 53 FF 75 0C 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C" ).search (
                engine.m_start, engine.m_end ).as < std::uintptr_t* > ( );

        m_addresses.m_random_float = reinterpret_cast< addresses_t::random_float_t >( GetProcAddress ( vstdlib_dll, xor_str ( "RandomFloat" ) ) );
        m_addresses.m_random_int = reinterpret_cast< addresses_t::random_int_t >( GetProcAddress ( vstdlib_dll, xor_str ( "RandomInt" ) ) );
        m_addresses.m_random_seed = reinterpret_cast< addresses_t::random_seed_t >( GetProcAddress ( vstdlib_dll, xor_str ( "RandomSeed" ) ) );
        m_addresses.m_smoke_count = BYTESEQ ( "A3 ? ? ? ? 57 8B CB" ).search ( client.m_start, client.m_end ) + 0x1u;
        m_addresses.m_post_process = BYTESEQ ( "83 EC 4C 80 3D" ).search ( client.m_start, client.m_end ) + 0x5;

        m_addresses.m_ret_to_allah = BYTESEQ ( "EB 0D FF 77 10" ).search ( engine.m_start, engine.m_end );

        m_addresses.m_set_collision_bounds = BYTESEQ ( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B" ).search ( 
            client.m_start, client.m_end );

        m_addresses.m_pred_player = *reinterpret_cast< valve::cs_player_t** >(
            BYTESEQ ( "89 ?? ?? ?? ?? ?? F3 0F 10 48 20" ).search ( client.m_start, client.m_end ) + 2u
            );

        m_addresses.m_ret_to_extrapolation = BYTESEQ ( "0F B6 0D ? ? ? ? 84 C0 0F 44 CF 88 0D ? ? ? ?" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_ret_to_setup_velocity = BYTESEQ ( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_ret_to_accumulate_layers = BYTESEQ ( "84 C0 75 0D F6 87" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_is_breakable = BYTESEQ ( "55 8B EC 51 56 8B F1 85 F6 74 68" ).search ( 
            client.m_start, client.m_end );

        m_addresses.m_set_abs_ang = BYTESEQ ( "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" ).search ( client.m_start, client.m_end );
        const auto calc_shotgun_spread_rel = BYTESEQ ("E8 ? ? ? ? EB 38 83 EC 08" ).search ( client.m_start, client.m_end );

        m_addresses.m_calc_shotgun_spread =
            calc_shotgun_spread_rel + 0x1u + sizeof ( std::uintptr_t )
            + *reinterpret_cast< std::ptrdiff_t* >( calc_shotgun_spread_rel + 0x1u );

        m_addresses.m_set_abs_origin = BYTESEQ ( "55 8B EC 83 E4 F8 51 53 56 57 8B F1" ).search ( client.m_start, client.m_end );

        m_addresses.m_cl_read_packets = BYTESEQ ( "53 8A D9 8B 0D ? ? ? ? 56 57 8B B9" ).search (
            engine.m_start, engine.m_end ).as < addresses_t::cl_read_packets_t > ( );

        m_addresses.m_attachment_helper = BYTESEQ ( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4" ).search ( client.m_start, client.m_end );

        m_addresses.m_set_clan_tag = (
            BYTESEQ ( "53 56 57 8B DA 8B F9 FF 15" ).search ( engine.m_start, engine.m_end )
            );

        m_addresses.m_clip_ray = ( BYTESEQ( "55 8B EC 83 E4 F8 F3 0F 10 42" ).search( client.m_start, client.m_end ) );

        m_addresses.m_unlock_packets = BYTESEQ( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ).search( engine.m_start, engine.m_end ) + 0x1;

        m_addresses.m_invalidate_physics_recursive = BYTESEQ( "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04" ).search( client.m_start, client.m_end );

        m_addresses.m_lookup_bone = BYTESEQ ( "55 8B EC 53 56 8B F1 57 83 BE ?? ?? ?? ?? ?? 75 14 8B 46 04 8D 4E 04 FF 50 20" ).search ( 
            client.m_start, client.m_end );

        m_addresses.m_allow_to_extrapolate = BYTESEQ ( "A2 ? ? ? ? 8B 45 E8" ).search (
            client.m_start, client.m_end );

        m_addresses.m_compute_hitbox_surround_box = ( BYTESEQ ( "E9 ? ? ? ? 32 C0 5D" ).search ( client.m_start, client.m_end ) );
        const std::uintptr_t angle_matrix_rel = BYTESEQ ( "E8 ? ? ? ? 8B 07 89 46 0C" ).search ( client.m_start, client.m_end );
        const std::uintptr_t show_and_update_selection_rel = BYTESEQ( "E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 ? C6 83" ).search( client.m_start, client.m_end );
        const std::uintptr_t clear_notices_rel = BYTESEQ( "E8 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 74 19" ).search( client.m_start, client.m_end );

        m_addresses.m_angle_matrix = reinterpret_cast< addresses_t::angle_matrix_t >(
            angle_matrix_rel + 0x1u + sizeof ( std::uintptr_t ) + *reinterpret_cast< std::ptrdiff_t* >( angle_matrix_rel + 0x1u )
            );

        valve::fn_show_and_update_selection = reinterpret_cast< valve::show_and_update_selection_t >(
            show_and_update_selection_rel + 0x1u + sizeof( std::uintptr_t ) + *reinterpret_cast< std::ptrdiff_t* >( show_and_update_selection_rel + 0x1u )
            );


        m_addresses.m_clear_notices =
            reinterpret_cast< addresses_t::clear_notices_t >( clear_notices_rel + 0x1u + sizeof( std::uintptr_t )
                + *reinterpret_cast< std::ptrdiff_t* >( clear_notices_rel + 0x1u ) );

        m_addresses.m_write_user_cmd = BYTESEQ ( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ).search ( 
            client.m_start, client.m_end ); 

        m_addresses.m_ret_insert_into_tree = BYTESEQ ( "89 44 24 14 EB 08 C7 44 24 ? ? ? ? ? 8B 45" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_upd_visb_entities = BYTESEQ ( "E8 ? ? ? ? 83 7D D8 00 7C 0F" ).search (
            client.m_start, client.m_end
        );

        m_addresses.m_nigga_who = BYTESEQ ( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ).search (
            engine.m_start, engine.m_end
        );
    }

    void c_ctx::init_cvars( ) {
        m_cvars.m_cl_forwardspeed = valve::g_cvar->find_var( "cl_forwardspeed" );
        m_cvars.m_cl_sidespeed = valve::g_cvar->find_var( "cl_sidespeed" );
        m_cvars.m_cl_upspeed = valve::g_cvar->find_var( "cl_upspeed" );

        m_cvars.m_cl_pitchdown = valve::g_cvar->find_var( "cl_pitchdown" );
        m_cvars.m_cl_pitchup = valve::g_cvar->find_var( "cl_pitchup" );
        m_cvars.m_cl_interp = valve::g_cvar->find_var( xor_str( "cl_interp" ) );
        m_cvars.m_cl_interp_ratio = valve::g_cvar->find_var( xor_str( "cl_interp_ratio" ) );
        m_cvars.m_cl_updaterate = valve::g_cvar->find_var( xor_str( "cl_updaterate" ) );
        m_cvars.m_mp_teammates_are_enemies = valve::g_cvar->find_var( "mp_teammates_are_enemies" );

        m_cvars.m_sv_maxunlag = valve::g_cvar->find_var( xor_str( "sv_maxunlag" ) );

        m_cvars.m_sv_maxvelocity = valve::g_cvar->find_var ( xor_str ( "sv_maxvelocity" ) );
        m_cvars.m_sv_friction = valve::g_cvar->find_var ( xor_str ( "sv_friction" ) );
        m_cvars.m_sv_accelerate = valve::g_cvar->find_var ( xor_str ( "sv_accelerate" ) );
        m_cvars.m_sv_accelerate_use_weapon_speed = valve::g_cvar->find_var ( xor_str ( "sv_accelerate_use_weapon_speed" ) );
        m_cvars.m_weapon_accuracy_shotgun_spread_patterns = valve::g_cvar->find_var ( xor_str ( "weapon_accuracy_shotgun_spread_patterns" ) );
    }

    void c_ctx::init_hooks( const modules_t& modules ) const {
        const code_section_t vguimatsurface{ modules.at( HASH( "vguimatsurface.dll" ) ) };
        const code_section_t cl_section{ modules.at( HASH( "client.dll" ) ) };
        const code_section_t eng_section{ modules.at( HASH( "engine.dll" ) ) };
        const code_section_t material_sys{ modules.at( HASH( "materialsystem.dll" ) ) };

        HOOK( BYTESEQ( "80 3D ? ? ? ? ? 8B 91 ? ? ? ? 8B 0D ? ? ? ? C6 05 ? ? ? ? 01" ).search(
            vguimatsurface.m_start, vguimatsurface.m_end, false
        ), hooks::lock_cursor, hooks::o_lock_cursor );

        HOOK( BYTESEQ( "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 21" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::physics_simulate, hooks::orig_physics_simulate );

        HOOK( BYTESEQ( "55 8B EC 53 8B 5D 08 56 57 FF 75 18" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::calc_view, hooks::orig_calc_view );

        HOOK( BYTESEQ( "55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::calc_viewmodel_bob, hooks::orig_calc_viewmodel_bob );

        HOOK( BYTESEQ( "55 8B EC 83 EC ? 56 8B F1 8A 46" ).search(
            material_sys.m_start, material_sys.m_end, false ),
            hooks::get_color_modulation, hooks::orig_get_clr_modulation );

        HOOK( BYTESEQ( "56 8B F1 8A 46 20 C0 E8 02 A8 01 75 0B 6A 00 6A 00 6A 00 E8 ? ? ? ? 80 7E 22 05 76 0E" ).search(
            material_sys.m_start, material_sys.m_end, false ),
            hooks::get_alpha_modulation, hooks::orig_get_alpha_modulation );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::do_extra_bones_processing, hooks::orig_do_extra_bones_processing );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::build_transformations, hooks::orig_build_transformations );

        HOOK( BYTESEQ( "55 8B EC 57 8B F9 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ?" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::accumulate_layers, hooks::orig_accumulate_layers );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::standard_blending_rules, hooks::orig_standard_blending_rules );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F0 B8 D8" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::setup_bones, hooks::orig_setup_bones );

        HOOK( BYTESEQ( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::update_client_side_anim, hooks::orig_update_client_side_anim );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60 00" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::modify_eye_pos, hooks::orig_modify_eye_pos );

        HOOK( BYTESEQ( "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 56 57 8B 7D 08 8B D9" ).search(
            eng_section.m_start, eng_section.m_end, false ),
            hooks::process_packet, hooks::orig_process_packet );

        HOOK( BYTESEQ( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18" ).search(
            eng_section.m_start, eng_section.m_end, false ),
            hooks::send_datagram, hooks::orig_send_datagram );

        HOOK( BYTESEQ( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A" ).search(
            eng_section.m_start, eng_section.m_end, false ),
            hooks::cl_move, hooks::orig_cl_move );

        HOOK( BYTESEQ( "55 8B EC 51 80 3D ? ? ? ? ? 0F 57" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::exposure_range, hooks::orig_exposure_range );

        HOOK( BYTESEQ( "53 0F B7 1D ? ? ? ? 56" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::process_interpolated_list, hooks::orig_process_interp_list );

        HOOK( BYTESEQ( "55 8B EC 8B 45 08 89 81 ? ? ? ? 8B 45 0C 89 81 ? ? ? ? 5D C2 08 00 CC CC CC CC CC CC CC 56" ).search(
            eng_section.m_start, eng_section.m_end, false ),
            hooks::packet_start, hooks::orig_packet_start );

        HOOK( BYTESEQ( "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?" ).search(
            eng_section.m_start, eng_section.m_end, false ),
            hooks::packet_end, hooks::orig_packet_end );

        HOOK( BYTESEQ( "55 8B EC 51 57 E8" ).search(
            cl_section.m_start, cl_section.m_end, false ),
            hooks::should_draw_view_model, hooks::orig_should_draw_view_model );

        const auto client_mode = **reinterpret_cast< std::uintptr_t*** >(
            ( *reinterpret_cast< std::uintptr_t** >( valve::g_client ) )[ 10u ] + 0x5u
            );

        const auto nigga_cheats = ( void* ) ( uintptr_t ( valve::g_client_state.get ( ) ) + 0x8 );

        HOOK_VFUNC( nigga_cheats, VARVAL( 24u, 24u ), hooks::svc_msg_voice_data, hooks::o_svc_msg_voice_data );
        HOOK_VFUNC( valve::g_client, VARVAL( 21u, 22u ), hooks::create_move_proxy, hooks::o_create_move );
        HOOK_VFUNC ( valve::g_panel, VARVAL ( 41u, 41u ), hooks::paint_traverse, hooks::o_paint_traverse ); 
        HOOK_VFUNC ( valve::g_prediction, VARVAL ( 19u, 19u ), hooks::run_cmd, hooks::orig_run_cmd );
        HOOK_VFUNC ( valve::g_client, VARVAL ( 36u, 36u ), hooks::frame_stage_notify, hooks::orig_frame_stage_notify );
        HOOK_VFUNC ( valve::g_movement, VARVAL ( 1u, 1u ), hooks::process_movement, hooks::orig_process_movement );
        HOOK_VFUNC ( client_mode, VARVAL ( 18u, 18u ), hooks::override_view, hooks::orig_override_view );
        HOOK_VFUNC ( client_mode, VARVAL ( 44u, 44u ), hooks::do_post_screen_space_effects, hooks::orig_do_post_screen_space_effects );
        HOOK_VFUNC ( valve::g_engine, VARVAL ( 101u, 101u ), hooks::aspect_ratio, hooks::orig_aspect_ratio );
        HOOK_VFUNC( valve::g_prediction, VARVAL( 20u, 20u ), hooks::setup_move, hooks::orig_setup_move );
        HOOK_VFUNC ( valve::g_mdl_render, VARVAL ( 21u, 21u ), hooks::draw_mdl_exec, hooks::orig_draw_mdl_exec );
        HOOK_VFUNC ( valve::g_engine->bsp_tree_query ( ), VARVAL ( 6u, 6u ), hooks::list_leaves_in_box, hooks::orig_list_leaves_in_box );
        HOOK_VFUNC ( nigga_cheats, VARVAL ( 36u, 36u ), hooks::process_temp_entities, hooks::orig_process_temp_entities );
        HOOK_VFUNC ( valve::g_engine, VARVAL ( 90u, 90u ), hooks::is_paused, hooks::orig_is_paused );
        HOOK_VFUNC ( valve::g_engine, VARVAL ( 93u, 93u ), hooks::is_hltv, hooks::orig_is_hltv );
        HOOK_VFUNC ( valve::g_client, VARVAL ( 23u, 24u ), hooks::write_user_cmd_delta_to_buffer, hooks::orig_write_user_cmd_delta_to_buffer );
    }

    void c_ctx::init( ) {

        modules_t modules{};
        while ( wait_for_all_modules( modules ) )
            std::this_thread::sleep_for( std::chrono::milliseconds{ 200u } );

        if ( MH_Initialize( ) != MH_OK )
            THROW_IF_DBG( "can't initialize minhook." );

        init_imgui( modules );

        init_interfaces( modules );

        init_offsets( modules );

        init_addresses ( modules );

        init_cvars( );

        init_hooks( modules );

        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "bullet_impact" ), false );
        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "player_hurt" ), false );
        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "weapon_fire" ), false );
        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "round_freeze_end" ), false );
        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "round_prestart" ), false );
        valve::g_game_event_mgr->add_listener ( &hooks::g_event_listener, xor_str ( "player_footstep" ), false );
        valve::g_game_event_mgr->add_listener( &hooks::g_event_listener, xor_str( "weapon_reload" ), false );
        valve::g_game_event_mgr->add_listener( &hooks::g_event_listener, xor_str( "item_equip" ), false );
        valve::g_game_event_mgr->add_listener( &hooks::g_event_listener, xor_str( "bomb_beep" ), false );

        hacks::g_chams->init_chams ( );

        valve::g_net_vars->parse_client_classes ( );

        auto& vel_mod_prop = std::get< valve::recv_prop_t* > ( valve::g_net_vars->entry ( xor_str ( "CCSPlayer->m_flVelocityModifier" ) ) );

        hooks::orig_velocity_modifier = reinterpret_cast< decltype( hooks::orig_velocity_modifier ) >( vel_mod_prop->m_proxy_fn );

        vel_mod_prop->m_proxy_fn = reinterpret_cast< std::uintptr_t >( &hooks::velocity_modifier );

        auto lby_prop = std::get< valve::recv_prop_t* > ( valve::g_net_vars->entry ( xor_str ( "CCSPlayer->m_flLowerBodyYawTarget" ) ) );

        hooks::orig_lby_proxy = reinterpret_cast < decltype ( hooks::orig_lby_proxy ) > ( lby_prop->m_proxy_fn );

        lby_prop->m_proxy_fn = reinterpret_cast< std::uintptr_t >( &hooks::lower_body_yaw_proxy );

        if ( MH_EnableHook( MH_ALL_HOOKS ) != MH_OK )
            THROW_IF_DBG( "can't enable all hooks." );
    }
}

void c_key_binds::set_keybind_state( s_keybind* bind, int val ) {
    bind->m_mode = val;
}

bool c_key_binds::get_keybind_state ( const s_keybind* bind )
{
    switch ( bind->m_mode )
    {
        case 0: return bind->m_code && GetAsyncKeyState ( bind->m_code ); break;
        case 1: return bind->m_code && GetKeyState ( bind->m_code ); break;
        case 2: return true; break;
        case 3: return false; break;
    }

    return false;
}

int c_key_binds::get_keybind_mode( const s_keybind* bind ) {
    return bind->m_mode;
}

#include "../../../sdk/keys.h"

inline const char* Keys__[ ] = {
"[-]",
"[M1]",
"[M2]",
"[CN]",
"[M3]",
"[M4]",
"[M5]",
"[-]",
"[BAC]",
"[TAB]",
"[-]",
"[-]",
"[CLR]",
"[RET]",
"[-]",
"[-]",
"[SHI]",
"[CTL]",
"[ALT]",
"[PAU]",
"[CAP]",
"[KAN]",
"[-]",
"[JUN]",
"[FIN]",
"[KAN]",
"[-]",
"[ESC]",
"[CON]",
"[NCO]",
"[ACC]",
"[MAD]",
"[SPA]",
"[PGU]",
"[PGD]",
"[END]",
"[HOM]",
"[LEF]",
"[UP]",
"[RIG]",
"[DOW]",
"[SEL]",
"[PRI]",
"[EXE]",
"[PRI]",
"[INS]",
"[DEL]",
"[HEL]",
"[0]",
"[1]",
"[2]",
"[3]",
"[4]",
"[5]",
"[6]",
"[7]",
"[8]",
"[9]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[A]",
"[B]",
"[C]",
"[D]",
"[E]",
"[F]",
"[G]",
"[H]",
"[I]",
"[J]",
"[K]",
"[L]",
"[M]",
"[N]",
"[O]",
"[P]",
"[Q]",
"[R]",
"[S]",
"[T]",
"[U]",
"[V]",
"[W]",
"[X]",
"[Y]",
"[Z]",
"[WIN]",
"[WIN]",
"[APP]",
"[-]",
"[SLE]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[NUM]",
"[MUL]",
"[ADD]",
"[SEP]",
"[MIN]",
"[DEC]",
"[DIV]",
"[F1]",
"[F2]",
"[F3]",
"[F4]",
"[F5]",
"[F6]",
"[F7]",
"[F8]",
"[F9]",
"[F10]",
"[F11]",
"[F12]",
"[F13]",
"[F14]",
"[F15]",
"[F16]",
"[F17]",
"[F18]",
"[F19]",
"[F20]",
"[F21]",
"[F22]",
"[F23]",
"[F24]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[NUM]",
"[SCR]",
"[EQU]",
"[MAS]",
"[TOY]",
"[OYA]",
"[OYA]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[-]",
"[SHI]",
"[SHI]",
"[CTR]",
"[CTR]",
"[ALT]",
"[ALT]"
};
const char* modes[ ] = { "hold", "toggle", "always on", "always off" };
void c_key_binds::KeybindNelfo ( const char* label, s_keybind* keybind, int& nah_bro, int x_pos, int y_pos, int& who, bool is_manual, int x_bind )
{
    if ( !is_manual ) {
        ImGui::PushItemWidth( 430.f );
        ImGui::Combo( label, &keybind->m_mode, nah_bro, who, modes, IM_ARRAYSIZE( modes ) );
        ImGui::PopItemWidth( );
    }
    else {
        ImGui::SetCursorPosX( x_pos );
        ImGui::Text( label );
        keybind->m_mode = 0;
    }
    std::string ButtonString;

    if ( keybind->m_is_binding )
        ButtonString = xor_str ( "..." );
    else if ( !keybind->m_code )
        ButtonString = xor_str ( "-" );
    else
    {
        ButtonString = Keys__[ keybind->m_code ];
    }
   // ImGui::SetCursorPosX ( x_pos );
  //  ImGui::SetCursorPosY( y_pos );

    ImGui::SameLine( );
    ImGui::SetCursorPosX( x_bind );
    std::string str = std::string( ButtonString + xor_str( "###BindButton_" ) + label );

    auto len_ = str.length( );

    if ( len_ >= 4 )
        len_ += 9;

    if ( ImGui::Button ( ( ButtonString + std::string ( xor_str ( "###BindButton_" ) ) + label ).c_str ( ), ImVec2 ( len_, 18 ) ) )
    {
        keybind->m_is_binding = true;
        m_last_code = 0;
    }

    //ImGui::SetCursorPosY( last_y_pos );

    if ( keybind->m_is_binding && m_last_code )
    {
        keybind->m_code = m_last_code;
        keybind->m_is_binding = false;
        m_last_code = 0;
    }
}

#undef HOOK
#undef HOOK_VFUNC

#undef THROW_IF_DBG