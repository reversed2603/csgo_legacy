#include "../../../csgo.hpp"
#include <execution>

namespace csgo::hacks {
	void c_aim_bot::handle_ctx( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		m_targets.clear( );
		m_angle = {};
		hacks::g_move->allow_early_stop( ) = true;

		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if ( !g_local_player->self( )->weapon( )
			|| !g_local_player->self( )->weapon( )->info( ) )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::knife )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::c4
			|| g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::grenade )
			return;

		if ( m_cfg->m_threading ) {
			static const auto once = [ ]( ) {
				const auto fn = reinterpret_cast< int( _cdecl* )( ) >(
					GetProcAddress( GetModuleHandle( xor_str( "tier0.dll" ) ), xor_str( "AllocateThreadID" ) )
					);

				std::counting_semaphore<> sem{ 0u };

				for ( std::size_t i{}; i < std::thread::hardware_concurrency( ); ++i )
					sdk::g_thread_pool->enqueue(
						[ ]( decltype( fn ) fn, std::counting_semaphore<>& sem ) {
							sem.acquire( );
							fn( );
						}, fn, std::ref( sem )
							);

				for ( std::size_t i{}; i < std::thread::hardware_concurrency( ); ++i )
					sem.release( );

				sdk::g_thread_pool->wait( );

				return true;
			}( );
		}

		if ( !m_cfg->m_rage_bot )
			return;

		setup_hitboxes( m_hit_boxes );

		if ( m_hit_boxes.empty( ) )
			return;

		if ( g_ctx->can_shoot ( ) && !valve::g_client_state.get ( )->m_choked_cmds
			&& !g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key ) 
			&& g_local_player->weapon_info ( )->m_type != valve::e_weapon_type::grenade ) {
			send_packet = false;
			user_cmd.m_buttons &= ~valve::e_buttons::in_attack;
			return;
		}

		add_targets ( );
		select_target ( );

		if( m_targets.size( ) > 0 ) {
			select ( user_cmd, send_packet );
		}
	}

	void c_aim_bot::get_hitbox_data ( c_hitbox* rtn, valve::cs_player_t* ent, int ihitbox, const valve::bones_t& matrix )
	{
		if ( ihitbox < 0 || ihitbox > 19 ) return;

		if ( !ent ) return;

		valve::studio_hdr_t* const studio_hdr = ent->mdl_ptr ( );

		if ( !studio_hdr )
			return;

		valve::studio_bbox_t* const hitbox = studio_hdr->m_studio->p_hitbox ( ihitbox, ent->hitbox_set_index ( ) );

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
		const int bullets, const valve::e_item_index item_index,
		const float recoil_index, const std::size_t i
	) {
		g_ctx->addresses ( ).m_random_seed ( i + 1u );

		auto v1 = g_ctx->addresses ( ).m_random_float ( 0.f, 1.f );
		auto v2 = g_ctx->addresses ( ).m_random_float ( 0.f, k_pi2 );

		float v3 {}, v4 {};

		using fn_t = void ( __stdcall* )( valve::e_item_index, int, int, float*, float* );

		if ( g_ctx->cvars ( ).m_weapon_accuracy_shotgun_spread_patterns->get_int ( ) > 0 )
			reinterpret_cast< fn_t >( g_ctx->addresses ( ).m_calc_shotgun_spread )( item_index, 0, static_cast< int >( bullets * recoil_index ), &v4, &v3 );
		else {
			v3 = g_ctx->addresses ( ).m_random_float ( 0.f, 1.f );
			v4 = g_ctx->addresses ( ).m_random_float ( 0.f, k_pi2 );
		}

		if ( recoil_index < 3.f
			&& item_index == valve::e_item_index::negev ) {
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

	std::optional < aim_target_t > c_aim_bot::extrapolate ( const player_entry_t& entry ) const {
		if ( entry.m_lag_records.empty ( ) )
			return std::nullopt;

		const auto& latest = entry.m_lag_records.front ( );

		if ( latest->m_choked_cmds > crypt_int ( 20 )
			|| latest->m_dormant
			|| latest->m_choked_cmds <= 0 )
			return std::nullopt;

		const auto& net_info = g_ctx->net_info( );

		if ( latest->valid( ) ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		const auto receive_tick = std::abs( ( valve::g_client_state.get( )->m_server_tick + ( valve::to_ticks( net_info.m_latency.m_out ) ) ) - valve::to_ticks( latest->m_sim_time ) );

		if ( ( receive_tick / latest->m_choked_cmds ) > 20
			|| ( receive_tick / latest->m_choked_cmds ) <= 0
			|| !receive_tick ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		const auto delta_ticks = valve::g_client_state.get( )->m_server_tick - latest->m_receive_tick;

		if ( valve::to_ticks( g_ctx->net_info( ).m_latency.m_out ) <= latest->m_choked_cmds - delta_ticks ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		extrapolation_data_t data { entry.m_player, latest };

		float change = 0.f, dir = 0.f;

		// get the direction of the current velocity.
		if ( data.m_velocity.y ( ) != 0.f || data.m_velocity.x ( ) != 0.f )
			dir = sdk::to_deg( std::atan2( data.m_velocity.y( ), data.m_velocity.x( ) ) );

		// we have more than one update
		// we can compute the direction.
		if ( entry.m_lag_records.size ( ) > 1 ) {
			// get the delta time between the 2 most recent records.
			float dt = latest->m_sim_time - entry.m_lag_records.at ( 1 )->m_sim_time;

			// init to 0.
			float prevdir = 0.f;

			// get the direction of the prevoius velocity.
			if ( entry.m_lag_records.at( 1 )->m_anim_velocity.y ( ) != 0.f || entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) != 0.f )
				prevdir = sdk::to_deg( std::atan2( entry.m_lag_records.at( 1 )->m_anim_velocity.y ( ), entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) ) );

			// compute the direction change per tick.
			change = ( sdk::norm_yaw( dir - prevdir ) / dt ) * valve::g_global_vars.get ( )->m_interval_per_tick;
		}

		if ( std::abs( change ) > 6.f )
			change = 0.f;

		data.m_change = change;
		data.m_dir = dir;

		for ( int i {}; i < latest->m_choked_cmds; ++i ) {
			data.m_dir = sdk::norm_yaw( data.m_dir + change );
			data.m_sim_time += valve::g_global_vars.get( )->m_interval_per_tick;
			
			player_move( data );
			//hacks::g_sim_ctx->handle_context ( data ); somehow calculate fwd/side of enemy, otherwise its p
		}	

		latest->m_extrapolated = true;

		aim_target_t ret{ const_cast< player_entry_t* >( &entry ), std::make_shared< lag_record_t >( ) };

		*ret.m_lag_record.value ( ).get ( ) = *latest.get ( );

		ret.m_lag_record.value( )->m_sim_time = data.m_sim_time;
		ret.m_lag_record.value( )->m_flags = data.m_flags;

		ret.m_lag_record.value( )->m_origin = data.m_origin;
		ret.m_lag_record.value( )->m_anim_velocity = data.m_velocity;

		const auto origin_delta = data.m_origin - latest->m_origin;

		for ( std::size_t i{}; i < latest->m_bones_count; ++i ) {
			auto& bone = latest->m_bones.at( i );

			bone[ 0 ][ 3 ] += origin_delta.x( );
			bone[ 1 ][ 3 ] += origin_delta.y( );
			bone[ 2 ][ 3 ] += origin_delta.z( );
		}

		latest->m_extrapolated_bones = latest->m_bones;

		return ret;
	}

	void c_aim_bot::player_move ( extrapolation_data_t& data ) const {
		static auto sv_gravity = valve::g_cvar->find_var ( xor_str ( "sv_gravity" ) );
		static auto sv_jump_impulse = valve::g_cvar->find_var ( xor_str ( "sv_jump_impulse" ) );
		static auto sv_enable_bhop = valve::g_cvar->find_var ( xor_str ( "sv_enablebunnyhopping" ) );
		if ( data.m_flags & valve::e_ent_flags::on_ground ) {
			if ( !sv_enable_bhop->get_int ( ) ) {
				const auto speed = data.m_velocity.length ( );

				const auto max_speed = data.m_player->max_speed ( ) * crypt_float ( 1.1f );
				if ( max_speed > 0.f
					&& speed > max_speed )
					data.m_velocity *= ( max_speed / speed );
			}

			data.m_velocity.z ( ) = sv_jump_impulse->get_float ( );
		}
		else
			data.m_velocity.z ( ) -=
			sv_gravity->get_float ( ) * valve::g_global_vars.get ( )->m_interval_per_tick;

		valve::trace_t trace {};
		valve::trace_filter_world_only_t trace_filter {};

		valve::g_engine_trace->trace_ray (
			{
			 data.m_origin,
			 data.m_origin + data.m_velocity * valve::g_global_vars.get ( )->m_interval_per_tick,
			 data.m_obb_min, data.m_obb_max
			},
			valve::e_mask::contents_solid, &trace_filter, &trace
		);

		if ( trace.m_frac != crypt_float ( 1.f ) ) {
			for ( int i {}; i < 2; ++i ) {
				data.m_velocity -= trace.m_plane.m_normal * data.m_velocity.dot ( trace.m_plane.m_normal );

				const auto adjust = data.m_velocity.dot ( trace.m_plane.m_normal );
				if ( adjust < 0.f )
					data.m_velocity -= trace.m_plane.m_normal * adjust;

				valve::g_engine_trace->trace_ray (
					{
					 trace.m_end,
					 trace.m_end + ( data.m_velocity * ( valve::g_global_vars.get ( )->m_interval_per_tick * ( 1.f - trace.m_frac ) ) ),
					 data.m_obb_min, data.m_obb_max
					},
					valve::e_mask::contents_solid, &trace_filter, &trace
				);

				if ( trace.m_frac == 1.f )
					break;
			}
		}

		data.m_origin = trace.m_end;

		valve::g_engine_trace->trace_ray (
			{
			 trace.m_end,
			 { trace.m_end.x ( ) , trace.m_end.y ( ) , trace.m_end.z ( ) - crypt_float ( 2.f ) },
			 data.m_obb_min, data.m_obb_max
			},
			valve::e_mask::contents_solid, &trace_filter, &trace
		);

		data.m_flags &= ~valve::e_ent_flags::on_ground;

		if ( trace.m_frac != crypt_float ( 1.f )
			&& trace.m_plane.m_normal.z ( ) > crypt_float ( 0.7f ) )
			data.m_flags |= valve::e_ent_flags::on_ground;
	}

	float get_min_dmg_override( ) {
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0.f;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_min_dmg_on_key;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_syka_min_dmg_on_key;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_min_dmg_on_key;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_min_dmg_on_key;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_min_dmg_on_key;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_min_dmg_on_key;
		default:
			return 0.f;
		}

		return 0.f;
	}

	int get_force_head_conditions( ) {
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_force_head_conditions_awp;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_force_head_conditions_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_force_head_conditions_scar;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_force_head_conditions_other;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_force_head_conditions_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_force_head_conditions_pistol;
		default:
			return 0;
		}

		return 0;
	}

	int get_force_body_conditions( ) {
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_force_body_conditions_awp;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_force_body_conditions_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_force_body_conditions_scar;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_force_body_conditions_other;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_force_body_conditions_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_force_body_conditions_pistol;
		default:
			return 0;
		}

		return 0;
	}

	bool get_min_dmg_override_state( ) {
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return false;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_key_binds->get_keybind_state ( &g_aim_bot->cfg( ).m_min_awp_dmg_key );
		case valve::e_item_index::ssg08:
			return g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_min_scout_dmg_key );
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_min_scar_dmg_key );
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_min_other_dmg_key );
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_min_heavy_pistol_dmg_key );
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_key_binds->get_keybind_state( &g_aim_bot->cfg( ).m_min_pistol_dmg_key );
		default:
			return false;
		}

		return false;
	}

	float get_min_dmg_to_set_up( )
	{
		if ( !g_local_player->self ( ) 
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0.f;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg ( ).m_min_dmg_awp;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_min_dmg_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_min_dmg_scar;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_min_dmg_other;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_min_dmg_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_min_dmg_pistol;
		default:
			return 0.f;
		}

		return 0.f;
	}

	__forceinline bool get_prefer_body( ) {
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return false;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_prefer_body;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_prefer_body;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_prefer_body;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_prefer_body;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_prefer_body;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_prefer_body;
		default:
			return false;
		}

		return false;
	}

	int get_autostop_type( )
	{
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_move->cfg( ).m_auto_stop_type_awp;
		case valve::e_item_index::ssg08:
			return g_move->cfg( ).m_auto_stop_type_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_move->cfg( ).m_auto_stop_type_scar;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_move->cfg( ).m_auto_stop_type_other;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_move->cfg( ).m_auto_stop_type_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_move->cfg( ).m_auto_stop_type_pistol;
		default:
			return 0;
		}

		return 0;
	}

	int get_hitboxes_setup( )
	{
		if ( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_hitboxes;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_hitboxes;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_hitboxes;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_hitboxes;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_hitboxes;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_hitboxes;
		default:
			return 0;
		}

		return 0;
	}

	float get_head_scale( )
	{
		if ( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg ( ).m_awp_head_scale;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_head_scale;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_head_scale;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_head_scale;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_head_scale;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_head_scale;
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
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_awp_body_scale;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_scout_body_scale;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_scar_body_scale;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_other_body_scale;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_heavy_pistol_body_scale;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_pistol_body_scale;
		default:
			return 0;
		}

		return 0;
	}

	__forceinline float get_hit_chance( ) {
		if ( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if ( !wpn )
			return 0;

		switch ( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return g_aim_bot->cfg( ).m_hit_chance_awp;
		case valve::e_item_index::ssg08:
			return g_aim_bot->cfg( ).m_hit_chance_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_aim_bot->cfg( ).m_hit_chance_scar;
		case valve::e_item_index::ak47:
		case valve::e_item_index::aug:
		case valve::e_item_index::bizon:
		case valve::e_item_index::famas:
		case valve::e_item_index::galil:
		case valve::e_item_index::m249:
		case valve::e_item_index::m4a4:
		case valve::e_item_index::m4a1s:
		case valve::e_item_index::mac10:
		case valve::e_item_index::mag7:
		case valve::e_item_index::mp5sd:
		case valve::e_item_index::mp7:
		case valve::e_item_index::mp9:
		case valve::e_item_index::negev:
		case valve::e_item_index::nova:
		case valve::e_item_index::sawed_off:
		case valve::e_item_index::sg553:
		case valve::e_item_index::ump45:
		case valve::e_item_index::xm1014:
		case valve::e_item_index::p90:
			return g_aim_bot->cfg( ).m_hit_chance_other;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_aim_bot->cfg( ).m_hit_chance_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_aim_bot->cfg( ).m_hit_chance_pistol;
		default:
			return 0;
		}

		return 0;
	}

	static int clip_ray_to_hitbox( const valve::ray_t& ray, valve::studio_bbox_t* hitbox, sdk::mat3x4_t& matrix, valve::trace_t& trace ) {
		if ( !g_ctx->addresses ( ).m_clip_ray
			|| !hitbox )
			return -1;

		trace.m_frac = 1.0f;
		trace.m_start_solid = false;

		return reinterpret_cast < int( __fastcall* )( const valve::ray_t&, valve::studio_bbox_t*, sdk::mat3x4_t&, valve::trace_t& ) > ( g_ctx->addresses( ).m_clip_ray )( ray, hitbox, matrix, trace );
	}

	int c_aim_bot::calc_hit_chance (
		valve::cs_player_t* player, std::shared_ptr < lag_record_t > record, const sdk::qang_t& angle, const std::ptrdiff_t hit_box
	) {
		build_seed_table ( );

		if ( static_cast < int > ( g_eng_pred->inaccuracy( ) * 10000.f ) == 0 ) {
			return 101;
		}

		const auto is_scope_able_weapon = g_local_player->weapon( )->item_index( ) == valve::e_item_index::scar20 || g_local_player->weapon( )->item_index( ) == valve::e_item_index::g3sg1
			|| g_local_player->weapon( )->item_index( ) == valve::e_item_index::ssg08 || g_local_player->weapon( )->item_index( ) == valve::e_item_index::awp;

		auto min_dmg = get_min_dmg_to_set_up ( ) + 1;

		if ( get_min_dmg_override_state( ) )
			min_dmg = get_min_dmg_override( ) + 1;

		min_dmg = std::clamp( static_cast< int >( min_dmg * ( 2.f / 100.f ) ), 1, 100 );

		c_hitbox ht;
		get_hitbox_data ( &ht, player, hit_box, record->m_bones );

		sdk::vec3_t fwd {}, right {}, up {};
		sdk::ang_vecs ( angle, &fwd, &right, &up );

		int hits {};
		const auto trace_spread = [ ](
			const sdk::vec3_t& fwd, const sdk::vec3_t& right, const sdk::vec3_t& up,
			valve::cs_player_t* player, const int accuracy_boost,
			const valve::e_item_index item_index, const float recoil_index,
			const int min_dmg, const std::size_t i, bool& hit ) {
				const auto spread_angle = calc_spread_angle( g_local_player->self( )->weapon( )->info( )->m_bullets, item_index, recoil_index, i );

				auto dir = fwd + ( right * spread_angle.x( ) ) + ( up * spread_angle.y( ) );

				dir.normalize( );

				auto em = g_ctx->shoot_pos( ) + ( dir * g_local_player->self( )->weapon( )->info( )->m_range );

				const auto pen_data = g_auto_wall->wall_penetration( g_ctx->shoot_pos( ),
					em,
					player );

				hit = pen_data.m_dmg > 0 && ( accuracy_boost <= 0 || pen_data.m_dmg >= min_dmg );
		};

	
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

			valve::trace_t trace {};
			valve::ray_t ray{ g_ctx->shoot_pos( ), end };

			intersected = clip_ray_to_hitbox( ray, ht.m_hitbox, record->m_bones[ ht.m_hitbox->m_bone ], trace ) >= 0;

			seeds.at( i ) = intersected;
						
		}
		for ( auto& hit : seeds )
			if ( hit )
				++hits;

		if ( static_cast< int >( ( hits / static_cast< float >( k_max_seeds ) ) * 100.f ) >= 15
			&& g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground
			&& is_scope_able_weapon
			&& !g_local_player->self( )->scoped( )
			&& g_ctx->can_shoot( ) ) {
			if ( g_eng_pred->min_inaccuracy( ) >= ( g_local_player->weapon( )->accuracy_penalty( ) * 0.02f ) )
				return 101;
		}

		return static_cast< int >( ( hits / static_cast< float >( k_max_seeds ) ) * 100.f );
	}

	void c_aim_bot::add_targets ( ) {
		m_targets.reserve ( valve::g_global_vars.get ( )->m_max_clients );

		for ( std::ptrdiff_t i = 1; i <= valve::g_global_vars.get ( )->m_max_clients; ++i ) {
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
		auto hitbox_stomach = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( valve::e_hitbox::stomach ) );
		auto hitbox_head = hitbox_set->get_bbox( static_cast < std::ptrdiff_t > ( valve::e_hitbox::head ) );

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
		body_point_.m_index = valve::e_hitbox::stomach;
		body_point_.m_pos = body_point;

		point_t head_point_{};
		head_point_.m_center = true;
		head_point_.m_index = valve::e_hitbox::head;
		head_point_.m_pos = head_point;

		points.clear( );

		points.push_back( body_point_ );
		points.push_back( head_point_ );

		record->adjust( target.m_entry->m_player );

		for ( auto& point : points ) {
			if ( !head_point_.m_valid
				&& !body_point_.m_valid )
			scan_point( target.m_entry, point, 1.f, false, shoot_pos );
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

			if ( lag_record->m_invalid )
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
					const auto& pen_data = target.m_best_point->m_pen_data;
					const auto& best_pen_data = best_aim_point.value( ).m_pen_data;

					if ( std::abs( pen_data.m_dmg - best_pen_data.m_dmg ) > crypt_int ( 10 ) ) {
						if ( pen_data.m_dmg <= target.m_entry->m_player->health( )
							|| best_pen_data.m_dmg <= target.m_entry->m_player->health( ) ) {
							if ( pen_data.m_dmg > best_pen_data.m_dmg ) {
								best_record = lag_record;
								best_aim_point = *target.m_best_point;

								continue;
							}
						}
					}

					if ( pen_data.m_dmg > best_pen_data.m_dmg ) {
						best_record = lag_record;
						best_aim_point = *target.m_best_point;
					}
				}
				else {
					if ( lag_record->m_flicked ) {
						best_record = lag_record;
					}
				}

				lag_backup.restore( lag_record->m_player );
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
			|| latest->m_dormant 
			|| latest->m_invalid )
			return std::nullopt;

		if ( !latest->valid( ) )
			return std::nullopt;

		if ( latest->m_broke_lc ) {

			const auto adjusted_arrive_tick = std::clamp( valve::to_ticks( ( ( g_ctx->net_info( ).m_latency.m_out ) + valve::g_global_vars.get( )->m_real_time )
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
	void c_aim_bot::calc_capsule_points ( aim_target_t& target, const valve::studio_bbox_t* hit_box, const std::ptrdiff_t index,
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

			target.m_points.emplace_back( final_point, static_cast < valve::e_hitbox > ( index ), false );
		}
	}

	void c_aim_bot::add_aim_point( aim_target_t& target, sdk::vec3_t& point, valve::e_hitbox index, bool is_center ) {
		target.m_points.emplace_back( point, index, is_center );
		g_aim_bot->m_nigga_hack.emplace_back( point, index, is_center );
	}

	void c_aim_bot::setup_points ( aim_target_t& target, std::shared_ptr < lag_record_t > record, valve::e_hitbox index, e_hit_scan_mode mode
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
		float scale = get_head_scale( ) / 100.f;

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

			if ( index > valve::e_hitbox::head ) {
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

				return calc_capsule_points( target, hitbox, static_cast < std::ptrdiff_t > ( index ),
					record->m_bones[ hitbox->m_bone ], bscale );
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

		if ( g_local_player->self ( )->weapon ( )->item_index ( ) == valve::e_item_index::taser ) {
			hitboxes.push_back ( { valve::e_hitbox::stomach, e_hit_scan_mode::prefer } );
			return;
		}

		hitboxes.push_back ( { valve::e_hitbox::stomach, e_hit_scan_mode::has_lethality } );
		hitboxes.push_back ( { valve::e_hitbox::pelvis, e_hit_scan_mode::has_lethality } );

		if ( get_hitboxes_setup ( ) & 1 ) {
			hitboxes.push_back ( { valve::e_hitbox::head, e_hit_scan_mode::normal } );
		}

		if ( get_hitboxes_setup( ) & 2 ) {
			hitboxes.push_back ( { valve::e_hitbox::lower_chest, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::chest, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::upper_chest, e_hit_scan_mode::normal } );
		}

		if ( get_hitboxes_setup( ) & 4 ) {
			hitboxes.push_back ( { valve::e_hitbox::pelvis, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::stomach, e_hit_scan_mode::normal } );
		}

		if ( get_hitboxes_setup( ) & 8 ) {
			hitboxes.push_back ( { valve::e_hitbox::left_upper_arm,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::right_upper_arm, e_hit_scan_mode::normal } );
		}

		if ( get_hitboxes_setup( ) & 16 ) {
			hitboxes.push_back ( { valve::e_hitbox::left_thigh, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::right_thigh,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::left_calf, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::right_calf,e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::left_foot, e_hit_scan_mode::normal } );
			hitboxes.push_back ( { valve::e_hitbox::right_foot, e_hit_scan_mode::normal } );
		}
	}

	void c_aim_bot::scan_point ( player_entry_t* entry,
		point_t& point, float min_dmg_key, bool min_dmg_key_pressed, sdk::vec3_t& shoot_pos ) {

		point.m_pen_data = g_auto_wall->wall_penetration ( shoot_pos, point.m_pos, entry->m_player );

		auto min_dmg = get_min_dmg_to_set_up ( );

		if ( /*g_key_binds->get_keybind_state( &m_cfg->m_min_dmg_key )*/ min_dmg_key_pressed )
			min_dmg = min_dmg_key + 1;

		if ( point.m_pen_data.m_dmg < entry->m_player->health ( ) )
			if ( point.m_pen_data.m_dmg < min_dmg )
				return;


		point.m_valid = true;
	}

	bool c_aim_bot::scan_points ( cc_def ( aim_target_t* ) target, std::vector < point_t >& points, bool additional_scan ) const {
		std::array < point_t*, 20 > best_points {};
		static auto min_dmg_on_key_val{ 0.f };
		min_dmg_on_key_val = get_min_dmg_override( );

		static bool min_dmg_key_pressed{ false };

		if ( get_min_dmg_override_state( ) )
			min_dmg_key_pressed = true;
		else
			min_dmg_key_pressed = false;
		for ( auto& point : points ) {

			if ( additional_scan )
			scan_point( target.get ( )->m_entry, point, static_cast < int > ( min_dmg_on_key_val ), min_dmg_key_pressed );

			if ( !point.m_valid 
				|| point.m_pen_data.m_dmg < 1 )
				continue;

			const auto hp = target.get ( )->m_entry->m_player->health( );

			auto& best_point = best_points.at ( static_cast < std::ptrdiff_t > ( point.m_index ) );

			if ( !best_point ) {
				best_point = &point; // init best point that we can compare to next points
				continue;
			}

			const auto& best_pen_data = best_point->m_pen_data;
			const auto& pen_data = point.m_pen_data;

			if ( point.m_center ) {
				if ( ( best_pen_data.m_hitgroup == pen_data.m_hitgroup )
					|| ( best_pen_data.m_remaining_pen == pen_data.m_remaining_pen && std::abs( best_pen_data.m_dmg - pen_data.m_dmg ) <= crypt_int ( 1 ) )
					|| ( best_pen_data.m_dmg > hp && pen_data.m_dmg > hp ) ) {
					best_point = &point;
				}

				continue;
			}	

			auto& cur_dmg = pen_data.m_dmg;
			auto& last_dmg = best_pen_data.m_dmg;

			if ( last_dmg == cur_dmg ) {
				continue;
			}

			if ( cur_dmg >= hp
				&& last_dmg < hp ) {
				best_point = &point;
				break; // lethal point, its perfect
			}

			if ( best_pen_data.m_hitgroup != pen_data.m_hitgroup
				|| best_pen_data.m_remaining_pen != pen_data.m_remaining_pen ) {
				if ( best_pen_data.m_remaining_pen != pen_data.m_remaining_pen
					|| std::abs( best_pen_data.m_dmg - pen_data.m_dmg ) > 1 ) {
					if ( best_pen_data.m_dmg <= hp || pen_data.m_dmg <= hp ) {
						if ( pen_data.m_dmg > best_pen_data.m_dmg )
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
				const auto& best_pen_data = target.get( )->m_best_point->m_pen_data;
				const auto& pen_data = point.m_pen_data;

				auto v31 = false;
				if ( std::abs( best_pen_data.m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data.m_dmg <= hp ) )
					v31 = pen_data.m_dmg > best_pen_data.m_dmg;

				if ( v31 )
					target.get( )->m_best_point = &point;
			}

			if ( point.m_index == valve::e_hitbox::stomach
				|| point.m_index == valve::e_hitbox::pelvis ) {
				if ( !target.get( )->m_best_body_point )
					target.get( )->m_best_body_point = &point;
				else {
					const auto& best_pen_data = target.get ( )->m_best_body_point->m_pen_data;
					const auto& pen_data = point.m_pen_data;

					auto v31 = false;
					if ( std::abs( best_pen_data.m_dmg - pen_data.m_dmg ) > 1 && ( pen_data.m_dmg <= hp || best_pen_data.m_dmg <= hp ) )
						v31 = pen_data.m_dmg > best_pen_data.m_dmg;

					if ( v31 )
						target.get( )->m_best_body_point = &point;
				}
			}
		}
		if ( target.get( )->m_best_body_point ) {
			g_aim_bot->m_cur_body_point = target.get( )->m_best_body_point->m_pos;
		}
		if ( target.get( )->m_best_point ) {
			g_aim_bot->m_cur_head_point = target.get( )->m_best_point->m_pos;
		}

		return true;
	}

	point_t* c_aim_bot::select_point ( cc_def ( aim_target_t* ) target, const int cmd_num ) {
		if ( !target.get( )->m_best_body_point
			|| target.get( )->m_best_body_point->m_pen_data.m_dmg < crypt_int( 1 ) ) {
			return target.get( )->m_best_point;
		}

		const auto hp = target.get( )->m_entry->m_player->health( );

		if ( g_key_binds->get_keybind_state( &m_cfg->m_baim_key ) )
			return target.get( )->m_best_body_point;

		if ( get_force_body_conditions( ) & 1 ) {
			if ( target.get( )->m_best_body_point->m_pen_data.m_dmg >= hp ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 2 ) {
			if ( !( target.get( )->m_lag_record.value( )->m_flags & valve::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 4 ) {
			if ( !target.get( )->m_entry->m_moved ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( get_force_body_conditions( ) & 8 ) {
			if ( target.get( )->m_lag_record.value ( )->m_has_fake ) {
				return target.get( )->m_best_body_point;
			}
		}

		if ( target.get( )->m_best_body_point->m_pen_data.m_dmg < hp ) {

			if ( get_force_head_conditions ( ) & 1
				&& target.get( )->m_entry->m_predicting_lby ) {
				return target.get( )->m_best_point;
			}

			if ( get_force_head_conditions( ) & 2
				&& target.get( )->m_lag_record.value( )->m_anim_velocity.length( 2u ) > 75.f
				&& !target.get( )->m_lag_record.value( )->m_fake_walking 
				&& ( target.get ( )->m_lag_record.value ( )->m_flags & valve::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_point;
			}

			if ( get_force_head_conditions( ) & 4
				&& target.get( )->m_lag_record.value( )->m_resolved ) {
				return target.get( )->m_best_point;
			}
		}

		if ( get_force_head_conditions( ) & 8
			&& target.get ( )->m_best_body_point->m_pen_data.m_dmg < hp
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
					&& target.get( )->m_best_body_point->m_pen_data.m_dmg >= hp_left )
					return target.get( )->m_best_body_point;
			}
		}

		if ( target.get( )->m_best_body_point->m_pen_data.m_dmg >= target.get( )->m_entry->m_player->health ( ) )
			return target.get( )->m_best_body_point;

		if ( g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key )
			&& target.get( )->m_best_body_point->m_pen_data.m_dmg * crypt_float ( 2.f ) >= target.get( )->m_entry->m_player->health ( )
			&& g_local_player->weapon ( )->item_index ( ) != valve::e_item_index::ssg08 
			&& get_prefer_body( ) ) {
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

		if ( g_local_player->self ( )->flags ( ) & valve::e_ent_flags::frozen )
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

		if ( weapon_data->m_type >= valve::e_weapon_type::pistol && weapon_data->m_type <= valve::e_weapon_type::machine_gun && weapon->clip1 ( ) < crypt_int ( 1 ) )
			return false;

		float curtime = valve::to_time ( g_local_player->self ( )->tick_base ( ) - shift_amount );
		if ( curtime < g_local_player->self ( )->next_attack ( ) )
			return false;

		if ( ( weapon->item_index ( ) == valve::e_item_index::glock || weapon->item_index ( ) == valve::e_item_index::famas ) && weapon->burst_shots_remaining ( ) > crypt_int ( 0 ) ) {
			if ( curtime >= weapon->next_burst_shot ( ) )
				return true;
		}

		if ( curtime < weapon->next_primary_attack ( ) )
			return false;

		if ( weapon->item_index ( ) != valve::e_item_index::revolver )
			return true;

		if ( skip_r8 )
			return true;

		return curtime >= weapon->postpone_fire_ready_time ( );
	}

	bool c_aim_bot::can_hit ( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box ) {
		auto hdr = *( valve::studio_hdr_t** ) record->m_player->mdl_ptr( );
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

	void c_aim_bot::select_target( ) {
		auto sort_targets =[&](aim_target_t& a, aim_target_t& b) {
			// this is the same player
			// in that case, do nothing
			if( a.m_entry->m_player == b.m_entry->m_player || a.m_entry->m_player->networkable( )->index( ) == b.m_entry->m_player->networkable( )->index( ) )
				return false;

			// get fov of player a
			float fov_a = sdk::calc_fov( valve::g_engine->view_angles ( ), g_ctx->shoot_pos( ), a.m_entry->m_player->world_space_center( ) );
	
			// get fov of player b
			float fov_b = sdk::calc_fov( valve::g_engine->view_angles ( ), g_ctx->shoot_pos( ), b.m_entry->m_player->world_space_center( ) );
		
			// if player a fov lower than player b fov prioritize him
			return fov_a < fov_b;
		};
			
		// if we have only 2 targets or less, no need to sort
		if( m_targets.size( ) <= 2 )
			return;

		// std::execution::par -> parallel sorting (multithreaded)
		// NOTE: not obligated, std::sort doesnt take alot of cpu power but its still better
		std::sort( std::execution::par, m_targets.begin( ), m_targets.end( ), sort_targets );

		// target limit based on our prioritized targets
		while( m_targets.size( ) > 3 )
			m_targets.pop_back( );
	}

	void c_aim_bot::select ( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		struct ideal_target_t {
			valve::cs_player_t* m_player {}; sdk::vec3_t m_pos{};
			float m_dmg{}; valve::e_hitbox m_hit_box{}; std::shared_ptr < lag_record_t > m_record{}; aim_target_t* m_target{};
		};
		std::unique_ptr < ideal_target_t > ideal_select = std::make_unique < ideal_target_t >( );
		static auto min_dmg_on_key_val { 0.f };
		min_dmg_on_key_val = get_min_dmg_override ( );

		static bool min_dmg_key_pressed{ false };

		if ( get_min_dmg_override_state ( ) )
			min_dmg_key_pressed = true;
		else
			min_dmg_key_pressed = false;

		if ( m_cfg->m_threading ) {
			for ( auto& target : m_targets )
				sdk::g_thread_pool->enqueue( [ ]( aim_target_t& target ) {
				lag_backup_t backup{};
				backup.setup( target.m_entry->m_player );
				target.m_lag_record.value( )->adjust( target.m_entry->m_player );

				target.m_points.clear( );

				for ( const auto& who : g_aim_bot->m_hit_boxes )
					setup_points( target, target.m_lag_record.value( ), who.m_index, who.m_mode );

				for ( auto& point : target.m_points ) {
					scan_point( target.m_entry, point, static_cast < int > ( min_dmg_on_key_val ), min_dmg_key_pressed );
				}

				backup.restore( target.m_entry->m_player );
					}, std::ref( target ) );

			sdk::g_thread_pool->wait( );
		}
		else {
			for ( auto& target : m_targets ) {
				if ( !target.m_lag_record.has_value( ) )
					continue;

				lag_backup_t backup{};
				backup.setup( target.m_entry->m_player );
				target.m_lag_record.value( )->adjust( target.m_entry->m_player );
				g_aim_bot->m_nigga_hack.clear( );
				target.m_points.clear( );

				for ( const auto& who : g_aim_bot->m_hit_boxes )
					setup_points( target, target.m_lag_record.value( ), who.m_index, who.m_mode );

				backup.restore( target.m_entry->m_player );
			}
		}

		for ( auto& target : m_targets ) {

			hacks::g_move->allow_early_stop( ) = false; 

			scan_points( &target, target.m_points, true );

			const auto point = select_point( &target, user_cmd.m_number );

			if ( point ) {
				ideal_select->m_player = target.m_entry->m_player;
				ideal_select->m_dmg = point->m_pen_data.m_dmg;
				ideal_select->m_record = target.m_lag_record.value( );
				ideal_select->m_hit_box = point->m_index;
				ideal_select->m_pos = point->m_pos;
				ideal_select->m_target = &target;
			}

			if ( ideal_select->m_player
				&& ideal_select->m_record ) {

				if ( hacks::g_exploits->m_type == 5 )
					hacks::g_exploits->m_type = 3;

				g_eng_pred->update_shoot_pos( user_cmd );

				lag_backup_t lag_backup {};
				lag_backup.setup ( ideal_select->m_player );

				ideal_select->m_record->adjust ( ideal_select->m_player );

				ideal_select->m_target->m_pos = ideal_select->m_pos;

				m_angle = ( ideal_select->m_pos - g_ctx->shoot_pos ( ) ).angles ( );

				g_ctx->was_shooting ( ) = false;

				if ( g_ctx->can_shoot( ) ) {

					auto wpn_idx = g_local_player->weapon( )->item_index( );
					bool can_scope = !g_local_player->self( )->scoped( ) && ( wpn_idx == valve::e_item_index::aug || wpn_idx == valve::e_item_index::sg553 || wpn_idx == valve::e_item_index::scar20 || wpn_idx == valve::e_item_index::g3sg1 || g_local_player->weapon_info( )->m_type == valve::e_weapon_type::sniper );

					if ( !( user_cmd.m_buttons & valve::e_buttons::in_jump ) ) {
						if ( can_scope
							&& m_cfg->m_auto_scope )
							user_cmd.m_buttons |= valve::e_buttons::in_attack2;
					}

					const auto hit_chance = calc_hit_chance( ideal_select->m_player, ideal_select->m_record, m_angle, static_cast < std::ptrdiff_t > ( ideal_select->m_hit_box ) );
					if ( hit_chance < ( g_local_player->weapon( )->item_index( ) == valve::e_item_index::taser ? 60.f : get_hit_chance( ) ) ) {
						lag_backup.restore( ideal_select->m_player );

						if ( !( user_cmd.m_buttons & valve::e_buttons::in_jump ) )
							m_should_stop = get_autostop_type( ) + 1;

						return;
					}

					std::stringstream msg;

					valve::player_info_t info;

					auto find = valve::g_engine->get_player_info( ideal_select->m_player->networkable ( )->index ( ), &info );

					auto get_hitbox_name_by_id = [ ] ( valve::e_hitbox id ) -> const char* {
						switch ( id ) {
						case valve::e_hitbox::head:
							return "head";
							break;
						case valve::e_hitbox::neck:
							return "neck";
							break;
						case valve::e_hitbox::pelvis:
							return "pelvis";
							break;
						case valve::e_hitbox::stomach:
							return "stomach";
							break;
						case valve::e_hitbox::lower_chest:
							return "lower chest";
							break;
						case valve::e_hitbox::chest:
							return "chest";
							break;
						case valve::e_hitbox::upper_chest:
							return "upper chest";
							break;
						default:
							return "hands/legs";
							break;
						}
					};

					std::string solve_method{};

					switch ( ideal_select->m_record->m_resolver_method ) {
					case e_solve_methods::no_fake:
						solve_method = "no fake";
						break;
					case e_solve_methods::lby_delta:
						solve_method = "lby delta";
						break;
					case e_solve_methods::fake_walk:
						solve_method = "fake walk";
					break;
					case e_solve_methods::last_move_lby:
						solve_method = "last move logic";
						break;
					case e_solve_methods::last_move:
						solve_method = "last move";
						break;
					case e_solve_methods::backwards:
						solve_method = "backwards";
					break;
					case e_solve_methods::freestand_l:
						solve_method = "anti-fs logic";
					break;
					case e_solve_methods::body_flick:
						solve_method = "flick";
						break;
					case e_solve_methods::brute:
						solve_method = "brute";
						break;
					case e_solve_methods::anti_fs:
						solve_method = "anti-fs";
						break;
					case e_solve_methods::brute_not_moved:
						solve_method = "brute [ no move data ]";
						break;
					case e_solve_methods::anti_fs_not_moved:
						solve_method = "anti-fs [ no move data ]";
						break;
					case e_solve_methods::air:
						solve_method = "in air";
						break;
					case e_solve_methods::move:
						solve_method = "move";
						break;
					default:
						solve_method = "unk";
						break;
					}

					if ( find ) {
						msg << xor_str( "fired shot: pred info [ dmg: " ) << std::to_string( static_cast < int > ( ideal_select->m_dmg ) ).data( ) << xor_str( " | " );
						msg << xor_str( "hc: " ) << std::to_string( hit_chance ).data( ) << xor_str( " | " );
						msg << xor_str( "hitbox: " ) << std::string( get_hitbox_name_by_id( ideal_select->m_hit_box ) ).data( ) << xor_str ( " ] | " );
						msg << xor_str( "speed_2d: " ) << std::to_string( static_cast < int > ( ideal_select->m_record->m_anim_velocity.length( 2u ) ) ).data( ) << xor_str( " | " );
						msg << xor_str( "resolver: " ) << solve_method.data ( ) << xor_str ( " | " );
						msg << xor_str( "velocity step: " ) << std::to_string( ideal_select->m_record->m_velocity_step ).data( );
					}

					constexpr uint8_t gray_clr [ 4 ] = { 201, 201, 201, 255 };

					const std::string msg_to_string = msg.str( );

					g_ctx->was_shooting( ) = true;
					g_ctx->allow_defensive( ) = false;

					static auto weapon_recoil_scale = valve::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) );

					g_shots->add(
						g_ctx->shoot_pos( ), ideal_select->m_target,
						hacks::g_exploits->m_next_shift_amount, user_cmd.m_number, valve::g_global_vars.get ( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
					);
					//g_shots->m_elements.back( ).m_str = msg.str( );
					valve::g_cvar->con_print ( false, *gray_clr, msg_to_string.c_str( ) );
					valve::g_cvar->con_print ( false, *gray_clr, xor_str( "\n" ) );

					user_cmd.m_buttons |= valve::e_buttons::in_attack;
					g_ctx->get_auto_peek_info( ).m_is_firing = true;
					g_ctx->anim_data( ).m_local_data.m_shot = true;
					user_cmd.m_tick = valve::to_ticks( ideal_select->m_record->m_sim_time + g_ctx->net_info( ).m_lerp );

					user_cmd.m_view_angles = m_angle;
					user_cmd.m_view_angles -= g_local_player->self( )->aim_punch( ) * weapon_recoil_scale->get_float( );

					user_cmd.m_view_angles.x( ) = std::remainder( user_cmd.m_view_angles.x( ), 360.f );
					user_cmd.m_view_angles.y( ) = std::remainder( user_cmd.m_view_angles.y( ), 360.f );
					user_cmd.m_view_angles.z( ) = std::remainder( user_cmd.m_view_angles.z( ), 360.f );

					user_cmd.m_view_angles.x( ) = std::clamp( user_cmd.m_view_angles.x( ), -89.f, 89.f );
					user_cmd.m_view_angles.y( ) = std::clamp( user_cmd.m_view_angles.y( ), -180.f, 180.f );
					user_cmd.m_view_angles.z( ) = std::clamp( user_cmd.m_view_angles.z( ), -90.f, 90.f );

					g_ctx->anim_data( ).m_local_data.m_last_shot_time = valve::g_global_vars.get ( )->m_cur_time;

					g_visuals->add_shot_mdl( ideal_select->m_player, ideal_select->m_record->m_bones.data( ) );
				}

				lag_backup.restore ( ideal_select->m_player );
			}
		}

		m_targets.clear( );
	}

	void c_knife_bot::handle_knife_bot( valve::user_cmd_t& cmd )
	{
		m_best_index = -1;

		if ( !g_local_player->self ( )->weapon( ) )
			return;

		if ( !g_local_player->self( )->weapon( )->info ( ) )
			return;

		if ( g_local_player->self( )->weapon( )->info( )->m_type != valve::e_weapon_type::knife )
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
			cmd.m_buttons |= valve::e_buttons::in_attack2;
		else
			cmd.m_buttons |= valve::e_buttons::in_attack;

		cmd.m_tick = valve::to_ticks( m_best_player->sim_time( ) + g_ctx->net_info( ).m_lerp );
	}
	__forceinline float dist_to( sdk::vec3_t& from, const sdk::vec3_t& value ) { return ( from - value ).length( ); }

	bool c_knife_bot::select_target( )
	{
		float good_distance = 75.0f;

		for ( int i = 1; i <= valve::g_global_vars.get ( )->m_max_clients; i++ )
		{
			valve::cs_player_t* player = ( valve::cs_player_t* ) valve::g_entity_list->get_entity( i );

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