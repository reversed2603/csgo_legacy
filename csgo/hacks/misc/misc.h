#pragma once
namespace csgo::hacks { 
    class c_misc { 
    private:
        struct cfg_t {    
            bool m_clan_tag { }, g_reset_tag { };
            int m_prev_tag { };
            bool m_aspect_ratio { }, m_spectators { }, m_key_binds { };
            s_keybind m_third_person_key { };
            float   m_third_person_dist { 120.f };
            float m_aspect_ratio_value { 1.f };
            bool      m_force_thirdperson_dead { }, m_hit_marker_sound { };
            int       m_camera_distance { 90 }, m_hit_marker_sound_val { };
            int       m_view_model_x{ }, m_view_model_y{ }, m_view_model_z{ };
            bool      m_buy_bot{ }, m_kill_feed{ }, m_filter_console{ };
            int       m_buy_bot_snipers{ }, m_buy_bot_pistols{ }, m_buy_bot_additional{ };
        };

        sdk::cfg_var_t< cfg_t > m_cfg { 0x07232e31u, { } };

    public:
        struct {    
            ImFont* m_tahoma14;
            ImFont* m_verdana;
            ImFont* m_icon_font;
            ImFont* m_warning_icon_font;
            ImFont* m_log_font;
            ImFont* m_log_icons, *m_xiaomi { }, *m_museo_700 { }, * m_museo_700_35{ };
            ImFont* m_museo_sans_name;
            ImFont* m_font_for_fkin_name{ }, * m_smallest_pixel{ };
            ImFont* m_muli_regular{ }, * m_roboto{ }, * m_josefin_sans{ }, * m_verdana_main{ };
            IDirect3DTexture9* m_robot_texture{ };
            std::string m_last_font{ };

            struct esp_t { 
                ImFont* m_verdana{ };
                ImFont* m_04b{ }, *m_idk{ };
                ImFont* m_visitor_tt2_brk{ }, *m_icon_font { };
            } m_esp { };

        }m_fonts { };


        void clan_tag( ) const;
        void third_person( );
        void draw_spectators( );
        void draw_watermark( );
        void buy_bot( );
        void kill_feed( );
        __forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
    };

    class c_ping_spike { 
    protected:
        struct cfg_t { 
            s_keybind m_ping_spike_key { };
            float m_to_spike { 200.f };
        };

        sdk::cfg_var_t< cfg_t > m_cfg { 0x05562e31u, { } };


    public:

        __forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
    };

    class c_skins { 
    private:
        struct cfg_t { 
            int m_knife_type{ }, m_glove_type{ };
            int m_cur_skin_deagle{ }, m_cur_skin_elite{ }, m_cur_skin_five_seven{ }, m_cur_skin_glock{ }, m_cur_skin_ak47{ }, m_cur_skin_aug{ }, m_cur_skin_awp{ }, m_cur_skin_famas{ }, m_cur_skin_g3sg1{ },
                m_cur_skin_galil{ }, m_cur_skin_m249{ }, m_cur_skin_m4a4{ }, m_cur_skin_mac10{ }, m_cur_skin_p90{ }, m_cur_skin_ump45{ }, m_cur_skin_xm1014{ }, m_cur_skin_bizon{ }, m_cur_skin_mag7{ },
                m_cur_skin_negev{ }, m_cur_skin_sawedoff{ }, m_cur_skin_tec9{ }, m_cur_skin_p2000{ }, m_cur_skin_mp7{ }, m_cur_skin_mp9{ }, m_cur_skin_nova{ }, m_cur_skin_p250{ }, m_cur_skin_scar20{ },
                m_cur_skin_sg553{ }, m_cur_skin_ssg08{ }, m_cur_skin_m4_s{ }, m_cur_skin_usp_s{ }, m_cur_skin_cz75a{ }, m_cur_skin_revolver{ }, m_cur_skin_knifes { };
        };
        sdk::cfg_var_t< cfg_t > m_cfg{ 0x04262e35u, { } };

        std::array < int, 525 > m_last_index{ };
        bool m_needs_update{ };

        int get_knife_index( );

        // const char* get_world_mdl_str( );
        std::string get_model_str( );
        std::string get_world_mdl_str( );

        int get_skin( game::cs_weapon_t* weapon );

        std::string get_glove_model( );
        std::string  get_killicon_str( );

        float m_update_time{ };
        bool m_update{ };

        void override_gloves( );
        void override_weapon( game::cs_weapon_t* wep, std::vector < game::cs_weapon_t* >& wpns );

        int correct_skin_idx( int cur_idx );

        int get_current_weapon_id( );
        int get_weapon_id( game::cs_weapon_t* );
        void fix_seq( game::base_entity_t* entity );
        std::array < int, 525 > m_skins{ };
    public:
        void handle_ctx( );
        void replace_deaths( game::game_event_t* event );
        __forceinline cfg_t& cfg( ) { return m_cfg.value( ); };
    };

    inline const auto g_skins = std::make_unique < c_skins > ( );
    inline const auto g_ping_spike = std::make_unique < c_ping_spike > ( );
    inline const auto g_misc = std::make_unique< c_misc > ( );
}