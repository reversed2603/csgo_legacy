#pragma once

namespace csgo::valve {
    struct renderable_t {
        VFUNC( bool( __thiscall* )( decltype( this ), sdk::mat3x4_t*, int, int, float ),
            setup_bones( sdk::mat3x4_t* bones, int max_bones, int mask, float time ), 13u, bones, max_bones, mask, time );
        VFUNC( valve::model_t* ( __thiscall* )( decltype( this ) ), model( ), 8u );
        VFUNC( std::uint16_t( __thiscall* )( decltype( this ) ), mdl_instance( ), 30u );
    };

    struct networkable_t {

        VFUNC( valve::client_class_t* ( __thiscall* )( decltype( this ) ), client_class( ), 2u );

        VFUNC( bool( __thiscall* )( decltype( this ) ), dormant( ), 9u );

        VFUNC( int( __thiscall* )( decltype( this ) ), index( ), 10u );
    };

    struct base_entity_t {
        POFFSET( renderable_t, renderable( ), sizeof( sdk::address_t ) );
        POFFSET( networkable_t, networkable( ), sizeof( sdk::address_t ) * 2u );

        VFUNC( const sdk::vec3_t&( __thiscall* )( decltype( this ) ), world_space_center( ), 78u );

        OFFSET( float, anim_time( ), g_ctx->offsets( ).m_base_entity.m_anim_time );

        OFFSET( int, model_idx( ), g_ctx->offsets( ).m_base_player.m_model_idx );

        __forceinline bool is_weapon( ) {
            using fn_t = bool( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) )[ 161u ]( this );
        }

        OFFSET( float, sim_time( ), g_ctx->offsets( ).m_base_entity.m_sim_time );
        OFFSET( float, old_sim_time( ), g_ctx->offsets( ).m_base_entity.m_sim_time + sizeof( float ) );
        OFFSET( int, sequence( ), g_ctx->offsets( ).m_base_animating.m_sequence );

        __forceinline var_mapping_t& var_mapping( ) {
            return *reinterpret_cast< var_mapping_t* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x24u
                );
        }

        OFFSET( int, hitbox_set_index( ), g_ctx->offsets( ).m_base_animating.m_hitbox_set_index );
        OFFSET( studio_hdr_t*, studio_hdr( ), g_ctx->offsets( ).m_base_animating.m_studio_hdr );
        OFFSET( float, cycle( ), g_ctx->offsets( ).m_base_animating.m_cycle );

        __forceinline void set_collision_bounds( 
            const  sdk::vec3_t& obb_min, const  sdk::vec3_t& obb_max
        ) {
            using collideable_fn_t = std::uintptr_t( __thiscall* )( decltype( this ) );

            const auto collideable = ( *reinterpret_cast< collideable_fn_t** >( this ) ) [ 3u ]( this );
            if( !collideable )
                return;

            if( !this )
                return;

            using fn_t = void( __thiscall* )( const std::uintptr_t, const sdk::vec3_t&, const  sdk::vec3_t& );

            reinterpret_cast< fn_t >( 
                g_ctx->addresses( ).m_set_collision_bounds
                )( collideable, obb_min, obb_max );
        }

        float& get_ent_spawn_time( ) {
            return *reinterpret_cast< float* >( reinterpret_cast< std::uintptr_t >( this ) + 0x29b0 );
        }

        OFFSET( pose_params_t, pose_params( ), g_ctx->offsets( ).m_base_animating.m_pose_params );
        OFFSET( bone_accessor_t, bone_accessor( ), g_ctx->offsets( ).m_base_animating.m_bone_accessor );
        OFFSET( bool, client_side_anim_proxy( ), g_ctx->offsets( ).m_base_animating.m_b_cl_side_anim );

        OFFSET( ent_handle_t, thrower_handle( ), g_ctx->offsets( ).m_base_animating.m_thrower_handle );

        OFFSET( float, last_setup_bones_time( ), g_ctx->offsets( ).m_base_animating.m_last_bone_setup_time );

        OFFSET( e_ent_flags, flags( ), g_ctx->offsets( ).m_base_entity.m_flags );

        __forceinline bool is_player( )
        {
            using fn_t = bool( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 152u ]( this );
        }

        __forceinline bool is_base_combat_wpn( )
        {
            using fn_t = bool( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) )[ 160u ]( this );
        }

        __forceinline void set_abs_origin( const sdk::vec3_t& abs_origin ) {
            using fn_t = void( __thiscall* )( decltype( this ), const sdk::vec3_t& );

            return reinterpret_cast< fn_t >( 
                g_ctx->addresses( ).m_set_abs_origin
                )( this, abs_origin );
        }

        OFFSET( bool, pin_pulled( ), g_ctx->offsets( ).m_base_grenade.m_pin_pulled );

        OFFSET( sdk::vec3_t, origin( ), g_ctx->offsets( ).m_base_entity.m_origin );
        OFFSET( sdk::vec3_t, velocity( ), g_ctx->offsets( ).m_base_entity.m_velocity );

        __forceinline void invalidate_bone_cache( ) {
            bone_accessor_t* accessor = &bone_accessor( );
            if( !accessor )
                return;

            accessor->m_writable_bones = 0;
            accessor->m_readable_bones = 0;

            mdl_bone_cnt( ) = 0;
            last_setup_bones_time( ) = std::numeric_limits< float >::lowest( );
        }

        void*& ik( ) {
            return * ( void** )( uintptr_t( this ) + ( g_ctx->offsets( ).m_base_animating.m_force_bone - 0x1C ) );
        }

        int& prev_bone_mask( ) {
            return * ( int* )( std::uintptr_t( this ) + ( g_ctx->offsets( ).m_base_animating.m_force_bone + 0x10 ) );
        }

        int& accumulated_bone_mask( ) {
            return * ( int* )( std::uintptr_t( this ) + ( g_ctx->offsets( ).m_base_animating.m_force_bone + 0x14 ) );
        }

        __forceinline int& last_non_skipped_frame( ) {
            return * ( int* )( uintptr_t( this ) + 0xA68 );
        }

        __forceinline int& custom_blend_rule_mask( ) {
            return * ( int* )( uintptr_t( this ) + 0xA24 );
        }

        __forceinline int& anim_lod_flags( ) {
            return * ( int* )( uintptr_t( this ) + 0xA28 );
        }

        __forceinline int& anim_occlusion_frame_count( ) {
            return * ( int* )( uintptr_t( this ) + 0xA30 );
        }

        __forceinline int& client_effects( ) {
            return * ( int* )( uintptr_t( this ) + 0x68 );
        }

        __forceinline int& effects_via_offset( ) {
            return * ( int* )( uintptr_t( this ) + 0xEC );
        }

        VFUNC( sdk::qang_t&( __thiscall* )( decltype( this ) ), abs_ang( ), 11u );

        __forceinline void set_abs_ang( const sdk::qang_t& abs_angles ) {
            using fn_t = void( __thiscall* )( decltype( this ), const sdk::qang_t& );

            return reinterpret_cast< fn_t >( 
                g_ctx->addresses( ).m_set_abs_ang
                )( this, abs_angles );
        }

        OFFSET( sdk::vec3_t, abs_origin( ), g_ctx->offsets( ).m_base_entity.m_abs_origin );
        OFFSET( sdk::vec3_t, abs_velocity( ), g_ctx->offsets( ).m_base_entity.m_abs_velocity );
        OFFSET( sdk::qang_t, abs_rotation( ), g_ctx->offsets( ).m_base_entity.m_abs_rotation );

        OFFSET( std::uint32_t, effects( ), g_ctx->offsets( ).m_base_entity.m_effects );

        OFFSET( bool, is_jiggle_bones_enabled( ), g_ctx->offsets( ).m_cs_player.m_is_jiggle_bones_enabled );

        OFFSET( e_move_type, move_type( ), g_ctx->offsets( ).m_base_entity.m_move_type );

        OFFSET( sdk::mat3x4_t, rgfl( ), g_ctx->offsets( ).m_base_entity.m_rgfl );

        OFFSET( sdk::vec3_t, obb_min( ), g_ctx->offsets( ).m_base_entity.m_mins );
        OFFSET( sdk::vec3_t, obb_max( ), g_ctx->offsets( ).m_base_entity.m_maxs );


        __forceinline valve::data_map_t* get_pred_desc_map( )
        {
            using fn_t = valve::data_map_t* ( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) )[ 17u ]( this );
        }

        __forceinline valve::type_desc_t* get_data_map_entry( valve::data_map_t* map, const char* name )
        {
            while( map )
            {
                for( int i = 0; i < map->m_size; i++ )
                {
                    if( map->m_descriptions[ i ].m_name == nullptr )
                        continue;

                    if( strcmp( name, map->m_descriptions[ i ].m_name ) == 0 )
                        return &map->m_descriptions[ i ];
                }
                map = map->m_base_map;
            }

            return 0;
        }

        __forceinline utl_vec_t< sdk::mat3x4_t >& bone_cache( ) {
            return *reinterpret_cast< utl_vec_t< sdk::mat3x4_t >* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x2900u
                );
        }

        OFFSET( sdk::ulong_t, mdl_bone_cnt( ), g_ctx->offsets( ).m_renderable.m_mdl_bone_cnt + sizeof( std::uintptr_t ) );
    };

    struct cascade_light_t : public base_entity_t {
        OFFSET( sdk::vec3_t, shadow_dir( ), g_ctx->offsets( ).m_cascade_light.m_shadow_dir );
    };
 
    struct inferno_t : public base_entity_t {
        __forceinline float& get_spawn_time( ) {
            return *reinterpret_cast< float* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x20u
                );
        }

        static float get_expiry_time( )
        {
            return 7.03125f;
        }
    };

    struct smoke_t : public base_entity_t {
        OFFSET( std::ptrdiff_t, smoke_effect_tick_begin( ), g_ctx->offsets( ).m_base_grenade.m_smoke_effect_tick_begin );
        OFFSET( bool, did_smoke_effect( ), g_ctx->offsets( ).m_base_grenade.m_did_smoke_effect );

        static float get_expiry_time( ) {
            return 19.0f;
        }
    };

    struct cs_weapon_t : public base_entity_t {
        OFFSET( float, next_primary_attack( ), g_ctx->offsets( ).m_base_weapon.m_next_primary_attack );
        OFFSET( float, next_secondary_attack( ), g_ctx->offsets( ).m_base_weapon.m_next_secondary_attack );
        OFFSET( float, throw_time( ), g_ctx->offsets( ).m_base_grenade.m_throw_time );
        OFFSET( float, throw_strength( ), g_ctx->offsets( ).m_base_grenade.m_throw_strength );
        OFFSET( e_item_index, item_index( ), g_ctx->offsets( ).m_base_attributable_item.m_item_index );
        OFFSET( int, fallback_paint_kit( ), g_ctx->offsets( ).m_base_attributable_item.m_fallback_paint_kit );
        OFFSET( int, fallback_seed( ), g_ctx->offsets( ).m_base_attributable_item.m_fallback_seed );
        OFFSET( float, fallback_wear( ), g_ctx->offsets( ).m_base_attributable_item.m_fallback_wear );
        OFFSET( int, fallback_stattrak( ), g_ctx->offsets( ).m_base_attributable_item.m_fallback_starttrak );
        OFFSET( int, orig_owner_xuid_low( ), g_ctx->offsets( ).m_base_attributable_item.m_orig_owner_xuid_low );
        OFFSET( int, entity_quality( ), g_ctx->offsets( ).m_base_attributable_item.m_entity_quality );
        OFFSET( int, account_id( ), g_ctx->offsets( ).m_base_attributable_item.m_account_id );
        OFFSET( int, item_id_high( ), g_ctx->offsets( ).m_base_attributable_item.m_item_id_high );
        OFFSET( bool, burst_mode( ), g_ctx->offsets( ).m_weapon_cs_base.m_burst_mode );
        OFFSET( float, last_shot_time( ), g_ctx->offsets( ).m_weapon_cs_base.m_last_shot_time );
        OFFSET( ent_handle_t, wpn_world_mdl( ), g_ctx->offsets( ).m_weapon_cs_base.m_wpn_world_mdl_handle );
        OFFSET( ent_handle_t, wpn_from_viewmodel_handle( ), g_ctx->offsets( ).m_predicted_view_model.m_weapon_handle );

        __forceinline  valve::utl_vec_t < valve::ref_counted_t* >& custom_materials_2( ) {
            return *reinterpret_cast< valve::utl_vec_t < valve::ref_counted_t* >* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x9dcu
                );
        }

        OFFSET( valve::utl_vec_t < valve::ref_counted_t* >, custom_materials( ), g_ctx->offsets( ).m_base_weapon.m_item + 0x14 );
        OFFSET( valve::utl_vec_t < valve::ref_counted_t* >, visual_data_processors( ), g_ctx->offsets( ).m_base_weapon.m_item + 0x220 );

        __forceinline bool& custom_material_inited( ) {
        return *reinterpret_cast< bool* >( 
            reinterpret_cast< std::uintptr_t >( this ) + 0x32ddu
            );
        }

        __forceinline cs_weapon_t* get_wpn( ) {
            return static_cast < cs_weapon_t* >( valve::g_entity_list->get_entity( wpn_from_viewmodel_handle( ) ) );
        }

        __forceinline weapon_info_t* info( );

        __forceinline float max_speed( bool scoped ) {
            float max_speed{ 260.f };

            if( info( ) ) {
                max_speed = scoped ? info( )->m_max_speed_alt : info( )->m_max_speed;
            }

            return max_speed;
        }

        __forceinline bool is_knife( )
        {
            if( !this )
                return false;

            auto idx = item_index( );

            return idx == e_item_index::knife_ct || idx == e_item_index::knife_bayonet || idx == e_item_index::knife_butterfly || idx == e_item_index::knife_falchion
                || idx == e_item_index::knife_flip || idx == e_item_index::knife_gut || idx == e_item_index::knife_karambit || idx == e_item_index::knife_m9_bayonet
                || idx == e_item_index::knife_t
                || idx == e_item_index::knife_stiletto
                || idx == e_item_index::knife_ursus || idx == e_item_index::knife_skeleton;
        }

        OFFSET( int, clip1( ), g_ctx->offsets( ).m_base_weapon.m_clip1 );
        OFFSET( int, primary_reserve_ammo_count( ), g_ctx->offsets( ).m_base_weapon.m_primary_reserve_ammo_count );
        OFFSET( int, world_dropped_mdl_idx( ), g_ctx->offsets( ).m_base_weapon.m_world_dropped_mdl_idx );
        OFFSET( int, world_mdl_idx( ), g_ctx->offsets( ).m_base_weapon.m_world_mdl_idx );
        OFFSET( int, view_mdl_idx( ), g_ctx->offsets( ).m_base_weapon.m_view_mdl_idx );
        OFFSET( float, recoil_index( ), g_ctx->offsets( ).m_weapon_cs_base.m_recoil_index );
        OFFSET( int, zoom_lvl( ), g_ctx->offsets( ).m_weapon_cs_base_gun.m_zoom_lvl );

        __forceinline bool& in_reload( ) {
            static const auto& net_var = g_net_vars->entry( xor_str( "m_bInReload" ) );

            return *reinterpret_cast< bool* >( 
                reinterpret_cast< std::uintptr_t >( this ) + std::get< std::uint32_t >( net_var )
                );
        }

        OFFSET( int, burst_shots_remaining( ), g_ctx->offsets( ).m_weapon_cs_base_gun.m_burst_shots_remaining );
        OFFSET( float, next_burst_shot( ), g_ctx->offsets( ).m_weapon_cs_base_gun.m_next_burst_shot );
        OFFSET( float, accuracy_penalty( ), g_ctx->offsets( ).m_weapon_cs_base.m_accuracy_penalty );

        OFFSET( float, postpone_fire_ready_time( ), g_ctx->offsets( ).m_weapon_cs_base.m_postpone_fire_ready_time );

        __forceinline float spread( )
        {
            using fn_t = float( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 439u ]( this );
        }

        __forceinline float inaccuracy( )
        {
            using fn_t = float( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 469u ]( this );
        }

        __forceinline void update_inaccuracy( )
        {
            using fn_t = void( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 471u ]( this );
        }
    };

    struct tone_map_t : public base_entity_t {
        OFFSET( bool, use_custom_bloom_scale( ), g_ctx->offsets( ).m_tone_map.m_use_custom_bloom_scale );
        OFFSET( bool, user_custom_auto_exposure_min( ), g_ctx->offsets( ).m_tone_map.m_user_custom_auto_exposure_min );
        OFFSET( bool, user_custom_auto_exposure_max( ), g_ctx->offsets( ).m_tone_map.m_user_custom_auto_exposure_max );
        OFFSET( float, custom_bloom_scale( ), g_ctx->offsets( ).m_tone_map.m_custom_bloom_scale );
        OFFSET( float, auto_custom_exposure_min( ), g_ctx->offsets( ).m_tone_map.m_auto_custom_exposure_min );
        OFFSET( float, auto_custom_exposure_max( ), g_ctx->offsets( ).m_tone_map.m_auto_custom_exposure_max );
    };

    struct cs_player_t : public base_entity_t {

        __forceinline bool friendly( cs_player_t* const with );
        OFFSET( int, shots_fired( ), g_ctx->offsets( ).m_cs_player.m_shots_fired );
        OFFSET( float, lby( ), g_ctx->offsets( ).m_cs_player.m_lby );
        __forceinline cs_weapon_t* weapon( );
        OFFSET( float, next_attack( ), g_ctx->offsets( ).m_base_combat_character.m_next_attack );
        __forceinline bool alive( );
        OFFSET( ent_handle_t, weapon_handle( ), g_ctx->offsets( ).m_base_combat_character.m_weapon_handle );
        OFFSET( ent_handle_t, ground_entity_handle( ), g_ctx->offsets( ).m_base_player.m_ground_entity );
        __forceinline std::optional< player_info_t > info( );

        __forceinline ent_handle_t ref_handle( ) {
            using fn_t = ent_handle_t( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) )[ 2u ]( this );
        }


        __forceinline anim_layers_t& anim_layers( ) {
            return **reinterpret_cast< anim_layers_t** >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x2970u
                );
        }

        __forceinline std::uintptr_t player_resource( );


        __forceinline int ping( ) {
            auto res = player_resource( );

            return * ( int* )( res + g_ctx->offsets( ).m_player_resource.m_ping + networkable( )->index( ) * 4 );
        }

        __forceinline std::ptrdiff_t lookup_bone( const char* name ) {
            using fn_t = int( __thiscall* )( decltype( this ), const char* );

            return reinterpret_cast< fn_t >( g_ctx->addresses( ).m_lookup_bone )( this, name );
        }

        __forceinline float get_layer_seq_cycle_rate( valve::anim_layer_t* layer, std::ptrdiff_t seq )
        {
            using fn_t = float( __thiscall* )( decltype( this ), valve::anim_layer_t*, std::ptrdiff_t );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 217u ]( this, layer, seq );
        }

       __forceinline void update_visibility_all_entities( )
       {
            static uintptr_t* update_visibility_all_entities = nullptr;
            if( update_visibility_all_entities == nullptr ) {

                static std::uintptr_t call_insctruction = g_ctx->addresses( ).m_upd_visb_entities;
                static DWORD rel_add = * ( DWORD* )( call_insctruction + 1 );
                static DWORD next_instruction = call_insctruction + 5;
                update_visibility_all_entities = ( uintptr_t* )( next_instruction + rel_add );

            }
            else
                reinterpret_cast< void( __thiscall* )( void* ) >( update_visibility_all_entities )( this );
        }

        __forceinline void modify_eye_pos( sdk::vec3_t& eye_pos, valve::bones_t& bones ) {
            if( !this )
                return;

            if( !this->anim_state( ) )
                return;

            if( !this->anim_state( )->m_landing && this->anim_state( )->m_duck_amount <= 0.f 
                && flags( ) & valve::e_ent_flags::on_ground ) {   
                return;
            }

            auto head_bone = lookup_bone( xor_str( "head_0" ) );

            if( head_bone != -1 ) {

                auto head_position = sdk::vec3_t( bones.data( ) [ 8 ][ 0 ][ 3 ],
                    bones.data( ) [ 8 ][ 1 ][ 3 ],
                    bones.data( ) [ 8 ][ 2 ][ 3 ] + 1.7f );

                auto head_height = head_position.z( );

                if( eye_pos.z( ) > head_height ) {
                    auto tmp = 0.0f;
                    tmp = ( fabsf( eye_pos.z( ) - head_height ) - 4.0f ) * 0.16666667f;
                    if( tmp >= 0.0f )
                        tmp = fminf( tmp, 1.0f );
                    eye_pos.z( ) = ( ( head_height - eye_pos.z( ) )
                        * ( ( ( tmp * tmp ) * 3.0f ) - ( ( ( tmp * tmp ) * 2.0f ) * tmp ) ) )
                        + eye_pos.z( );
                }
            }
        }

        __forceinline sdk::vec3_t who_tf_asked( int hitbox_id, valve::bones_t bones ) {
            if( !this )
                return sdk::vec3_t( );

            auto hdr = mdl_ptr( );

            if( !hdr )
                return sdk::vec3_t( );

            auto hitbox_set = hdr->m_studio->get_hitbox_set( hitbox_set_index( ) );

            if( !hitbox_set )
                return sdk::vec3_t( );

            auto hitbox = hitbox_set->get_bbox( hitbox_id );

            if( !hitbox )
                return sdk::vec3_t( );

            sdk::vec3_t min { }, max { };

            sdk::vector_transform( hitbox->m_mins, bones[ hitbox->m_bone ], min );
            sdk::vector_transform( hitbox->m_maxs, bones[ hitbox->m_bone ], max );

            return ( min + max ) * 0.5f;
        }

        __forceinline sdk::vec3_t get_shoot_pos( valve::bones_t& bones ) {
            if( !this )
                return sdk::vec3_t( );

            sdk::vec3_t shoot_pos { };

            shoot_pos = origin( ) + view_offset( );

            modify_eye_pos( shoot_pos, bones );

            return shoot_pos;
        }
        
        __forceinline bool setup_bones( sdk::mat3x4_t *out, int max, int mask, float time ) {
            using fn_t = bool( __thiscall * )( void *, sdk::mat3x4_t*, int, int, float );

            return ( *reinterpret_cast< fn_t** >( renderable( ) ) )[ 13 ] ( renderable( ), out, max, mask, time );
        }

        __forceinline bool& use_new_anim_state( ) {
            return *reinterpret_cast< bool* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x39e1u
                );
        }

        __forceinline std::ptrdiff_t lookup_seq_act( std::ptrdiff_t seq );

        OFFSET( int, tick_base( ), g_ctx->offsets( ).m_base_player.m_tick_base );
        OFFSET( int, final_pred_tick( ), g_ctx->offsets( ).m_base_player.m_tick_base + 0x4u );
        OFFSET( bool, has_helmet( ), g_ctx->offsets( ).m_base_player.m_has_helmet );
        OFFSET( int, armor_val( ), g_ctx->offsets( ).m_base_player.m_armor_val );
        OFFSET( bool, has_heavy_armor( ), g_ctx->offsets( ).m_base_player.m_has_heavy_armor );
        OFFSET( int, money( ), g_ctx->offsets( ).m_base_player.m_money );
        OFFSET( float, max_speed( ), g_ctx->offsets( ).m_base_player.m_max_player_speed );
        OFFSET( float, flash_dur( ), g_ctx->offsets( ).m_base_animating.m_flash_dur );
        __forceinline studio_hdr_t* mdl_ptr( );
        OFFSET( e_life_state, life_state( ), g_ctx->offsets( ).m_base_player.m_life_state );
        OFFSET( int, observer_mode( ), g_ctx->offsets( ).m_base_player.m_observer_mode );
        OFFSET( int, health( ), g_ctx->offsets( ).m_base_entity.m_health );
        OFFSET( e_team, team( ), g_ctx->offsets( ).m_base_entity.m_team_num );
        OFFSET( float, duck_amt( ), g_ctx->offsets( ).m_base_player.m_duck_amt );
        OFFSET( int, skin( ), g_ctx->offsets( ).m_base_player.m_skin );
        OFFSET( int, body( ), g_ctx->offsets( ).m_base_player.m_body );
        OFFSET( int, phys_collision_state( ), g_ctx->offsets( ).m_cs_player.m_collision_state );
        OFFSET( float, duck_speed( ), g_ctx->offsets( ).m_base_player.m_duck_speed );
        OFFSET( float, surface_friction( ), g_ctx->offsets( ).m_base_player.m_surface_friction );

        __forceinline void update_client_side_anim( ) {
            using fn_t = void( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) ) [ 218u ]( this );
        }

        __forceinline void update_collision_bounds( ) {
            using fn_t = void( __thiscall* )( decltype( this ) );

            return ( *reinterpret_cast< fn_t** >( this ) )[ 329u ]( this );
        }

        __forceinline sdk::vec3_t wpn_shoot_pos( ) {
            using fn_t = void( __thiscall* )( decltype( this ), sdk::vec3_t& pos );

            sdk::vec3_t eye_pos{ };

( *reinterpret_cast< fn_t** >( this ) )[ 277u ]( this, eye_pos );

            return eye_pos;
        }

        __forceinline void attachment_helper( valve::studio_hdr_t* studio_hdr ) {
            using fn_t = void( __thiscall* )( decltype( this ), valve::studio_hdr_t* );

            return reinterpret_cast< fn_t >( 
                g_ctx->addresses( ).m_attachment_helper
                )( this, studio_hdr );
        }

        OFFSET( float, fall_velocity( ), g_ctx->offsets( ).m_base_player.m_fall_velocity );
        OFFSET( sdk::vec3_t, base_velocity( ), g_ctx->offsets( ).m_base_player.m_base_velocity );

        OFFSET( float, spawn_time( ), g_ctx->offsets( ).m_base_player.m_spawn_time );

        OFFSET( sdk::qang_t, aim_punch( ), g_ctx->offsets( ).m_base_player.m_aim_punch );
        OFFSET( sdk::qang_t, view_punch( ), g_ctx->offsets( ).m_base_player.m_view_punch );
        OFFSET( sdk::vec3_t, view_offset( ), g_ctx->offsets( ).m_base_player.m_view_offset );
        OFFSET( sdk::qang_t, aim_punch_vel( ), g_ctx->offsets( ).m_base_player.m_aim_punch_vel );
        OFFSET( ent_handle_t, observer_target_handle( ), g_ctx->offsets( ).m_base_player.m_observer_target );

        OFFSET( bool, scoped( ), g_ctx->offsets( ).m_cs_player.m_scoped );
        OFFSET( sdk::qang_t, eye_angles( ), g_ctx->offsets( ).m_cs_player.m_eye_angles );
        OFFSET( float, velocity_modifier( ), g_ctx->offsets( ).m_cs_player.m_velocity_modifier );
        OFFSET( bool, strafing( ), g_ctx->offsets( ).m_cs_player.m_strafing );
        OFFSET( float, third_person_recoil( ), g_ctx->offsets( ).m_cs_player.m_third_person_recoil );
        OFFSET( bool, walking( ), g_ctx->offsets( ).m_cs_player.m_walking );
        OFFSET( bool, wait_for_no_attack( ), g_ctx->offsets( ).m_cs_player.m_wait_for_no_attack );
        OFFSET( int, move_state( ), g_ctx->offsets( ).m_cs_player.m_move_state );
        OFFSET( anim_state_t*, anim_state( ), g_ctx->offsets( ).m_cs_player.m_anim_state );
        OFFSET( valve::ent_handle_t, view_model_handle( ), g_ctx->offsets( ).m_cs_player.m_view_model );

        __forceinline int& sim_tick( )
        {
            return *reinterpret_cast< int* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x2a8u
                );
        }

        OFFSET( bool, defusing( ), g_ctx->offsets( ).m_cs_player.m_defusing );

        __forceinline cmd_context_t& cmd_context( )
        {
            return *reinterpret_cast< cmd_context_t* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x34d0u
                );
        }

        __forceinline user_cmd_t*& cur_user_cmd( )
        {
            return *reinterpret_cast< user_cmd_t** >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x3314u
                );
        }

        __forceinline user_cmd_t& last_user_cmd( )
        {
            return *reinterpret_cast< user_cmd_t* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0x326cu
                );
        }

        __forceinline int& ieflags( )
        {
            return *reinterpret_cast< int* >( 
                reinterpret_cast< std::uintptr_t >( this ) + 0xe4u
                );
        }
    };
    class i_client_unknown;

    class i_client_renderable {
    public:
        virtual i_client_unknown* get_client_unknown( ) = 0;
    };
    class c_base_handle;
    class i_handle_entity {
    public:
        virtual							~i_handle_entity( ) = default;
        virtual void					set_ref_handle( const c_base_handle& handle ) = 0;
        virtual const c_base_handle& get_handle( ) const = 0;
    };

    class i_client_unknown : public i_handle_entity {
    public:
        virtual void* get_collideable( ) = 0;
        virtual void* get_client_networkable( ) = 0;
        virtual i_client_renderable* get_client_renderable( ) = 0;
        virtual void* get_client_entity( ) = 0;
        virtual base_entity_t* get_base_entity( ) = 0;
        virtual void* get_client_thinkable( ) = 0;
        virtual void* get_client_alpha_property( ) = 0;
    };

    class i_client_networkable {
    public:
        virtual i_client_unknown* get_client_unk( ) = 0;
        virtual void release( ) = 0;
        virtual void* client_class( ) = 0;
        virtual void who( int who ) = 0;
        virtual void on_pre_data_changed( int type ) = 0;
        virtual void on_data_changed( int type ) = 0;
        virtual void pre_data_update( int type ) = 0;
        virtual void post_data_update( int type ) = 0;
    };

    struct base_view_model_t : public base_entity_t {
        OFFSET( int, anim_parity( ), g_ctx->offsets( ).m_c_base_view_model.m_animation_parity );
        OFFSET( int, sequence( ), g_ctx->offsets( ).m_c_base_view_model.m_sequence );
    };
}

#include "impl/entities.inl"