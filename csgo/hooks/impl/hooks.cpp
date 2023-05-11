#include "../../csgo.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

namespace csgo::hooks { 
    LRESULT __stdcall wnd_proc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam ) { 

        switch( msg )
        { 
            case WM_CHAR:
            { 
                wchar_t wch;
                MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,( char* )&wparam, 1, &wch, 1 );
                gui::GetIO( ).AddInputCharacter( wch );

                return o_wnd_proc( wnd, msg, wparam, lparam );
            }

            case WM_XBUTTONDOWN:

                switch( GET_KEYSTATE_WPARAM( wparam ) )
                { 
                    case MK_XBUTTON1: g_key_binds->m_last_code = VK_XBUTTON1; break;
                    case MK_XBUTTON2: g_key_binds->m_last_code = VK_XBUTTON2; break;
                }

                break;

            case WM_MBUTTONDOWN:

                g_key_binds->m_last_code = VK_MBUTTON;
                break;

            case WM_RBUTTONDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            case WM_LBUTTONDOWN:

                g_key_binds->m_last_code = wparam;
                break;
        }

        if( msg == WM_KEYUP
            && wparam == VK_INSERT )
            g_menu->main( ).m_hidden ^= 1;

        if( !g_menu->main( ).m_hidden ) { 
            ImGui_ImplWin32_WndProcHandler( wnd, msg, wparam, lparam );

            return 1;
        }

        return o_wnd_proc( wnd, msg, wparam, lparam );
    }

    long D3DAPI dx9_reset( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params ) { 
        ImGui_ImplDX9_InvalidateDeviceObjects( );

        const auto ret = o_dx9_reset( device, params );

        ImGui_ImplDX9_CreateDeviceObjects( );
        post_process::on_reset( );

        return ret;
    }

    long D3DAPI dx9_present( IDirect3DDevice9* device,
        RECT* src_rect, RECT* dst_rect, HWND dst_wnd_override, RGNDATA* dirty_region
    ) { 
        thread_local bool pr_set = false;

        if( !pr_set )
        { 
            SetThreadPriorityBoost( GetCurrentThread( ), FALSE );
            SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_HIGHEST );
            pr_set = true;
        }

        DWORD colorwrite, srgbwrite;
        IDirect3DVertexDeclaration9* vert_dec = nullptr;
        IDirect3DVertexShader9* vert_shader = nullptr;
        DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;

        device->GetRenderState( D3DRS_COLORWRITEENABLE, &colorwrite );
        device->GetRenderState( D3DRS_SRGBWRITEENABLE, &srgbwrite );
        device->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
        device->SetRenderState( D3DRS_SRGBWRITEENABLE, false );
        device->GetRenderState( D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE );
        device->GetVertexDeclaration( &vert_dec );
        device->GetVertexShader( &vert_shader );
        device->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
        device->SetRenderState( D3DRS_SRGBWRITEENABLE, false );
        device->SetSamplerState( NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
        device->SetSamplerState( NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
        device->SetSamplerState( NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
        device->SetSamplerState( NULL, D3DSAMP_SRGBTEXTURE, NULL );
        ImGui_ImplDX9_NewFrame( );
        ImGui_ImplWin32_NewFrame( );

        gui::NewFrame( );
        post_process::set_device_next( device );

        g_menu->render( );

        g_render->add_to_draw_list( );

        hacks::g_misc->draw_watermark( );

        gui::EndFrame( );

        gui::Render( );

        ImGui_ImplDX9_RenderDrawData( gui::GetDrawData( ) );

        device->SetRenderState( D3DRS_COLORWRITEENABLE, colorwrite );
        device->SetRenderState( D3DRS_SRGBWRITEENABLE, srgbwrite );
        device->SetRenderState( D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE );
        device->SetRenderState( D3DRS_SRGBWRITEENABLE, true );
        device->SetVertexDeclaration( vert_dec );
        device->SetVertexShader( vert_shader );

        return o_dx9_present( device, src_rect, dst_rect, dst_wnd_override, dirty_region );
    }

    void __fastcall modify_eye_pos( game::anim_state_t* ecx, std::uintptr_t edx, sdk::vec3_t& pos ) { 
        if( g_ctx->left_create_move( ) )
            return orig_modify_eye_pos( ecx, edx, pos );

        return;
    }

    void __fastcall interpolate_server_entities( )
    { 
        if( !g_local_player->self( ) )
            return orig_interpolate_server_entities( );

        g_local_player->self( )->set_abs_ang( sdk::qang_t( 0.f, g_ctx->anim_data( ).m_local_data.m_abs_ang, 0.f ) );

	    orig_interpolate_server_entities( );

        for( int i = 1; i <= game::g_global_vars.get( )->m_max_clients; ++i ) { 
            const auto entity = static_cast< game::cs_player_t* > ( 
				game::g_entity_list->get_entity( i )
				 );

            if( !entity )
                continue;

            if( entity == g_local_player->self( )
                || !entity->alive( ) 
                || entity->networkable( )->dormant( ) )
                continue;

            entity->update_visibility_all_entities( );

            // generate visual matrix
            g_ctx->anim_data( ).m_allow_setup_bones = true;
            entity->setup_bones( nullptr, 256, 0x7FF00, entity->sim_time( ) );
            g_ctx->anim_data( ).m_allow_setup_bones = false;
        }
    }

    void __fastcall lock_cursor( const std::uintptr_t ecx, const std::uintptr_t edx ) { 
        using unlock_cursor_t = void( __thiscall* )( const std::uintptr_t );
        if( !g_menu->main( ).m_hidden )
            return ( *sdk::address_t{ ecx }.as< unlock_cursor_t** > ( ) )[ 66u ]( ecx );

        o_lock_cursor( ecx, edx );
    }

    int __fastcall do_post_screen_space_effects( 
        const std::uintptr_t ecx, const std::uintptr_t edx, game::view_setup_t* const setup
    ) { 
        hacks::g_visuals->draw_shot_mdl( );
        hacks::g_visuals->draw_glow( );

        return orig_do_post_screen_space_effects( ecx, edx, setup );
    }

    void __fastcall setup_move( const std::uintptr_t ecx, const std::uintptr_t edx, game::cs_player_t* player, game::user_cmd_t* user_cmd, game::c_move_helper* move_helper, game::move_data_t* m_moving_data ) { 
        orig_setup_move( ecx, edx, player, user_cmd, move_helper, m_moving_data );
    }

    NAKED void __stdcall create_move_proxy( int seq_number, float input_sample_frame_time, bool active ) { 
        __asm { 
            push ebp
            mov ebp, esp

            push ebx
            push esp
            push dword ptr[ ebp + 16 ]
            push dword ptr[ ebp + 12 ]
            push dword ptr[ ebp + 8 ]

            call create_move

            pop ebx

            pop ebp
            retn 12
        }
    }

    void __fastcall calc_viewmodel_bob( void* ecx, const std::uintptr_t edx, sdk::vec3_t& view_bob ) { 

        if( hacks::g_visuals->cfg( ).m_removals & 256 && ecx != g_local_player->self( ) )
            return;

        return orig_calc_viewmodel_bob( ecx, edx, view_bob );
    }

    bool __fastcall setup_bones( 
        const std::uintptr_t ecx, const std::uintptr_t edx, sdk::mat3x4_t* const bones, int max_bones, int mask, float time
    ) { 
        const auto player = reinterpret_cast < game::cs_player_t* > ( ecx - sizeof( std::ptrdiff_t ) );

        if( !player
            || !player->alive( )
            || player->networkable( )->index( ) < 1
            || player->networkable( )->index( ) > 64 )
            return orig_setup_bones( ecx, edx, bones, max_bones, mask, time );

        if( player->team( ) == g_local_player->self( )->team( ) )
            if( player != g_local_player->self( ) )
                return orig_setup_bones( ecx, edx, bones, max_bones, mask, time );

        if( !g_ctx->anim_data( ).m_allow_setup_bones ) { 
            if( !bones
                || max_bones == -1 )
                return true;

            if( player == g_local_player->self( ) ) { 
                std::memcpy( 
                    bones, g_ctx->anim_data( ).m_local_data.m_bones.data( ),
                    std::min( max_bones, 256 ) * sizeof( sdk::mat3x4_t )
                );
            }
            else { 
                const auto& entry = hacks::g_lag_comp->entry( player->networkable( )->index( ) - 1 );

                std::memcpy( 
                    bones, entry.m_bones.data( ),
                    std::min( max_bones, 256 ) * sizeof( sdk::mat3x4_t )
                );
            }

            return true;
        }

        return orig_setup_bones( ecx, edx, bones, max_bones, mask, time );
    }

    __forceinline void set_origin( sdk::mat3x4_t& who, const sdk::vec3_t p ) { 
        who [ 0 ][ 3 ] = p.x( );
        who [ 1 ][ 3 ] = p.y( );
        who [ 2 ][ 3 ] = p.z( );
    }

   void __fastcall update_client_side_anim( game::cs_player_t* const player, const std::uintptr_t edx ) { 
        if( !player
            || !player->alive( )
            || player->networkable( )->index( ) < 1
            || player->networkable( )->index( ) > 64 )
            return orig_update_client_side_anim( player, edx );

        if( player->team( ) == g_local_player->self( )->team( ) )
            if( player != g_local_player->self( ) )
                return orig_update_client_side_anim( player, edx );

        if( !g_ctx->anim_data( ).m_allow_update ) { 

            if( g_local_player->self( ) == player ) { 
                for( std::ptrdiff_t i { }; i < game::k_max_bones; ++i ) { 
                    set_origin( g_ctx->anim_data( ).m_local_data.m_bones [ i ], g_local_player->self( )->origin( ) - g_ctx->anim_data( ).m_local_data.m_bone_origins.at( i ) );
                }
                if( !g_ctx->anim_data( ).m_local_data.m_bones.empty( ) ) { 
                    std::memcpy( player->bone_cache( ).m_mem.m_ptr,
                        g_ctx->anim_data( ).m_local_data.m_bones.data( ),
                        sizeof( sdk::mat3x4_t ) * player->bone_cache( ).m_size );

                    std::memcpy( player->bone_accessor( ).m_bones,
                        g_ctx->anim_data( ).m_local_data.m_bones.data( ),
                        sizeof( sdk::mat3x4_t ) * player->bone_cache( ).m_size );
                }
            }

            player->attachment_helper( player->mdl_ptr( ) );
            return;
        }

        for( std::ptrdiff_t i { }; i < player->anim_layers( ).size( ); ++i ) { 
            player->anim_layers( ).at( i ).m_owner = player;
        }

        orig_update_client_side_anim( player, edx );
    }

    void __fastcall standard_blending_rules( 
        game::cs_player_t* const ecx, const std::uintptr_t edx, game::studio_hdr_t* const mdl_data, int a1, int a2, float a3, int mask
    ) { 
        if( ecx->networkable( )->index( ) < 1
            || ecx->networkable( )->index( ) > 64 )
            return orig_standard_blending_rules( ecx, edx, mdl_data, a1, a2, a3, mask );

        int bone_mask = mask;

        if( ecx != g_local_player->self( ) )
            bone_mask = 0x3FD03; // bone used by server

        ecx->effects( ) |= 8;

        orig_standard_blending_rules( ecx, edx, mdl_data, a1, a2, a3, bone_mask );

        ecx->effects( ) &= ~8;
    }

    void __fastcall accumulate_layers( 
        game::cs_player_t* const ecx, const std::uintptr_t edx, int a0, int a1, float a2, int a3
    ) { 
        if( ecx->networkable( )->index( ) < 1
           || ecx->networkable( )->index( ) > 64 )
            return orig_accumulate_layers( ecx, edx, a0, a1, a2, a3 );

        if( const auto anim_state = ecx->anim_state( ) ) { 
            const auto backup_first_update = anim_state->m_first_update;

            anim_state->m_first_update = true;

            orig_accumulate_layers( ecx, edx, a0, a1, a2, a3 );

            anim_state->m_first_update = backup_first_update;

            return;
        }

        orig_accumulate_layers( ecx, edx, a0, a1, a2, a3 );
    }

    int __fastcall list_leaves_in_box( 
        const std::uintptr_t ecx, const std::uintptr_t edx,
        const sdk::vec3_t& mins, const sdk::vec3_t& maxs, const uint16_t* const list, const int max
    ) { 

        if( !g_local_player->self( ) )
            return orig_list_leaves_in_box( ecx, edx, mins, maxs, list, max );

        if( * ( uint32_t* ) __builtin_return_address( 0 ) != 0x8B087D8B )
            return orig_list_leaves_in_box( ecx, edx, mins, maxs, list, max );

        struct renderable_info_t { 
            game::i_client_renderable* m_renderable { };
            std::uintptr_t	m_alpha_property { };
            int				m_enum_count { };
            int				m_render_frame { };
            std::uint16_t	m_first_shadow { };
            std::uint16_t	m_leaf_list { };
            short			m_area { };
            std::uint16_t	m_flags0 { };
            std::uint16_t	m_flags1 { };
            sdk::vec3_t			m_bloated_abs_min { };
            sdk::vec3_t			m_bloated_abs_max { };
            sdk::vec3_t			m_abs_min { };
            sdk::vec3_t			m_abs_max { };
            char			pad0 [ 4u ] { };
        };

        const auto info = *reinterpret_cast< renderable_info_t** > ( 
            reinterpret_cast< std::uintptr_t > ( _AddressOfReturnAddress( ) ) + 0x14u
            );
        if( !info
            || !info->m_renderable )
            return orig_list_leaves_in_box( ecx, edx, mins, maxs, list, max );

        const auto entity = info->m_renderable->get_client_unknown( )->get_base_entity( );
        if( !entity
            || !entity->is_player( ) )
            return orig_list_leaves_in_box( ecx, edx, mins, maxs, list, max );

        info->m_flags0 &= ~0x100;
        info->m_flags1 |= 0xC0;

        return orig_list_leaves_in_box( 
            ecx, edx,
            { -16384.f, -16384.f, -16384.f },
            { 16384.f, 16384.f, 16384.f },
            list, max
        );
    }

    void __fastcall build_transformations( 
        game::cs_player_t* ecx, void* edx, game::studio_hdr_t* hdr, sdk::vec3_t* pos, sdk::vec4_t* q, sdk::mat3x4_t* cam_transform, int bone_mask, byte* computed
    ) { 
        if( !ecx || ecx->networkable( )->index( ) < 1 ||
            ecx->networkable( )->index( ) > 64 )
            return orig_build_transformations( ecx, edx, hdr, pos, q, cam_transform, bone_mask, computed );

        const auto effects = ecx->effects( );

        ecx->effects( ) |= 8;
        ecx->is_jiggle_bones_enabled( ) = false;
        orig_build_transformations( ecx, edx, hdr, pos, q, cam_transform, bone_mask, computed );
        ecx->effects( ) = effects;
    }

    void __fastcall do_extra_bones_processing( 
        game::cs_player_t* const ecx, const std::uintptr_t edx, int a0, int a1, int a2, int a3, int a4, int a5
    ) { 
        return;
    }

    bool __fastcall svc_msg_voice_data( std::uintptr_t ecx, std::uintptr_t edx, void* msg ) { 
        if( !msg )
            return o_svc_msg_voice_data( ecx, edx, msg );

        auto msg_ptr = ( game::csvc_msg_data_legacy_t* ) msg;

        if( msg_ptr->m_client + 1 == game::g_engine->get_local_player( ) ) { 
            return o_svc_msg_voice_data( ecx, edx, msg );
        }

        cheat_data_t* ptr = ( cheat_data_t* ) &msg_ptr->m_xuid_low;

        if( ptr->m_unique_key == crypt_int( 228 ) ) { 
            const auto player = static_cast < game::cs_player_t* > ( game::g_entity_list->get_entity( static_cast < int > ( ptr->m_player_idx ) ) );
            const auto sender = static_cast < game::cs_player_t* > ( game::g_entity_list->get_entity( static_cast < int > ( msg_ptr->m_client + 1 ) ) );

            if( sender ) { 
                if( sender->team( ) == g_local_player->self( )->team( ) )
                    return o_svc_msg_voice_data( ecx, edx, msg );
            }

            if( player ) { 
                if( player->alive( ) ) { 
                    if( player->networkable( )->dormant( ) ) { 
                        hacks::g_visuals->m_dormant_data.at( player->networkable( )->index( ) ).m_origin = sdk::vec3_t( ptr->m_x, ptr->m_y, ptr->m_z );
                        hacks::g_visuals->m_dormant_data.at( player->networkable( )->index( ) ).m_use_shared = true;
                        player->origin( ) = sdk::vec3_t( ptr->m_x, ptr->m_y, ptr->m_z );
                        player->set_abs_origin( sdk::vec3_t( ptr->m_x, ptr->m_y, ptr->m_z ) );
                        hacks::g_visuals->m_dormant_data.at( player->networkable( )->index( ) ).m_last_shared_time = game::g_global_vars.get( )->m_cur_time;
                    }
                }
            }
        }

        return o_svc_msg_voice_data( ecx, edx, msg );
    }

    void __fastcall packet_start( 
        const std::uintptr_t ecx, const std::uintptr_t edx, const int in_seq, const int out_acked
    ) { 
        if( !game::g_engine->in_game( )
            || !g_local_player->self( )
            || !g_local_player->self( )->alive( ) )
            return orig_packet_start( ecx, edx, in_seq, out_acked );

        if( !g_ctx->get_out_cmds( ).empty( ) ) { 
            for( auto it = g_ctx->get_out_cmds( ).rbegin( ); it != g_ctx->get_out_cmds( ).rend( ); ++it ) { 
                if( !it->m_is_outgoing ) { 
                    continue;
                }

                if( it->m_command_nr == out_acked
                    || ( out_acked > it->m_command_nr && ( !it->m_is_used || it->m_prev_command_nr == out_acked ) ) ) { 

                    it->m_prev_command_nr = out_acked;
                    it->m_is_used = true;
                    orig_packet_start( ecx, edx, in_seq, it->m_command_nr );

                    break;
                }
            }

            auto result = false;

            for( auto it = g_ctx->get_out_cmds( ).begin( ); it != g_ctx->get_out_cmds( ).end( ); ) { 
                if( out_acked == it->m_command_nr || out_acked == it->m_prev_command_nr )
                    result = true;

                if( out_acked > it->m_command_nr && out_acked > it->m_prev_command_nr ) { 
                    it = g_ctx->get_out_cmds( ).erase( it );
                }
                else { 
                    it++;
                }
            }

            if( !result )
                orig_packet_start( ecx, edx, in_seq, out_acked );
        }
        else { 
            orig_packet_start( ecx, edx, in_seq, out_acked );
        }

    }

    void __fastcall packet_end( const std::uintptr_t ecx, const std::uintptr_t edx ) {
        if ( !g_local_player->self ( )
            || game::g_client_state.get( )->m_server_tick != game::g_client_state.get( )->m_delta_tick )
            return orig_packet_end( ecx, edx );

        const auto& local_data = hacks::g_eng_pred->local_data( ).at( game::g_client_state.get( )->m_cmd_ack % 150 );
        if( local_data.m_spawn_time == g_local_player->self( )->spawn_time( )
            && local_data.m_fake_amount > 0
            && local_data.m_tick_base > g_local_player->self( )->tick_base( )
            && ( local_data.m_tick_base - g_local_player->self( )->tick_base( ) ) <= 17 ) {
            g_local_player->self( )->tick_base( ) = local_data.m_tick_base + 1;
        }

        orig_packet_end( ecx, edx );
    }

    void __fastcall calc_view( void* ecx, const std::uintptr_t edx, sdk::vec3_t& eye_origin, const sdk::qang_t& eye_ang, float& z_near, float& z_far, float& fov ) { 
        if( !g_local_player->self( )
            || ecx != g_local_player->self( ) )
            return orig_calc_view( ecx, edx, eye_origin, eye_ang, z_near, z_far, fov );

        sdk::qang_t aim_punch_ang = g_local_player->self( )->aim_punch( );
        sdk::qang_t view_punch_ang = g_local_player->self( )->view_punch( );
        const auto backup_use_new_anim_state = g_local_player->self( )->use_new_anim_state( );

        g_local_player->self( )->use_new_anim_state( ) = false;

        if( hacks::g_visuals->cfg( ).m_removals & 128 )
            g_local_player->self( )->aim_punch( ) = sdk::qang_t( );

        if( hacks::g_visuals->cfg( ).m_removals & 64 )
            g_local_player->self( )->view_punch( ) = sdk::qang_t( );

        orig_calc_view( ecx, edx, eye_origin, eye_ang, z_near, z_far, fov );

        g_local_player->self( )->use_new_anim_state( ) = backup_use_new_anim_state;

        if( hacks::g_visuals->cfg( ).m_removals & 128 )
            g_local_player->self( )->aim_punch( ) = aim_punch_ang;

        if( hacks::g_visuals->cfg( ).m_removals & 64 )
            g_local_player->self( )->view_punch( ) = view_punch_ang;
    }

    void __stdcall create_move( int seq_number, float input_sample_frame_time, bool active, bool& send_packet ) { 
        thread_local bool pr_set = false;

        if( !pr_set )
        {   
            SetThreadPriorityBoost( GetCurrentThread( ), FALSE );
            SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_HIGHEST );
            pr_set = true;
        }

        o_create_move( game::g_client, seq_number, input_sample_frame_time, active );

        if( !game::g_engine->in_game( )
            || !g_local_player->self( )
            || !g_local_player->self( )->alive( ) )
            return;

        const auto slot = seq_number % game::k_mp_backup;

        g_local_player->create_move( send_packet,
            game::g_input->m_cmds[ slot ], game::g_input->m_vfyd_cmds[ slot ]
        );
    }

    void __fastcall exposure_range( float* mins, float* maxs ) { 
        if( mins )
            *mins = 1.f;

        if( maxs )
            *maxs = 1.f;

        orig_exposure_range( mins, maxs );
    }

    void __cdecl cl_move( float samples, bool final_tick ) { 
        if( !game::g_engine->in_game( ) 
            || !g_local_player->self( ) 
            || !g_local_player->self( )->alive( ) )
        { 
            orig_cl_move( samples, final_tick );
            return;
        }

       // game::user_cmd_t cmd{ };
       // bool who_tf{ };

        /*if( hacks::g_exploits->try_to_recharge( who_tf, cmd ) ) { 
            auto& out = g_ctx->get_out_cmds( ).emplace_back( );
            out.m_is_outgoing = false;
            out.m_command_nr = game::g_client_state.get( )->m_last_cmd_out;
            out.m_is_used = false;
            out.m_prev_command_nr = 0;
            return;
        }*/

        g_ctx->ticks_data( ).m_cl_tick_count = game::g_global_vars.get( )->m_tick_count;

        game::global_vars_base_t m_backup_record = *game::g_global_vars.get( );
        auto cl_tick_count = game::g_client_state.get( )->m_client_tick;
        auto server_tick = game::g_client_state.get( )->m_server_tick;

        g_ctx->addresses( ).m_cl_read_packets( final_tick );

        game::g_client_state.get( )->m_client_tick = cl_tick_count;
        game::g_client_state.get( )->m_server_tick = server_tick;
        *game::g_global_vars.get( ) = m_backup_record;

        if( hacks::g_exploits->m_cur_shift_amount )
            final_tick = true;

        orig_cl_move( samples, final_tick );
    }

    bool __fastcall write_user_cmd_delta_to_buffer( 
        const std::uintptr_t ecx, const std::uintptr_t edx,
        const int slot, game::bf_write_t* const buffer,
        int from, int to, const bool is_new_cmd
    ) { 
        if( !g_local_player->self( ) )
            return orig_write_user_cmd_delta_to_buffer( ecx, edx, slot, buffer, from, to, is_new_cmd );

        const auto move_msg = reinterpret_cast< game::move_msg_t* > ( 
            *reinterpret_cast< std::uintptr_t* > ( 
                reinterpret_cast< std::uintptr_t > ( _AddressOfReturnAddress( ) ) - sizeof( std::uintptr_t )
                ) - 0x58u
            );

        if( hacks::g_exploits->m_cur_shift_amount
            || game::g_client_state.get( )->m_last_cmd_out == hacks::g_exploits->m_recharge_cmd 
            || hacks::g_exploits->m_type == hacks::c_exploits::exploits_type_t::type_ready ) { 
            if( from == -1 ) { 
                if( game::g_client_state.get( )->m_last_cmd_out == hacks::g_exploits->m_recharge_cmd ) { 
                    move_msg->m_new_cmds = 1;
                    move_msg->m_backup_cmds = 0;

                    const auto next_cmd_number = game::g_client_state.get( )->m_choked_cmds + game::g_client_state.get( )->m_last_cmd_out + 1;

                    for( to = next_cmd_number - move_msg->m_new_cmds + 1; to <= next_cmd_number; ++to ) { 
                        if( !orig_write_user_cmd_delta_to_buffer( ecx, edx, slot, buffer, from, to, true ) )
                            break;

                        from = to;
                    }
                }
                else if( hacks::g_exploits->m_type == hacks::c_exploits::exploits_type_t::type_defensive )
                { 
                    hacks::g_exploits->handle_break_lc( ecx, edx, slot, buffer, from, to, move_msg );
                    return true;
                }
                else
                    hacks::g_exploits->process_real_cmds( ecx, edx, slot, buffer, from, to, move_msg );
            }

            return true;
        }

        if( from == -1 ) { 
            const auto m_new_cmds = std::min( move_msg->m_new_cmds + hacks::g_exploits->m_ticks_allowed, 16 );

            int m_ticks_allowed { };

            const auto m_new_allowed = m_new_cmds - move_msg->m_new_cmds;
            if( m_new_allowed >= 0 )
                m_ticks_allowed = m_new_allowed;

            hacks::g_exploits->m_ticks_allowed = m_ticks_allowed;
        }

        return orig_write_user_cmd_delta_to_buffer( ecx, edx, slot, buffer, from, to, is_new_cmd );
    }

    void __fastcall paint_traverse( const std::uintptr_t ecx, const std::uintptr_t edx, const std::uint32_t id, bool force_repaint, bool allow_force )
    { 
        std::string panel_name = game::g_panel->get_name( id );
        if( strstr( game::g_panel->get_name( id ), xor_str( "HudZoom" ) ) )
            if( hacks::g_visuals->cfg( ).m_removals & 1 )
                return;

        o_paint_traverse( ecx, edx, id, force_repaint, allow_force );

        static auto draw_panel_id = 0u;

        if( !draw_panel_id ) { 
            if( panel_name.compare( xor_str( "MatSystemTopPanel" ) ) )
                return;

            draw_panel_id = id;
        }

        game::g_prediction->set_local_view_angles( g_ctx->anim_data( ).m_local_data.m_anim_ang );

        if( id != draw_panel_id )
            return;

        hacks::g_misc->kill_feed( );

        g_render->m_draw_list->Clear( );
        g_render->m_draw_list->PushClipRectFullScreen( );

        g_render->m_screen_size = *reinterpret_cast< sdk::vec2_t* > ( &gui::GetIO( ).DisplaySize );

        hacks::g_visuals->handle_player_drawings( );

        hacks::g_visuals->draw_hitmarkers( );

        hacks::g_logs->draw_data( );

        hacks::g_visuals->handle_world_drawings( );

        static auto last_server_tick = game::g_client_state.get( )->m_server_tick;
        if( game::g_client_state.get( )->m_server_tick != last_server_tick ) { 
            hacks::g_visuals->m_throwed_grenades.clear( );

            last_server_tick = game::g_client_state.get( )->m_server_tick;
        }

        for( int i{ }; i <= game::g_entity_list->highest_ent_index( ); ++i ) { 

            const auto entity = game::g_entity_list->get_entity( i );
            if( !entity )
                continue;

            if( const auto client_class = entity->networkable( )->client_class( ); client_class != nullptr
                && !( entity->is_player( )
                && entity->is_weapon( ) ) ) { 
                if( entity->networkable( )->dormant( ) )
                    continue;

                hacks::g_visuals->handle_warning_pred( entity, static_cast < game::e_class_id > ( client_class->m_class_id ) );
            }
        }

        hacks::g_visuals->add_grenade_simulation( hacks::g_visuals->m_grenade_trajectory, false );

        hacks::g_visuals->draw_scope_lines( );

        hacks::g_visuals->draw_auto_peek( );

        hacks::g_visuals->manuals_indicators( );
        hacks::g_visuals->draw_key_binds( );
        hacks::g_misc->draw_spectators( );
        { 
            const auto lock = std::unique_lock<std::mutex> ( g_render->m_mutex );

            *g_render->m_data_draw_list = *g_render->m_draw_list;
        }
    }

    void __fastcall run_cmd( std::uintptr_t ecx, std::uintptr_t edx, game::cs_player_t* player, game::user_cmd_t* user_cmd, game::c_move_helper* move_helper )
    { 
        if( !player ||
            !g_local_player || !g_local_player->self( ) ||
            player != g_local_player->self( ) ) { 

            orig_run_cmd( ecx, edx, player, user_cmd, move_helper );
            return;
        }

        if( user_cmd->m_tick > ( g_ctx->ticks_data( ).m_cl_tick_count + 8 ) ) { 
            user_cmd->m_predicted = true;
            ++g_local_player->self( )->tick_base( );
            return;
        }

        orig_run_cmd( ecx, edx, player, user_cmd, move_helper );

        hacks::g_eng_pred->save_view_model( ); // also can be done in start of physics_sim since physics_sim calling run_cmd but however 
    }

    void __stdcall draw_mdl_exec( 
        uintptr_t ctx, const game::draw_model_state_t& state, const game::model_render_info_t& info, sdk::mat3x4_t* bones
    ) { 
        static game::c_material* xblur_mat = game::g_mat_sys->find_mat( "dev/blurfilterx_nohdr", "Other textures", true );
        static game::c_material* yblur_mat = game::g_mat_sys->find_mat( "dev/blurfiltery_nohdr", "Other textures", true );
        static game::c_material* scope = game::g_mat_sys->find_mat( "dev/scope_bluroverlay", "Other textures", true );

        if( xblur_mat )
            xblur_mat->set_flag( 1 << 2, true );

        if( yblur_mat )
            yblur_mat->set_flag( 1 << 2, true );

        if( scope )
            scope->set_flag( 1 << 2, true );

        if( game::g_mdl_render->is_forced_mat_override( ) ) 
            return orig_draw_mdl_exec( game::g_mdl_render, ctx, state, info, bones );

        if( game::g_engine->in_game( ) ) { 
            if( strstr( info.m_model->m_path, xor_str( "player/contactshadow" ) ) != nullptr ) { 
                return;
            }

            bool ret = hacks::g_chams->draw_mdl( game::g_mdl_render, ctx, state, info, bones );
            game::g_render_view->set_blend( 1.f );
     
            if( ret )
                return;
        }

        orig_draw_mdl_exec( game::g_mdl_render, ctx, state, info, bones );
        game::g_studio_render->forced_mat_override( nullptr );
        game::g_render_view->set_blend( 1.f );
    }

    void __cdecl velocity_modifier( game::recv_proxy_data_t* const data, game::base_entity_t* const entity, void* const out )
    { 
        orig_velocity_modifier( data, entity, out );

        if( !g_local_player || !g_local_player->self( )
            || entity->networkable( )-> index( ) != g_local_player->self( )->networkable( )->index( )
            || data->m_value.m_float == hacks::g_eng_pred->net_velocity_modifier( ) )
            return;

        hacks::g_eng_pred->net_velocity_modifier( ) = data->m_value.m_float;

        game::g_prediction->m_prev_ack_had_errors = true;
    }

    void __fastcall process_movement( std::uintptr_t ecx, std::uintptr_t edx, game::cs_player_t* player, game::move_data_t* m_moving_data )
    { 
        m_moving_data->m_game_code_moved_player = false;

        return orig_process_movement( ecx, edx, player, m_moving_data );
    }

    int process_interpolated_list( ) { 
        static auto allow_to_extrp = * ( bool** )( g_ctx->addresses( ).m_allow_to_extrapolate + 0x1 );

        if( allow_to_extrp )
            *allow_to_extrp = false;

        return orig_process_interp_list( );
    }

    struct incoming_seq_t { 
        std::ptrdiff_t m_in_seq { };
        std::ptrdiff_t m_reliable_state { };
    };

    std::vector < incoming_seq_t > incoming_seq { };
   
    int __fastcall net_showfragments( const std::uintptr_t ecx, const std::uintptr_t edx ) { 
       return orig_net_showfragments( ecx, edx );
    }

    int __fastcall send_datagram( game::client_state_t::net_chan_t* net_chan, const std::uintptr_t edx, void* buff ) { 
        if( !game::g_engine->in_game( )
            || !g_local_player->self( )
            || !g_local_player->self( )->alive( )
            || net_chan != game::g_client_state.get( )->m_net_chan 
            || !g_key_binds->get_keybind_state( &hacks::g_ping_spike->cfg( ).m_ping_spike_key ) )
            return orig_send_datagram( net_chan, edx, buff );

        const auto backup_in_seq = net_chan->m_in_seq;
        const auto backup_in_rel_state = net_chan->m_in_rel_state;

        auto flow_outgoing = game::g_engine->net_channel_info( )->latency( game::e_net_flow::out );
        auto target_ping = ( hacks::g_ping_spike->cfg( ).m_to_spike / 1000.f );

        if( flow_outgoing < target_ping ) { 
            auto target_in_seq = net_chan->m_in_seq - game::to_ticks( target_ping - flow_outgoing );
            net_chan->m_in_seq = target_in_seq;

            for( auto& seq : incoming_seq ) { 
                if( seq.m_in_seq != target_in_seq )
                    continue;

                net_chan->m_in_rel_state = seq.m_reliable_state;
            }
        }

        int original = orig_send_datagram( net_chan, edx, buff );

        net_chan->m_in_seq = backup_in_seq;
        net_chan->m_in_rel_state = backup_in_rel_state;
        return original;
    }

    void __fastcall process_packet( game::client_state_t::net_chan_t* net_chan, const std::uintptr_t edx, void* packet, bool header ) { 
        orig_process_packet( net_chan, edx, packet, header );

        for( game::event_info_t* it { game::g_client_state.get( )->m_events( ) }; it != nullptr; it = it->m_next ) { 
            if( !it->m_class_id )
                continue;

            it->m_fire_delay = 0.f;
        }

        game::g_engine->fire_events( );

        if( net_chan == game::g_client_state.get( )->m_net_chan ) { 
            if( g_local_player->self( ) && g_local_player->self( )->alive( ) ) { 
                incoming_seq.push_back( incoming_seq_t{ net_chan->m_in_seq, net_chan->m_in_rel_state } );

                /* who */
                for( auto it = incoming_seq.begin( ); it != incoming_seq.end( ); ++it ) { 
                    auto delta = abs( net_chan->m_in_seq - it->m_in_seq );
                    if( delta > crypt_float( 1024.f ) ) { // ok look its retarded but that will fix alot of issues on high ping spike
                        it = incoming_seq.erase( it );
                    }
                }
            }
            else
                incoming_seq.clear( );
        }
    }

    bool __fastcall process_temp_entities( const std::uintptr_t ecx, const std::uintptr_t edx, const std::uintptr_t msg ) { 
        const auto backup_max_clients = game::g_client_state.get( )->m_max_clients;

        game::g_client_state.get( )->m_max_clients = 1;

        const auto ret = orig_process_temp_entities( ecx, edx, msg );

        game::g_client_state.get( )->m_max_clients = backup_max_clients;

        game::g_engine->fire_events( );

        return ret;
    }

    bool __fastcall is_paused( const std::uintptr_t ecx, const std::uintptr_t edx ) { 
        if( *reinterpret_cast< std::uintptr_t* > ( _AddressOfReturnAddress( ) ) == g_ctx->addresses( ).m_ret_to_extrapolation )
            return true;

        return orig_is_paused( ecx, edx );
    }

    bool __fastcall is_hltv( const std::uintptr_t ecx, const std::uintptr_t edx ) { 

        if( g_ctx->anim_data( ).m_allow_setup_bones )
            return true;

        if( g_ctx->anim_data( ).m_allow_update )
            return true;

        if( *reinterpret_cast< std::uintptr_t* > ( _AddressOfReturnAddress( ) ) == g_ctx->addresses( ).m_ret_to_accumulate_layers ||
            *reinterpret_cast< std::uintptr_t* > ( _AddressOfReturnAddress( ) ) == g_ctx->addresses( ).m_ret_to_setup_velocity )
            return true;

        return orig_is_hltv( ecx, edx );
    }

    void __cdecl lower_body_yaw_proxy( game::recv_proxy_data_t* data, void* entity, void* output ) { 
        const auto player = reinterpret_cast< game::cs_player_t* > ( entity );
        hacks::g_resolver->parse_lby_proxy( player, &data->m_value.m_float );
        return orig_lby_proxy( data, entity, output );
    }

    void __fastcall physics_simulate( game::cs_player_t* const ecx, const std::uintptr_t edx ) { 
        if( ecx != g_local_player->self( )
            || !ecx->alive( )
            || ecx->sim_tick( ) == game::g_global_vars.get( )->m_tick_count )
            return orig_physics_simulate( ecx, edx );

        const auto& user_cmd = ecx->cmd_context( ).m_user_cmd;

        if( user_cmd.m_tick > game::g_global_vars.get( )->m_tick_count + 8 ) { 
            ecx->sim_tick( ) = game::g_global_vars.get( )->m_tick_count;
            ecx->cmd_context( ).m_user_cmd.m_predicted = true;
            ++ecx->tick_base( );
            return hacks::g_eng_pred->net_vars( ).at( user_cmd.m_number % 150 ).store( user_cmd.m_number );
        }

        if( user_cmd.m_number == ( game::g_client_state.get( )->m_cmd_ack + 1 ) ) { 
            ecx->velocity_modifier( ) = hacks::g_eng_pred->net_velocity_modifier( );
        }

        bool in_attack = user_cmd.m_buttons & ( game::e_buttons::in_attack | game::e_buttons::in_attack2 );

        hacks::g_eng_pred->net_vars( ).at( user_cmd.m_number % 150 ).m_r8 = { ecx->tick_base( ), in_attack,
                                                                                hacks::g_aim_bot->can_shoot( true, 0, true ) };

        const auto weapon = ecx->weapon( );

        if( weapon ) { 
            weapon->postpone_fire_ready_time( ) = std::numeric_limits< float >::max( );

            if( weapon->item_index( ) == game::e_item_index::revolver ) { 
                const auto max_cmds = ( int( 1.0f / game::g_global_vars.get( )->m_interval_per_tick ) ) / 2;
                if( max_cmds > 1 ) { 
                    auto v27 = 0;

                    auto v15 = user_cmd.m_number - 1;

                    for( auto i = 1u; i < max_cmds; ++i ) { 
                        v27 = v15;

                        const auto& r8_data = hacks::g_eng_pred->net_vars( ).at( v15 % 150 ).m_r8;

                        if( !r8_data.m_in_attack
                            || !r8_data.m_can_shoot )
                            break;

                        --v15;
                    }

                    if( v27 ) { 
                        const auto v17 = 1 + static_cast< int > ( 0.03348f / game::g_global_vars.get( )->m_interval_per_tick );
                        if( user_cmd.m_number - v27 >= v17 )
                        weapon->postpone_fire_ready_time( ) = game::to_time( hacks::g_eng_pred->net_vars( ).at( ( v27 + v17 ) % 150 ).m_r8.m_tick ) + 0.2f;
                    }
                }
            }
        }

        const auto backup_tick_base = ecx->tick_base( );

        const auto& local_data = hacks::g_eng_pred->local_data( ).at( user_cmd.m_number % 150 );
        if( local_data.m_spawn_time == ecx->spawn_time( ) && local_data.m_override_tick_base )
            ecx->tick_base( ) = local_data.m_adjusted_tick_base;

        game::g_global_vars.get( )->m_cur_time = game::to_time( ecx->tick_base( ) );

        if( user_cmd.m_tick < g_ctx->ticks_data( ).m_cl_tick_count + g_ctx->ticks_data( ).m_tick_rate + 8 )
            hacks::g_eng_pred->net_vars( ).at( ( user_cmd.m_number - 1 ) % 150 ).restore( user_cmd.m_number - 1 );

        orig_physics_simulate( ecx, edx );

        ecx->phys_collision_state( ) = 0;

        if( local_data.m_spawn_time == ecx->spawn_time( )
            && local_data.m_override_tick_base && local_data.m_restore_tick_base ) {
            ecx->tick_base( ) = backup_tick_base + ecx->tick_base( ) - local_data.m_adjusted_tick_base;
        }

        hacks::g_eng_pred->net_vars( ).at( user_cmd.m_number % 150 ).store( user_cmd.m_number );
    }

    bool __fastcall should_draw_view_model( std::uintptr_t ecx, std::uintptr_t edx ) { 
        if( !game::g_engine->in_game( )
            || !g_local_player->self( )->alive( ) )
            return orig_should_draw_view_model( ecx, edx );

        if( g_local_player->self( )->scoped( ) 
            && hacks::g_visuals->cfg( ).m_show_weapon_in_scope )
            return true;

        return orig_should_draw_view_model( ecx, edx );
    }

    float __fastcall get_view_model_fov( )
    {
        return hacks::g_visuals->cfg( ).m_view_model_fov;
    }

    void __stdcall frame_stage_notify( const game::e_frame_stage stage )
    { 
        hacks::g_eng_pred->last_frame_stage( ) = stage;

        if( !g_local_player || !g_local_player->self( ) )
            return orig_frame_stage_notify( stage );

        const auto in_game = game::g_engine->in_game( );

        if( stage == game::e_frame_stage::render_start ) { 
            if( in_game ) { 
                hacks::g_exploits->skip_lag_interpolation( false );
                hacks::g_misc->clan_tag( );
                hacks::g_shot_construct->on_render_start( );
                hacks::g_visuals->removals( );
                hacks::g_visuals->skybox_changer( );

                auto misc_cfg = hacks::g_misc->cfg( );

                if( g_local_player->self( ) ) {
                    game::g_cvar->find_var( xor_str( "weapon_debug_spread_show" ) )->set_int( g_local_player->self( )->scoped( ) ? 0 :
                        g_local_player->self( )->alive( ) 
                        ? misc_cfg.m_force_crosshair ? 3 : 0 : 0 );
                }

	            game::g_cvar->find_var( xor_str( "con_filter_text" ) )->set_str( xor_str( "[secret_hack24]" ) );
	               
                auto con_filter_enable = game::g_cvar->find_var( xor_str( "con_filter_enable" ) );

                if( misc_cfg.m_filter_console != con_filter_enable->get_int( ) ) {
		            game::g_cvar->find_var( xor_str( "con_filter_enable" ) )->set_int( misc_cfg.m_filter_console ? 1 : 0 );

                    game::g_engine->exec_cmd( xor_str( "clear" ) );
                }

                auto visual_cfg = hacks::g_visuals->cfg( );

                static auto enable_fog = game::g_cvar->find_var( xor_str( "fog_enable" ) );
                static auto override_fog = game::g_cvar->find_var( xor_str( "fog_override" ) );
                static auto fog_clr = game::g_cvar->find_var( xor_str( "fog_color" ) );
                static auto fog_start = game::g_cvar->find_var( xor_str( "fog_start" ) );
                static auto fog_end = game::g_cvar->find_var( xor_str( "fog_end" ) );
                static auto fog_density = game::g_cvar->find_var( xor_str( "fog_maxdensity" ) );

                static float fog_end_final{ }, fog_start_final{ }, fog_density_final{ };

                if( g_ctx->cvars( ).viewmodel_offset_x->get_int( ) != misc_cfg.m_view_model_x ) {
                    g_ctx->cvars( ).viewmodel_offset_x->set_int( misc_cfg.m_view_model_x );
                }

	            if( g_ctx->cvars( ).viewmodel_offset_y->get_int( ) != misc_cfg.m_view_model_y ) {
                    g_ctx->cvars( ).viewmodel_offset_y->set_int( misc_cfg.m_view_model_y );
                }

	            if( g_ctx->cvars( ).viewmodel_offset_z->get_int( ) != misc_cfg.m_view_model_z ) {
                    g_ctx->cvars( ).viewmodel_offset_z->set_int( misc_cfg.m_view_model_z );
                }

                if( game::g_engine->in_game( ) ) {
                    if( visual_cfg.m_fog ) {
                        fog_end_final = std::lerp( fog_end_final, visual_cfg.m_fog_end, 4.5f * game::g_global_vars.get( )->m_frame_time );
                        fog_start_final = std::lerp( fog_start_final, visual_cfg.m_fog_start, 4.5f * game::g_global_vars.get( )->m_frame_time );
                        fog_density_final = std::lerp( fog_density_final, visual_cfg.m_fog_clr[ 3 ] * 100.f, 4.5f * game::g_global_vars.get( )->m_frame_time );
                    }
                    else {
                        fog_end_final = std::lerp( fog_end_final, 1500.f, 4.5f * game::g_global_vars.get( )->m_frame_time );
                        fog_start_final = std::lerp( fog_start_final, 1200.f, 4.5f * game::g_global_vars.get( )->m_frame_time );
                        fog_density_final = std::lerp( fog_density_final, 0.f, 1.25f * game::g_global_vars.get( )->m_frame_time );
                    }

                    enable_fog->set_int( fog_density_final > 10.f && fog_start_final < 1180.f );
                    override_fog->set_int( fog_density_final > 10.f && fog_start_final < 1200.f );

                    fog_clr->set_str( std::string( std::to_string( visual_cfg.m_fog_clr[ 0 ] * 255.f ) + " " +
                        std::to_string( visual_cfg.m_fog_clr[ 1 ] * 255.f ) + " " 
                        + std::to_string( visual_cfg.m_fog_clr[ 2 ] * 255.f ) ).c_str( ) );

                    fog_start->set_int( fog_start_final );
                    fog_end->set_int( fog_end_final );
                    fog_density->set_float( fog_density_final / 100.f );
                }

                hacks::g_visuals->draw_beam( );

                static int last_impacts_count{ };

                /* FF 71 0C F3 0F 11 84 24 ? ? ? ? F3 0F 10 84 24 ? ? ? ? */
                const auto& client_impacts_list = *reinterpret_cast< game::utl_vec_t< client_hit_verify_t >* > ( 
                    reinterpret_cast< std::uintptr_t > ( g_local_player->self( ) ) + 0xba84u
                    );
                
                if( visual_cfg.m_bullet_impacts ) { 
                    auto clr_client = visual_cfg.m_bullet_impacts_client_clr;
                    sdk::col_t client_impact_clr = sdk::col_t( clr_client[ 0 ] * 255.f, clr_client[ 1 ] * 255.f, clr_client[ 2 ] * 255.f, clr_client[ 3 ] * 255.f );

                    for( auto i = client_impacts_list.m_size; i > last_impacts_count; --i ) { 
                         game::g_debug_overlay->add_box_overlay( client_impacts_list.at( i - 1 ).m_pos, { -1.25f, -1.25f, -1.25f }, { 1.25f, 1.25f, 1.25f },
                             { }, client_impact_clr.r( ), client_impact_clr.g( ), client_impact_clr.b( ), client_impact_clr.a( ), 4.f );
                    }

                    last_impacts_count = client_impacts_list.m_size;

                    auto clr_server = visual_cfg.m_bullet_impacts_server_clr;
                    sdk::col_t server_impact_clr = sdk::col_t( clr_server[ 0 ] * 255.f, clr_server[ 1 ] * 255.f, clr_server[ 2 ] * 255.f, clr_server[ 3 ] * 255.f );

                    for( auto i = hacks::g_visuals->m_bullet_impacts.begin( ); i != hacks::g_visuals->m_bullet_impacts.end( ); i = hacks::g_visuals->m_bullet_impacts.erase( i ) ) { 
                        game::g_debug_overlay->add_box_overlay( i->m_pos, { -1.25f, -1.25f, -1.25f }, { 1.25f, 1.25f, 1.25f }, { },
                            server_impact_clr.r( ), server_impact_clr.g( ), server_impact_clr.b( ), server_impact_clr.a( ), 4.f );
                    }
                }
                else if( !visual_cfg.m_bullet_impacts ) { 
                    hacks::g_visuals->m_bullet_impacts.clear( );
                }

                for( std::size_t i{ 1 }; i <= game::g_global_vars.get( )->m_max_clients; ++i ) { 
                    const auto player = static_cast < game::cs_player_t* > ( game::g_entity_list->get_entity( i ) );

                    if( !player ||
                        !player->alive( )
                        || player->networkable( )->dormant( ) )
                        continue;

                    hacks::g_visuals->m_shared.send_net_data( player );
                }
            }
        }

        if( stage == game::e_frame_stage::post_data_update_start ) { 
            hacks::g_skins->handle_ctx( );
        }

        if( g_local_player->self( ) ) { 
            for( size_t i{ 1 }; i <= game::g_global_vars.get( )->m_max_clients; ++i ) { 
                const auto player = static_cast <game::cs_player_t*> ( game::g_entity_list->get_entity( i ) );

                if( !player ||
                    !player->alive( )
                    || player->networkable( )->dormant( ) )
                    continue;

                bool interp_status = false;

                if ( player == g_local_player->self( ) && !hacks::g_exploits->m_in_charge )
                    interp_status = true;

                auto& var_mapping = player->var_mapping( );

                for( size_t j{ }; j < var_mapping.m_interpolated_entries; ++j )
                    var_mapping.m_entries.at( j ).m_needs_to_interpolate = interp_status;
            }
        }

        orig_frame_stage_notify( stage );

        if( stage == game::e_frame_stage::render_start )
            hacks::g_exploits->skip_lag_interpolation( true );

        if( stage == game::e_frame_stage::net_update_end ) { 
            if( in_game ) { 
                hacks::g_eng_pred->velocity_modifier_to_data_map( );

                game::g_engine->fire_events( );

                const int correction_ticks = hacks::g_exploits->clock_correction( );
                if( correction_ticks == -1 )
                    hacks::g_exploits->m_simulation_diff = 0;
                else { 
                    if( g_local_player->self( )->sim_time( ) > g_local_player->self( )->old_sim_time( ) ) { 
                        int sim_diff = game::to_ticks( g_local_player->self( )->sim_time( ) ) - game::g_client_state.get( )->m_server_tick;
                       
                        if( std::abs( sim_diff ) <= correction_ticks )
                            hacks::g_exploits->m_simulation_diff = sim_diff;
                    }
                }
                hacks::g_lag_comp->handle_net_update( );
            }
        }

        if( in_game ) { 
            if( stage == game::e_frame_stage::post_data_update_start ) { 
                hacks::g_eng_pred->adjust_view_model( );
            }
        }
    }

    float __stdcall aspect_ratio( int width, int height )
    { 
        if( !hacks::g_misc->cfg( ).m_aspect_ratio )
            return orig_aspect_ratio( width, height );
        else
            return hacks::g_misc->cfg( ).m_aspect_ratio_value;
    }

    void __fastcall override_view( std::uintptr_t ecx, std::uintptr_t edx, game::view_setup_t* const setup ) { 
        if( !( g_local_player->self( ) && g_local_player->self( )->alive( ) ) ) 
            return orig_override_view( ecx, edx, setup );

        if( game::g_engine->in_game( )
            && game::g_engine->get_local_player( ) ) { 
            setup->m_fov = hacks::g_misc->cfg( ).m_camera_distance;

            g_local_player->self( )->view_model( ).set_abs_ang( sdk::qang_t( setup->m_angles.x( ), setup->m_angles.y( ), hacks::g_misc->cfg( ).m_view_model_roll ) );

            if( !( hacks::g_visuals->cfg( ).m_removals & 2 )
                && g_local_player->self( )->weapon( )
                && g_local_player->self( )->scoped( ) ) { 
                int zoom_lvl = g_local_player->weapon( )->zoom_lvl( );
                setup->m_fov /= zoom_lvl;
            }

            hacks::g_misc->third_person( );
        }

        orig_override_view( ecx, edx, setup );
    }

    void event_listener_t::fire_game_event( game::game_event_t* const event ) { 
        hacks::g_shots->on_new_event( event );
    }

    void __fastcall get_color_modulation( void* ecx, void* edx, float* red, float* green, float* blue )
    { 
        auto material = ( game::c_material* )( ecx );

        if( !material || material->is_error_mat( ) )
            return orig_get_clr_modulation( ecx, edx, red, green, blue );

        orig_get_clr_modulation( ecx, edx, red, green, blue );

        if( strstr( material->get_tex_group_name( ), xor_str( "World textures" ) ) )
        { 
            sdk::col_t world_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_world_modulation [ 0 ] * 255.f, 
                hacks::g_visuals->cfg( ).m_world_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_world_modulation [ 2 ] * 255.f, 
                hacks::g_visuals->cfg( ).m_world_modulation [ 3 ] * 255.f );

            *red = world_clr.r( ) / 255.f;
            *green = world_clr.g( ) / 255.f;
            *blue = world_clr.b( ) / 255.f;
        }
        else if( strstr( material->get_tex_group_name( ), xor_str( "StaticProp textures" ) ) )
        { 
            sdk::col_t props_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_props_modulation [ 0 ] * 255.f,
                hacks::g_visuals->cfg( ).m_props_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_props_modulation [ 2 ] * 255.f,
                hacks::g_visuals->cfg( ).m_props_modulation [ 3 ] * 255.f );

            *red = props_clr.r( ) / 255.f;
            *green = props_clr.g( ) / 255.f;
            *blue = props_clr.b( ) / 255.f;
        }
        else if( strstr( material->get_tex_group_name( ), xor_str( "SkyBox textures" ) ) )
        { 
            sdk::col_t sky_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_sky_modulation [ 0 ] * 255.f,
                hacks::g_visuals->cfg( ).m_sky_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_sky_modulation [ 2 ] * 255.f,
                hacks::g_visuals->cfg( ).m_sky_modulation [ 3 ] * 255.f );

            *red = sky_clr.r( ) / 255.f;
            *green = sky_clr.g( ) / 255.f;
            *blue = sky_clr.b( ) / 255.f;
        }
    }

    float __fastcall get_alpha_modulation( void* ecx, void* edx )
    { 
        auto material = ( game::c_material* )( ecx );
        if( !material || material->is_error_mat( ) )
            return orig_get_alpha_modulation( ecx, edx );

        sdk::col_t world_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_world_modulation [ 0 ] * 255.f,
            hacks::g_visuals->cfg( ).m_world_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_world_modulation [ 2 ] * 255.f,
            hacks::g_visuals->cfg( ).m_world_modulation [ 3 ] * 255.f );

        sdk::col_t props_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_props_modulation [ 0 ] * 255.f,
            hacks::g_visuals->cfg( ).m_props_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_props_modulation [ 2 ] * 255.f,
            hacks::g_visuals->cfg( ).m_props_modulation [ 3 ] * 255.f );

        sdk::col_t sky_clr = sdk::col_t( hacks::g_visuals->cfg( ).m_sky_modulation [ 0 ] * 255.f,
            hacks::g_visuals->cfg( ).m_sky_modulation [ 1 ] * 255.f, hacks::g_visuals->cfg( ).m_sky_modulation [ 2 ] * 255.f,
            hacks::g_visuals->cfg( ).m_sky_modulation [ 3 ] * 255.f );

        if( strstr( material->get_tex_group_name( ), xor_str( "World textures" ) ) )
            return world_clr.a( ) / 255.f;
        else if( strstr( material->get_tex_group_name( ), xor_str( "StaticProp textures" ) ) )
            return props_clr.a( ) / 255.f;
        else if( strstr( material->get_tex_group_name( ), xor_str( "SkyBox textures" ) ) )
            return sky_clr.a( ) / 255.f;

        return orig_get_alpha_modulation( ecx, edx );
    }

    bool __fastcall should_skip_animation_frame( void* this_pointer, void* edx ) { 
 
        // the function is only called by SetupBones so there is no need to check for return address
        // returning false prevents copying of cached bone data
 
        return false;
    }
       
    void __fastcall check_for_sequence_change( void* this_pointer, void* edx, void* hdr, int cur_sequence, bool force_new_sequence, bool interpolate ) { 

        // no sequence interpolation over here mate
        // forces the animation queue to clear
 
        return orig_check_for_seq_change( this_pointer, edx, hdr, cur_sequence, force_new_sequence, false );
    }
}