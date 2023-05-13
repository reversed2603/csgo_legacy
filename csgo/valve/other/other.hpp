#pragma once

namespace csgo::game { 
        
    enum struct e_net_flow : int { 
        out = 0,
        in
    };

    inline constexpr auto k_mp_backup = 150u;

    // https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/public/studio.h#L77
    inline constexpr unsigned int k_max_bones = 256;

    inline constexpr auto k_net_frames_backup = 64u;

    inline constexpr auto k_net_frames_mask = k_net_frames_backup - 1u;

    inline constexpr auto k_lower_realign_delay = 1.1f;

    enum struct ent_handle_t : sdk::ulong_t { };

    enum struct e_buttons : int { 
        in_attack       = 1 << 0,
        in_jump         = 1 << 1,
        in_duck         = 1 << 2,
        in_fwd          = 1 << 3,
        in_back         = 1 << 4,
        in_use          = 1 << 5,
        in_cancel       = 1 << 6,
        in_left         = 1 << 7,
        in_right        = 1 << 8,
        in_move_left    = 1 << 9,
        in_move_right   = 1 << 10,
        in_move = in_fwd | in_back | in_move_right | in_move_left,
        in_attack2      = 1 << 11,
        in_run          = 1 << 12,
        in_reload       = 1 << 13,
        in_alt1         = 1 << 14,
        in_alt2         = 1 << 15,
        in_score        = 1 << 16,
        in_speed        = 1 << 17,
        in_walk         = 1 << 18,
        in_zoom         = 1 << 19,
        in_weapon1      = 1 << 20,
        in_weapon2      = 1 << 21,
        in_bullrush     = 1 << 22,
        in_grenade1     = 1 << 23,
        in_grenade2     = 1 << 24,
        in_attack3      = 1 << 25
    };
    ENUM_BIT_OPERATORS( e_buttons, true );

    struct move_data_t { 
        bool            m_first_run_of_functions{ },
                        m_game_code_moved_player{ },
                        m_no_air_control{ };
        ent_handle_t    m_player_handle{ };
        int             m_impulse_cmd{ };
        sdk::qang_t     m_view_angles{ },
                        m_abs_view_angles{ };
        e_buttons       m_buttons{ }, m_old_buttons{ };
        sdk::vec3_t     m_move{ };
        float           m_max_speed{ }, m_max_client_speed{ };
        sdk::vec3_t     m_velocity{ }, m_trailing_velocity{ };
        float           m_trailing_velocity_time{ };
        sdk::qang_t     m_angles{ }, m_old_angles{ };
        float           m_out_step_height{ };
        sdk::vec3_t     m_out_wish_velocity{ },
                        m_out_jump_velocity{ },
                        m_constraint_center{ };
        float           m_constraint_radius{ },
                        m_constraint_width{ },
                        m_constraint_speed_factor{ };
        bool            m_constraint_past_radius{ };
        sdk::vec3_t     m_abs_origin{ };
    };

    struct recv_prop_t;

    struct recv_table_t { 
        recv_prop_t*    m_props{ };
        int             m_props_count{ };
        void*           m_decoder{ };
        const char*     m_net_table_name{ };
        bool            m_initialized{ },
                        m_in_main_list{ };
    };

    struct recv_prop_t { 
        char*           m_var_name{ };
        int             m_send_type{ },
                        m_flags{ },
                        m_str_buffer_size{ };
        bool            m_in_array{ };
        sdk::address_t  m_extra_data{ };
        recv_prop_t*    m_array_prop{ };
        sdk::address_t  m_array_len_proxy { };
        std::uintptr_t		m_proxy_fn { };
        sdk::address_t       m_data_table_proxy_fn{ };
        recv_table_t*   m_data_table{ };
        std::uint32_t   m_offset{ };
        int             m_element_stride{ },
                        m_elements{ };
        const char*     m_parent_array_prop_name{ };
    };
    struct move_msg_t { 
        char	pad0 [ 12u ] { };
        int		m_backup_cmds { };
        int		m_new_cmds { };
    };

    struct view_setup_t { 
        int			m_x { };
        int			m_x_unscaled { };
        int			m_y { };
        int			m_y_unscaled { };
        int			m_width { };
        int			m_old_width { };
        int			m_height { };
        int			m_old_height { };
        char		pad0[ 144u ] { };
        float		m_fov { };
        float		m_view_model_fov { };
        sdk::vec3_t		m_origin { };
        sdk::qang_t	m_angles { };
        float       m_znear { };
        float       m_zfar { };
        float       m_znear_viewmodel { };
        float       m_zfar_viewmodel { };
        float       m_aspect_ratio { };
        float       m_near_blur_depth { };
        float       m_near_focus_depth { };
        float       m_far_focus_depth { };
        float       m_far_blur_depth { };
        float       m_near_blur_radius { };
        float       m_far_blur_radius { };
        int         m_quality { };
        int         m_motionblur_mode { };
        float       m_shutter_time { };
        sdk::vec3_t      m_shutter_open_pos { };
        sdk::qang_t    m_shutter_open_angles { };
        sdk::vec3_t      m_shutter_close_pos { };
        sdk::qang_t	m_shutter_close_angles { };
        float       m_offcenter_top { };
        float       m_offcenter_bottom { };
        float       m_offcenter_left { };
        float       m_offcenter_right { };
        int         m_edge_blur { };
    };

    struct recv_proxy_data_t { 
        recv_prop_t* m_prop { };

        struct { 
            union { 
                float			m_float;
                long			m_int;
                const char* m_str;
                void* m_data;
                sdk::vec3_t			m_vec3;
                std::int64_t	m_int64 { };
            };

            int					m_type;
        }				m_value { };

        int				m_element { };
        int				m_object_id { };
    };

    struct data_map_t;

    enum struct e_field_type : std::uint32_t { 
        _void,
        _float,
        str,
        vec3,
        vec4,
        _int,
        _bool,
        _short,
        _char,
        clr32,
        embedded,
        custom,
        classptr,
        ehandle,
        edict,
        pos_vec,
        time,
        tick,
        mdl_name,
        sound_name,
        input,
        fn,
        mat,
        vmat_worldspace,
        mat3x4_worldspace,
        interval,
        mdl_index,
        material_index,
        vec2
    };

    struct type_desc_t { 
        int             m_type{ };
        const char*     m_name{ };
        std::uint32_t   m_offset{ };
        std::uint16_t   m_size{ },
                        m_flags{ };
        const char*     m_external_name{ };
        sdk::address_t  m_save_restore_ops{ },
                        m_input_fn{ };
        data_map_t*     m_data_map{ };
        std::uint32_t   m_field_size_in_bytes{ };
        type_desc_t*    m_override_field{ };
        std::uint32_t   m_override_count{ };
        float           m_tolerance{ };
        int             m_flat_offset[ 2u ]{ };
        std::uint16_t   m_flat_group{ };
    };

    struct data_map_t { 
        type_desc_t* m_descriptions{ };
        int          m_size{ };
        char const*  m_name{ };
        data_map_t*  m_base_map{ };
        bool         m_chains_validated{ },
                     m_packed_offsets_computed{ };
        int          m_packed_size{ };
    };

    struct client_class_t { 
        void*           m_create_fn{ };
        void*           m_create_event_fn{ };
        const char*     m_network_name{ };
        recv_table_t*   m_recv_table{ };
        client_class_t* m_next{ };
        int             m_class_id{ };
    };

    struct user_cmd_t { 
        OFFSET_VFUNC( std::uint32_t( __thiscall* )( decltype( this ) ), checksum( ), g_ctx->offsets( ).m_user_cmd_checksum );

        __forceinline void sanitize( );
        std::uint8_t    pad0[ 4u ] { };
        int             m_number { }, m_tick { };
        sdk::qang_t     m_view_angles { };
        sdk::vec3_t     m_aim_dir { }, m_move { };
        e_buttons       m_buttons { };
        std::uint8_t    m_impulse { };
        int             m_weapon_select { },
            m_weapon_sub_type { },
            m_random_seed { };
        sdk::s16vec2_t  m_mouse_accum { };
        bool            m_predicted { };
        sdk::qang_t     m_head_angles { };
        sdk::vec3_t     m_head_offset { };
    };

    struct cmd_context_t { 
        bool		m_needs_processing { };
        user_cmd_t	m_user_cmd { };
        int			m_cmd_number { };
    };

    struct vfyd_user_cmd_t { 
        user_cmd_t      m_cmd{ };
        std::uint32_t   m_checksum{ };
    };

    struct var_mapping_t { 
        struct entry_t { 
            std::uint16_t	m_type { };
            std::uint16_t	m_needs_to_interpolate { };
            void* m_data { };
            void* m_watcher { };
        };

        utl_vec_t< entry_t >	m_entries { };
        std::ptrdiff_t		    m_interpolated_entries { };
        float					m_last_interpolation_time { };
    };

    struct bf_write_t { 
        __forceinline void write_user_cmd( user_cmd_t* const to, user_cmd_t* const from ) { 
            using fn_t = void( __fastcall* )( decltype( this ), user_cmd_t* const, user_cmd_t* const );

            reinterpret_cast< fn_t > ( g_ctx->addresses( ).m_write_user_cmd )( this, to, from );

            __asm { 
                add esp, 4 
            }
        }
    };
    class c_mat_var { 
    public:
        VFUNC( void( __thiscall* )( decltype( this ), float ), set_value( float val ), 4u, val );
        VFUNC( void( __thiscall* )( decltype( this ), int ), set_value( int val ), 5u, val );
        VFUNC( void( __thiscall* )( decltype( this ), char const* ), set_value( char const* val ), 6u, val );
        VFUNC( void( __thiscall* )( decltype( this ), float, float, float ), set_value( float x, float y, float z ), 11u, x, y, z );
    };

    struct beam_info_t;

    struct beams_t { 
        __forceinline void draw_beam( const std::uintptr_t beam ) { 
            using fn_t = void( __thiscall* )( decltype( this ), const std::uintptr_t );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 4u ]( this, beam );
        }

        __forceinline std::uintptr_t create_beam_points( beam_info_t& info ) { 
            using fn_t = std::uintptr_t( __thiscall* )( decltype( this ), beam_info_t& );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 12u ]( this, info );
        }

        __forceinline std::uintptr_t create_beam_ring_point( beam_info_t& info ) { 
            using fn_t = std::uintptr_t( __thiscall* )( decltype( this ), beam_info_t& );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 16u ]( this, info );
        }
    } inline* g_beams{ };

    class c_material { 
    public:
        virtual const char* get_name( ) const = 0;
        virtual const char* get_tex_group_name( ) const = 0;

        VFUNC( c_mat_var* ( __thiscall* ) 
( decltype( this ), const char*, bool*, bool ), find_var( const char* name, bool* found, bool complain = true ), 11u, name, found, complain );
        VFUNC( void* ( __thiscall* )( decltype( this ) ), increment_ref_count( ), 12u );
        VFUNC( void* ( __thiscall* )( decltype( this ), float ), alpha_modulate( float alpha ), 27u, alpha );
        VFUNC( void* ( __thiscall* )( decltype( this ), float, float, float ), clr_modulate( float r, float g, float b ), 28u, r, g, b );
        VFUNC( void* ( __thiscall* )( decltype( this ), int, bool ), set_flag( int flag, bool val ), 29u, flag, val );
        VFUNC( bool( __thiscall* )( decltype( this ) ), is_error_mat( ), 42u );
    };

    struct cvar_t { 
        using callback_t = void( __cdecl* )( );

        using change_callback_t = void( __cdecl* )( void*, const char* const, const float );

        VFUNC( const char* ( __thiscall* )( decltype( this ) ), get_str( ), 11u );

        VFUNC( float( __thiscall* )( decltype( this ) ), get_float( ), 12u );

        VFUNC( int( __thiscall* )( decltype( this ) ), get_int( ), 13u );

        VFUNC( void( __thiscall* )( decltype( this ), const char* ), set_str( const char* value ), 14u, value );

        VFUNC( void( __thiscall* )( decltype( this ), float ), set_float( const float value ), 15u, value );

        VFUNC( void( __thiscall* )( decltype( this ), int ), set_int( const int value ), 16u, value );

        std::uint8_t                    pad0[ 4u ]{ };

        cvar_t*                         m_next{ };
        bool                            m_registered{ };

        const char*                     m_name{ }, *m_help{ };
        int                             m_flags{ };

        callback_t                      m_callback{ };
        cvar_t*                         m_parent{ };

        const char*                     m_def_value{ }, *m_str{ };
        int                             m_str_len{ };

        float                           m_float{ };
        int                             m_int{ };

        bool                            m_has_min{ };
        float                           m_min{ };

        bool                            m_has_max{ };
        float                           m_max{ };

        utl_vec_t< change_callback_t >  m_change_callbacks{ };
    };

    struct plane_t { 
        sdk::vec3_t     m_normal{ };
        float           m_dist{ };
        std::uint8_t    m_type{ },
                        m_sign_bits{ },
                        pad0[ 2u ]{ };
    };

    struct surface_t { 
        const char*     m_name{ };
        short           m_surface_props{ };
        std::uint16_t   m_flags{ };
    };

    struct surface_data_t { 
        struct { 
            float m_friction{ },
                  m_elasticity{ },
                  m_density{ },
                  m_thickness{ },
                  m_dampening{ };
        }               m_physics{ };

        struct { 
            float m_reflectivity{ },
                  m_hardness_factor{ },
                  m_roughness_factor{ },
                  m_rough_threshold{ },
                  m_hard_threshold{ },
                  m_hard_velocity_threshold{ },
                  m_high_pitch_occlusion{ },
                  m_mid_pitch_occlusion{ },
                  m_low_pitch_occlusion{ };
        }               m_audio{ };

        struct { 
            std::uint16_t  m_walk_step_left{ },
                           m_walk_step_right{ },
                           m_run_step_left{ },
                           m_run_step_right{ },
                           m_impact_soft{ },
                           m_impact_hard{ },
                           m_scrape_smooth{ },
                           m_scrape_rough{ },
                           m_bullet_impact{ },
                           m_rolling{ },
                           m_break_sound{ },
                           m_strain_sound{ };
        }               m_sounds{ };

        struct { 
            float           m_max_speed_factor{ },
                            m_jump_factor{ },
                            m_pen_modifier{ },
                            m_dmg_modifier{ };
            std::uint16_t   m_material{ };
            std::uint8_t    m_climbable{ };
        }               m_game{ };

        std::uint8_t    pad0[ 48u ]{ };
    };

    struct net_channel_info_t { 
        __forceinline float latency( const e_net_flow flow )
        { 
            using fn_t = float( __thiscall* )( decltype( this ), const e_net_flow );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 9u ]( this, flow );
        }

        __forceinline float avg_latency( const e_net_flow flow )
        { 
            using fn_t = float( __thiscall* )( decltype( this ), const e_net_flow );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 10u ]( this, flow );
        }
    };

    struct net_channel_t : public net_channel_info_t { 
    public:
        char pad_0000 [ 20 ];           //0x0004
        bool m_processing_msg;  //0x0014
        bool m_should_del;        //0x0015
        char pad_0016 [ 2 ];            //0x0016
        int32_t m_out_seq_num;    //0x0018 last send outgoing sequence number
        int32_t m_in_seq_num;     //0x001C last received incoming sequnec number
        int32_t m_out_seq_num_attack; //0x0020 last received acknowledge outgoing sequnce number
        int32_t m_out_reliable_state; //0x0024 state of outgoing reliable data( 0/1 ) flip flop used for loss detection
        int32_t m_in_reliable_state;  //0x0028 state of incoming reliable data
        int32_t m_choked_packets;    //0x002C number of choked packets
        char pad_0030 [ 1044 ];         //0x0030
    };

    struct game_event_t { 
        __forceinline const char* name( );

        __forceinline float get_float( const char* name );

        __forceinline int get_int( const char* name );

        __forceinline const char* get_str( const char* name, const char* def_val = "" );
    };

    struct base_event_listener_t { 
        virtual void unk( ) { };

        virtual void fire_game_event( game_event_t* const event ) = 0;

        virtual int debug_event_id( ) { return 42; }
    };

    struct ray_t { 
        __forceinline constexpr  ray_t( ) = default;

        __forceinline ray_t( const sdk::vec3_t& start, const sdk::vec3_t& end )
            : m_start{ start }, m_delta{ end - start },
            m_ray{ true }, m_swept{ m_delta.length_sqr( ) != 0.f } { }

        __forceinline ray_t( 
            const sdk::vec3_t& start, const sdk::vec3_t& end,
            const sdk::vec3_t& mins, const sdk::vec3_t& maxs
        ) : m_start{ start + ( mins + maxs ) / 2.f },
            m_delta{ end - start },
            m_start_offset{ ( mins + maxs ) / -2.f },
            m_extents{ ( maxs - mins ) / 2.f },
            m_ray{ m_extents.length_sqr( ) < 1e-6f },
            m_swept{ m_delta.length_sqr( ) != 0.f } { }

        sdk::vec3_t             m_start{ };
        std::uint8_t            pad0[ 4u ]{ };

        sdk::vec3_t             m_delta{ };
        std::uint8_t            pad1[ 4u ]{ };

        sdk::vec3_t             m_start_offset{ };
        std::uint8_t            pad2[ 4u ]{ };

        sdk::vec3_t             m_extents{ };
        std::uint8_t            pad3[ 4u ]{ };

        const sdk::mat3x4_t*    m_world_axis_transform{ };

        bool                    m_ray{ }, m_swept{ };
    };

    enum struct e_hitbox : int { 
        head,
        neck,
#ifdef CSGO2018
        lower_neck,
#endif
        pelvis,
        stomach,
        lower_chest,
        chest,
        upper_chest,
        right_thigh,
        left_thigh,
        right_calf,
        left_calf,
        right_foot,
        left_foot,
        right_hand,
        left_hand,
        right_upper_arm,
        right_forearm,
        left_upper_arm,
        left_forearm,
        max
    };

    enum struct e_hitgroup : int { 
        generic,
        head,
        chest,
        stomach,
        left_arm,
        right_arm,
        left_leg,
        right_leg,
        gear = 10
    };

    struct base_entity_t;

    struct base_trace_filter_t { 
        virtual bool should_hit_entity( base_entity_t* entity, int mask ) const = 0;

        virtual int type( ) const = 0;
    };

    struct trace_filter_t : public base_trace_filter_t { 
        bool should_hit_entity( base_entity_t* entity, int ) const;

        virtual int type( ) const { 
            return 1;
        }

        inline void set_ignore_class( char* class_ ) { 
            m_ignore_cc = class_;
        }

        void* m_ignore_entity{ };
        char* m_ignore_cc{ };
    };

    struct trace_filter_world_only_t : public base_trace_filter_t { 
        virtual bool should_hit_entity( base_entity_t* const entity, const int mask ) const { return false; }

        virtual int type( ) const { return 1; }
    };
    using should_hit_fn_t = bool( __cdecl* )( game::base_entity_t* const, const int );

    struct trace_filter_simple_t { 
        __forceinline trace_filter_simple_t( )
            : m_vtable { g_ctx->addresses( ).m_trace_filter_simple_vtable }
        { 
        }

        __forceinline trace_filter_simple_t( 
            game::base_entity_t* const ignore_entity, const int collision_group
        ) : m_vtable { g_ctx->addresses( ).m_trace_filter_simple_vtable },
            m_ignore_entity { ignore_entity }, m_collision_group { collision_group } { }

        std::uintptr_t	m_vtable { };
        game::base_entity_t* m_ignore_entity { };
        int				m_collision_group { };
        should_hit_fn_t	m_should_hit_fn { };
    };

    struct trace_filter_skip_two_entities_t { 
        __forceinline trace_filter_skip_two_entities_t( )
            : m_vtable{ g_ctx->addresses( ).m_trace_filter_skip_two_entities_vtable }
        { 
        }

        __forceinline trace_filter_skip_two_entities_t( 
            game::base_entity_t* const ignore_entity0, game::base_entity_t* const ignore_entity1, const int collision_group = 0
        ) : m_vtable{ g_ctx->addresses( ).m_trace_filter_skip_two_entities_vtable },
            m_ignore_entity0{ ignore_entity0 }, m_collision_group{ collision_group }, m_ignore_entity1{ ignore_entity1 } { }

        std::uintptr_t	m_vtable{ };
        game::base_entity_t* m_ignore_entity0{ };
        int				m_collision_group{ };
        should_hit_fn_t	m_should_hit_fn{ };
        game::base_entity_t* m_ignore_entity1{ };
    };

    struct bone_cache_t { 
    public:
        sdk::mat3x4_t* m_cached_bones;
        char m_pad0 [ 0x8 ];
        int        m_bone_cnt;
    };

    struct trace_t { 

        sdk::vec3_t     m_start{ }, m_end{ };

        plane_t         m_plane{ };

        float           m_frac{ };
        int             m_contents{ };
        std::uint16_t   m_disp_flags{ };

        bool            m_all_solid{ },
                        m_start_solid{ };

        float           m_frac_left_solid{ };
        surface_t       m_surface{ };

        e_hitgroup      m_hitgroup{ };
        short           m_physics_bone{ };

        std::uint16_t   m_world_surface_index{ };

        base_entity_t*  m_entity{ };
        e_hitbox        m_hitbox{ };

        __forceinline bool hit( ) const;
        __forceinline bool did_hit_world( ) const;
        __forceinline bool did_hit_non_world_ent( ) const;

        __forceinline void clear( )
        { 
            std::memset( this, 0, sizeof( trace_t ) );

            m_frac = 1.f;
            m_surface.m_name = "**studio**";
        }
    };

    enum struct e_model_type : int { 
        bad,
        brush,
        sprite,
        studio
    };

    struct model_t { 
        std::uint32_t   m_file_handle{ };
        char            m_path[ 260u ]{ };
        
        int             m_load_flags{ },
                        m_server_count{ };

        e_model_type    m_type{ };
        int             m_flags{ };

        sdk::vec3_t     m_mins{ }, m_maxs{ };
        float           m_radius{ };

        key_values_t*   m_key_values{ };

        
        union { 
            std::uintptr_t    m_brush{ };
            std::uint16_t    m_studio;
            std::uintptr_t   m_sprite;
        };
    };

    struct player_info_t { 
        std::uint64_t   m_ver{ };
        union { 
            std::uint64_t m_xuid{ };

            struct { 
                std::uint32_t   m_xuid_low,
                                m_xuid_high;
            };
        };

        char            m_name[ 128u ]{ };

        int             m_uid{ };
        char            m_guid[ 33u ]{ };

        std::uint32_t   m_friends_id{ };
        char            m_friends_name[ 128u ]{ };

        bool            m_fake_player{ }, m_hltv{ };

        std::uint32_t   m_custom_files[ 4u ]{ };
        std::uint8_t    m_downloaded_files_count{ };
    };

    enum struct e_weapon_type : int { 
        knife,
        pistol,
        smg,
        rifle,
        shotgun,
        sniper,
        machine_gun,
        c4,
        placeholder,
        grenade,
        healthshot = 11,
#ifndef CSGO2018
        fists,
        breach_charge,
        bump_mine,
        tablet,
        melee
#endif
    };

    struct weapon_info_t { 
        std::uint8_t    pad0[ 4u ]{ };
        const char*     m_con_name{ };
        char            pad1[ 12u ]{ };
        int             m_max_clip1{ },
                        m_max_clip2{ },
                        m_default_clip1{ },
                        m_default_clip2{ },
                        m_primary_reserve_ammo_max{ },
                        m_secondary_reserve_ammo_max{ };
        const char*     m_world_model{ },
                        *m_view_model{ },
                        *m_dropped_model{ };
        std::uint8_t    pad2[ 80u ]{ };
        const char*     m_hud_name{ },
                        *m_name{ };
        std::uint8_t    pad3[ 2u ]{ };
        bool            m_is_melee{ };
        std::uint8_t    pad4[ 9u ]{ };
        float           m_weight{ };
        std::uint8_t    pad5[ 40u ]{ };
        e_weapon_type   m_type{ };
#ifndef CSGO2018
        int             m_unk_type{ };
#endif
        int             m_price{ },
                        m_kill_award{ };
        const char*     m_anim_prefix{ };
        float           m_cycle_time{ },
                        m_cycle_time_alt{ },
                        m_time_to_idle_after_fire{ },
                        m_idle_interval{ };
        bool            m_full_auto{ };
        std::uint8_t    pad6[ 3u ]{ };
        int             m_dmg{ };
#ifndef CSGO2018
        float           m_headshot_multiplier{ };
#endif
        float           m_armor_ratio{ };
        int             m_bullets{ };
        float           m_penetration{ },
                        m_flinch_velocity_modifier_large{ },
                        m_flinch_velocity_modifier_small{ },
                        m_range{ },
                        m_range_modifier{ },
                        m_throw_velocity{ };
        std::uint8_t    pad7[ 12u ]{ };
        bool            m_has_silencer{ };
        std::uint8_t    pad8[ 15u ]{ };
        float           m_max_speed{ },
                        m_max_speed_alt{ };
#ifndef CSGO2018
        std::uint8_t    pad9[ 4u ]{ };
#endif
        float           m_spread{ },
                        m_spread_alt{ },
                        m_inaccuracy_crouch{ },
                        m_inaccuracy_crouch_alt{ },
                        m_inaccuracy_stand{ },
                        m_inaccuracy_stand_alt{ },
                        m_inaccuracy_jump_initial{ };
#ifndef CSGO2018
        float           m_inaccuracy_jump_apex{ };
#endif
        float           m_inaccuracy_jump{ },
                        m_inaccuracy_jump_alt{ },
                        m_inaccuracy_land{ },
                        m_inaccuracy_land_alt{ },
                        m_inaccuracy_ladder{ },
                        m_inaccuracy_ladder_alt{ },
                        m_inaccuracy_fire{ },
                        m_inaccuracy_fire_alt{ },
                        m_inaccuracy_move{ },
                        m_inaccuracy_move_alt{ },
                        m_inaccuracy_reload{ };
        int             m_recoil_seed{ };
        float           m_recoil_angle{ },
                        m_recoil_angle_alt{ },
                        m_recoil_angle_variance{ },
                        m_recoil_angle_variance_alt{ },
                        m_recoil_magnitude{ },
                        m_recoil_magnitude_alt{ },
                        m_recoil_magnitude_variance{ },
                        m_recoil_magnitude_variance_alt{ };
    };

    struct cs_player_t;

    struct studio_bone_t { 
        int             m_name_index{ },
                        m_parent{ },
                        m_bone_controller[ 6u ]{ };
        sdk::vec3_t     m_pos{ };
        float           m_quat[ 4u ]{ };
        sdk::vec3_t     m_rotation{ },
                        m_pos_scale{ },
                        m_rotation_scale{ };
        sdk::mat3x4_t   m_pose_to_bone{ };
        float           m_align[ 4u ]{ };
        int             m_flags{ },
                        m_proc_type{ },
                        m_proc_index{ },
                        m_physics_bone{ },
                        m_surface_prop_index{ },
                        m_contents{ },
                        m_surface_prop_lookup{ };
        std::uint8_t    pad0[ 28u ]{ };
    };

    struct studio_bbox_t { 
        int             m_bone{ },
                        m_group{ };
        sdk::vec3_t     m_mins{ },
                        m_maxs{ };
        int             m_name_index{ };
        sdk::qang_t     m_rotation{ };
        float           m_radius{ };
        std::uint8_t    pad0[ 16u ]{ };
    };

    struct studio_hitbox_set_t { 
        __forceinline const char* name( ) const;

        __forceinline studio_bbox_t* get_bbox( const int i ) const;

        int m_name_index{ },
            m_hitboxes_count{ },
            m_hitbox_index{ };
    };

    struct beam_info_t { 
        int			m_type{ };
        base_entity_t* m_start_entity{ };
        int			m_start_attachment{ };
        base_entity_t* m_end_entity{ };
        int			m_end_attachment{ };
        sdk::vec3_t		m_start{ };
        sdk::vec3_t		m_end{ };
        int			m_model_index{ -1 };
        const char* m_model_name{ };
        int			m_halo_index{ -1 };
        const char* m_halo_name{ };
        float		m_halo_scale{ };
        float		m_life{ };
        float		m_width{ };
        float		m_end_width{ };
        float		m_fade_length{ };
        float		m_amplitude{ };
        float		m_brightness{ };
        float		m_speed{ };
        int			m_start_frame{ };
        float		m_frame_rate{ };
        float		m_red{ };
        float		m_green{ };
        float		m_blue{ };
        bool		m_renderable{ true };
        int			m_segments{ -1 };
        int			m_flags{ };
        sdk::vec3_t		m_center{ };
        float		m_start_radius{ };
        float		m_end_radius{ };
    };;

    /* need to fix this trash somehow */
    struct csvc_msg_data_legacy_t
    { 
        char pad0[ 0x8 ];
        int32_t m_client;
        int32_t m_audible_mask;
        uint32_t m_xuid_low;
        uint32_t m_xuid_high;
        std::string* m_voice_data;
        bool m_proximity;
        bool m_caster;
        int32_t m_format;
        int32_t m_sequence_bytes;
        uint32_t m_section_number;
        uint32_t m_uncompressed_sample_offset;
    };

    struct cclc_msg_data_legacy_t
    { 
        uint32_t INetMessage_Vtable;
        char pad_0004[ 4 ];
        uint32_t m_CCLCMsg_VoiceData_Vtable; 
        char pad_000C[ 8 ];
        void* m_data; // idk
        uint32_t m_xuid_low{ }; // 6 idx -> unqiue_idx
        uint32_t m_xuid_high{ }; // 7 idx -> player id
        int32_t m_format; // 8 idx -> format should be zero
        int32_t m_sequence_bytes; // 9 idx -> x origin
        uint32_t m_section_number; // 10 idx -> y origin
        uint32_t m_uncompressed_sample_offset; // 11 idx -> z origin
        int32_t m_cached_size; // 12 idx -> dontcare
        uint32_t m_flags; // 13 idx -> 63 should be

        uint8_t m_no_stack_overflow[ 0xFF ];
    };


    struct studio_hdr_t { 
        using bone_flags_t = utl_vec_t< std::uint32_t >;

        struct studio_t { 
            __forceinline studio_bone_t* get_bone( const int i ) const;

            __forceinline studio_hitbox_set_t* get_hitbox_set( const int i ) const;

            inline game::studio_bbox_t* p_hitbox( int i, int set )
            { 
                auto s = get_hitbox_set( set );
                if( !s )
                    return NULL;

                return s->get_bbox( i );
            }

            int             m_id{ },
                            m_ver{ };
            long            m_checksum{ };
            char            m_name[ 64u ]{ };
            int             m_length{ };
            sdk::vec3_t     m_eye_pos{ },
                            m_illum_pos{ },
                            m_hull_min{ },
                            m_hull_max{ },
                            m_obb_min{ },
                            m_obb_max{ };
            std::uint32_t   m_flags{ };
            int             m_bones_count{ },
                            m_bone_index{ },
                            m_bone_controllers_count{ },
                            m_bone_controller_index{ },
                            m_hitbox_sets_count{ },
                            m_hitbox_set_index{ };
        }               *m_studio{ };

        std::uint8_t    pad0[ 44u ]{ };
        bone_flags_t    m_bone_flags{ };
    };

    struct weapon_cs_base_gun_t;

    struct anim_layer_t;

    struct anim_state_t { 
        OFFSET_VFUNC( void( __thiscall* )( decltype( this ) ), reset( ), g_ctx->offsets( ).m_anim_state.m_reset );

        OFFSET_VFUNC( void( __vectorcall* )( decltype( this ), void*, float, float, float, void* ),
            update( float pitch, float yaw ), g_ctx->offsets( ).m_anim_state.m_update, nullptr, 0.f, yaw, pitch, nullptr );

        __forceinline std::ptrdiff_t select_sequence_from_acitivty_modifier( std::ptrdiff_t activity ) { 
            bool is_player_ducked = m_duck_amount > 0.55f;
            bool is_player_running = m_speed_as_portion_of_walk_speed > 0.25f;

            int32_t cur_sequence = -1;
            switch( activity )
            { 
            case 985:
            { 
                cur_sequence = 15 + static_cast < int32_t > ( is_player_running );
                if( is_player_ducked )
                    cur_sequence = 17 + static_cast < int32_t > ( is_player_running );
            }
            break;

            case 981:
            { 
                cur_sequence = 8;
                if( m_last_weapon != m_weapon )
                    cur_sequence = 9;
            }
            break;

            case 980:
            { 
                cur_sequence = 6;
            }
            break;

            case 986:
            { 
                cur_sequence = 14;
            }
            break;


            case 988:
            { 
                cur_sequence = 20;
                if( is_player_running )
                    cur_sequence = 22;

                if( is_player_ducked )
                { 
                    cur_sequence = 21;
                    if( is_player_running )
                        cur_sequence = 19;
                }
            }
            break;

            case 989:
            { 
                cur_sequence = 23;
                if( is_player_ducked )
                    cur_sequence = 24;
            }
            break;

            case 987:
            { 
                cur_sequence = 13;
            }
            break;
            default: break;
            }

            return cur_sequence;
        }


        void set_layer_seq( game::anim_layer_t* layer, int act );

        std::uint8_t            pad0[ 4u ]{ };
        bool                    m_first_update{ };
        std::uint8_t            pad1[ 91u ]{ };
        cs_player_t*            m_player{ };
        weapon_cs_base_gun_t*   m_weapon{ },
                                *m_last_weapon{ };
        float                   m_last_update_time{ };
        int                     m_last_update_frame{ };
        float                   m_last_update_increment{ },
                                m_eye_yaw{ },
                                m_eye_pitch{ },
                                m_foot_yaw{ },
                                m_last_foot_yaw{ },
                                m_move_yaw{ },
                                m_move_yaw_ideal{ },
                                m_move_yaw_cur_to_ideal{ },
                                m_time_to_align_lower_body{ },
                                m_primary_cycle{ },
                                m_move_weight{ },
                                m_move_weight_smoothed{ },
                                m_duck_amount{ },
                                m_duck_additional{ },
                                m_recrouch_weight{ };
        sdk::vec3_t             m_origin{ },
                                m_last_origin{ },
                                m_velocity{ },
                                m_velocity_normalized{ },
                                m_velocity_normalized_non_zero{ };
        float                   m_speed_2d{ },
                                m_up_speed{ },
                                m_speed_as_portion_of_run_speed{ },
                                m_speed_as_portion_of_walk_speed{ },
                                m_speed_as_portion_of_crouch_speed{ },
                                m_time_since_started_moving{ },
                                m_time_since_stopped_moving{ };
        bool                    m_on_ground{ },
                                m_landing{ };
        std::uint8_t            pad6[ 6u ]{ };
        float                   m_time_since_in_air{ },
                                m_left_ground_height{ },
                                m_land_anim_multiplier{ },
                                m_walk_to_run_transition{ };
        std::uint8_t            pad7[ 4u ]{ };
        float                   m_in_air_smooth_value{ };
        bool                    m_on_ladder{ };
        std::uint8_t            pad8[ 47u ]{ };
        float                   m_velocity_test_time{ };
        sdk::vec3_t             m_last_velocity{ },
                                m_dst_acceleration{ },
                                m_acceleration{ };
        float                   m_acceleration_weight{ };
        std::uint8_t            pad9[ 12u ]{ };
        float                   m_strafe_weight{ };
        std::uint8_t            pad10[ 4u ]{ };
        float                   m_strafe_cycle{ };
        int                     m_strafe_sequence{ };
        std::uint8_t            pad11[ 388u ]{ };
        float                   m_camera_shooth_height{ };
        bool                    m_smooth_height_valid{ };
        std::uint8_t            pad12[ 11u ]{ };
        float                   m_aim_yaw_min{ },
                                m_aim_yaw_max{ },
                                m_aim_pitch_min{ },
                                m_aim_pitch_max{ };
        int                     m_ver{ };
    };
    struct model_render_info_t { 
       sdk::vec3_t				m_origin;			// 0
       sdk::qang_t				m_angles;			// 12
        void* m_renderable;		// 24
        const model_t* m_model;			// 28
        const sdk::mat3x4_t* m_model_to_world;	// 32
        const sdk::mat3x4_t* m_lighting_offset;	// 36
        const sdk::vec3_t* m_lighting_origin;	// 40
        int					m_flags;			// 44
        int					m_index;			// 48
        int					m_skin;				// 52
        int					m_body;				// 56
        int					m_hitboxset;		// 60
        uint16_t			m_instance;			// 64

        __forceinline model_render_info_t( ) : m_model_to_world { }, m_lighting_offset { }, m_lighting_origin { } { }
    };

    struct notice_text_t { 
        wchar_t m_text[ 512 ];
        int m_unk0; // 0x400
        float m_unk1; // 0x404
        float m_unk2; // 0x408
        int m_unk3;   // 0x40C
        float m_time; // 0x410
        int m_unk4;	// 0x414
        float m_fade; // 0x418
        int m_unk5;   // 0x41C
    };

    struct kill_feed_t { 
        char m_pad[ 0x7c ] { };
        game::utl_vec_t< notice_text_t > m_notices { };
    };

    struct hud_element_t { 
        VFUNC( const char* ( __thiscall* )( decltype( this ) ), name( ), 12u );
    };

    struct hud_t { 
        char m_pad[ 0x1c ] { };
        game::utl_vec_t < hud_element_t* > m_elements{ };
        __forceinline hud_element_t* find_element( unsigned int hash ) { 
            size_t size = m_elements.size( );
            for( size_t i{ }; i < size; ++i ) { 
                if( sdk::hash( m_elements.at( i )->name( ) ) == hash ) { 
                    return m_elements.at( i );
                }
            }

            return nullptr;
        }
    } inline* g_hud { };

    struct studio_hwdata_t;

    struct draw_model_state_t { 
        studio_hdr_t* m_studio_hdr { };
        std::uintptr_t m_studio_hw_data { };
        void* m_cl_renderable { };
        const sdk::mat3x4_t* m_bones { };
        void* m_decals { };
        std::ptrdiff_t m_draw_flags { };
        std::ptrdiff_t m_lod { };
    };

    class c_mdl_render { 
    public:
        VFUNC( void( __thiscall* )( decltype( this ), c_material*, int ), forced_mat_override( c_material* mat = nullptr, int type = 0 ),
            1u, mat, type );
        VFUNC( bool( __thiscall* )( decltype( this ) ), is_forced_mat_override( ), 2u );
        VFUNC( void( __thiscall* )( decltype( this ), std::uintptr_t, const draw_model_state_t&, const model_render_info_t&, sdk::mat3x4_t* ),
            draw_mdl_exec( std::uintptr_t ctx, const draw_model_state_t& mdl_state, const model_render_info_t& mdl_render, sdk::mat3x4_t* bones ),
            21u, ctx, mdl_state, mdl_render, bones );
    } inline* g_mdl_render { };

    class c_studio_render_ctx { 
    public:
        char nigga_0 [ 0x250 ];
        c_material* m_forced_mat;

    public:
        VFUNC( void( __thiscall* )( decltype( this ), c_material* mat, int, int ), forced_mat_override( c_material* mat, int who = 0, int asked = 0 ),
            33u, mat, who, asked );

    } inline* g_studio_render { };

    struct anim_layer_t { 
        float           m_anim_time{ }, 
                        m_fade_out_time{ };

        studio_hdr_t*   m_studio_hdr{ };

        int             m_disp_src{ }, m_disp_dst{ },
                        m_order{ }, m_seq{ };

        float           m_prev_cycle{ }, m_weight{ },
                        m_weight_delta_rate{ }, m_playback_rate{ }, m_cycle{ };

        cs_player_t*    m_owner{ };
        int             m_inv_physics_bits{ };
    };

    using pose_params_t = std::array< float, 24u >;
    using anim_layers_t = std::array < anim_layer_t, 13u >;

    using bones_t = std::array < sdk::mat3x4_t, game::k_max_bones >;

    struct bone_accessor_t { 
        void* m_animating;
        sdk::mat3x4_t* m_bones;
        std::ptrdiff_t        m_readable_bones;
        std::ptrdiff_t        m_writable_bones;
    };

    enum e_class_id : std::ptrdiff_t { 
        inferno = 88u,
        base_cs_grenade = 8u,
        base_cs_grenade_projectile = 9u,
        molotov_grenade = 97u,
        molotov_projectile = 98u,
        smoke_grenade = 133u,
        smoke_grenade_projectile = 134u,
        cascade_light = 30u,
        tone_map_controller = 59u,
        c_planted_c4 = 108u,
        c_c4 = 29u
    };

    enum struct e_item_index : std::uint16_t { 
        none,
        deagle,
        elite,
        five_seven,
        glock,
        ak47 = 7u,
        aug,
        awp,
        famas,
        g3sg1,
        galil = 13u,
        m249,
        m4a4 = 16u,
        mac10,
        p90 = 19u,
        zone_repulsor,
        mp5sd = 23u,
        ump45,
        xm1014,
        bizon,
        mag7,
        negev,
        sawed_off,
        tec9,
        taser,
        p2000,
        mp7,
        mp9,
        nova,
        p250,
        shield,
        scar20,
        sg553,
        ssg08,
        knife_gg,
        knife_t,
        flashbang,
        he_grenade,
        smoke_grenade,
        molotov,
        decoy,
        inc_grenade,
        c4,
        healthshot = 57u,
        knife_ct = 59u,
        m4a1s,
        usps,
        cz75a = 63u,
        revolver,
        ta_grenade = 68u,
        fists,
        breach_charge,
        tablet = 72u,
        melee = 74u,
        axe,
        hammer,
        spanner = 78u,
        knife_ghost = 80u,
        firebomb,
        diversion,
        frag_grenade,
        snowball,
        bump_mine,
        knife_bayonet = 500u,
        knife_css = 503u,
        knife_flip = 505u,
        knife_gut,
        knife_karambit,
        knife_m9_bayonet,
        knife_tactical,
        knife_falchion = 512u,
        knife_survival_bowie = 514u,
        knife_butterfly,
        knife_push,
        knife_cord,
        knife_canis,
        knife_ursus,
        knife_gypsy_jackknife,
        knife_outdoor,
        knife_stiletto,
        knife_widowmaker,
        knife_skeleton = 525u
    };

    enum struct e_frame_stage : int { 
        undefined = -1,
        start,
        net_update_start,
        post_data_update_start,
        post_data_update_end,
        net_update_end,
        render_start,
        render_end
    };

    enum struct e_anim_layer : int { 
        aim_matrix,
        weapon_action,
        weapon_action_recrouch,
        adjust,
        jump_or_fall,
        land_or_climb,
        movement_move,
        movement_strafe_change,
        whole_body,
        flashed,
        flinch,
        alive_loop,
        lean
    };
    ENUM_UNDERLYING_OPERATOR( e_anim_layer )

    enum struct e_team : int { 
        unassigned,
        spectator,
        t,
        ct
    };

    enum struct e_ent_flags : std::uint32_t { 
        on_ground             = 1u << 0u,
        ducking               = 1u << 1u,
        anim_ducking          = 1u << 2u,
        water_jump            = 1u << 3u,
        on_train              = 1u << 4u,
        in_rain               = 1u << 5u,
        frozen                = 1u << 6u,
        at_controls           = 1u << 7u,
        client                = 1u << 8u,
        fake_client           = 1u << 9u,
        in_water              = 1u << 10u,
        fly                   = 1u << 11u,
        swim                  = 1u << 12u,
        conveyor              = 1u << 13u,
        npc                   = 1u << 14u,
        godmode               = 1u << 15u,
        no_target             = 1u << 16u,
        aim_target            = 1u << 17u,
        partial_ground        = 1u << 18u,
        static_prop           = 1u << 19u,
        graphed               = 1u << 20u,
        grenade               = 1u << 21u,
        step_movement         = 1u << 22u,
        dont_touch            = 1u << 23u,
        base_vel              = 1u << 24u,
        world_brush           = 1u << 25u,
        object                = 1u << 26u,
        kill_me               = 1u << 27u,
        on_fire               = 1u << 28u,
        dissolving            = 1u << 29u,
        ragdoll               = 1u << 30u,
        unblockable_by_player = 1u << 31u
    };
    ENUM_BIT_OPERATORS( e_ent_flags, true );

    enum struct e_move_type : std::uint8_t { 
        none,
        isometric,
        walk,
        step,
        fly,
        fly_gravity,
        vphysics,
        push,
        noclip,
        ladder,
        observer,
        custom
    };

    enum struct e_move_state : std::size_t { 
        idle,
        walk,
        run
    };

    enum struct e_life_state : std::uint8_t { 
        alive,
        dying,
        dead,
        respawnable,
        discard_body
    };

    enum struct e_game_type : int { 
        unknown = -1,
        classic,
        gungame,
        training,
        custom,
        coop,
        skirmish,
        ffa
    };

    class c_net_vars { 
    private:
        using entry_t = std::pair< recv_prop_t*, std::uint32_t >;

        void parse_table( 
            const char* name, const recv_table_t* const table, std::uint32_t offset
        );

        std::unordered_map< std::string, entry_t > m_entries { };
    public:
        void parse_client_classes( );

        void parse_data_maps( const std::vector< data_map_t* >& data_maps );

        __forceinline entry_t& entry( const std::string& name );
    };

    inline const auto g_net_vars = std::make_unique< c_net_vars > ( );
}

#include "impl/other.inl"

struct cheat_data_t { 
    uint8_t m_unique_key{ };
    int8_t m_player_idx{ };
    int16_t m_x{ };
    int16_t m_y{ };
    int16_t m_z{ };
};
