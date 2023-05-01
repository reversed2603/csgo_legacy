#pragma once

struct ImDrawList;

struct IDirect3DDevice9;

namespace post_process
{ 
#ifdef _WIN32
    void set_device_next( IDirect3DDevice9* device ) noexcept;
    void clear_textures( ) noexcept;
    void on_reset( ) noexcept;
#endif

    void perform_full_screen_blur( ImDrawList* drawList, float alpha ) noexcept;
    void perform_blur( ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, float alpha ) noexcept;
    void perform_full_screen_mono_chrome( ImDrawList* drawList, float amount ) noexcept;
}
