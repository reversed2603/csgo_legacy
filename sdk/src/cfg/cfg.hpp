#pragma once

#include "detail/detail.hpp"

namespace sdk { 
	template < typename _value_t >
	struct cfg_var_t final : public detail::base_cfg_var_t { 
	private:
		_value_t m_value { };
	public:
		__forceinline constexpr cfg_var_t( ) = default;

		__forceinline cfg_var_t( 
			const std::size_t hash, const _value_t value
		 );

		__forceinline _value_t* operator & ( );

		__forceinline operator _value_t( ) const;

		__forceinline _value_t* operator ->( );

		__forceinline const _value_t* operator ->( ) const;

		__forceinline _value_t& value( );

		__forceinline const _value_t& value( ) const;

		void save( nlohmann::json& object ) const override;

		void load( const nlohmann::json& object ) override;
	};

	class c_cfg { 
	public:
		static constexpr std::uint8_t k_byte_xor = 0xffu;

		std::vector< detail::base_cfg_var_t* > m_vars { };
	public:
		virtual ~c_cfg( ) = default;

		virtual void save( const std::string_view name ) const;

		virtual void load( const std::string_view name );

		__forceinline void add_var( detail::base_cfg_var_t* const var );

		__forceinline detail::base_cfg_var_t* find_var( const std::size_t hash ) const;
	};

	inline const auto g_cfg = std::make_unique< c_cfg >( );
}

#include "impl/cfg.inl"