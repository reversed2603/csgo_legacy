#include "../../../csgo.hpp"

namespace csgo::hacks {
	void c_aim_bot::handle_ctx( game::user_cmd_t& user_cmd, bool& send_packet ) {
		m_targets.clear( );
		m_angle = {};
		hacks::g_move->allow_early_stop( ) = true;

		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if ( !g_local_player->self( )->weapon( )
			|| !g_local_player->self( )->weapon( )->info( ) )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::knife )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::c4
			|| g_local_player->self( )->weapon( )->info( )->m_type == game::e_weapon_type::grenade )
			return;

		if ( !m_cfg->m_rage_bot )
			return;

		setup_hitboxes( m_hit_boxes );

		if ( m_hit_boxes.empty( ) )
			return;

		if (g_ctx->can_shoot ( ) && !game::g_client_state.get ( )->m_choked_cmds
			&& !g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key ) 
			&& g_local_player->weapon_info ( )->m_type != game::e_weapon_type::grenade ) {
			send_packet = false;
			user_cmd.m_buttons &= ~game::e_buttons::in_attack;
			return;
		}

		add_targets ( );

		select ( user_cmd, send_packet );
	}

	void c_aim_bot::get_hitbox_data ( c_hitbox* rtn, game::cs_player_t* ent, int ihitbox, const game::bones_t& matrix )
	{
		if ( ihitbox < 0 || ihitbox > 19 ) return;

		if ( !ent ) return;

		game::studio_hdr_t* const studio_hdr = ent->mdl_ptr ( );

		if ( !studio_hdr )
			return;

		game::studio_bbox_t* const hitbox = studio_hdr->m_studio->p_hitbox ( ihitbox, ent->hitbox_set_index ( ) );

		if ( !hitbox )
			return;

		const auto is_capsule = hitbox->m_radius != -1.f;

		sdk::vec3_t min, max;
		if ( is_capsule ) {
			min = hitbox->m_mins.transform ( matrix [ hitbox->m_bone ] );
			max = hitbox->m_maxs.transform ( matrix [ hitbox->m_bone ] );
		}
		else
		{
			min = sdk::vector_rotate ( hitbox->m_mins, hitbox->m_rotation );
			max = sdk::vector_rotate ( hitbox->m_maxs, hitbox->m_rotation );

			min = min.transform ( matrix [ hitbox->m_bone ] );
			max = max.transform ( matrix [ hitbox->m_bone ] );
		}

		rtn->m_hitbox_id = ihitbox;
		rtn->m_is_oob = !is_capsule;
		rtn->m_radius = hitbox->m_radius;
		rtn->m_mins = min;
		rtn->m_maxs = max;
		rtn->m_hitgroup = hitbox->m_group;
		rtn->m_hitbox = hitbox;
		rtn->m_start_scaled = g_ctx->shoot_pos ( ).i_transform ( matrix [ hitbox->m_bone ] );
		rtn->m_bone = hitbox->m_bone;
	}

	constexpr auto k_pi = 3.14159265358979323846f;

	constexpr auto k_pi2 = k_pi * 2.f;
	sdk::vec2_t calc_spread_angle (
		const int bullets, const game::e_item_index item_index,
		const float recoil_index, const std::size_t i
	) {
		g_ctx->addresses ( ).m_random_seed ( i + 1u );

		auto v1 = g_ctx->addresses ( ).m_random_float ( 0.f, 1.f );
		auto v2 = g_ctx->addresses ( ).m_random_float ( 0.f, k_pi2 );

		float v3 {}, v4 {};

		using fn_t = void ( __stdcall* )( game::e_item_index, int, int, float*, float* );

		if ( g_ctx->cvars ( ).m_weapon_accuracy_shotgun_spread_patterns->get_int ( ) > 0 )
			reinterpret_cast< fn_t >( g_ctx->addresses ( ).m_calc_shotgun_spread )( item_index, 0, static_cast< int >( bullets * recoil_index ), &v4, &v3 );
		else {
			v3 = g_ctx->addresses ( ).m_random_float ( 0.f, 1.f );
			v4 = g_ctx->addresses ( ).m_random_float ( 0.f, k_pi2 );
		}

		if ( recoil_index < 3.f
			&& item_index == game::e_item_index::negev ) {
			for ( auto i = 3; i > recoil_index; --i ) {
				v1 *= v1;
				v3 *= v3;
			}

			v1 = 1.f - v1;
			v3 = 1.f - v3;
		}

		const auto inaccuracy = v1 * g_eng_pred->inaccuracy ( );
		const auto spread = v3 * g_eng_pred->spread ( );

		return {
			std::cos ( v2 ) * inaccuracy + std::cos ( v4 ) * spread,
			std::sin ( v2 ) * inaccuracy + std::sin ( v4 ) * spread
		};
	}
	static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};
	static const int total_seeds = 128;

	void build_seed_table ( ) {
		if ( !precomputed_seeds.empty ( ) )
			return;

		for ( auto i = 0; i < total_seeds; i++ ) {
			g_ctx->addresses ( ).m_random_seed ( i + 1 );

			const auto pi_seed = g_ctx->addresses ( ).m_random_float ( 0.f, sdk::pi * 2 );

			precomputed_seeds.emplace_back ( g_ctx->addresses ( ).m_random_float ( 0.f, 1.f ),
				sin ( pi_seed ), cos ( pi_seed ) );
		}
	}

	int c_aim_bot::get_autostop_type( )
	{
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case game::e_item_index::awp:
			return g_aim_bot->cfg( ).m_auto_stop_type_awp;
		case game::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_auto_stop_type_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_auto_stop_type_scar;
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_aim_bot->cfg( ).m_auto_stop_type_other;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_auto_stop_type_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_aim_bot->cfg( ).m_auto_stop_type_pistol;
		default:
			return 0;
		}

		return 0;
	}

	int c_aim_bot::get_hitboxes_setup( )
	{
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case game::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_hitboxes;
		case game::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_hitboxes;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_hitboxes;
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_hitboxes;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_hitboxes;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_hitboxes;
		default:
			return 0;
		}

		return 0;
	}

	float get_body_scale( )
	{
		if ( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case game::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_point_scale;
		case game::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_point_scale;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_point_scale;
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_point_scale;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_point_scale;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_point_scale;
		default:
			return 0;
		}

		return 0;
	}

	
	int c_aim_bot::get_min_dmg_override_key( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return -1;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return -1;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_awp_dmg_key );
		case game::e_item_index::ssg08:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_scout_dmg_key );
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_scar_dmg_key );
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_other_dmg_key );
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_heavy_pistol_dmg_key );
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_key_binds->get_keybind_mode( &g_aim_bot->cfg( ).m_min_pistol_dmg_key );
		default:
			return -1;
		}

		return -1;
	}

	int c_aim_bot::get_min_dmg_override( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_awp_min_dmg_on_key;
		case game::e_item_index::ssg08:
			return m_cfg->m_scout_min_dmg_on_key;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_scar_min_dmg_on_key;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_min_dmg_on_key;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_pistol_min_dmg_on_key;
		default:
			return m_cfg->m_other_min_dmg_on_key;
		}

		return 0.f;
	}

	int c_aim_bot::get_force_head_conditions( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_force_head_conditions_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_force_head_conditions_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_force_head_conditions_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_force_head_conditions_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_force_head_conditions_pistol;
		default:
			return m_cfg->m_force_head_conditions_other;
		}

		return 0;
	}

	int c_aim_bot::get_force_body_conditions( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_force_body_conditions_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_force_body_conditions_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_force_body_conditions_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_force_body_conditions_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_force_body_conditions_pistol;
		default:
			return m_cfg->m_force_body_conditions_other;
		}

		return 0;
	}

	bool c_aim_bot::get_min_dmg_override_state( ) { 
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return false;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_awp_dmg_key );
		case game::e_item_index::ssg08:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scout_dmg_key );
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scar_dmg_key );
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_heavy_pistol_dmg_key );
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_pistol_dmg_key );
		default:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_other_dmg_key );
		}

		return false;
	}

	int c_aim_bot::get_min_dmg_to_set_up( )
	{ 
		if( !g_local_player->self( ) 
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{ 
		case game::e_item_index::awp:
			return m_cfg->m_min_dmg_awp;
		case game::e_item_index::ssg08:
			return m_cfg->m_min_dmg_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return m_cfg->m_min_dmg_scar;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return m_cfg->m_min_dmg_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return m_cfg->m_min_dmg_pistol;
		default:
			return m_cfg->m_min_dmg_other;
		}

		return 0.f;
	}

	__forceinline float c_aim_bot::get_hit_chance( ) {
		if ( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case game::e_item_index::awp:
			return g_aim_bot->cfg( ).m_hit_chance_awp;
		case game::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_hit_chance_scout;
		case game::e_item_index::scar20:
		case game::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_hit_chance_scar;
		case game::e_item_index::ak47:
		case game::e_item_index::aug:
		case game::e_item_index::bizon:
		case game::e_item_index::famas:
		case game::e_item_index::galil:
		case game::e_item_index::m249:
		case game::e_item_index::m4a4:
		case game::e_item_index::m4a1s:
		case game::e_item_index::mac10:
		case game::e_item_index::mag7:
		case game::e_item_index::mp5sd:
		case game::e_item_index::mp7:
		case game::e_item_index::mp9:
		case game::e_item_index::negev:
		case game::e_item_index::nova:
		case game::e_item_index::sawed_off:
		case game::e_item_index::sg553:
		case game::e_item_index::ump45:
		case game::e_item_index::xm1014:
		case game::e_item_index::p90:
			return g_aim_bot->cfg( ).m_hit_chance_other;
		case game::e_item_index::revolver:
		case game::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_hit_chance_heavy_pistol;
		case game::e_item_index::cz75a:
		case game::e_item_index::elite:
		case game::e_item_index::five_seven:
		case game::e_item_index::p2000:
		case game::e_item_index::glock:
		case game::e_item_index::p250:
		case game::e_item_index::tec9:
		case game::e_item_index::usps:
			return g_aim_bot->cfg( ).m_hit_chance_pistol;
		default:
			return 0;
		}

		return 0;
	}

	static int clip_ray_to_hitbox( const game::ray_t& ray, game::studio_bbox_t* hitbox, sdk::mat3x4_t& matrix, game::trace_t& trace ) {
		if ( !g_ctx->addresses ( ).m_clip_ray
			|| !hitbox )
			return -1;

		trace.m_frac = 1.0f;
		trace.m_start_solid = false;

		return reinterpret_cast < int( __fastcall* )( const game::ray_t&, game::studio_bbox_t*, sdk::mat3x4_t&, game::trace_t& ) > ( g_ctx->addresses( ).m_clip_ray )( ray, hitbox, matrix, trace );
	}

	int c_aim_bot::calc_hit_chance (
		game::cs_player_t* player, std::shared_ptr < lag_record_t > record, const sdk::qang_t& angle, const std::ptrdiff_t hit_box
	) {
		build_seed_table ( );

		if ( static_cast < int > ( g_eng_pred->inaccuracy( ) * 10000.f ) == 0 ) {
			return 101;
		}

		const auto is_scope_able_weapon = g_local_player->weapon( )->item_index( ) == game::e_item_index::scar20 || g_local_player->weapon( )->item_index( ) == game::e_item_index::g3sg1
			|| g_local_player->weapon( )->item_index( ) == game::e_item_index::ssg08 || g_local_player->weapon( )->item_index( ) == game::e_item_index::awp;

		auto min_dmg = get_min_dmg_to_set_up ( ) + 1;

		if ( get_min_dmg_override_state( ) )
			min_dmg = get_min_dmg_override( ) + 1;

		min_dmg = std::clamp( static_cast< int >( min_dmg * ( 2.f / 100.f ) ), 1, 100 );

		c_hitbox ht;
		get_hitbox_data ( &ht, player, hit_box, record->m_bones );

		sdk::vec3_t fwd {}, right {}, up {};
		sdk::ang_vecs ( angle, &fwd, &right, &up );

		int hits {};
	
		const auto item_index = g_local_player->self ( )->weapon ( )->item_index ( );
		const auto recoil_index = g_local_player->self ( )->weapon ( )->recoil_index ( );
		constexpr auto k_max_seeds = 128u;
		const auto weapon_inaccuracy = g_local_player->self ( )->weapon ( )->inaccuracy ( );
		sdk::vec3_t total_spread, end;
		float inaccuracy, spread_x, spread_y;
		std::tuple<float, float, float>* seed;
		std::array< bool, k_max_seeds > seeds {};
		for ( std::size_t i {}; i < k_max_seeds; ++i )
		{
			seed = &precomputed_seeds [ i ];

			inaccuracy = std::get<0> ( *seed ) * weapon_inaccuracy;
			spread_x = std::get<2> ( *seed ) * inaccuracy;
			spread_y = std::get<1> ( *seed ) * inaccuracy;
			total_spread = ( fwd + right * spread_x + up * spread_y ).normalized ( );

			end = g_ctx->shoot_pos ( ) + ( total_spread * crypt_float ( 8192.f ) );
			bool intersected = false;

			game::trace_t trace {};
			game::ray_t ray{ g_ctx->shoot_pos( ), end };

			intersected = clip_ray_to_hitbox( ray, ht.m_hitbox, record->m_bones[ ht.m_hitbox->m_bone ], trace ) >= 0;

			seeds.at( i ) = intersected;
						
		}
		for ( auto& hit : seeds )
			if ( hit )
				++hits;

		if ( static_cast< int >( ( hits / static_cast< float >( k_max_seeds ) ) * 100.f ) >= 15
			&& g_local_player->self( )->flags( ) & game::e_ent_flags::on_ground
			&& is_scope_able_weapon
			&& !g_local_player->self( )->scoped( )
			&& g_ctx->can_shoot( ) ) {
			if ( g_eng_pred->min_inaccuracy( ) >= ( g_local_player->weapon( )->accuracy_penalty( ) * 0.02f ) )
				return 101;
		}

		return static_cast< int >( ( hits / static_cast< float >( k_max_seeds ) ) * 100.f );
	}

	void c_aim_bot::add_targets ( ) {
		m_targets.reserve ( game::g_global_vars.get ( )->m_max_clients );

		for ( std::ptrdiff_t i = 1; i <= game::g_global_vars.get ( )->m_max_clients; ++i ) {
			auto& entry = hacks::g_lag_comp->entry( i - 1 );

			if ( !entry.m_player
				|| !entry.m_player->alive( )
				|| entry.m_player->networkable( )->dormant( ) )
				continue;

			if ( entry.m_player->team( )
				== g_local_player->self( )->team( ) )
				continue;

			if ( entry.m_lag_records.empty( ) )
				continue;

			auto record = select_ideal_record( entry );

			if ( !record.has_value( ) )
				continue;

			m_targets.emplace_back( aim_target_t( &entry, record.value ( ).m_lag_record ) );
		}
	}

	void c_aim_bot::scan_center_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, sdk::vec3_t shoot_pos, std::vector < point_t >& points ) const {
		const auto hitbox_set = target.m_entry->m_player->mdl_ptr( )->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );
		auto hitbox_stomach = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::stomach ) );
		auto hitbox_head = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( game::e_hitbox::head ) );

		if ( !hitbox_stomach
			&& !hitbox_head )
			return;

		sdk::vec3_t body_point{};
		sdk::vec3_t head_point{};
		sdk::vector_transform(
			( hitbox_stomach->m_mins + hitbox_stomach->m_maxs ) / 2.f,
			record->m_bones[ hitbox_stomach->m_bone ], body_point
		);
		sdk::vector_transform(
			( hitbox_head->m_mins + hitbox_head->m_maxs ) / 2.f,
			record->m_bones[ hitbox_head->m_bone ], head_point
		);

		/* fkin constructors gone crazy mf visual studio kys */
		point_t body_point_{};
		body_point_.m_center = true;
		body_point_.m_index = game::e_hitbox::stomach;
		body_point_.m_pos = body_point;

		point_t head_point_{};
		head_point_.m_center = true;
		head_point_.m_index = game::e_hitbox::head;
		head_point_.m_pos = head_point;

		points.clear( );

		points.push_back( body_point_ );
		points.push_back( head_point_ );

		record->adjust( target.m_entry->m_player );

		for( auto& point : points ) {
			if ( !head_point_.m_valid
				|| !body_point_.m_valid )
			scan_point( target.m_entry, point, shoot_pos );
		}
	}

	std::optional < aim_target_t > c_aim_bot::select_ideal_record( const player_entry_t& entry ) const {

		if ( entry.m_lag_records.empty( ) )
			return std::nullopt;

		if ( entry.m_lag_records.size( ) == 1u ) {
			return get_latest_record( entry );
		}

		std::shared_ptr< lag_record_t > best_record {};
		std::optional< point_t > best_aim_point{};
		const auto rend = entry.m_lag_records.end ( );
		for ( auto i = entry.m_lag_records.begin ( ); i != rend; i = std::next ( i ) ) {
			const auto& lag_record = *i;

			if ( !lag_record->valid( ) )
				continue;

			if ( !lag_record->valid( ) 
				&& !lag_record->m_dormant ) {
				if ( lag_record->m_broke_lc ) {
					break;
				}

				continue;
			}

			if ( lag_record->m_choked_cmds < crypt_int ( 20 )
				&& !lag_record->m_dormant ) {
				std::vector < point_t > points{};
				aim_target_t target{};
				target.m_entry = const_cast < player_entry_t* > ( &entry );
				target.m_lag_record = lag_record;

				lag_backup_t lag_backup{};
				lag_backup.setup( lag_record->m_player );

				scan_center_points( target, lag_record, g_ctx->shoot_pos( ), points );

		    	lag_backup.restore( lag_record->m_player );

				if ( !scan_points( &target, points ) ) {
					if ( !best_record )
						best_record = lag_record;

					continue;
				}

				if ( !best_record
					|| !best_aim_point.has_value( ) ) {
					if ( target.m_best_point ) {
						best_record = lag_record;
						best_aim_point = *target.m_best_point;

						continue;
					}
				}

				if ( !best_record ) {
					best_record = lag_record;
					continue;
				}

				if ( target.m_best_point ) {
					const auto& pen_data = target.m_best_point;
					const auto& best_pen_data = best_aim_point.value( );

					if ( std::abs( pen_data->m_dmg - best_pen_data.m_dmg ) > crypt_int ( 10 ) ) {
						if ( pen_data->m_dmg <= target.m_entry->m_player->health( )
							|| best_pen_data.m_dmg <= target.m_entry->m_player->health( ) ) {
							if ( pen_data->m_dmg > best_pen_data.m_dmg ) {
								best_record = lag_record;
								best_aim_point = *target.m_best_point;

								continue;
							}
						}
					}

					if ( pen_data->m_dmg > best_pen_data.m_dmg ) {
						best_record = lag_record;
						best_aim_point = *target.m_best_point;
					}
				}
				else {
					if ( lag_record->m_broke_lby ) {
						best_record = lag_record;
					}
				}
			}
		}

		if ( !best_record )
			return std::nullopt;

		if ( best_record->m_broke_lc ) { // player broke lc let's extrapolate him
			return extrapolate ( entry );
		}
		else {
			aim_target_t ret{};
			ret.m_entry = const_cast < player_entry_t* > ( &entry );
			ret.m_lag_record = best_record;
			return ret;
		}
	}

	std::optional < aim_target_t > c_aim_bot::get_latest_record ( const player_entry_t& entry ) const {
		const auto& latest = entry.m_lag_records.front ( );
		if ( latest->m_lag_ticks <= 0
			|| latest->m_lag_ticks >= 20	
			|| latest->m_dormant )
			return std::nullopt;

		if ( !latest->valid( ) )
			return std::nullopt;

		if ( latest->m_broke_lc ) {

			const auto adjusted_arrive_tick = std::clamp( game::to_ticks( ( ( g_ctx->net_info( ).m_latency.m_out ) + game::g_global_vars.get( )->m_real_time )
				- entry.m_receive_time ), 0, 100 );

			if ( ( adjusted_arrive_tick - latest->m_choked_cmds ) >= 0 )
				return std::nullopt;
		}

		aim_target_t ret{};
		ret.m_entry = const_cast < player_entry_t* > ( &entry );
		ret.m_lag_record = latest;

		return ret;
	}	

	__forceinline sdk::vec3_t origin_ ( sdk::mat3x4_t who ) {
		return sdk::vec3_t ( who [ 0 ][ 3 ], who [ 1 ][ 3 ], who [ 2 ][ 3 ] );
	}
	__forceinline float dot ( sdk::vec3_t from, const sdk::vec3_t& v ) {
		return ( from.x ( ) * v.x ( ) + from.y ( ) * v.y ( ) + from.z ( ) * v.z ( ) );
	}

	__forceinline float dot ( sdk::vec3_t from, float* v ) {
		return ( from.x ( ) * v [ 0 ] + from.y ( ) * v [ 1 ] + from.z ( ) * v [ 2 ] );
	}
	__forceinline sdk::mat3x4_t vector_matrix ( const sdk::vec3_t& in ) {
		sdk::vec3_t right {}, up {};

		if ( in.x ( ) == 0.f
			&& in.y ( ) == 0.f ) {
			right = { 0.f, -1.f, 0.f };
			up = { -in.z ( ), 0.f, 0.f };
		}
		else {
			right = in.cross ( { 0.f, 0.f, 1.f } ).normalized ( );
			up = right.cross ( in ).normalized ( );
		}

		sdk::mat3x4_t ret {};

		ret [ 0 ][ 0 ] = in.x ( );
		ret [ 1 ][ 0 ] = in.y ( );
		ret [ 2 ][ 0 ] = in.z ( );

		ret [ 0 ][ 1 ] = -right.x ( );
		ret [ 1 ][ 1 ] = -right.y ( );
		ret [ 2 ][ 1 ] = -right.z ( );

		ret [ 0 ][ 2 ] = up.x ( );
		ret [ 1 ][ 2 ] = up.y ( );
		ret [ 2 ][ 2 ] = up.z ( );

		return ret;
	}
	void c_aim_bot::calc_capsule_points ( aim_target_t& target, const game::studio_bbox_t* hit_box, const std::ptrdiff_t index,
		const sdk::mat3x4_t matrix, float scale ) {
		sdk::vec3_t min {}, max {};

		min = hit_box->m_mins.transform ( matrix );
		max = hit_box->m_maxs.transform ( matrix );

		static auto matrix0 = vector_matrix ( { 0.f, 0.f, 1.f } );

		auto matrix1 = vector_matrix ( ( max - min ).normalized ( ) );

		for ( const auto& vertices : {
				sdk::vec3_t{ 0.95f, 0.f, 0.f },
				sdk::vec3_t{ -0.95f, 0.f, 0.f },
				sdk::vec3_t{ 0.f, 0.95f, 0.f },
				sdk::vec3_t{ 0.f, -0.95f, 0.f },
				sdk::vec3_t{ 0.f, 0.f, 0.95f },
				sdk::vec3_t{ 0.f, 0.f, -0.95f }
			} ) {
			sdk::vec3_t point {};

			point = vertices.rotate ( matrix0 );
			point = point.rotate ( matrix1 );

			point *= scale;

			if ( vertices.z ( ) > crypt_float ( 0.f ) )
				point += min - max;

			auto final_point = point + max;

			target.m_points.emplace_back( final_point, static_cast < game::e_hitbox > ( index ), false );
		}
	}

	void c_aim_bot::add_aim_point( aim_target_t& target, sdk::vec3_t& point, game::e_hitbox index, bool is_center ) {
		target.m_points.emplace_back( point, index, is_center );
	}

	void c_aim_bot::setup_points ( aim_target_t& target, std::shared_ptr < lag_record_t > record, game::e_hitbox index, e_hit_scan_mode mode
	) {

		auto hdr = target.m_entry->m_player->mdl_ptr( );
		if ( !hdr )
			return;

		auto set = hdr->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );
		if ( !set )
			return;

		auto hitbox = set->get_bbox( static_cast < std::ptrdiff_t > ( index ) );
		if ( !hitbox )
			return;

		sdk::vec3_t point{};

		const auto center = ( hitbox->m_mins + hitbox->m_maxs ) / crypt_float( 2.f );

		sdk::vector_transform( center, record->m_bones[ hitbox->m_bone ], point );

		target.m_points.emplace_back( point, index, true );

		// get hitbox scales.
		float scale = get_body_scale( ) / 75.f;

		float bscale = get_body_scale( ) / 100.f;

		// these indexes represent boxes.
		if ( hitbox->m_radius <= crypt_float( 0.f ) ) {
			point = { center.x( ) + ( hitbox->m_mins.x( ) - center.x( ) ) * scale, center.y( ), center.z( ) };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

			point = { center.x( ) + ( hitbox->m_maxs.x( ) - center.x( ) ) * scale, center.y( ), center.z( ) };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

		}
		else {
			sdk::vec3_t center = ( hitbox->m_mins + hitbox->m_maxs ) / crypt_float( 2.f );

			sdk::vec3_t point{};

			if ( index > game::e_hitbox::head ) {
				if ( static_cast < std::ptrdiff_t > ( index ) == crypt_int( 14 ) ||
					static_cast < std::ptrdiff_t > ( index ) == crypt_int( 15 )
					|| static_cast < std::ptrdiff_t > ( index ) == crypt_int( 16 )
					|| static_cast < std::ptrdiff_t > ( index ) == crypt_int( 17 ) ||
					static_cast < std::ptrdiff_t > ( index ) == crypt_int( 18 ) || static_cast < std::ptrdiff_t > ( index ) == crypt_int( 19 ) ) {
					if ( bscale > 0.9f )
						bscale = 0.9f;
				}
				else {
					if ( static_cast < std::ptrdiff_t > ( index ) != crypt_int( 4 )
						&& static_cast < std::ptrdiff_t > ( index ) != crypt_int( 20 ) ) {
						if ( static_cast < std::ptrdiff_t > ( index ) == crypt_int( 8 )
							|| static_cast < std::ptrdiff_t > ( index ) == crypt_int( 9 )
							|| static_cast < std::ptrdiff_t > ( index ) == crypt_int( 10 )
							|| static_cast < std::ptrdiff_t > ( index ) == 11
							|| static_cast < std::ptrdiff_t > ( index ) == 12
							|| static_cast < std::ptrdiff_t > ( index ) == 13 ) {
							if ( bscale > crypt_float( 0.9f ) )
								bscale = crypt_float( 0.9f );

							point = { center.x( ), hitbox->m_maxs.y( ) - hitbox->m_radius * scale, center.z( ) };

							sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

							target.m_points.emplace_back( point, index, false );
						}

						return;
					}

					if ( bscale > 0.9f )
						bscale = 0.9f;

					if ( static_cast < std::ptrdiff_t > ( index ) == 20 ) {
						point = { center.x( ), hitbox->m_maxs.y( ) - hitbox->m_radius * scale, center.z( ) };

						sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

						target.m_points.emplace_back( point, index, false );

						return;
					}
				}

				//return calc_capsule_points( target, hitbox, static_cast < std::ptrdiff_t > ( index ),
					//record->m_bones[ hitbox->m_bone ], bscale );
			}

			point = { hitbox->m_maxs.x( ) + crypt_float( 0.70710678f ) * ( hitbox->m_radius * scale ), hitbox->m_maxs.y( ) - crypt_float( 0.70710678f ) * ( hitbox->m_radius * scale ), hitbox->m_maxs.z( ) };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

			point = { hitbox->m_maxs.x( ) , hitbox->m_maxs.y( ) , hitbox->m_maxs.z( ) + hitbox->m_radius * scale };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

			point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) - hitbox->m_radius * scale };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

			point = { hitbox->m_maxs.x( ) , hitbox->m_maxs.y( ) - hitbox->m_radius * scale, hitbox->m_maxs.z( ) };

			sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

			target.m_points.emplace_back( point, index, false );

			if ( target.m_entry->m_player->anim_state( ) && target.m_lag_record.value( )->m_anim_velocity.length( 2u ) <= crypt_float( 0.1f ) && target.m_lag_record.value( )->m_eye_angles.x( ) <= target.m_entry->m_player->anim_state( )->m_aim_pitch_max )
			{
				point = { hitbox->m_maxs.x( ) - hitbox->m_radius * scale, hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) };

				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );

				target.m_points.emplace_back( point, index, false );
			}
		}
	}

	void c_aim_bot::setup_hitboxes ( std::vector < hit_box_data_t >& hitboxes ) {
		hitboxes.clear ( );

		if ( g_local_player->self ( )->weapon ( )->item_index ( ) == game::e_item_index::taser ) {
			hitboxes.push_back ( { game::e_hitbox::stomach } );
			return;
		}

		hitboxes.push_back ( { game::e_hitbox::stomach } );
		hitboxes.push_back ( { game::e_hitbox::pelvis } );
		const int hitboxes_selected = g_aim_bot->get_hitboxes_setup( );

		if ( hitboxes_selected & 1 ) {
			hitboxes.push_back ( { game::e_hitbox::head, e_hit_scan_mode::normal } );
		}

		if ( hitboxes_selected & 2 ) {
			hitboxes.push_back ( { game::e_hitbox::lower_chest, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::chest, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::upper_chest, e_hit_scan_mode::normal } );
		}

		if ( hitboxes_selected & 4 ) {
			hitboxes.push_back ( { game::e_hitbox::pelvis, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::stomach, e_hit_scan_mode::normal } );
		}

		if ( hitboxes_selected & 8 ) {
			hitboxes.push_back ( { game::e_hitbox::left_upper_arm,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::right_upper_arm, e_hit_scan_mode::normal } );
		}

		if ( hitboxes_selected & 16 ) {
			hitboxes.push_back ( { game::e_hitbox::left_thigh, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::right_thigh,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::left_calf, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::right_calf,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::left_foot, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { game::e_hitbox::right_foot, e_hit_scan_mode::normal } );
		}
	}

	void c_aim_bot::scan_point ( player_entry_t* entry,
		point_t& point, sdk::vec3_t& shoot_pos ) {

		if( !g_auto_wall->wall_penetration ( shoot_pos, &point, entry->m_player ) )
			return;

		int min_dmg = g_aim_bot->get_min_dmg_to_set_up( );
		
		if( g_aim_bot->get_min_dmg_override_state( ) )
			min_dmg = g_aim_bot->get_min_dmg_override( );

		if ( point.m_dmg < entry->m_player->health ( ) )
			if ( point.m_dmg < min_dmg )
				return;


		point.m_valid = true;
	}

	bool c_aim_bot::scan_points ( cc_def ( aim_target_t* ) target, std::vector < point_t >& points, bool additional_scan ) const {
		std::array < point_t*, 20 > best_points {};

		for ( auto& point : points ) {

			if ( additional_scan )
			scan_point( target.get ( )->m_entry, point );

			if ( !point.m_valid 
				|| point.m_dmg < 1 )
				continue;

			const auto hp = target.get ( )->m_entry->m_player->health( );

			auto& best_point = best_points.at ( static_cast < std::ptrdiff_t > ( point.m_index ) );

			if ( !best_point ) {
				best_point = &point; // init best point that we can compare to next points
				continue;
			}

			const auto& best_pen_data = best_point;
			const auto& pen_data = point;

			if ( point.m_center ) {
				if ( ( best_pen_data->m_hitgroup == pen_data.m_hitgroup )
					|| ( best_pen_data->m_remaining_pen == pen_data.m_remaining_pen && std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) <= crypt_int ( 1 ) )
					|| ( best_pen_data->m_dmg > hp && pen_data.m_dmg > hp ) ) {
					best_point = &point;
				}

				continue;
			}	

			auto& cur_dmg = pen_data.m_dmg;
			auto& last_dmg = best_pen_data->m_dmg;

			if ( last_dmg == cur_dmg ) {
				continue;
			}

			if ( cur_dmg >= hp
				&& last_dmg < hp ) {
				best_point = &point;
				break; // lethal point, its perfect
			}

			if ( best_pen_data->m_hitgroup != pen_data.m_hitgroup
				|| best_pen_data->m_remaining_pen != pen_data.m_remaining_pen ) {
				if ( best_pen_data->m_remaining_pen != pen_data.m_remaining_pen
					|| std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 ) {
					if ( best_pen_data->m_dmg <= hp || pen_data.m_dmg <= hp ) {
						if ( pen_data.m_dmg > best_pen_data->m_dmg )
							best_point = &point;

						break; // nah escape from scan
					}
				}
			}

			if ( last_dmg < hp
				&& cur_dmg < hp ) {
				if ( std::abs( last_dmg - cur_dmg ) <= 1 ) {
					continue;
				}
	
				if ( cur_dmg > last_dmg + 5 ) {
					best_point = &point;
					break; // point is fine
				}
			}

		}

		std::vector < point_t > next_points {};

		for ( auto& best_point : best_points ) {
			if ( best_point )
				next_points.emplace_back ( std::move ( *best_point ) );

			points = next_points;
		}

		if ( points.empty ( ) )
			return false;

		const auto hp = target.get( )->m_entry->m_player->health( );

		for ( auto& point : points ) {
			if ( !target.get( )->m_best_point )
				target.get( )->m_best_point = &point;
			else {
				const auto& best_pen_data = target.get( )->m_best_point;
				const auto& pen_data = point;

				auto v31 = false;
				if ( std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data->m_dmg <= hp ) )
					v31 = pen_data.m_dmg > best_pen_data->m_dmg;

				if ( v31 )
					target.get( )->m_best_point = &point;
			}

			if ( point.m_index == game::e_hitbox::stomach
				|| point.m_index == game::e_hitbox::pelvis ) {
				if ( !target.get( )->m_best_body_point )
					target.get( )->m_best_body_point = &point;
				else {
					const auto& best_pen_data = target.get ( )->m_best_body_point;
					const auto& pen_data = point;

					auto v31 = false;
					if ( std::abs( best_pen_data->m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data->m_dmg <= hp ) )
						v31 = pen_data.m_dmg > best_pen_data->m_dmg;

					if ( v31 )
						target.get( )->m_best_body_point = &point;
				}
			}
		}
		if ( target.get( )->m_best_body_point ) {
			target.get( )->m_best_point = target.get( )->m_best_body_point;
		}

		return true;
	}

	point_t* c_aim_bot::select_point ( cc_def ( aim_target_t* ) target, const int cmd_num ) {
		if ( !target.get( )->m_best_body_point
			|| target.get( )->m_best_body_point->m_dmg < crypt_int( 1 ) ) {
			return target.get( )->m_best_point;
		}

		const auto hp = target.get( )->m_entry->m_player->health( );

		if ( g_key_binds->get_keybind_state( &m_cfg->m_baim_key ) )
			return target.get( )->m_best_body_point;

		if ( get_force_body_conditions( ) & 1 ) {
			if ( target.get( )->m_best_body_point->m_dmg >= hp ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 2 ) {
			if ( !( target.get( )->m_lag_record.value( )->m_flags & game::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 4 ) {
			if ( !target.get( )->m_entry->m_moving_data.m_moved ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 8 ) {
			if ( target.get( )->m_lag_record.value ( )->m_has_fake ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( target.get( )->m_best_body_point->m_dmg < hp ) {

			if ( get_force_head_conditions ( ) & 1
				&& target.get( )->m_entry->m_predicting_lby ) {
				return target.get( )->m_best_point;
			}

			if ( get_force_head_conditions( ) & 2
				&& target.get( )->m_lag_record.value( )->m_anim_velocity.length( 2u ) > 75.f
				&& !target.get( )->m_lag_record.value( )->m_fake_walking 
				&& ( target.get ( )->m_lag_record.value ( )->m_flags & game::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_point;
			}

			if ( get_force_head_conditions( ) & 4
				&& target.get( )->m_lag_record.value( )->m_resolved ) {
				return target.get( )->m_best_point;
			}
		}

		if ( get_force_head_conditions( ) & 8
			&& target.get ( )->m_best_body_point->m_dmg < hp
			&& g_local_player->self( )->weapon( )->clip1 ( ) <= 1 ) {
			return target.get( )->m_best_point;
		}

		const auto& shots = g_shot_construct->m_shots;

		if ( !shots.empty( ) ) {
			const auto& last_shot = shots.back( );

			if ( last_shot.m_target->m_entry->m_player == target.get( )->m_entry->m_player
				&& std::abs( last_shot.m_cmd_num - cmd_num ) <= crypt_int ( 150 ) ) {
				const auto hp_left = last_shot.m_target->m_entry->m_player->health( ) - last_shot.m_damage;

				if ( hp_left
					&& target.get( )->m_best_body_point->m_dmg >= hp_left )
					return target.get( )->m_best_body_point;
			}
		}

		if ( target.get( )->m_best_body_point->m_dmg >= target.get( )->m_entry->m_player->health ( ) )
			return target.get( )->m_best_body_point;

		if ( g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key )
			&& target.get( )->m_best_body_point->m_dmg * crypt_float ( 2.f ) >= target.get( )->m_entry->m_player->health ( )
			&& g_local_player->weapon ( )->item_index ( ) != game::e_item_index::ssg08 ) {
			return target.get( )->m_best_body_point;
		}

		return target.get( )->m_best_point;
	}

	bool c_aim_bot::can_shoot (
		bool skip_r8, const int shift_amount, const bool what
	) const {

		if ( !g_local_player->self ( )
			|| !g_local_player->self ( )->alive ( ) )
			return false;

		auto weapon = g_local_player->self ( )->weapon ( );
		if ( !weapon )
			return false;

		auto weapon_data = weapon->info ( );
		if ( !weapon_data )
			return false;

		if ( g_local_player->self ( )->flags ( ) & game::e_ent_flags::frozen )
			return false;

		if ( g_local_player->self ( )->wait_for_no_attack ( ) )
			return false;

		if ( g_local_player->self ( )->defusing ( ) )
			return false;

		auto anim_layer = g_local_player->self ( )->anim_layers ( ).at ( 1u );
		
		if ( anim_layer.m_owner ) {
			if ( g_local_player->self ( )->lookup_seq_act ( anim_layer.m_seq ) == crypt_int ( 967 )
				&& anim_layer.m_weight != 0.f )
				return false;
		}

		if ( weapon_data->m_type >= game::e_weapon_type::pistol && weapon_data->m_type <= game::e_weapon_type::machine_gun && weapon->clip1 ( ) < crypt_int ( 1 ) )
			return false;

		float curtime = game::to_time ( g_local_player->self ( )->tick_base ( ) - shift_amount );
		if ( curtime < g_local_player->self ( )->next_attack ( ) )
			return false;

		if ( ( weapon->item_index ( ) == game::e_item_index::glock || weapon->item_index ( ) == game::e_item_index::famas ) && weapon->burst_shots_remaining ( ) > crypt_int ( 0 ) ) {
			if ( curtime >= weapon->next_burst_shot ( ) )
				return true;
		}

		if ( curtime < weapon->next_primary_attack ( ) )
			return false;

		if ( weapon->item_index ( ) != game::e_item_index::revolver )
			return true;

		if ( skip_r8 )
			return true;

		return curtime >= weapon->postpone_fire_ready_time ( );
	}

	bool c_aim_bot::can_hit ( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box ) {
		auto hdr = *( game::studio_hdr_t** ) record->m_player->mdl_ptr( );
		if ( !hdr )
			return false;

		auto hitbox_set = hdr->m_studio->get_hitbox_set( record->m_player->hitbox_set_index ( ) );

		if ( !hitbox_set )
			return false;

		auto hitbox_ptr = hitbox_set->get_bbox( box );

		if ( !hitbox_ptr )
			return false;

		bool is_capsule = hitbox_ptr->m_radius != -1.0f;

		auto mins = hitbox_ptr->m_mins.transform( record->m_bones[ hitbox_ptr->m_bone ] );
		auto maxs = hitbox_ptr->m_maxs.transform( record->m_bones[ hitbox_ptr->m_bone ] );

		bool is_intersected{};

		if ( !is_capsule ) { 
			is_intersected = sdk::intersect_bounding_box( start, end, mins, maxs );
		}
		else
		{
			is_intersected = sdk::intersect( start, end, mins, maxs, hitbox_ptr->m_radius );
		}

		return is_intersected;
	}

	aim_target_t* c_aim_bot::select_target( ) {
		if ( m_targets.empty( ) )
			return nullptr;

		auto best_target = &m_targets.front( );

		const auto end = m_targets.end( );
		for ( auto it = std::next( m_targets.begin( ) ); it != end; it = std::next( it ) ) {
			const auto hp = it->m_entry->m_player->health( );

			const auto& best_pen_data = best_target->m_best_point;
			const auto& pen_data = it->m_best_point;
			if ( pen_data->m_dmg < hp ) {
				if ( best_pen_data->m_dmg <= hp
					&& pen_data->m_dmg > best_pen_data->m_dmg )
					best_target = &*it;
			}
			else {
				best_target = &*it;
			}
		}

		return best_target;
	}

	void c_aim_bot::select ( game::user_cmd_t& user_cmd, bool& send_packet ) {
		if ( m_targets.empty ( ) )
			return;

		struct ideal_target_t {
			game::cs_player_t* m_player {}; sdk::vec3_t m_pos{};
			float m_dmg{}; game::e_hitbox m_hit_box{}; std::shared_ptr < lag_record_t > m_record{}; aim_target_t* m_target{};
		};
		std::unique_ptr < ideal_target_t > ideal_select = std::make_unique < ideal_target_t >( );

		for ( auto& target : m_targets ) {
			if ( !target.m_lag_record.has_value( ) )
				continue;

			lag_backup_t backup{};
			backup.setup( target.m_entry->m_player );
			target.m_lag_record.value( )->adjust( target.m_entry->m_player );
			target.m_points.clear( );

			for ( const auto& who : g_aim_bot->m_hit_boxes )
				setup_points( target, target.m_lag_record.value( ), who.m_index, who.m_mode );

			backup.restore( target.m_entry->m_player );
		}
		
		m_targets.erase(
			std::remove_if(
				m_targets.begin( ), m_targets.end( ),
				[ & ]( aim_target_t& target ) {
					return !scan_points( &target, target.m_points, true );
				}
			),
			m_targets.end( )
					);
		const auto target = select_target( );
		if ( !target )
			return m_targets.clear( );

		hacks::g_move->allow_early_stop( ) = false; 

		const auto point = select_point( target, user_cmd.m_number );

		if ( point ) {
			ideal_select->m_player = target->m_entry->m_player;
			ideal_select->m_dmg = point->m_dmg;
			ideal_select->m_record = target->m_lag_record.value( );
			ideal_select->m_hit_box = point->m_index;
			ideal_select->m_pos = point->m_pos;
			ideal_select->m_target = target;
		}

		if ( ideal_select->m_player
			&& ideal_select->m_record ) {

			g_eng_pred->update_shoot_pos( m_angle );

			lag_backup_t lag_backup {};
			lag_backup.setup ( ideal_select->m_player );

			ideal_select->m_record->adjust ( ideal_select->m_player );

			ideal_select->m_target->m_pos = ideal_select->m_pos;

			m_angle = ( ideal_select->m_pos - g_ctx->shoot_pos ( ) ).angles ( );

			g_ctx->was_shooting ( ) = false;

			if ( g_ctx->can_shoot( ) ) {

				auto wpn_idx = g_local_player->weapon( )->item_index( );
				bool can_scope = !g_local_player->self( )->scoped( ) && ( wpn_idx == game::e_item_index::aug || wpn_idx == game::e_item_index::sg553 || wpn_idx == game::e_item_index::scar20 || wpn_idx == game::e_item_index::g3sg1 || g_local_player->weapon_info( )->m_type == game::e_weapon_type::sniper );

				if ( !( user_cmd.m_buttons & game::e_buttons::in_jump ) ) {
					if ( can_scope
						&& m_cfg->m_auto_scope )
						user_cmd.m_buttons |= game::e_buttons::in_attack2;
				}

				const auto hit_chance = calc_hit_chance( ideal_select->m_player, ideal_select->m_record, m_angle, static_cast < std::ptrdiff_t > ( ideal_select->m_hit_box ) );
				if ( hit_chance < ( g_local_player->weapon( )->item_index( ) == game::e_item_index::taser ? 60.f : get_hit_chance( ) ) ) {
					lag_backup.restore( ideal_select->m_player );

					if ( !( user_cmd.m_buttons & game::e_buttons::in_jump ) )
						m_should_stop = get_autostop_type( ) + 1;

					return;
				}

				std::stringstream msg;

				game::player_info_t info;

				auto find = game::g_engine->get_player_info( ideal_select->m_player->networkable ( )->index ( ), &info );

				auto get_hitbox_name_by_id = [ ] ( game::e_hitbox id ) -> const char* {
					switch ( id ) {
					case game::e_hitbox::head:
						return "head";
						break;
					case game::e_hitbox::neck:
						return "neck";
						break;
					case game::e_hitbox::pelvis:
						return "pelvis";
						break;
					case game::e_hitbox::stomach:
						return "stomach";
						break;
					case game::e_hitbox::lower_chest:
						return "lower chest";
						break;
					case game::e_hitbox::chest:
						return "chest";
						break;
					case game::e_hitbox::upper_chest:
						return "upper chest";
						break;
					default:
						return "hands/legs";
						break;
					}
				};

				std::string solve_method = resolver_mode( ideal_select->m_record->m_resolver_method );

				if ( find ) {
					msg << xor_str( "[ advance.tech ] client shot -> pred dmg: " ) << std::to_string( static_cast < int > ( ideal_select->m_dmg ) ).data( ) << xor_str( " | " );
					msg << xor_str( "hc: " ) << std::to_string( hit_chance ).data( ) << xor_str( " | " );
					msg << xor_str( "speed_2d: " ) << std::to_string( static_cast < int > ( ideal_select->m_record->m_anim_velocity.length( 2u ) ) ).data( ) << xor_str( " | " );
					msg << xor_str( "hitbox: " ) << std::string( get_hitbox_name_by_id( ideal_select->m_hit_box ) ).data( ) << xor_str ( " | " );
					msg << xor_str( "solve method: " ) << solve_method.data ( ) << xor_str ( " | " );
					msg << xor_str( "velocity stage: " ) << std::to_string( ideal_select->m_record->m_velocity_step ).data( );
				}

				g_ctx->was_shooting( ) = true;

				static auto weapon_recoil_scale = game::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) );

				g_shots->add(
					g_ctx->shoot_pos( ), ideal_select->m_target,
					hacks::g_exploits->m_shift_amount, user_cmd.m_number, game::g_global_vars.get ( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
				);

				g_shots->m_elements.back( ).m_str = msg.str( );

				user_cmd.m_buttons |= game::e_buttons::in_attack;
				g_ctx->get_auto_peek_info( ).m_is_firing = true;
				g_ctx->anim_data( ).m_local_data.m_shot = true;
				user_cmd.m_tick = game::to_ticks( ideal_select->m_record->m_sim_time + g_ctx->net_info( ).m_lerp );

				user_cmd.m_view_angles = m_angle;
				user_cmd.m_view_angles -= g_local_player->self( )->aim_punch( ) * weapon_recoil_scale->get_float( );

				user_cmd.m_view_angles.x( ) = std::remainder( user_cmd.m_view_angles.x( ), 360.f );
				user_cmd.m_view_angles.y( ) = std::remainder( user_cmd.m_view_angles.y( ), 360.f );
				user_cmd.m_view_angles.z( ) = std::remainder( user_cmd.m_view_angles.z( ), 360.f );

				user_cmd.m_view_angles.x( ) = std::clamp( user_cmd.m_view_angles.x( ), -89.f, 89.f );
				user_cmd.m_view_angles.y( ) = std::clamp( user_cmd.m_view_angles.y( ), -180.f, 180.f );
				user_cmd.m_view_angles.z( ) = std::clamp( user_cmd.m_view_angles.z( ), -90.f, 90.f );

				g_ctx->anim_data( ).m_local_data.m_last_shot_time = game::g_global_vars.get ( )->m_cur_time;

				g_visuals->add_shot_mdl( ideal_select->m_player, ideal_select->m_record->m_bones.data( ) );
			}

			lag_backup.restore ( ideal_select->m_player );
		}

		m_targets.clear( );
	}

	void c_knife_bot::handle_knife_bot( game::user_cmd_t& cmd )
	{
		m_best_index = -1;

		if ( !g_local_player->self ( )->weapon( ) )
			return;

		if ( !g_local_player->self( )->weapon( )->info ( ) )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type != game::e_weapon_type::knife )
			return;

		if ( !select_target( ) )
			return;

		auto best_angle = get_hitbox_pos( 5, m_best_player );
		auto entity_angle = sdk::calc_ang( g_ctx->shoot_pos( ), best_angle );
		auto health = m_best_player->health( );
		auto stab = false;

		if ( m_best_player->has_heavy_armor( ) )
		{
			if ( health <= 55 && health > get_min_dmg( ) )
				stab = true;
		}
		else
		{
			if ( health <= 65 && health > get_min_dmg( ) )
				stab = true;
		}

		if ( health >= 90 )
			stab = true;

		stab = stab && m_best_distance < 60;

		cmd.m_view_angles = sdk::qang_t ( entity_angle.x ( ), entity_angle.y ( ), entity_angle.z ( ) );

		if ( stab )
			cmd.m_buttons |= game::e_buttons::in_attack2;
		else
			cmd.m_buttons |= game::e_buttons::in_attack;

		cmd.m_tick = game::to_ticks( m_best_player->sim_time( ) + g_ctx->net_info( ).m_lerp );
	}
	__forceinline float dist_to( sdk::vec3_t& from, const sdk::vec3_t& value ) { return ( from - value ).length( ); }

	bool c_knife_bot::select_target( )
	{
		float good_distance = 75.0f;

		for ( int i = 1; i <= game::g_global_vars.get ( )->m_max_clients; i++ )
		{
			game::cs_player_t* player = ( game::cs_player_t* ) game::g_entity_list->get_entity( i );

			if ( !player || player == g_local_player->self ( ) || player->networkable ( )->dormant( ) || !player->alive( ) || player->team( ) == g_local_player->self( )->team( ) )
				continue;

			sdk::vec3_t local_position = g_local_player->self( )->origin( );
			local_position.z ( ) += 50.0f;

			sdk::vec3_t entity_position = player->origin( );
			entity_position.z ( ) += 50.0f;

			float current_distance = dist_to( local_position, entity_position );

			if ( current_distance < good_distance )
			{
				good_distance = current_distance;
				m_best_index = i;
				m_best_player = player;
			}
		}

		m_best_distance = good_distance;

		return m_best_index != -1;
	}
}