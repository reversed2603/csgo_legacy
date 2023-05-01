#include "../../../csgo.hpp"

namespace csgo::game { 
	void c_net_vars::parse_table( 
		const char* name, const recv_table_t* const table, std::uint32_t offset
	 )
	{ 
		for( int i { }; i < table->m_props_count; ++i ) { 
			const auto prop = &table->m_props[ i ];

			const auto child = prop->m_data_table;
			if( child
				&& child->m_props_count > 0 )
				parse_table( name, child, prop->m_offset + offset );

			char str[ 256u ] { };

			strcpy_s( str, name );
			strcat_s( str, "->" );
			strcat_s( str, prop->m_var_name );

			m_entries[ str ] = { prop, prop->m_offset + offset };
		}
	}

	void c_net_vars::parse_client_classes( )
	{ 
		for( auto client_class = game::g_client->all_classes( ); client_class; client_class = client_class->m_next ) { 
			if( !client_class->m_recv_table )
				continue;

			parse_table( client_class->m_network_name, client_class->m_recv_table, 0u );
		}
	}

	void c_net_vars::parse_data_maps( const std::vector< data_map_t* >& data_maps )
	{ 
		for( const auto& data_map : data_maps ) { 
			for( int i { }; i < data_map->m_size; ++i ) { 
				const auto& description = data_map->m_descriptions[ i ];
				if( !description.m_name )
					continue;

				m_entries[ description.m_name ] = { nullptr, description.m_offset };
			}
		}
	}

	void anim_state_t::set_layer_seq( game::anim_layer_t* layer, int act )
	{ 
		int32_t sequence = select_sequence_from_acitivty_modifier( act );
		if( sequence < 2 )
			return;

		layer->m_cycle = 0.0f;
		layer->m_weight = 0.0f;
		layer->m_seq = sequence;
		layer->m_playback_rate = g_local_player->self( )->get_layer_seq_cycle_rate( layer, sequence );
	}

	bool trace_filter_t::should_hit_entity( base_entity_t* entity, int ) const { 
		auto ent_cc = entity->networkable( )->client_class( );
		if( ent_cc && strcmp( m_ignore_cc, "" ) ) { 
			if( ent_cc->m_network_name == m_ignore_cc )
				return false;
		}

		return !( entity == m_ignore_entity );
	}
}