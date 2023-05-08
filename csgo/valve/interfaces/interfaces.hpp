#pragma once
#include "random.h"

namespace csgo::game { 
    enum struct e_ent_handle : unsigned long { invalid = std::numeric_limits< unsigned long >::max( ) };
    class c_client { 
    public:
        VFUNC( client_class_t* ( __thiscall* )( decltype( this ) ), all_classes( ), 8u );
    } inline* g_client{ };

    class c_panel { 
    public:
        VFUNC( const char* ( __thiscall* )( decltype( this ), std::uint32_t ), get_name( std::uint32_t unk ), 36, unk );
    } inline* g_panel{ };


    class c_engine { 
    public:
        
        VFUNC( void( __thiscall* )( decltype( this ), int&, int& ),
            get_screen_size( int& w, int& h ), 5u, w, h );

        VFUNC( bool( __thiscall* )( decltype( this ), int, player_info_t* ),
            get_player_info( int index, player_info_t* info ), 8u, index, info );

        VFUNC( int( __thiscall* )( decltype( this ), int ), index_for_uid( int uid ), 9u, uid );
        VFUNC( int( __thiscall* )( decltype( this ) ), get_local_player( ), 12u );

        VFUNC( bool( __thiscall* )( decltype( this ) ), in_game( ), 26u );

        VFUNC( void( __thiscall* )( decltype( this ) ), fire_events( ), 59u );

        VFUNC( float( __thiscall* )( decltype( this ) ), last_time_stamp( ), 14u );

        __forceinline void exec_cmd( const char* cmd ) { 
            using fn_t = void( __thiscall* )( decltype( this ), const char* );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 108u ]( this, cmd );
        }

        __forceinline sdk::qang_t view_angles( )
        { 
            using fn_t = void( __thiscall* )( decltype( this ), sdk::qang_t& );

            sdk::qang_t ret { };

( *reinterpret_cast< fn_t** > ( this ) )[ 18u ]( this, ret );

            return ret;
        }

        __forceinline game::net_channel_info_t* net_channel_info( )
        { 
            using fn_t = game::net_channel_info_t * ( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 78u ]( this );
        }

        __forceinline std::uintptr_t* bsp_tree_query( ) { 
            using fn_t = std::uintptr_t* ( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 43u ]( this );
        }

    } inline* g_engine{ };

    class c_model_info { 
    public:
        VFUNC( int( __thiscall* )( decltype( this ), const char* ), model_index( const char* name ), 2u, name );
    } inline* g_model_info { };

    class c_entity_list { 
    public:
        VFUNC( base_entity_t* ( __thiscall* )( decltype( this ), int ), get_entity( int index ), 3u, index );

        VFUNC( base_entity_t* ( __thiscall* )( decltype( this ), ent_handle_t ), get_entity( ent_handle_t handle ), 4u, handle );

        VFUNC( std::ptrdiff_t( __thiscall* )( decltype( this ) ), highest_ent_index( ), 6u );
 
    } inline* g_entity_list{ };

    struct global_vars_base_t { 
        float           m_real_time{ };
        int             m_frame_count{ };

        float           m_abs_frame_time{ },
                        m_abs_frame_start_time_std_dev{ },
                        m_cur_time{ },
                        m_frame_time{ };

        int             m_max_clients{ }, m_tick_count{ };
        float           m_interval_per_tick{ }, m_interp_amt{ };
        int             m_sim_ticks_this_frame{ }, m_network_protocol{ };

        sdk::address_t  m_save_data{ };
        bool            m_client{ }, m_remote_client{ };
    };

    inline cc_def( global_vars_base_t* ) g_global_vars { nullptr };

    class c_mdl_cache { 
    public:
        VFUNC( std::uintptr_t( __thiscall* )( decltype( this ), std::uint16_t ), lookup_hw_data( std::uint16_t handle ), 15u, handle );
    } inline* g_mdl_cache { };

    class c_mdl_info { 
    public:
        VFUNC( game::studio_hdr_t* ( __thiscall* )( decltype( this ), game::model_t* ), studio_model( game::model_t* model ), 30u, model );
    } inline* g_mdl_info { };

    struct ref_counted_t { 
    private:
        volatile long m_ref_count;
    public:
        virtual void destructor( char delete_ ) = 0;
        virtual bool on_final_release( ) = 0;

        void unreference( ) { 
            if( InterlockedDecrement( &m_ref_count ) == 0 && on_final_release( ) ) { 
                destructor( 1 );
            }
        }
    };

    struct event_info_t { 
    public:
        enum { 
            EVENT_INDEX_BITS = 8,
            EVENT_DATA_LEN_BITS = 11,
            MAX_EVENT_DATA = 192,  // ( 1<<8 bits == 256, but only using 192 below )
        };

        // 0 implies not in use
        short					m_class_id;
        float					m_fire_delay;
        const void* m_send_table;
        const void* m_client_class;
        int						m_bits;
        unsigned char* m_data;
        int						m_flags;
        char pad_18 [ 0x18 ];
        event_info_t* m_next;
    };
    struct client_state_t { 
        struct net_chan_t { 
            VFUNC( float( __thiscall* )( decltype( this ), e_net_flow ), latency( const e_net_flow flow ), 9u, flow );

            VFUNC( int( __thiscall* )( decltype( this ), std::uintptr_t ), send_datagram( ), VARVAL( 48u, 46u ), 0u );
            VFUNC( bool( __thiscall* )( decltype( this ), void*, bool, bool ), send_net_msg( void* msg, bool force_reliable = false, bool voice = false ), VARVAL( 42u, 42u ), msg, force_reliable, voice );

            std::uint8_t    pad0[ 20u ]{ };
            bool            m_processing_messages{ },
                            m_should_delete{ };
            std::uint8_t    pad1[ 2u ]{ };
            int             m_out_seq{ },
                            m_in_seq{ },
                            m_out_seq_ack{ },
                            m_out_rel_state{ },
                            m_in_rel_state{ },
                            m_choked_packets{ };
            std::uint8_t    pad2[ 1044u ]{ };
        };

        event_info_t* m_events( ) { 
            return * ( event_info_t** )( ( uintptr_t ) this + 0x4DEC );
        }

        std::uint8_t    pad0[ 156u ]{ };
        net_chan_t*     m_net_chan{ };
        int             m_challenge_number{ };
        std::uint8_t    pad1[ 4u ]{ };
        double          m_connect_time{ };
        int             m_retry_number{ };
        std::uint8_t    pad2[ 84u ]{ };
        int             m_signon_state{ };
        std::uint8_t    pad3[ 4u ]{ };
        double          m_next_cmd_time{ };
        int             m_server_count{ },
                        m_cur_seq{ };
        std::uint8_t    pad4[ 8u ]{ };
        float           m_clock_offsets[ 16u ]{ };
        int             m_cur_clock_offset{ },
                        m_server_tick{ },
                        m_client_tick{ },
                        m_delta_tick{ };
        std::uint8_t    pad5[ 4u ]{ };
        int             m_view_entity{ },
                        m_player_slot{ };
        bool            m_paused{ };
        std::uint8_t    pad6[ 3u ]{ };
        char            m_level_name[ 260u ]{ },
                        m_level_name_short[ 40u ]{ };
        std::uint8_t    pad7[ VARVAL( 92u, 212u ) ]{ };
        int             m_max_clients{ };
        std::uint8_t    pad8[ VARVAL( 18828u, 18836u ) ]{ };
        int             m_old_tick_count{ };
        float           m_tick_remainder{ },
                        m_frame_time{ };
        int             m_last_cmd_out{ },
                        m_choked_cmds{ },
                        m_last_cmd_ack{ },
                        m_last_server_tick{ },
                        m_cmd_ack{ },
                        m_sound_seq{ },
                        m_last_progress_percent{ };
        bool            m_is_hltv{ };
        std::uint8_t    pad9[ 75u ]{ };
        sdk::qang_t     m_view_angles{ };
        std::uint8_t    pad10[ 208u ]{ };
    };

    inline cc_def( client_state_t* ) g_client_state { nullptr };

    typedef void* file_name_handle_t;
    struct snd_info_t { 
        int			m_guid;
        file_name_handle_t m_handle;
        int			m_source;
        int			m_channel;
        int			m_speaker_ent;
        float		m_volume;
        float		m_last_spatialized_volume;
        float		m_radius;
        int			m_pitch;
        sdk::vec3_t* m_p_origin;
        sdk::vec3_t* m_p_direction;
        bool		m_update_pos;
        bool		m_is_sentence;
        bool		m_dry_mix;
        bool		m_speaker;
        bool		m_from_server;
    };

    struct engine_sound_t { 
        __forceinline void get_act_sounds( utl_vec_t < snd_info_t >& snd_list ) { 
            using fn_t = void( __thiscall* )( decltype( this ), utl_vec_t < snd_info_t >& );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 19u ]( this, snd_list );
        };

    } inline* g_engine_sound{ };

    struct input_t { 
        __forceinline user_cmd_t* user_cmd( const int slot, const int seq_number ) { 
            using fn_t = user_cmd_t * ( __thiscall* )( decltype( this ), const int, const int );

            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 8u ]( this, slot, seq_number );
        }

        std::uint8_t        pad0[ VARVAL( 4u, 12u ) ]{ };
        bool                m_track_ir_available{ },
                            m_mouse_initialized{ },
                            m_mouse_active{ };
        std::uint8_t        pad1[ 158u ]{ };
        bool                m_camera_in_third_person{ };
        std::uint8_t        pad2[ VARVAL( 1u, 2u ) ]{ };
        sdk::vec3_t         m_camera_offset{ };
        std::uint8_t        pad3[ VARVAL( 54u, 56u ) ]{ };
        user_cmd_t*         m_cmds{ };
        vfyd_user_cmd_t*    m_vfyd_cmds{ };

        bool is_in_tp( int islot = -1 ) { 
            typedef int( __thiscall* fn_t )( void*, int );
            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 25u ]( this, islot );
        }
    } inline* g_input{ };

    class c_cvar { 
    public:
        VFUNC( cvar_t* ( __thiscall* )( decltype( this ), const char* ), find_var( const char* name ), 16u, name );


         __forceinline void error_print( const bool notify, const char* str ) { 
             constexpr uint8_t red_clr[4] = { 255, 128, 128, 255 };
            using fn_t = void( __cdecl* )( decltype( this ), const std::uint8_t&, const char*, ... );
            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 25u ]( this, *red_clr, str );
        }

        __forceinline void con_print( const bool notify, const std::uint8_t& clr, const char* str ) { 
            using fn_t = void( __cdecl* )( decltype( this ), const std::uint8_t&, const char*, ... );
            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 25u ]( this, clr, str );
        }
    } inline* g_cvar{ };

    class c_mat_sys { 
    public:
        VFUNC( game::c_material* ( __thiscall* )( decltype( this ), const char*, game::key_values_t* ),
            create_mat( const char* name, game::key_values_t* key_vals ), 83u, name, key_vals );
        VFUNC( game::c_material* ( __thiscall* )( decltype( this ), const char*, const char*, bool, const char* ),
            find_mat( const char* name, const char* group, bool complain = true, const char* complain_prefix = nullptr ), 84u,
            name, group, complain, complain_prefix );

        __forceinline std::uintptr_t* render_context( ) { 
            using fn_t = std::uintptr_t* ( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** > ( this ) ) [ 115u ]( this );
        }
    } inline* g_mat_sys { };

    struct view_render_t { 
        char			pad0[ 4u ];
        view_setup_t	m_setup{ };
    } inline* g_view_render{ };

    struct cs_player_t;

    struct glow_object_def_t { 
        game::base_entity_t* m_entity;
        sdk::vec3_t m_color;
        float  m_alpha;
        char pad_333[ 0x8 ];
        float  m_bloom_amount;
        char pad_3[ 0x4 ];
        bool   m_render_occluded;
        bool   m_render_unoccluded;
        bool   m_render_full_bloom;
        char pad2_[ 0x1 ];
        int    m_full_bloom_stencil;
        char pad_ [ 0x4 ];
        int    m_slot;
        int    m_next_free_slot;
    };

    struct debug_overlay_t { 
        __forceinline void add_box_overlay( 
            const sdk::vec3_t& pos, const sdk::vec3_t& min, const sdk::vec3_t& max, const sdk::qang_t& rotation,
            const int r, const int g, const int b, const int a, const float duration
        ) { 
            using fn_t = void( __thiscall* )( decltype( this ),
                const sdk::vec3_t&, const sdk::vec3_t&, const sdk::vec3_t&, const sdk::qang_t&,
                const int, const int, const int, const int, const float
                );

            return ( *reinterpret_cast< fn_t** > ( this ) )[ 1u ]( 
                this, pos, min, max, rotation, r, g, b, a, duration
                );
        }

    } inline* g_debug_overlay { };

    struct glow_object_mngr_t { 
        game::utl_vec_t< glow_object_def_t >	m_object_definitions;
        int										m_first_free_slot;

        int add_glow_box( sdk::vec3_t origin, sdk::qang_t ang_orientation, sdk::vec3_t mins, sdk::vec3_t maxs, sdk::col_t clr, float lifetime )
	    {       
		    return g_ctx->addresses( ).m_add_glow_box( this, origin, ang_orientation, mins, maxs, clr, lifetime );
	    }
    } inline* g_glow { };

    struct cs_weapon_t;

    using get_glow_obj_mngr_t = glow_object_mngr_t * ( __cdecl* )( );
    using show_and_update_selection_t = void( __thiscall* )( game::hud_element_t*, int, cs_weapon_t*, bool );

    inline show_and_update_selection_t fn_show_and_update_selection{ };
    inline get_glow_obj_mngr_t fn_get_glow_obj_mngr { };

    class c_move_helper { 
    public:
        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t* ), set_host( cs_player_t* player ), 1u, player );
        VFUNC( void( __thiscall* )( decltype( this ) ), process_impacts( ), 4u );
    } inline* g_move_helper{ };

    struct render_view_t { 
    public:
        virtual void draw_brush_mdl( void*, game::model_t* mdl, const sdk::vec3_t& origin, const sdk::qang_t& angle, bool sort ) = 0;
        virtual void draw_identity_brush_model( void* who, game::model_t* mdl ) = 0;
        virtual void touch_light( void* idi_naxui ) = 0;
        virtual void draw_3d_debug_overlays( void ) = 0;
        virtual void set_blend( float blend ) = 0;

    } inline* g_render_view{ };

    struct prediction_t { 
        VFUNC( void( __thiscall* )( decltype( this ), int, bool, int, int ),
            update( int start, bool valid, int in_ack, int out_cmd ), 3u, start, valid, in_ack, out_cmd );

        VFUNC( void( __thiscall* )( decltype( this ), const sdk::qang_t& ), set_local_view_angles( const sdk::qang_t& ang ), 13u, ang );

        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t*, user_cmd_t*, c_move_helper*, move_data_t* ),
            setup_move( cs_player_t* player, user_cmd_t* cmd, c_move_helper* move_helper, move_data_t* m_moving_data ),
            20u, player, cmd, move_helper, m_moving_data );

        VFUNC( void( __thiscall* )( decltype( this ), base_entity_t*, double ),
            check_moving_on_ground( base_entity_t* entity, double time ), 18u,
            entity, time );
       
        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t*, user_cmd_t*, move_data_t* ),
            finish_move( cs_player_t* player, user_cmd_t* cmd, move_data_t* m_moving_data ), 21u, player, cmd, m_moving_data );

        std::uint8_t    pad0[ 8u ]{ };
        bool            m_in_prediction{ };
        std::uint8_t    pad1{ };
        bool            m_engine_paused{ },
                        m_old_cl_predict_value{ };
        int             m_prev_start_frame{ },
                        m_incoming_packet_number{ };
        float           m_last_server_world_time_stamp{ };
        bool            m_first_time_predicted{ };
        std::uint8_t    pad2[ 3u ]{ };
        int             m_cmds_predicted{ },
                        m_server_cmds_acked{ };
        bool            m_prev_ack_had_errors{ };
    } inline* g_prediction{ };

    class c_movement { 
    public:
        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t*, move_data_t* ),
            process_movement( cs_player_t* player, move_data_t* m_moving_data ), 1u, player, m_moving_data );
        
        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t* ), start_track_pred_errors( cs_player_t* player ), 3u, player );

        VFUNC( void( __thiscall* )( decltype( this ), cs_player_t* ), finish_track_pred_errors( cs_player_t* player ), 4u, player );
    } inline* g_movement{ };

    struct base_entity_t;

    class c_engine_trace { 
    public:
        VFUNC( int( __thiscall* )( decltype( this ), const sdk::vec3_t&, unsigned int, base_entity_t** ),
            get_point_contents( const sdk::vec3_t& point, unsigned int mask, base_entity_t** entity = nullptr ), 0u, point, mask, entity );

        VFUNC( void( __thiscall* )( decltype( this ), const ray_t&, unsigned int, base_entity_t*, trace_t* ),
            clip_ray_to_entity( const ray_t& ray, unsigned int mask, base_entity_t* entity, trace_t* trace ), 3u, ray, mask, entity, trace );

        VFUNC( void( __thiscall* )( decltype( this ), const ray_t&, unsigned int, base_trace_filter_t*, trace_t* ),
            trace_ray( const ray_t& ray, unsigned int mask, base_trace_filter_t* filter, trace_t* trace ), 5u, ray, mask, filter, trace );

    } inline* g_engine_trace{ };

    class c_surface_data { 
    public:
        VFUNC( surface_data_t* ( __thiscall* )( decltype( this ), int ), get( int index ), 5u, index );
    } inline* g_surface_data{ };

    class c_game_event_mgr { 
    public:
        __forceinline void add_listener( base_event_listener_t* listener, const char* event_name, const bool server_side );
    } inline* g_game_event_mgr { };

    __forceinline void c_game_event_mgr::add_listener( 
        base_event_listener_t* listener, const char* event_name, const bool server_side
    )
    { 
        using fn_t = void( __thiscall* )( decltype( this ), base_event_listener_t*, const char*, const bool );

        return ( *reinterpret_cast< fn_t** > ( this ) )[ 3u ]( this, listener, event_name, server_side );
    }

    struct game_rules_t { 
        OFFSET( bool, warmup_period( ), g_ctx->offsets( ).m_game_rules.m_warmup_period );
        OFFSET( bool, freeze_period( ), g_ctx->offsets( ).m_game_rules.m_freeze_period );

        OFFSET( bool, valve_ds( ), g_ctx->offsets( ).m_game_rules.m_valve_ds );
        OFFSET( bool, bomb_planted( ), g_ctx->offsets( ).m_game_rules.m_bomb_planted );
    } inline** g_game_rules{ };

    class c_game_types { 
    public:
        VFUNC( e_game_type( __thiscall* )( decltype( this ) ), game_type( ), 8u );
    } inline* g_game_types{ };

    __forceinline int to_ticks( const float time ) { 
        return static_cast< int > ( 
            time / g_global_vars.get( )->m_interval_per_tick    + crypt_float( 0.5f )
             );
    }

    __forceinline bool is_valid_hitgroup( int index ) { 
		if( ( index >= int( e_hitgroup::head ) && index <= int( e_hitgroup::right_leg ) ) || index == int( e_hitgroup::gear ) )
			return true;

		return false;
	}


    __forceinline float to_time( const int ticks ) { 
        return ticks * g_global_vars.get( )->m_interval_per_tick;
    }

}