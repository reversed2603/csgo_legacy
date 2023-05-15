#pragma once
#define CSGO2018 
namespace csgo { 
    namespace game { 
        struct cvar_t;

        struct recv_prop_t;

        struct cs_player_t;

        struct kill_feed_t;
    }

    class c_ctx { 
    public:
        using modules_t = std::unordered_map< sdk::hash_t, sdk::x86_pe_image_t* >;

        using interfaces_t = std::unordered_map< sdk::hash_t, sdk::address_t >;

        struct ent_offset_t { 
            game::recv_prop_t* m_prop{ };
            std::uint32_t       m_offset{ };
        };

        using ent_offsets_t = std::unordered_map< sdk::hash_t, ent_offset_t >;

        bool wait_for_all_modules( modules_t& modules ) const;

        void init_imgui( const modules_t& modules ) const;

        void parse_interfaces( sdk::x86_pe_image_t* const image, interfaces_t& interfaces ) const;

        void init_interfaces( const modules_t& modules ) const;

        bool parse_ent_offsets( ent_offsets_t& offsets, const modules_t& modules ) const;

        void init_offsets( const modules_t& modules );

        void init_cvars( );

        void init_addresses( const modules_t& modules );

        void init_hooks( const modules_t& modules ) const;

        struct { 
            sdk::address_t  m_local_player { },
                m_weapon_system { },
                m_user_cmd_checksum { };

            struct { 
                sdk::address_t m_reset { }, m_update { };
            }               m_anim_state { };

            struct { 
                std::uint32_t m_bone_cache { }, m_mdl_bone_cnt { };
            }               m_renderable { };

            struct { 
                std::uint32_t   m_health { }, m_team_num { },
                    m_sim_time { }, m_flags { }, 
                    m_origin { }, m_owner_ent{ }, m_velocity { },
                    m_abs_origin { }, m_abs_velocity { }, m_effects { },
                    m_abs_rotation{ }, m_anim_time{ }, m_spotted{ }, m_model_idx{ }, m_move_type{ }, m_rgfl{ }, m_precip_type{ },
                    m_mins { }, m_maxs { };
            }               m_base_entity { }, m_precipitation{ };

            struct { 
                std::uint32_t m_ping{ };
            } m_player_resource { };

            struct { 
                std::uint32_t   m_sequence { },
                    m_hitbox_set_index { }, m_studio_hdr { }, m_lock_std_hdr { }, m_set_abs_ang { },
                    m_pose_params { }, m_cycle { }, m_anim_layers { }, m_flash_dur { }, m_bone_accessor { }, m_last_bone_setup_time { },
                    m_most_recent_model_bone_counter { }, m_force_bone { }, m_b_cl_side_anim { }, m_thrower_handle { };
            }               m_base_animating { };

            struct { 
                std::uint32_t   m_pin_pulled { },
                    m_throw_time { }, m_throw_strength { }, m_smoke_effect_tick_begin { },
                    m_did_smoke_effect { };
            }               m_base_grenade { };

            struct { 
                std::uint32_t   m_item_index{ }, m_item_id_high{ }, m_entity_quality{ }, m_account_id{ }, m_fallback_paint_kit{ }, m_fallback_seed{ }, m_fallback_wear{ }, m_fallback_starttrak { },
                                m_orig_owner_xuid_low { };
            }               m_base_attributable_item { };

            struct { 
                std::uint32_t m_shadow_dir{ };
            } m_cascade_light { };

            struct { 
                std::uint32_t   m_clip1{ }, m_primary_reserve_ammo_count{ }, m_item{ }, m_world_dropped_mdl_idx{ }, m_world_mdl_idx{ }, m_view_mdl_idx{ },
                    m_next_primary_attack { }, m_next_secondary_attack { };
            }               m_base_weapon { };

            struct { 
                std::uint32_t   m_burst_mode { },
                    m_last_shot_time { }, m_recoil_index { }, m_accuracy_penalty { }, m_wpn_world_mdl_handle { },
                    m_postpone_fire_ready_time { };
            }               m_weapon_cs_base { };

            struct { 
                std::uint32_t   m_zoom_lvl { },
                    m_burst_shots_remaining { }, m_next_burst_shot { };
            }               m_weapon_cs_base_gun { };

            struct { 
                std::uint32_t   m_weapon_handle { }, m_next_attack { };
            }               m_base_combat_character { };

            struct { 
                std::uint32_t   m_tick_base { }, m_life_state { }, m_observer_mode { }, m_observer_target { }, m_armor_val, m_has_heavy_armor { }, m_has_helmet { }, m_money { },
                    m_duck_amt { }, m_duck_speed { }, m_ground_entity { }, m_skin { }, m_body { },
                    m_spawn_time { }, m_aim_punch { }, m_next_attack { },
                    m_view_punch { }, m_view_offset { }, m_aim_punch_vel { }, m_surface_friction { }, m_fall_velocity { }, m_base_velocity { }, m_model_idx { }, m_max_player_speed { };
            }               m_base_player { };

            struct { 
                std::uint32_t   m_lby { }, m_eye_angles { }, m_scoped { }, m_view_model { }, m_shots_fired { },
                    m_walking { }, m_strafing { }, m_move_state { }, m_collision_state { }, m_wait_for_no_attack { },
#ifndef CSGO2018
                    m_survival_team { },
#endif
                    m_anim_state { }, m_velocity_modifier { }, m_defusing { }, m_is_jiggle_bones_enabled{ }, m_third_person_recoil { };
            }               m_cs_player { };    

            struct { 
                std::uint32_t   m_warmup_period { }, m_freeze_period { },
                    m_valve_ds { }, m_bomb_planted { };
            }               m_game_rules { };

            struct { 
                std::uint32_t  m_vec_force{ }, m_sequence { }, m_animation_parity { };
            }               m_c_base_view_model { }, m_ent_ragdoll{ };

            struct { 
                std::uint32_t m_weapon_handle{ };
            } m_predicted_view_model { };

            struct { 
                std::uint32_t m_use_custom_bloom_scale{ }, m_user_custom_auto_exposure_min{ }, m_user_custom_auto_exposure_max{ }, m_custom_bloom_scale{ }, m_auto_custom_exposure_min{ }, m_auto_custom_exposure_max{ };
            }m_tone_map { };

        } m_offsets{ };

        struct { 
            game::cvar_t* m_cl_forwardspeed { },
                        * m_cl_sidespeed { }, * m_cl_upspeed { }, * m_cl_interp{ },
                        * m_cl_interp_ratio{ }, * m_cl_updaterate{ }, * m_sv_maxunlag { },
                        * m_cl_pitchdown { }, * m_cl_pitchup { },
                        * m_mp_teammates_are_enemies { }, * m_sv_maxvelocity { },
                        * m_sv_friction { }, * m_sv_accelerate { }, *m_weapon_accuracy_shotgun_spread_patterns { },
                        * m_sv_accelerate_use_weapon_speed { },
                        * viewmodel_offset_x{ }, *viewmodel_offset_y{ }, *viewmodel_offset_z{ }, *m_rain_radius{ };
        } m_cvars{ };

        struct addresses_t { 

            using angle_matrix_t = void( __fastcall* )( const sdk::qang_t&, sdk::mat3x4_t& );
            using clear_notices_t = void( __thiscall* )( game::kill_feed_t* );
            using random_float_t = float( __cdecl* )( const float, const float );
            using random_int_t = int( __cdecl* )( const int, const int );
            using random_seed_t = void( __cdecl* )( const int );
            using cl_read_packets_t = void( __cdecl* )( bool final_tick );
            using add_glow_box_t = int( __thiscall* )( void*, sdk::vec3_t, sdk::qang_t, sdk::vec3_t, sdk::vec3_t, sdk::col_t, float );

            random_float_t		m_random_float { };
            random_int_t	    m_random_int { };
            cl_read_packets_t   m_cl_read_packets { };
            random_seed_t	    m_random_seed { };
            clear_notices_t     m_clear_notices{ };
            std::uintptr_t m_v_matrix{ }, m_clip_ray{ }, m_set_clan_tag{ }, m_ret_to_extrapolation{ }, m_upd_visb_entities{ }, m_calc_shotgun_spread{ }, m_compute_hitbox_surround_box{ }, m_lookup_seq_act{ }, m_smoke_count{ }, m_post_process{ },
                m_sky_box { }, m_set_abs_ang { }, m_set_abs_origin { }, m_invalidate_bone_cache { }, m_lookup_bone{ }, m_attachment_helper { }, m_set_collision_bounds { }, m_ret_insert_into_tree { },
                m_is_breakable{ }, m_trace_filter_simple_vtable{ }, m_trace_filter_skip_two_entities_vtable{ }, m_ret_to_accumulate_layers{ }, m_write_user_cmd{ }, m_allow_to_extrapolate{ }, m_ret_to_setup_velocity{ },
                m_nigga_who{ }, m_ret_to_allah{ }, m_unlock_packets{ }, m_invalidate_physics_recursive{ }, m_cull_check{ }, m_voice_msg_ctor{ };
            std::uintptr_t* m_ret_to_check_receiving_list{ }, * m_ret_to_read_sub_channel_data_return{ };

            add_glow_box_t m_add_glow_box{ };

            int* m_pred_seed { };
            angle_matrix_t 	m_angle_matrix { };
        public:
            game::cs_player_t* m_pred_player { };
        } m_addresses;

        struct anim_data_t { 
            bool m_allow_update { };
            bool m_allow_setup_bones { };

            struct local_data_t { 
                float m_anim_frame { };
                float m_anim_time { };
                float m_lby { }, m_lby_upd { }, m_last_shot_time { }, m_processed_yaw { }, m_max_body_yaw { }, m_min_body_yaw { }, m_speed_as_portion_walk { }, m_speed_as_portion_crouch { }, m_walk_trans { };
                float m_speed_2d { };
                sdk::qang_t m_anim_ang { };
                bool m_upd_cycle{ }, m_on_ground{ }, m_old_shot{ }, m_shot{ }, m_old_packet{ }, m_can_break{ };
                float m_abs_ang { };
                std::array < sdk::mat3x4_t, 256 > m_bones { };
                std::array < sdk::vec3_t, 256 > m_bone_origins { };
                int					m_shot_cmd_number{ };
                bool m_shot_valid_wpn{ };
                float m_spawn_time{ };

                struct anim_event_t { 
                    std::uint8_t m_move_type{ };
                    std::uint32_t m_flags{ };
                } m_anim_event { };

                __forceinline void reset( ) { 
                    m_bones = { };
                    m_bone_origins = { };
                    m_lby_upd = 0.f;
                    m_lby = 0.f;
                    m_spawn_time = 0.f;
                    m_anim_event.m_move_type = { };
                    m_anim_event.m_flags = { };
                }

            } m_local_data { };

        } m_anim_data;

        struct s_auto_peek_data
        { 
            sdk::vec3_t m_start_pos;
            bool m_is_firing;

        }m_apeek_data { };

        struct net_info_t { 
            float	m_lerp{ };

            struct { 
                float m_in{ }, m_out{ };
            }		m_latency{ };
        } m_net_info{ };

        struct out_cmds_data_t
        { 
            int m_command_nr;
            int m_prev_command_nr;

            bool m_is_outgoing;
            bool m_is_used;
        } m_out_cmds;

        struct ticks_data_t { 
            float m_tick_rate{ };
            float m_cl_tick_count{ };
        } m_ticks_data { };

        bool m_send_packet{ }, m_frame_droped { }, m_left_create_move { };
        int  m_buy_bot{ };

        std::deque < out_cmds_data_t > m_out_cmds_data { };

        bool m_is_out_of_epsilon { };
        bool m_can_shoot { };
        bool m_was_shooting { };
        bool m_defensive_allowed { }, m_local_is_peeking { };
        int m_cur_cfg_slot{ };
        sdk::vec3_t m_shoot_pos { }, m_aim_shoot_pos { };
    public:

        sdk::vec3_t m_pre_autostop_move;

        void init( );

        __forceinline const auto& offsets( ) const;

        __forceinline const auto& cvars( ) const;

        __forceinline auto& addresses( );

        __forceinline auto& ticks_data( );

        __forceinline auto& cur_cfg_slot( );

        __forceinline auto& left_create_move( );

        __forceinline auto& frame_droped( );

        __forceinline auto& is_out_of_epsilon( );

        __forceinline auto& send_packet( );

        __forceinline auto& can_shoot( );

        __forceinline auto& anim_data( );

        __forceinline auto& buy_bot( );

        __forceinline auto& was_shooting( );
        
        __forceinline auto& shoot_pos( );

        __forceinline auto& aim_shoot_pos( );

        __forceinline auto& local_is_peeking( );

        __forceinline net_info_t& net_info( );

        __forceinline std::deque<out_cmds_data_t>& get_out_cmds( );

        __forceinline s_auto_peek_data& get_auto_peek_info( );

        __forceinline auto& defensive_allowed( );
    };

    inline std::unique_ptr < c_ctx > g_ctx = std::make_unique< c_ctx > ( );
}

    struct s_keybind { 
        s_keybind( )
        { 
            m_code = 0;
            m_mode = 0;
            m_is_binding = false;
        }

        s_keybind( char code, int mode )
        { 
            m_code = code;
            m_mode = mode;
            m_is_binding = false;
        }

        char m_code;
        int m_mode;
        bool m_is_binding;
    };

    class c_key_binds { 
    public:

        bool get_keybind_state( const s_keybind* bind );
        void set_keybind_state( s_keybind* bind, int val );
        int get_keybind_mode( const s_keybind* bind );
        void add_keybind( const char* label, s_keybind* keybind, bool is_manual = false, int x_bind = 300 );
    

    public:
        char m_last_code = NULL;
    };

    inline std::unique_ptr < c_key_binds > g_key_binds = std::make_unique < c_key_binds > ( );
#include "impl/ctx.inl"