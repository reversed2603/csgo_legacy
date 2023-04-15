#include "../../../csgo.hpp"
#include <execution>

namespace csgo::hacks {
	void c_aim_bot::handle_ctx( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		m_targets.clear( );
		m_angle = { };
		hacks::g_move->allow_early_stop( ) = true;

		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return;

		if( !g_local_player->self( )->weapon( )
			|| !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::knife )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::c4
			|| g_local_player->self( )->weapon( )->info( )->m_type == valve::e_weapon_type::grenade )
			return;

		if( m_cfg->m_threading ) {
			static const auto once = [ ]( ) {
				const auto fn = reinterpret_cast< int( _cdecl* )( ) >( 
					GetProcAddress( GetModuleHandle( xor_str( "tier0.dll" ) ), xor_str( "AllocateThreadID" ) )
					 );

				std::counting_semaphore<> sem{ 0u };

				for( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
					sdk::g_thread_pool->enqueue( 
						[ ]( decltype( fn ) fn, std::counting_semaphore<>& sem ) {
							sem.acquire( );
							fn( );
						}, fn, std::ref( sem )
							 );

				for( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
					sem.release( );

				sdk::g_thread_pool->wait( );

				return true;
			}( );
		}

		if( !m_cfg->m_rage_bot )
			return;

		setup_hitboxes( m_hit_boxes );

		if( m_hit_boxes.empty( ) )
			return;

		add_targets( );

		m_silent_aim = false;

		if( g_ctx->can_shoot( ) && !valve::g_client_state.get( )->m_choked_cmds
			&& !g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key ) 
			&& g_local_player->weapon_info( )->m_type != valve::e_weapon_type::grenade ) {
			send_packet = false;
			m_silent_aim = true;
		}

		if( m_targets.size( ) > 0 ) 
			select( user_cmd, send_packet );

		if( m_silent_aim )
			user_cmd.m_buttons &= ~valve::e_buttons::in_attack;
	}

	void c_aim_bot::get_hitbox_data( c_hitbox* rtn, valve::cs_player_t* ent, int ihitbox, const valve::bones_t& matrix )
	{
		if( ihitbox < 0 || ihitbox > 19 ) return;

		if( !ent ) return;

		valve::studio_hdr_t* const studio_hdr = ent->mdl_ptr( );

		if( !studio_hdr )
			return;

		valve::studio_bbox_t* const hitbox = studio_hdr->m_studio->p_hitbox( ihitbox, ent->hitbox_set_index( ) );

		if( !hitbox )
			return;

		const auto is_capsule = hitbox->m_radius != -1.f;

		sdk::vec3_t min, max;
		if( is_capsule ) {
			min = hitbox->m_mins.transform( matrix [ hitbox->m_bone ] );
			max = hitbox->m_maxs.transform( matrix [ hitbox->m_bone ] );
		}
		else
		{
			min = sdk::vector_rotate( hitbox->m_mins, hitbox->m_rotation );
			max = sdk::vector_rotate( hitbox->m_maxs, hitbox->m_rotation );

			min = min.transform( matrix [ hitbox->m_bone ] );
			max = max.transform( matrix [ hitbox->m_bone ] );
		}

		rtn->m_hitbox_id = ihitbox;
		rtn->m_is_oob = !is_capsule;
		rtn->m_radius = hitbox->m_radius;
		rtn->m_mins = min;
		rtn->m_maxs = max;
		rtn->m_hitgroup = hitbox->m_group;
		rtn->m_hitbox = hitbox;
		rtn->m_start_scaled = g_ctx->shoot_pos( ).i_transform( matrix [ hitbox->m_bone ] );
		rtn->m_bone = hitbox->m_bone;
	}

	constexpr auto k_pi = 3.14159265358979323846f;
	constexpr auto k_pi2 = k_pi * 2.f;
	static const int total_seeds = 255u;
	static std::vector<std::tuple<float, float, float>> precomputed_seeds = { };
	float random_float[4][255];

	sdk::vec2_t calc_spread_angle( 
		const int bullets, const valve::e_item_index item_index,
		const float recoil_index, const std::size_t i
	 ) {

		float r_1 = random_float[ 0 ][ i ];
		float r_pi1 = random_float[ 1 ][ i ];
		float r_2 = random_float[ 2 ][ i ];
		float r_pi2 = random_float[ 3 ][ i ];

		if( recoil_index < 3.f
			&& item_index == valve::e_item_index::negev ) {
			for( auto i = 3; i > recoil_index; --i ) {
				r_1 *= r_1;
				r_2 *= r_2;
			}

			r_1 = 1.f - r_1;
			r_2 = 1.f - r_2;
		}

		const auto inaccuracy = r_1 * g_eng_pred->inaccuracy( );
		const auto spread = r_2 * g_eng_pred->spread( );

		return {
			std::cosf( r_pi1 ) * inaccuracy + std::cosf( r_pi2 ) * spread,
			std::sinf( r_pi1 ) * inaccuracy + std::sinf( r_pi2 ) * spread
		};
	}

	std::optional < aim_target_t > c_aim_bot::extrapolate( const player_entry_t& entry ) const {
		if( entry.m_lag_records.empty( ) )
			return std::nullopt;

		const auto& latest = entry.m_lag_records.front( );

		if( latest->m_choked_cmds > crypt_int( 20 )
			|| latest->m_dormant
			|| latest->m_choked_cmds <= 0 )
			return std::nullopt;

		const auto& net_info = g_ctx->net_info( );

		if( latest->valid( ) ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		const auto receive_tick = std::abs( ( valve::g_client_state.get( )->m_server_tick +( valve::to_ticks( net_info.m_latency.m_out ) ) ) - valve::to_ticks( latest->m_sim_time ) );

		if( ( receive_tick / latest->m_choked_cmds ) > 20
			|| ( receive_tick / latest->m_choked_cmds ) <= 0
			|| !receive_tick ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		const auto delta_ticks = valve::g_client_state.get( )->m_server_tick - latest->m_receive_tick;

		if( valve::to_ticks( g_ctx->net_info( ).m_latency.m_out ) <= latest->m_choked_cmds - delta_ticks ) {
			aim_target_t ret{ const_cast< player_entry_t* >( &entry ), latest };
			return ret;
		}

		extrapolation_data_t data { entry.m_player, latest };

		float change = 0.f, dir = 0.f;

		// get the direction of the current velocity.
		if( data.m_velocity.y( ) != 0.f || data.m_velocity.x( ) != 0.f )
			dir = sdk::to_deg( std::atan2( data.m_velocity.y( ), data.m_velocity.x( ) ) );

		// we have more than one update
		// we can compute the direction.
		if( entry.m_lag_records.size( ) > 1 ) {
			// get the delta time between the 2 most recent records.
			float dt = latest->m_sim_time - entry.m_lag_records.at( 1 )->m_sim_time;

			// init to 0.
			float prevdir = 0.f;

			// get the direction of the prevoius velocity.
			if( entry.m_lag_records.at( 1 )->m_anim_velocity.y( ) != 0.f || entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) != 0.f )
				prevdir = sdk::to_deg( std::atan2( entry.m_lag_records.at( 1 )->m_anim_velocity.y( ), entry.m_lag_records.at( 1 )->m_anim_velocity.x( ) ) );

			// compute the direction change per tick.
			change = ( sdk::norm_yaw( dir - prevdir ) / dt ) * valve::g_global_vars.get( )->m_interval_per_tick;
		}

		if( std::abs( change ) > 6.f )
			change = 0.f;

		data.m_change = change;
		data.m_dir = dir;

		for( int i { }; i < latest->m_choked_cmds; ++i ) {
			data.m_dir = sdk::norm_yaw( data.m_dir + change );
			data.m_sim_time += valve::g_global_vars.get( )->m_interval_per_tick;
			
			player_move( data );
			//hacks::g_sim_ctx->handle_context( data ); somehow calculate fwd/side of enemy, otherwise its p
		}	

		latest->m_extrapolated = true;

		aim_target_t ret{ const_cast< player_entry_t* >( &entry ), std::make_shared< lag_record_t >( ) };

		*ret.m_lag_record.value( ).get( ) = *latest.get( );

		ret.m_lag_record.value( )->m_sim_time = data.m_sim_time;
		ret.m_lag_record.value( )->m_flags = data.m_flags;

		ret.m_lag_record.value( )->m_origin = data.m_origin;
		ret.m_lag_record.value( )->m_anim_velocity = data.m_velocity;

		const auto origin_delta = data.m_origin - latest->m_origin;

		for( std::size_t i{ }; i < latest->m_bones_count; ++i ) {
			auto& bone = latest->m_bones.at( i );

			bone[ 0 ][ 3 ] += origin_delta.x( );
			bone[ 1 ][ 3 ] += origin_delta.y( );
			bone[ 2 ][ 3 ] += origin_delta.z( );
		}

		latest->m_extrapolated_bones = latest->m_bones;

		return ret;
	}

	void c_aim_bot::player_move( extrapolation_data_t& data ) const {
		static auto sv_gravity = valve::g_cvar->find_var( xor_str( "sv_gravity" ) );
		static auto sv_jump_impulse = valve::g_cvar->find_var( xor_str( "sv_jump_impulse" ) );
		static auto sv_enable_bhop = valve::g_cvar->find_var( xor_str( "sv_enablebunnyhopping" ) );
		if( data.m_flags & valve::e_ent_flags::on_ground ) {
			if( !sv_enable_bhop->get_int( ) ) {
				const auto speed = data.m_velocity.length( );

				const auto max_speed = data.m_player->max_speed( ) * crypt_float( 1.1f );
				if( max_speed > 0.f
					&& speed > max_speed )
					data.m_velocity *= ( max_speed / speed );
			}

			data.m_velocity.z( ) = sv_jump_impulse->get_float( );
		}
		else
			data.m_velocity.z( ) -=
			sv_gravity->get_float( ) * valve::g_global_vars.get( )->m_interval_per_tick;

		valve::trace_t trace { };
		valve::trace_filter_world_only_t trace_filter { };

		valve::g_engine_trace->trace_ray( 
			{
			 data.m_origin,
			 data.m_origin + data.m_velocity * valve::g_global_vars.get( )->m_interval_per_tick,
			 data.m_obb_min, data.m_obb_max
			},
			CONTENTS_SOLID, &trace_filter, &trace
		 );

		if( trace.m_frac != crypt_float( 1.f ) ) {
			for( int i { }; i < 2; ++i ) {
				data.m_velocity -= trace.m_plane.m_normal * data.m_velocity.dot( trace.m_plane.m_normal );

				const auto adjust = data.m_velocity.dot( trace.m_plane.m_normal );
				if( adjust < 0.f )
					data.m_velocity -= trace.m_plane.m_normal * adjust;

				valve::g_engine_trace->trace_ray( 
					{
					 trace.m_end,
					 trace.m_end +( data.m_velocity *( valve::g_global_vars.get( )->m_interval_per_tick *( 1.f - trace.m_frac ) ) ),
					 data.m_obb_min, data.m_obb_max
					},
					CONTENTS_SOLID, &trace_filter, &trace
				 );

				if( trace.m_frac == 1.f )
					break;
			}
		}

		data.m_origin = trace.m_end;

		valve::g_engine_trace->trace_ray( 
			{
			 trace.m_end,
			 { trace.m_end.x( ) , trace.m_end.y( ) , trace.m_end.z( ) - crypt_float( 2.f ) },
			 data.m_obb_min, data.m_obb_max
			},
			CONTENTS_SOLID, &trace_filter, &trace
		 );

		data.m_flags &= ~valve::e_ent_flags::on_ground;

		if( trace.m_frac != crypt_float( 1.f )
			&& trace.m_plane.m_normal.z( ) > crypt_float( 0.7f ) )
			data.m_flags |= valve::e_ent_flags::on_ground;
	}

	float c_aim_bot::get_min_dmg_override( ) {
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_awp_min_dmg_on_key;
		case valve::e_item_index::ssg08:
			return m_cfg->m_scout_min_dmg_on_key;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_scar_min_dmg_on_key;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_min_dmg_on_key;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
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
		case valve::e_item_index::awp:
			return m_cfg->m_force_head_conditions_awp;
		case valve::e_item_index::ssg08:
			return m_cfg->m_force_head_conditions_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_force_head_conditions_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_force_head_conditions_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
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
		case valve::e_item_index::awp:
			return m_cfg->m_force_body_conditions_awp;
		case valve::e_item_index::ssg08:
			return m_cfg->m_force_body_conditions_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_force_body_conditions_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_force_body_conditions_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
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
		case valve::e_item_index::awp:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_awp_dmg_key );
		case valve::e_item_index::ssg08:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scout_dmg_key );
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_scar_dmg_key );
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_heavy_pistol_dmg_key );
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_pistol_dmg_key );
		default:
			return g_key_binds->get_keybind_state( &m_cfg->m_min_other_dmg_key );
		}

		return false;
	}

	float c_aim_bot::get_min_dmg_to_set_up( )
	{
		if( !g_local_player->self( ) 
			|| !g_local_player->self( )->alive( ) )
			return 0.f;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0.f;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_min_dmg_awp;
		case valve::e_item_index::ssg08:
			return m_cfg->m_min_dmg_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_min_dmg_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_min_dmg_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return m_cfg->m_min_dmg_pistol;
		default:
			return m_cfg->m_min_dmg_other;
		}

		return 0.f;
	}

	
	int c_aim_bot::get_dt_stop_type( )
	{
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_auto_stop_type_dt_awp;
		case valve::e_item_index::ssg08:
			return m_cfg->m_auto_stop_type_dt_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_auto_stop_type_dt_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_auto_stop_type_dt_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return m_cfg->m_auto_stop_type_dt_pistol;
		default:
			return m_cfg->m_auto_stop_type_dt_other;
		}

		return 0;
	}

	int c_aim_bot::get_autostop_type( )
	{
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		
		bool shifting = valve::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick <= 16;

		if( shifting ) // if in shift
			return get_dt_stop_type( ); // return dt stop type

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_auto_stop_type_awp;
		case valve::e_item_index::ssg08:
			return m_cfg->m_auto_stop_type_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_auto_stop_type_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_auto_stop_type_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return m_cfg->m_auto_stop_type_pistol;
		default:
			return g_aim_bot->cfg( ).m_auto_stop_type_other;
		}

		return 0;
	}


	int c_aim_bot::get_hitboxes_setup( )
	{
		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_awp_hitboxes;
		case valve::e_item_index::ssg08:
			return m_cfg->m_scout_hitboxes;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_scar_hitboxes;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_hitboxes;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return m_cfg->m_pistol_hitboxes;
		default:
			return g_aim_bot->cfg( ).m_other_hitboxes;
		}

		return 0;
	}

	float c_aim_bot::get_pointscale( )
	{
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return m_cfg->m_awp_point_scale;
		case valve::e_item_index::ssg08:
			return m_cfg->m_scout_point_scale;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return m_cfg->m_scar_point_scale;
	
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return m_cfg->m_heavy_pistol_point_scale;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return m_cfg->m_pistol_point_scale;
		default:
			return g_aim_bot->cfg( ).m_other_point_scale;
		}

		return 0;
	}

	float c_aim_bot::get_hit_chance( ) {
		if( !g_local_player->self( ) || !g_local_player->self( )->alive( ) )
			return 0;

		auto wpn = g_local_player->self( )->weapon( );

		if( !wpn )
			return 0;

		bool shifting = valve::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick <= 16;

		switch( wpn->item_index( ) )
		{
		case valve::e_item_index::awp:
			return shifting ? m_cfg->m_dt_hit_chance_awp : m_cfg->m_hit_chance_awp;
		case valve::e_item_index::ssg08:
			return shifting ? m_cfg->m_dt_hit_chance_scout : m_cfg->m_hit_chance_scout;
		case valve::e_item_index::scar20:
		case valve::e_item_index::g3sg1:
			return shifting ? m_cfg->m_dt_hit_chance_scar : m_cfg->m_hit_chance_scar;
		case valve::e_item_index::revolver:
		case valve::e_item_index::deagle:
			return shifting ? m_cfg->m_dt_hit_chance_heavy_pistol: m_cfg->m_hit_chance_heavy_pistol;
		case valve::e_item_index::cz75a:
		case valve::e_item_index::elite:
		case valve::e_item_index::five_seven:
		case valve::e_item_index::p2000:
		case valve::e_item_index::glock:
		case valve::e_item_index::p250:
		case valve::e_item_index::tec9:
		case valve::e_item_index::usps:
			return shifting ? m_cfg->m_dt_hit_chance_pistol : m_cfg->m_hit_chance_pistol;
		default:
			return shifting ? m_cfg->m_dt_hit_chance_other : m_cfg->m_hit_chance_other;
		}

		return 0;
	}



	void build_seed_table( ) {

		static bool setupped = false;

		if( setupped )
			return;

		for( auto i = 0; i <= total_seeds; i++ ) {
			g_ctx->addresses( ).m_random_seed( i );
			random_float[0][i] = g_ctx->addresses( ).m_random_float( 0.f, 1.f );
			random_float[1][i] = g_ctx->addresses( ).m_random_float( 0.f, k_pi2 );
			random_float[2][i] = g_ctx->addresses( ).m_random_float( 0.f, 1.f );
			random_float[3][i] = g_ctx->addresses( ).m_random_float( 0.f, k_pi2 );
		}

		setupped = true;
	}

	bool c_aim_bot::calc_hit_chance( 
		valve::cs_player_t* player, const sdk::qang_t& angle
	 ) {


		float chance = get_hit_chance( );

		build_seed_table( );

		sdk::vec3_t fwd { }, right { }, up { };
		sdk::ang_vecs( angle, &fwd, &right, &up );

		int hits{ };
		valve::cs_weapon_t* weapon = g_local_player->self( )->weapon( );

		if( !weapon )
			return false;

		valve::weapon_info_t* wpn_data = weapon->info( );

		if( !wpn_data )
			return false;

		const float recoil_index = weapon->recoil_index( );
		const float wpn_range = wpn_data->m_range;
		const int bullets = wpn_data->m_bullets;
		const valve::e_item_index item_id = weapon->item_index( );
		sdk::vec3_t dir{ }, end{ }, start{ g_ctx->shoot_pos( ) };
		sdk::vec2_t spread_angle{ };
		valve::trace_t tr{};

		for( int i { 0 }; i < total_seeds; i++ ) {

			spread_angle = calc_spread_angle( bullets, item_id, recoil_index, i );
			dir = fwd + ( right * spread_angle.x( ) ) + ( up * spread_angle.y( ) );
			dir.normalize( );
			
			end = start + dir * wpn_range;
	
			valve::g_engine_trace->clip_ray_to_entity( valve::ray_t( start, end ), CS_MASK_SHOOT_PLAYER, player, &tr );

			// check if we hit a valid player / hitgroup on the player and increment total hits.
			if( tr.m_entity == player && valve::is_valid_hitgroup ( int( tr.m_hitgroup ) ) )
				++hits;
		}

		return static_cast< float >( ( hits / static_cast< float >( total_seeds ) ) * 100.f ) >= chance;
	}

	void c_aim_bot::add_targets( ) {
		m_targets.reserve( valve::g_global_vars.get( )->m_max_clients );

		for( int i = 1; i <= valve::g_global_vars.get( )->m_max_clients; ++i ) {
			auto& entry = hacks::g_lag_comp->entry( i - 1 );

			if( !entry.m_player
				|| !entry.m_player->alive( )
				|| entry.m_player->networkable( )->dormant( ) )
				continue;

			if( entry.m_player->team( )
				== g_local_player->self( )->team( ) )
				continue;

			//if( entry.m_lag_records.empty( ) )
			//	valve::g_cvar->error_print( true, "lagrecords are empty!\n" );

			auto record = select_ideal_record( entry );

			if( !record.has_value( ) ) {
				// valve::g_cvar->error_print( true, "no valid record found!\n" );
				continue;
			}

			m_targets.emplace_back( aim_target_t( &entry, record.value( ).m_lag_record ) );
		}
	}

	void c_aim_bot::scan_center_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, sdk::vec3_t shoot_pos, std::vector < point_t >& points ) const {
		const auto hitbox_set = target.m_entry->m_player->mdl_ptr( )->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );

		auto hitbox_head = hitbox_set->get_bbox( static_cast < std::ptrdiff_t >( valve::e_hitbox::head ) );
		auto hitbox_stomach = hitbox_set->get_bbox( static_cast < std::ptrdiff_t >( valve::e_hitbox::stomach ) );

		if( !hitbox_stomach || !hitbox_head )
			return;

		sdk::vec3_t body_point{ };
		sdk::vec3_t head_point{ };

		sdk::vector_transform( 
			( hitbox_stomach->m_mins + hitbox_stomach->m_maxs ) / 2.f,
			record->m_bones[ hitbox_stomach->m_bone ], body_point
		 );

		sdk::vector_transform( 
			( hitbox_head->m_mins + hitbox_head->m_maxs ) / 2.f,
			record->m_bones[ hitbox_head->m_bone ], head_point
		 );

		/* fkin constructors gone crazy mf visual studio kys */
		point_t body_point_{ };
		body_point_.m_center = true;
		body_point_.m_index = valve::e_hitbox::stomach;
		body_point_.m_pos = body_point;

		point_t head_point_{ };
		head_point_.m_center = true;
		head_point_.m_index = valve::e_hitbox::head;
		head_point_.m_pos = head_point;

		points.clear( );
		points.push_back( body_point_ );
		points.push_back( head_point_ );

		record->adjust( target.m_entry->m_player );

		// note: made it use 1 dmg override cus we dont rly care if it uses mindmg or not
		for( auto& point : points ) {

			if( !head_point_.m_valid && !body_point_.m_valid )
				scan_point( target.m_entry, point, 1.f, true, shoot_pos );
		}
	}

	std::optional < aim_target_t > c_aim_bot::select_ideal_record( const player_entry_t& entry ) const {

		if( entry.m_lag_records.empty( ) || entry.m_lag_records.size( ) <= 1 || !entry.m_lag_records.front( )->m_has_valid_bones ) {
			// valve::g_cvar->error_print( true, "[ debug ] m_lag_records is empty\n" );
			return std::nullopt;
		}

		// if he's breaking lc, extrapolate him 
		if( entry.m_lag_records.front( )->m_broke_lc ) {
			// valve::g_cvar->error_print( true, "[ debug ] front record has broken lc, run extrapolation\n" );
			return extrapolate( entry );
		}


		// backup matrixes
		lag_backup_t lag_backup{ };
		lag_backup.setup( entry.m_player );

		// get front
		auto& front = entry.m_lag_records.front( );

		// note: ok this is ghetto, but it works in a simple way
		// when you shift tickbase, your tickbase goes backward by your 'shift amount - 1' ( -1 cus getting predicted adding +1 )
		// making the front record not hittable, now if you're lucky enough or the record is slow or standing
		// you'll be able to still shoot at the front lagrecord without missing it
		if( front && ( front->valid( ) || front->m_anim_velocity.length( 2u ) <= 40.f ) ) {

			std::vector < point_t > points_front{ };
			aim_target_t target_front{ const_cast <player_entry_t*>( &entry ), front };

			// generate & scan points
			scan_center_points( target_front, front, g_ctx->shoot_pos( ), points_front );

			// for ( int i = 0; i < points_front.size( ); i++ )
			// 	hacks::g_logs->push_log( tfm::format( "[ debug ] [ pre ] point idx: %i/%i | damage: %f[%s]\n", i, points_front.size( ), points_front[i].m_pen_data.m_dmg, points_front[i].m_valid ), sdk::col_t( 255, 255, 255, 255 ) );

			bool can_hit_front = scan_points( &target_front, points_front, false, true );

			// for ( int i = 0; i < points_front.size( ); i++ )
			// 	hacks::g_logs->push_log( tfm::format( "[ debug ] [ post ] point idx: %i/%i | damage: %f[%s]\n", i, points_front.size( ), points_front[i].m_pen_data.m_dmg, points_front[i].m_valid ), sdk::col_t( 255, 255, 255, 255 ) );

	
			// restore matrixes etc..
			lag_backup.restore( entry.m_player );
		
			// if we can hit first record, dont try backtracking
			// note: saves up fps & processing time
			if( can_hit_front ) {
				// 	valve::g_cvar->error_print( true, "[ debug ] front record is hittable, skipping backtrack\n" );
				return get_latest_record( entry );
			}
		}

		// if we only have few records, force front
		if( entry.m_lag_records.size( ) <= 3 )
			return get_latest_record( entry );

		// -> we arrived here and couldnt hit front record
		// start backtracking process
		std::shared_ptr< lag_record_t > best_record{ }; // fix for shooting at invalid record probably
		std::optional< point_t > best_aim_point{ };
		const auto rend = entry.m_lag_records.end( );
		sdk::vec3_t last_origin{ 0, 0, 0 };
		int scanned_records = 1;

		for( auto i = entry.m_lag_records.begin( ); i != rend; i = std::next( i ) ) {

			const auto& lag_record = *i;

			// we already scanned this record
			// and it was not hittable, skip it
			if( lag_record == entry.m_lag_records.front( ) )
				continue;

			// record isnt valid, skip it
			if( !lag_record->valid( ) || ( ( lag_record->m_origin - last_origin ).length( ) <= 4.f && m_cfg->m_limit_records_per_tick ) )
				continue;

			std::vector < point_t > points{ };
			aim_target_t target{ const_cast < player_entry_t* >( &entry ), lag_record };

			// generate and scan points for this record
			scan_center_points( target, lag_record, g_ctx->shoot_pos( ), points );

			// increment scanned record amount
			++scanned_records;

			// save latest origin
			last_origin = lag_record->m_origin;

			// no hittable point have been found, skip this record
			if( !scan_points( &target, points, false, true ) )
				continue;

			// if we have no best point, it means front wasnt hittable
			if( !best_aim_point.has_value( ) ) {
				best_record = lag_record;

				// lol this is ghetto but will do i suppose
				if( target.m_best_point ) 
					best_aim_point = *target.m_best_point;
				else if( target.m_best_body_point )
					best_aim_point = *target.m_best_body_point;
				else
					break; // somehow theyre all invalid just break

				continue;
			}

			if( scanned_records <= 2 )
				continue;

			const float health = target.m_entry->m_player->health( );
			const auto& pen_data = target.m_best_point->m_pen_data;
			const auto& best_pen_data = best_aim_point.value( ).m_pen_data;

			if( !target.m_best_point ) {
				target.m_best_point = target.m_best_body_point;
			}
			
			if( !target.m_best_point )
				continue;

			if( target.m_best_body_point ) {
				if( target.m_best_point->m_pen_data.m_dmg < 1 || target.m_best_point->m_pen_data.m_dmg < target.m_best_body_point->m_pen_data.m_dmg )
					target.m_best_point = target.m_best_body_point;
			}

			if( target.m_best_point->m_pen_data.m_dmg < 1 )
				continue;

			// this record's priority is different than current record
			if( lag_record->m_resolved != best_record->m_resolved ) {
		
				// this record is resolved but not the best record
				if( lag_record->m_resolved ) {

					// this record is lethal and has more damage or less than 5 damage
					// note: shoot for lower damage but on a safer record
					if( pen_data.m_dmg >= health 
						|| pen_data.m_dmg - best_pen_data.m_dmg > -5.f ) {
						
						// replace best record by current record
						best_record = lag_record;

						// lol this is ghetto but will do i suppose
						if( target.m_best_point )
							best_aim_point = *target.m_best_point;
					}

					// continue;
				}

				// note: here you could make a dmg check or something but atm i just prioritize resolved record

				continue;
			}

			if( &pen_data && &best_pen_data ) {
				// we dealt more damage
				if( pen_data.m_dmg > best_pen_data.m_dmg )
				{
					best_record = lag_record;
					best_aim_point = *target.m_best_point;

					// if this record is lethal, stop here
					if( best_pen_data.m_dmg >= health )
						break;

					// go to next
					continue;
				}
			}
		}

		lag_backup.restore( entry.m_player );

		if( !best_record )
			return std::nullopt;

		return aim_target_t{ const_cast < player_entry_t* >( &entry ), best_record };
	}

	std::optional < aim_target_t > c_aim_bot::get_latest_record( const player_entry_t& entry ) const {
		const auto& latest = entry.m_lag_records.front( );
		if( latest->m_lag_ticks <= 0
			|| latest->m_lag_ticks >= 20	
			|| latest->m_dormant ) { 

			// 	valve::g_cvar->error_print( true, "[ debug ] front record has invalid lag or is dormant\n" );
			return std::nullopt;
		}

		
		// yo, wanna see some ghetto shit?
		if( !latest->valid( ) && latest->m_anim_velocity.length( 2u ) > 40.f ) { // here u go
			// valve::g_cvar->error_print( true, "[ debug ] front record is invalid\n" );
			return std::nullopt;
		}

		if( latest->m_broke_lc ) {

			const auto adjusted_arrive_tick = std::clamp( valve::to_ticks( ( ( g_ctx->net_info( ).m_latency.m_out ) + valve::g_global_vars.get( )->m_real_time )
				- entry.m_receive_time ), 0, 100 );

			if( ( adjusted_arrive_tick - latest->m_choked_cmds ) >= 0 ) {
				// valve::g_cvar->error_print( true, "[ debug ] front record time has expired\n" );
				return std::nullopt;
			}
		}

		aim_target_t ret{ };
		ret.m_entry = const_cast < player_entry_t* >( &entry );
		ret.m_lag_record = latest;

		return ret;
	}	

	ALWAYS_INLINE sdk::mat3x4_t vector_matrix( const sdk::vec3_t& in ) {
		sdk::vec3_t right { }, up { };

		if( in.x( ) == 0.f
			&& in.y( ) == 0.f ) {
			right = { 0.f, -1.f, 0.f };
			up = { -in.z( ), 0.f, 0.f };
		}
		else {
			right = in.cross( { 0.f, 0.f, 1.f } ).normalized( );
			up = right.cross( in ).normalized( );
		}

		sdk::mat3x4_t ret { };

		ret [ 0 ][ 0 ] = in.x( );
		ret [ 1 ][ 0 ] = in.y( );
		ret [ 2 ][ 0 ] = in.z( );

		ret [ 0 ][ 1 ] = -right.x( );
		ret [ 1 ][ 1 ] = -right.y( );
		ret [ 2 ][ 1 ] = -right.z( );

		ret [ 0 ][ 2 ] = up.x( );
		ret [ 1 ][ 2 ] = up.y( );
		ret [ 2 ][ 2 ] = up.z( );

		return ret;
	}

	float calc_point_scale( 
		const float spread, const float max,
		const float dist, const sdk::vec3_t& dir,
		const sdk::vec3_t& right, const sdk::vec3_t& up
	 )
	{
		const auto accuracy = g_eng_pred->inaccuracy( ) + spread;

		auto angle = right * accuracy + dir + up * accuracy;

		angle.normalize( );

		const auto delta = sdk::angle_diff( sdk::to_deg( std::atan2( dir.y( ), dir.x( ) ) ), sdk::to_deg( std::atan2( angle.y( ), angle.x( ) ) ) );

		const auto scale = max + dist / std::tan( sdk::to_rad( 180.f -( delta + 90.f ) ) );
		if( scale > max )
			return 0.9f;

		float final_scale{ };
		if( scale >= 0.f )
			final_scale = scale;

		return final_scale / max;
	}

	void c_aim_bot::setup_points( aim_target_t& target, std::shared_ptr < lag_record_t > record, valve::e_hitbox index, e_hit_scan_mode mode
	 ) {

		auto hdr = target.m_entry->m_player->mdl_ptr( );
		if( !hdr )
			return;

		auto set = hdr->m_studio->get_hitbox_set( target.m_entry->m_player->hitbox_set_index( ) );
		if( !set )
			return;

		auto hitbox = set->get_bbox( static_cast < std::ptrdiff_t >( index ) );
		if( !hitbox )
			return;

		sdk::vec3_t point{ };

		// center.
		const auto center = ( hitbox->m_mins + hitbox->m_maxs ) / 2.f;
		sdk::vector_transform( center, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, true );

		// get hitbox scales.
		float scale = g_aim_bot->get_pointscale( ) / 100.f;
		
		const auto max = ( hitbox->m_maxs - hitbox->m_mins ).length( ) * 0.5f + hitbox->m_radius;

		auto dir = ( point - g_ctx->shoot_pos( ) );

		const auto dist = dir.normalize( );

		sdk::vec3_t right{ }, up{ };

		if( dir.x( ) == 0.f
			&& dir.y( ) == 0.f )
		{
			right = { 0.f, -1.f, 0.f };
			up = { -dir.z( ), 0.f, 0.f };
		}
		else
		{
			right = dir.cross( { 0.f, 0.f, 1.f } ).normalized( );
			up = right.cross( dir ).normalized( );
		}

		if( scale <= 0.3f ) {
			scale = calc_point_scale( g_eng_pred->spread( ), max, dist, dir, right, up );
			if( scale <= 0.f
				&& g_eng_pred->spread( ) > g_eng_pred->min_inaccuracy( ) )
				scale = calc_point_scale( g_eng_pred->min_inaccuracy( ), max, dist, dir, right, up );

			scale = std::clamp( scale, 0.3f, 0.91f );
		}

		// pain
		if( scale <= 0.f )
			return;

		// feet
		if( hitbox->m_radius <= 0.f )
		{
			if( index == valve::e_hitbox::left_foot || index == valve::e_hitbox::right_foot ) {
				// front
				point = { center.x( ) +( hitbox->m_mins.x( ) - center.x( ) ) * scale, center.y( ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// back
				point = { center.x( ) +( hitbox->m_maxs.x( ) - center.x( ) ) * scale, center.y( ), center.z( )};
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			return;
		}

		if( index != valve::e_hitbox::head ) {
				
			if( index == valve::e_hitbox::pelvis ) {

				// back
				point = { center.x( ), hitbox->m_maxs.y( ) -( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// front
				point = { center.x( ), hitbox->m_maxs.y( ) +( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// this is cancer but its to avoid issues with going outside of the model
				scale *= 0.8f;

				// left
				point = { center.x( ), center.y( ), hitbox->m_maxs.z( ) +( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// right
				point = { center.x( ), center.y( ), hitbox->m_mins.z( ) -( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			else if( index == valve::e_hitbox::chest ) {
				// back
				point = { center.x( ), hitbox->m_maxs.y( ) -( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				scale *= 0.8f;

				// left
				point = { center.x( ), center.y( ), hitbox->m_maxs.z( ) +( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				// right
				point = { center.x( ), center.y( ), hitbox->m_mins.z( ) -( hitbox->m_radius * scale ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );
			}

			else if( index == valve::e_hitbox::stomach || index == valve::e_hitbox::lower_chest || index == valve::e_hitbox::upper_chest ) {
				// back
				point = { center.x( ), hitbox->m_maxs.y( ) -( hitbox->m_radius * scale ), center.z( ) };
				sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
				target.m_points.emplace_back( point, index, false );

				return;
			}

			// exit
			return;
		}

		// top back
		point = { hitbox->m_maxs.x( ) + 0.70710678f *( hitbox->m_radius * scale ), hitbox->m_maxs.y( ) - 0.70710678f *( hitbox->m_radius * scale ), hitbox->m_maxs.z( ) };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// side
		point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) + hitbox->m_radius * scale };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// side
		point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ), hitbox->m_maxs.z( ) - hitbox->m_radius * scale };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );

		// back
		point = { hitbox->m_maxs.x( ), hitbox->m_maxs.y( ) - hitbox->m_radius * scale, hitbox->m_maxs.z( ) };
		sdk::vector_transform( point, record->m_bones[ hitbox->m_bone ], point );
		target.m_points.emplace_back( point, index, false );
	}

	void c_aim_bot::setup_hitboxes( std::vector < hit_box_data_t >& hitboxes ) {
		hitboxes.clear( );

		if( g_local_player->self( )->weapon( )->item_index( ) == valve::e_item_index::taser ) {
			hitboxes.push_back( { valve::e_hitbox::stomach, e_hit_scan_mode::prefer } );
			return;
		}

		auto hitboxes_selected = g_aim_bot->get_hitboxes_setup( );

		if( hitboxes_selected & 1 ) {
			hitboxes.push_back( { valve::e_hitbox::head, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 2 ) {
			hitboxes.push_back( { valve::e_hitbox::lower_chest, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::chest, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::upper_chest, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 4 ) {
			hitboxes.push_back( { valve::e_hitbox::pelvis, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::stomach, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 8 ) {
			hitboxes.push_back( { valve::e_hitbox::left_upper_arm,e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::right_upper_arm, e_hit_scan_mode::normal } );
		}

		if( hitboxes_selected & 16 ) {
			hitboxes.push_back( { valve::e_hitbox::left_thigh, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::right_thigh,e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::left_calf, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::right_calf,e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::left_foot, e_hit_scan_mode::normal } );
			hitboxes.push_back( { valve::e_hitbox::right_foot, e_hit_scan_mode::normal } );
		}
	}

	void c_aim_bot::scan_point( player_entry_t* entry,
		point_t& point, float min_dmg_key, bool min_dmg_key_pressed, sdk::vec3_t& shoot_pos ) {

		point.m_pen_data = g_auto_wall->wall_penetration( shoot_pos, point.m_pos, entry->m_player );

		const int hp = entry->m_player->health( );
		float min_dmg = g_aim_bot->get_min_dmg_to_set_up( );

		if( min_dmg >= hp )
			min_dmg = hp + ( min_dmg - 100.f );

		if( min_dmg_key_pressed )
			min_dmg = min_dmg_key;

		point.m_valid = ( point.m_pen_data.m_dmg >= entry->m_player->health( ) || point.m_pen_data.m_dmg >= min_dmg );
	}

	bool c_aim_bot::scan_points( cc_def( aim_target_t* ) target, std::vector < point_t >& points, bool additional_scan, bool lag_record_check ) const {
		std::array < point_t*, 21 > best_points { }; // lets make + 1 to make sure it doesnt overflow somehow
		static auto min_dmg_on_key_val{ 0.f };
		min_dmg_on_key_val = g_aim_bot->get_min_dmg_override( );

		static bool min_dmg_key_pressed{ false };

		if( g_aim_bot->get_min_dmg_override_state( ) )
			min_dmg_key_pressed = true;
		else
			min_dmg_key_pressed = false;

		lag_backup_t backup{ };
		backup.setup( target.get( )->m_entry->m_player );

		target.get( )->m_lag_record.value( )->adjust( target.get( )->m_entry->m_player );

		const int hp = target.get( )->m_entry->m_player->health( );
		bool ret = false;

		for( auto& point : points ) {

			if( additional_scan ) 
				scan_point( target.get( )->m_entry, point, static_cast < int >( min_dmg_on_key_val ), min_dmg_key_pressed );
			
			if( !point.m_valid || point.m_pen_data.m_dmg < 1 )
				continue;

			ret = true;

			bool body = point.m_index > valve::e_hitbox::lower_neck;

			if( body ) {
				if( !target.get( )->m_best_body_point ) {
					target.get( )->m_best_body_point = &point;
					continue;
				}

				const auto& pen_data = point.m_pen_data;
				const auto& best_pen_data = target.get( )->m_best_body_point->m_pen_data;

				if( target.get( )->m_best_body_point->m_center == point.m_center ) {

					if( pen_data.m_dmg - best_pen_data.m_dmg > 1.f )
						target.get( )->m_best_body_point = &point;

					if( best_pen_data.m_dmg >= hp )
						break;

					continue;
				}

				const float damage_restriction = point.m_center ? -2.f : 2.f;

				if( pen_data.m_dmg - best_pen_data.m_dmg > damage_restriction ) 
					target.get( )->m_best_body_point = &point;

				if( best_pen_data.m_dmg >= hp )
					break;

				continue;
			}


			if( !target.get( )->m_best_point ) {
				target.get( )->m_best_point = &point;
				continue;
			}

			const auto& pen_data = point.m_pen_data;
			const auto& best_pen_data = target.get( )->m_best_point->m_pen_data;

			if( target.get( )->m_best_point->m_center == point.m_center ) {

				if( pen_data.m_dmg - best_pen_data.m_dmg > 2.f )
					target.get( )->m_best_point = &point;

				continue;
			}

			const float damage_restriction = point.m_center ? -2.f : 2.f;

			if( pen_data.m_dmg - best_pen_data.m_dmg > damage_restriction ) 
				target.get( )->m_best_point = &point;

			continue;
		}

		backup.restore( target.get( )->m_entry->m_player );

		
		if( lag_record_check && target.get( )->m_best_body_point && target.get( )->m_best_point ) {

			float body_dmg = target.get( )->m_best_body_point->m_pen_data.m_dmg;

			if( body_dmg >= hp || body_dmg >= target.get( )->m_best_point->m_pen_data.m_dmg )
				target.get( )->m_best_point = target.get( )->m_best_body_point;
		}

		return ret;
	}

	point_t* c_aim_bot::select_point( cc_def( aim_target_t* ) target, const int cmd_num ) {
		if( !target.get( )->m_best_body_point
			|| target.get( )->m_best_body_point->m_pen_data.m_dmg < crypt_int( 1 ) ) {
			return target.get( )->m_best_point;
		}

		if( !target.get( )->m_best_point || target.get( )->m_best_point->m_pen_data.m_dmg < crypt_int( 1 ) )
			return target.get( )->m_best_body_point;

		if( target.get( )->m_best_body_point->m_pen_data.m_dmg >= target.get( )->m_best_point->m_pen_data.m_dmg
			|| target.get( )->m_best_body_point->m_pen_data.m_dmg >= target.get( )->m_entry->m_player->health( ) )
			return target.get( )->m_best_body_point; 

		const auto hp = target.get( )->m_entry->m_player->health( );

		if( g_key_binds->get_keybind_state( &m_cfg->m_baim_key ) )
			return target.get( )->m_best_body_point;

		int body_cond = get_force_body_conditions( );

		if( body_cond & 1 ) {
			if( !( target.get( )->m_lag_record.value( )->m_flags & valve::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_body_point;
			}
		}

		if( body_cond & 2 ) {
			if( !target.get( )->m_entry->m_moved ) {
				return target.get( )->m_best_body_point;
			}
		}

		if( body_cond & 4 ) {
			if( !target.get( )->m_lag_record.value( )->m_resolved ) {
				return target.get( )->m_best_body_point;
			}
		}

		int head_cond = get_force_head_conditions( );

		if( target.get( )->m_best_body_point->m_pen_data.m_dmg < hp ) {

			if( head_cond & 1
				&& target.get( )->m_entry->m_predicting_lby ) {
				return target.get( )->m_best_point;
			}

			if( head_cond & 2
				&& target.get( )->m_lag_record.value( )->m_anim_velocity.length( 2u ) > 75.f
				&& !target.get( )->m_lag_record.value( )->m_fake_walking 
				&&( target.get( )->m_lag_record.value( )->m_flags & valve::e_ent_flags::on_ground ) ) {
				return target.get( )->m_best_point;
			}

			if( head_cond & 4
				&& target.get( )->m_lag_record.value( )->m_resolved ) {
				return target.get( )->m_best_point;
			}
		}

		const auto& shots = g_shot_construct->m_shots;

		if( !shots.empty( ) && body_cond & 8 ) {
			const auto& last_shot = shots.back( );

			if( last_shot.m_target->m_entry->m_player == target.get( )->m_entry->m_player
				&& std::abs( last_shot.m_cmd_num - cmd_num ) <= crypt_int( 150 ) ) {
				const auto hp_left = last_shot.m_target->m_entry->m_player->health( ) - last_shot.m_damage;

				if( hp_left
					&& target.get( )->m_best_body_point->m_pen_data.m_dmg >= hp_left )
					return target.get( )->m_best_body_point;
			}
		}

		if( target.get( )->m_best_body_point->m_pen_data.m_dmg >= target.get( )->m_entry->m_player->health( ) )
			return target.get( )->m_best_body_point;

		if( g_key_binds->get_keybind_state( &hacks::g_exploits->cfg( ).m_dt_key )
			&& target.get( )->m_best_body_point->m_pen_data.m_dmg * crypt_float( 2.f ) >= target.get( )->m_entry->m_player->health( )
			&& g_local_player->weapon( )->item_index( ) != valve::e_item_index::ssg08 
			&&  ( body_cond & 8 )
			&& ( std::abs( valve::g_global_vars.get( )->m_tick_count - g_exploits->m_last_shift_tick ) <= 16
				|| g_exploits->m_ticks_allowed >= 14 ) ) {
			return target.get( )->m_best_body_point;
		}

		return target.get( )->m_best_point;
	}

	bool c_aim_bot::can_shoot( 
		bool skip_r8, const int shift_amount, const bool lol
	 ) const {

		if( !g_local_player->self( )
			|| !g_local_player->self( )->alive( ) )
			return false;

		auto weapon = g_local_player->self( )->weapon( );
		if( !weapon )
			return false;

		auto weapon_data = weapon->info( );
		if( !weapon_data )
			return false;

		if( g_local_player->self( )->flags( ) & valve::e_ent_flags::frozen )
			return false;

		if( g_local_player->self( )->wait_for_no_attack( ) )
			return false;

		if( g_local_player->self( )->defusing( ) )
			return false;

		auto anim_layer = g_local_player->self( )->anim_layers( ).at( 1u );
		
		if( anim_layer.m_owner ) {
			if( g_local_player->self( )->lookup_seq_act( anim_layer.m_seq ) == crypt_int( 967 )
				&& anim_layer.m_weight != 0.f )
				return false;
		}

		if( weapon_data->m_type >= valve::e_weapon_type::pistol && weapon_data->m_type <= valve::e_weapon_type::machine_gun && weapon->clip1( ) < crypt_int( 1 ) )
			return false;

		float curtime = valve::to_time( g_local_player->self( )->tick_base( ) - shift_amount );
		if( curtime < g_local_player->self( )->next_attack( ) )
			return false;

		if( lol )
			return true;

		if( ( weapon->item_index( ) == valve::e_item_index::glock || weapon->item_index( ) == valve::e_item_index::famas ) && weapon->burst_shots_remaining( ) > crypt_int( 0 ) ) {
			if( curtime >= weapon->next_burst_shot( ) )
				return true;
		}

		if( curtime < weapon->next_primary_attack( ) )
			return false;

		if( weapon->item_index( ) != valve::e_item_index::revolver )
			return true;

		if( skip_r8 )
			return true;

		return curtime >= weapon->postpone_fire_ready_time( );
	}

	bool c_aim_bot::can_hit( sdk::vec3_t start, sdk::vec3_t end, std::shared_ptr < lag_record_t > record, int box ) {
		auto hdr = *( valve::studio_hdr_t** ) record->m_player->mdl_ptr( );
		if( !hdr )
			return false;

		auto hitbox_set = hdr->m_studio->get_hitbox_set( record->m_player->hitbox_set_index( ) );

		if( !hitbox_set )
			return false;

		auto hitbox_ptr = hitbox_set->get_bbox( box );

		if( !hitbox_ptr )
			return false;

		bool is_capsule = hitbox_ptr->m_radius != -1.0f;

		auto mins = hitbox_ptr->m_mins.transform( record->m_bones[ hitbox_ptr->m_bone ] );
		auto maxs = hitbox_ptr->m_maxs.transform( record->m_bones[ hitbox_ptr->m_bone ] );

		bool is_intersected{ };

		if( !is_capsule ) { 
			is_intersected = sdk::intersect_bounding_box( start, end, mins, maxs );
		}
		else
		{
			is_intersected = sdk::intersect( start, end, mins, maxs, hitbox_ptr->m_radius );
		}

		return is_intersected;
	}

	void c_aim_bot::select( valve::user_cmd_t& user_cmd, bool& send_packet ) {
		struct ideal_target_t {
			valve::cs_player_t* m_player { }; sdk::vec3_t m_pos{ };
			float m_dmg{ }; valve::e_hitbox m_hit_box{ }; std::shared_ptr < lag_record_t > m_record{ }; aim_target_t* m_target{ };
		};
		std::unique_ptr < ideal_target_t > ideal_select = std::make_unique < ideal_target_t >( );
		static auto min_dmg_on_key_val { 0.f };
		min_dmg_on_key_val = get_min_dmg_override( );

		static bool min_dmg_key_pressed{ false };

		if( get_min_dmg_override_state( ) )
			min_dmg_key_pressed = true;
		else
			min_dmg_key_pressed = false;

		if( m_cfg->m_threading ) {
			for( auto& target : m_targets )
				sdk::g_thread_pool->enqueue( [ ]( aim_target_t& target ) {
				lag_backup_t backup{ };
				backup.setup( target.m_entry->m_player );


				// note: it didnt check if lagrecord had a value or not before
				// changed that, comment it if it creates any issues
				if( target.m_lag_record.has_value( ) && target.m_lag_record.value( )->m_has_valid_bones ) {

					target.m_lag_record.value( )->adjust( target.m_entry->m_player );
					target.m_points.clear( );

					for ( const auto& hitbox : g_aim_bot->m_hit_boxes )
						setup_points( target, target.m_lag_record.value( ), hitbox.m_index, hitbox.m_mode );

					for ( auto& point : target.m_points ) {
						scan_point( target.m_entry, point, static_cast<int>( min_dmg_on_key_val ), min_dmg_key_pressed );
					}

				}

				backup.restore( target.m_entry->m_player );
					}, std::ref( target ) );

			sdk::g_thread_pool->wait( );
		}
		else {
			for( auto& target : m_targets ) {
				if( !target.m_lag_record.has_value( ) )
					continue;

				lag_backup_t backup{ };
				backup.setup( target.m_entry->m_player );
				target.m_lag_record.value( )->adjust( target.m_entry->m_player );
				g_aim_bot->m_nigga_hack.clear( );
				target.m_points.clear( );

				for( const auto& who : g_aim_bot->m_hit_boxes )
					setup_points( target, target.m_lag_record.value( ), who.m_index, who.m_mode );	

				backup.restore( target.m_entry->m_player );
			}
		}

		hacks::g_move->allow_early_stop( ) = false; 

		for( auto& target : m_targets ) {

			if( !scan_points( &target, target.m_points, !m_cfg->m_threading ) )
				continue;

			const auto point = select_point( &target, user_cmd.m_number );

			if( point ) {
				if( !ideal_select->m_player || point->m_pen_data.m_dmg > ideal_select->m_dmg ) {
					ideal_select->m_player = target.m_entry->m_player;
					ideal_select->m_dmg = point->m_pen_data.m_dmg;
					ideal_select->m_record = target.m_lag_record.value( );
					ideal_select->m_hit_box = point->m_index;
					ideal_select->m_pos = point->m_pos;
					ideal_select->m_target = &target;

					if( ideal_select->m_dmg >= ideal_select->m_player->health( ) )
						break;
				}
			}
		}

		if( ideal_select->m_player && ideal_select->m_record ) {

			if( hacks::g_exploits->m_type == 5 )
				hacks::g_exploits->m_type = 3;

			ideal_select->m_target->m_pos = ideal_select->m_pos;

			m_angle = ( ideal_select->m_pos - g_ctx->shoot_pos( ) ).angles( );

			g_eng_pred->update_shoot_pos( user_cmd );

			sdk::vec3_t new_shoot_pos = g_ctx->shoot_pos( );

			m_angle = ( ideal_select->m_pos - new_shoot_pos ).angles( );

			g_ctx->was_shooting( ) = false;

			valve::weapon_info_t* wpn_info = g_local_player->weapon( )->info( );

			if( wpn_info ) {

				bool between_shots = ( m_cfg->m_stop_modifiers & 2 ) 
					&& g_ctx->get_auto_peek_info( ).m_start_pos == sdk::vec3_t( ) 
					&& ( wpn_info->m_full_auto 
						|| wpn_info->m_type == valve::e_weapon_type::pistol );

				if( can_shoot( true, 0, between_shots ) 
					|| ( g_exploits->m_ticks_allowed >= 14
							&& can_shoot( true, 14, between_shots ) ) ) { 
					m_should_stop = get_autostop_type( ) + 1;
				}
			}

			if( g_ctx->can_shoot( ) && !m_silent_aim ) {

				auto wpn_idx = g_local_player->weapon( )->item_index( );
				bool can_scope = !g_local_player->self( )->scoped( ) && ( wpn_idx == valve::e_item_index::aug || wpn_idx == valve::e_item_index::sg553 || wpn_idx == valve::e_item_index::scar20 || wpn_idx == valve::e_item_index::g3sg1 || g_local_player->weapon_info( )->m_type == valve::e_weapon_type::sniper );

				if( !( user_cmd.m_buttons & valve::e_buttons::in_jump ) ) {
					if( can_scope
						&& m_cfg->m_auto_scope )
						user_cmd.m_buttons |= valve::e_buttons::in_attack2;
				}

				lag_backup_t lag_backup{ };
				lag_backup.setup( ideal_select->m_player );
				ideal_select->m_record->adjust( ideal_select->m_player );
				const bool hit_chance = calc_hit_chance( ideal_select->m_player, m_angle );

				lag_backup.restore( ideal_select->m_player );

				if( hit_chance ) {
					std::stringstream msg;

					int idx = ideal_select->m_player->networkable( )->index( );
					valve::player_info_t info;

					auto find = valve::g_engine->get_player_info( idx, &info );

					auto get_hitbox_name_by_id = [ ]( valve::e_hitbox id ) -> const char* {
						switch( id ) {
						case valve::e_hitbox::head:
							return "head";
							break;
						case valve::e_hitbox::neck:
							return "neck";
							break;
						case valve::e_hitbox::pelvis:
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
						case valve::e_hitbox::right_thigh:
						case valve::e_hitbox::right_calf:
						case valve::e_hitbox::right_foot:
							return "right leg";
							break;
						case valve::e_hitbox::left_thigh:
						case valve::e_hitbox::left_calf:
						case valve::e_hitbox::left_foot:
							return "left leg";
							break;
						default:
							return "hands/legs";
							break;
						}
					};

					std::string solve_method{ };

					switch( ideal_select->m_record->m_resolver_method ) {
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
						solve_method = "last move";
						break;
					case e_solve_methods::body_flick:
						solve_method = "flick";
						break;
					case e_solve_methods::backwards:
						solve_method = "backwards";
						break;
					case e_solve_methods::forwards:
						solve_method = "forwards";
						break;
					case e_solve_methods::freestand_l:
						solve_method = "anti-fs";
						break;
					case e_solve_methods::brute:
						solve_method = "brute";
						break;
					case e_solve_methods::brute_not_moved:
						solve_method = "brute [ no move data ]";
						break;
					case e_solve_methods::just_stopped:
						solve_method = "anim lby";
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

					if( find ) {


						auto& entry = hacks::g_lag_comp->entry( idx - 1 );

						int rounded_damage =  ( int )std::round( ideal_select->m_dmg );
						int hitbox = ( int )ideal_select->m_hit_box;
						int rounded_vel = ( int )std::round( ideal_select->m_record->m_anim_velocity.length( 2u ) );

						msg << "fired shot at " << info.m_name;
						msg << " in " << std::string( get_hitbox_name_by_id( ideal_select->m_hit_box ) ).data( );
						msg << " (" << std::to_string( hitbox ) << ")";
						msg << " for " << std::to_string( rounded_damage ) << " damage";
						msg << " [ resolver: " << solve_method.data( ) << " |";
						msg << " vel: " << std::to_string( rounded_vel ) << " |";
						msg << " data: " << std::to_string( entry.m_moved ) << " ]";
						/*
						msg << xor_str( "fired shot | dmg: " ) << std::to_string( static_cast < int >( ideal_select->m_dmg ) ).data( ) << xor_str( " | " );
						msg << xor_str( "hitbox: " ) << std::string( get_hitbox_name_by_id( ideal_select->m_hit_box ) ).data( ) << xor_str( " | " );
						msg << xor_str( "resolver: " ) << solve_method.data( );*/
					}

					constexpr uint8_t gray_clr [ 4 ] = { 201, 201, 201, 255 };

					const std::string msg_to_string = msg.str( );

					g_ctx->was_shooting( ) = true;
					if( g_ctx->was_shooting( ) ) {
						g_ctx->allow_defensive( ) = false;
					}

					static auto weapon_recoil_scale = valve::g_cvar->find_var( xor_str( "weapon_recoil_scale" ) );

					g_shots->add( 
						g_ctx->shoot_pos( ), ideal_select->m_target,
						hacks::g_exploits->m_next_shift_amount, user_cmd.m_number, valve::g_global_vars.get( )->m_real_time, g_ctx->net_info( ).m_latency.m_out + g_ctx->net_info( ).m_latency.m_in
						 );

					valve::g_cvar->con_print( false, *gray_clr, msg_to_string.c_str( ) );
					valve::g_cvar->con_print( false, *gray_clr, xor_str( "\n" ) );

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

					g_ctx->anim_data( ).m_local_data.m_last_shot_time = valve::g_global_vars.get( )->m_cur_time;

					g_visuals->add_shot_mdl( ideal_select->m_player, ideal_select->m_record->m_bones.data( ) );
				}
			}			
		}

		m_targets.clear( );
	}

	void c_knife_bot::handle_knife_bot( valve::user_cmd_t& cmd )
	{
		m_best_index = -1;

		if( !g_local_player->self( )->weapon( ) )
			return;

		if( !g_local_player->self( )->weapon( )->info( ) )
			return;

		if( g_local_player->self( )->weapon( )->info( )->m_type != valve::e_weapon_type::knife )
			return;

		if( !select_target( ) )
			return;

		auto best_angle = get_hitbox_pos( 5, m_best_player );
		auto entity_angle = sdk::calc_ang( g_ctx->shoot_pos( ), best_angle );
		auto health = m_best_player->health( );
		auto stab = false;

		if( m_best_player->has_heavy_armor( ) )
		{
			if( health <= 55 && health > get_min_dmg( ) )
				stab = true;
		}
		else
		{
			if( health <= 65 && health > get_min_dmg( ) )
				stab = true;
		}

		if( health >= 90 )
			stab = true;

		stab = stab && m_best_distance < 60;

		cmd.m_view_angles = sdk::qang_t( entity_angle.x( ), entity_angle.y( ), entity_angle.z( ) );

		if( stab )
			cmd.m_buttons |= valve::e_buttons::in_attack2;
		else
			cmd.m_buttons |= valve::e_buttons::in_attack;

		cmd.m_tick = valve::to_ticks( m_best_player->sim_time( ) + g_ctx->net_info( ).m_lerp );
	}
	ALWAYS_INLINE float dist_to( sdk::vec3_t& from, const sdk::vec3_t& value ) { return ( from - value ).length( ); }

	bool c_knife_bot::select_target( )
	{
		float good_distance = 75.0f;

		for( int i = 1; i <= valve::g_global_vars.get( )->m_max_clients; i++ )
		{
			valve::cs_player_t* player = ( valve::cs_player_t* ) valve::g_entity_list->get_entity( i );

			if( !player || player == g_local_player->self( ) || player->networkable( )->dormant( ) || !player->alive( ) || player->team( ) == g_local_player->self( )->team( ) )
				continue;

			sdk::vec3_t local_position = g_local_player->self( )->origin( );
			local_position.z( ) += 50.0f;

			sdk::vec3_t entity_position = player->origin( );
			entity_position.z( ) += 50.0f;

			float current_distance = dist_to( local_position, entity_position );

			if( current_distance < good_distance )
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