#pragma once
namespace csgo::hacks { 
	class c_local_sync { 
	protected:
		float m_last_upd { };
		float m_last_choke { };

		std::array< game::anim_layer_t, 13 > get_anim_layers( )
		{ 
			std::array< game::anim_layer_t, 13 > out_put;

			std::memcpy( out_put.data( ), g_local_player->self( )->anim_layers( ).data( ), sizeof( game::anim_layer_t ) * 13 );
			std::memcpy( &out_put.at( 4 ), &m_anim_layers.at( 4 ), sizeof( game::anim_layer_t ) );
			std::memcpy( &out_put.at( 5 ), &m_anim_layers.at( 5 ), sizeof( game::anim_layer_t ) );
			std::memcpy( &out_put.at( 6 ), &m_anim_layers.at( 6 ), sizeof( game::anim_layer_t ) );
			std::memcpy( &out_put.at( 7 ), &m_anim_layers.at( 7 ), sizeof( game::anim_layer_t ) );
			std::memcpy( &out_put.at( 11 ), &m_anim_layers.at( 11 ), sizeof( game::anim_layer_t ) );
			std::memcpy( &out_put.at( 12 ), &m_anim_layers.at( 12 ), sizeof( game::anim_layer_t ) );

			return out_put;
		}

	public:
		void handle_ctx( const game::user_cmd_t& user_cmd, bool& send_packet );
		void handle_anim_interp( );
		void setup_bones( std::array < sdk::mat3x4_t, game::k_max_bones >& out, float time, int custom_max = -1 );
		void do_anim_event( );
		void simulate( );
		game::anim_layers_t m_anim_layers { };
		game::pose_params_t m_pose_params { };

		game::anim_layers_t m_old_layers { };
		game::pose_params_t m_old_params { };
	};

	class c_anim_sync { 
	protected:

	public:
		void handle_player_update( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry );
		void catch_ground( cc_def( lag_record_t* ) current, cc_def( previous_lag_data_t* )  previous, player_entry_t& entry );
		__forceinline void simulate( lag_record_t* current, cc_def( previous_lag_data_t* ) previous, player_entry_t& entry );
		void setup_bones( game::cs_player_t* player, std::array < sdk::mat3x4_t, 256 >& out, float time, bool visual_matrix = false );
	};

	class c_resolver { 
	protected:
		void set_solve_mode( cc_def( lag_record_t* ), player_entry_t& );
		void solve_air( cc_def( lag_record_t* ), cc_def( previous_lag_data_t* ), player_entry_t& );
		void solve_walk( cc_def( lag_record_t* ), player_entry_t& );
		void solve_stand( cc_def( lag_record_t* ), cc_def( previous_lag_data_t* ), player_entry_t& );

		__forceinline float get_away_angle( lag_record_t* current );
		__forceinline bool is_sideways( cc_def( lag_record_t* ) current, float yaw, bool high_delta );
	public:
		void handle_ctx( cc_def( lag_record_t* ), cc_def( previous_lag_data_t* ), player_entry_t& );
		void parse_lby_proxy( game::cs_player_t* player, float* new_lby );
		void anti_freestand( player_entry_t& entry );
	};

	class c_setup_bones { 
	public:
		void handle_ctx( game::cs_player_t* player, sdk::mat3x4_t* bones, int bone_count, int bone_mask, float time );
	};

	inline const std::unique_ptr < c_setup_bones > g_setup_bones = std::make_unique < c_setup_bones >( );
	inline const std::unique_ptr < c_resolver > g_resolver = std::make_unique < c_resolver >( );
	inline const std::unique_ptr < c_anim_sync > g_anim_sync = std::make_unique < c_anim_sync >( );
	inline const std::unique_ptr < c_local_sync > g_local_sync = std::make_unique < c_local_sync >( );
}

#include "impl/anim_sync.inl"