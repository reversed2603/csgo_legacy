#pragma once

namespace csgo::hooks {
    LRESULT __stdcall wnd_proc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
    inline decltype( &wnd_proc ) o_wnd_proc{ };

    long D3DAPI dx9_reset( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* present_params );
    inline decltype( &dx9_reset ) o_dx9_reset{ };

    long D3DAPI dx9_present( IDirect3DDevice9* device,
        RECT* src_rect, RECT* dst_rect, HWND dst_wnd_override, RGNDATA* dirty_region
    );
    inline decltype( &dx9_present ) o_dx9_present{ };

    void __fastcall lock_cursor( const std::uintptr_t ecx, const std::uintptr_t edx );
    inline decltype( &lock_cursor ) o_lock_cursor{ };

    void __fastcall paint_traverse( const std::uintptr_t ecx, const std::uintptr_t edx,
        const std::uint32_t id, bool force_repaint, bool allow_force
    );
    inline decltype( &paint_traverse ) o_paint_traverse;

    void __fastcall run_cmd( std::uintptr_t ecx, std::uintptr_t edx, valve::cs_player_t* player, valve::user_cmd_t* user_cmd, valve::c_move_helper* move_helper );
    inline decltype( &run_cmd ) orig_run_cmd { };

    int __fastcall do_post_screen_space_effects( 
        const std::uintptr_t ecx, const std::uintptr_t edx, valve::view_setup_t* const setup
    );
    inline decltype( &do_post_screen_space_effects ) orig_do_post_screen_space_effects { };

    bool __fastcall svc_msg_voice_data( std::uintptr_t ecx, std::uintptr_t edx, void* msg );
    inline decltype( &svc_msg_voice_data ) o_svc_msg_voice_data{ };

    void __fastcall setup_move( const std::uintptr_t ecx, const std::uintptr_t edx, valve::cs_player_t* player, valve::user_cmd_t* user_cmd, valve::c_move_helper* move_helper, valve::move_data_t* move_data );
    inline decltype( &setup_move ) orig_setup_move{ };

    void __cdecl velocity_modifier( valve::recv_proxy_data_t* const data, valve::base_entity_t* const entity, void* const out );
    inline decltype( &velocity_modifier ) orig_velocity_modifier { };

    bool __fastcall write_user_cmd_delta_to_buffer( 
        const std::uintptr_t ecx, const std::uintptr_t edx,
        const int slot, valve::bf_write_t* const buffer, int from, int to, const bool is_new_cmd
    );
    inline decltype( &write_user_cmd_delta_to_buffer ) orig_write_user_cmd_delta_to_buffer { };

    bool __fastcall setup_bones( 
        const std::uintptr_t ecx, const std::uintptr_t edx, sdk::mat3x4_t* const bones, int max_bones, int mask, float time 
    );
    inline decltype( &setup_bones ) orig_setup_bones { };

    void __fastcall update_client_side_anim( valve::cs_player_t* const player, const std::uintptr_t edx );
    inline decltype( &update_client_side_anim ) orig_update_client_side_anim { };

    void __fastcall process_movement( std::uintptr_t ecx, std::uintptr_t edx, valve::cs_player_t* player, valve::move_data_t* move_data );
    inline decltype( &process_movement ) orig_process_movement { };

    void __fastcall interpolate_server_entities(  );
    inline decltype( &interpolate_server_entities ) orig_interpolate_server_entities{ };

    int process_interpolated_list( );
    inline decltype( &process_interpolated_list ) orig_process_interp_list { };

    void __fastcall get_color_modulation( void* ecx, void* edx, float* red, float* green, float* blue );
    inline decltype( &get_color_modulation ) orig_get_clr_modulation { };

    float __fastcall get_alpha_modulation( void* ecx, void* edx );
    inline decltype( &get_alpha_modulation ) orig_get_alpha_modulation { };

    void __fastcall do_extra_bones_processing( 
        valve::cs_player_t* const ecx, const std::uintptr_t edx, int a0, int a1, int a2, int a3, int a4, int a5
    );
    inline decltype( &do_extra_bones_processing ) orig_do_extra_bones_processing { };

    void __fastcall packet_start( 
        const std::uintptr_t ecx, const std::uintptr_t edx, const int in_seq, const int out_acked
    );
    inline decltype( &packet_start ) orig_packet_start { };

    void __fastcall packet_end( const std::uintptr_t ecx, const std::uintptr_t edx );
    inline decltype( &packet_end ) orig_packet_end{ };


    void __fastcall build_transformations( 
        valve::cs_player_t* ecx, void* edx, valve::studio_hdr_t* hdr, sdk::vec3_t* pos, sdk::vec4_t* q, sdk::mat3x4_t* cam_transform, int bone_mask, byte* computed 
    );
    inline decltype( &build_transformations ) orig_build_transformations { };

    int __fastcall list_leaves_in_box( 
        const std::uintptr_t ecx, const std::uintptr_t edx,
        const sdk::vec3_t& mins, const sdk::vec3_t& maxs, const uint16_t* const list, const int max
    );
    inline decltype( &list_leaves_in_box ) orig_list_leaves_in_box { };

    void __fastcall accumulate_layers( 
        valve::cs_player_t* const ecx, const std::uintptr_t edx, int a0, int a1, float a2, int a3
    );
    inline decltype( &accumulate_layers ) orig_accumulate_layers { };

    void __stdcall draw_mdl_exec( 
        uintptr_t ctx, const valve::draw_model_state_t& state, const valve::model_render_info_t&, sdk::mat3x4_t* bones
    );
    using o_draw_mdl_exec_t = void( __thiscall* )( void*, uintptr_t, const valve::draw_model_state_t&, const valve::model_render_info_t&, sdk::mat3x4_t* );
    inline o_draw_mdl_exec_t orig_draw_mdl_exec { };

    void __fastcall standard_blending_rules( 
        valve::cs_player_t* const ecx, const std::uintptr_t edx, valve::studio_hdr_t* const mdl_data, int a1, int a2, float a3, int mask
    );
    inline decltype( &standard_blending_rules ) orig_standard_blending_rules { };

    void __fastcall physics_simulate( valve::cs_player_t* const ecx, const std::uintptr_t edx );
    inline decltype( &physics_simulate ) orig_physics_simulate { };

    void __cdecl lower_body_yaw_proxy( valve::recv_proxy_data_t* data, void* entity, void* output );
    inline decltype( &lower_body_yaw_proxy ) orig_lby_proxy { };

    void __fastcall process_packet( valve::client_state_t::net_chan_t* net_chan, const std::uintptr_t edx, void* packet, bool header );
    inline decltype( &process_packet ) orig_process_packet { };

    int __fastcall send_datagram( valve::client_state_t::net_chan_t* net_chan, const std::uintptr_t edx, void* buff );
    inline decltype( &send_datagram ) orig_send_datagram { };

    bool __fastcall should_draw_view_model( std::uintptr_t ecx, std::uintptr_t edx );
    inline decltype( &should_draw_view_model ) orig_should_draw_view_model{ };

    int __fastcall net_showfragments( const std::uintptr_t ecx, const std::uintptr_t edx );
    inline decltype( &net_showfragments ) orig_net_showfragments { };

    void __stdcall frame_stage_notify( const valve::e_frame_stage stage );
    inline decltype( &frame_stage_notify ) orig_frame_stage_notify { };

    void __fastcall override_view( 
        const std::uintptr_t ecx, const std::uintptr_t edx, valve::view_setup_t* const setup
    );
    inline decltype( &override_view ) orig_override_view { };

    bool __fastcall process_temp_entities( const std::uintptr_t ecx, const std::uintptr_t edx, const std::uintptr_t msg );
    inline decltype( &process_temp_entities ) orig_process_temp_entities { };

    void __stdcall create_move_proxy( int seq_number, float input_sample_frame_time, bool active );
    void __stdcall create_move( int seq_number, float input_sample_frame_time, bool active, bool& send_packet );

    void __fastcall calc_view( void* ecx, const std::uintptr_t edx, sdk::vec3_t& eye_origin, const sdk::qang_t& eye_ang, float& z_near, float& z_far, float& fov );
    inline decltype( &calc_view ) orig_calc_view { };

    void __cdecl  cl_move( float accumulate_extra_samples, bool final_tick );
    inline decltype( &cl_move ) orig_cl_move { };

    void __fastcall exposure_range( float* a1, float* a2 );
    inline decltype( &exposure_range ) orig_exposure_range{ };

    void __fastcall calc_viewmodel_bob( void* ecx, const std::uintptr_t edx, sdk::vec3_t& view_bob );
    inline decltype( &calc_viewmodel_bob ) orig_calc_viewmodel_bob { };

    void __fastcall modify_eye_pos( valve::anim_state_t* ecx, std::uintptr_t edx, sdk::vec3_t& pos );
    inline decltype( &modify_eye_pos ) orig_modify_eye_pos { };

    float __stdcall aspect_ratio( int width, int height );
    inline decltype( &aspect_ratio ) orig_aspect_ratio { };

    bool __fastcall is_paused( const std::uintptr_t ecx, const std::uintptr_t edx );
    inline decltype( &is_paused ) orig_is_paused { };

    bool __fastcall is_hltv( const std::uintptr_t ecx, const std::uintptr_t edx );
    inline decltype( &is_hltv ) orig_is_hltv { };


    struct event_listener_t : public valve::base_event_listener_t {
        void fire_game_event( valve::game_event_t* const event ) override;
    } inline g_event_listener { };

    using o_create_move_t = void( __thiscall* )( valve::c_client* const, int, float, bool );
    inline o_create_move_t o_create_move{ };
}