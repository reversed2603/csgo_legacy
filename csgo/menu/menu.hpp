#pragma once

namespace csgo {
    class c_menu {
    private:
        struct {
           
            struct {
              
            } m_misc;

            bool m_hidden{ };
            int m_current_tab{ };
        } m_main{ };
    public:
        void render( );
        bool m_is_locked{ };

        __forceinline auto& main( );
    };

    inline const auto g_menu = std::make_unique< c_menu >( );
}

#include "impl/menu.inl"