#pragma once

#include "../sdk/sdk.hpp"

#include <d3d9.h>
#include <d3dx9.h>

#include <minhook.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx9.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_internal.h>

#define OFFSET( type, name, offset ) \
    __forceinline type& name { \
        return *sdk::address_t{ this }.self_offset( offset ).as< type* > ( ); \
    } \

#define POFFSET( type, name, offset ) \
    __forceinline type* name { \
        return sdk::address_t{ this }.self_offset( offset ).as< type* > ( ); \
    } \

#define VFUNC( type, name, index, ... ) \
    __forceinline auto name { \
        using fn_t = type; \
        \
        return ( *sdk::address_t{ this }.as< fn_t** > ( ) )[ index ]( this, __VA_ARGS__ ); \
    } \

#define OFFSET_VFUNC( type, name, offset, ... ) \
    __forceinline auto name { \
        return offset.as< type > ( )( this, __VA_ARGS__ ); \
    } \

#define CSGO2018 

#ifdef CSGO2018
#define VARVAL( old, latest ) old
#else
#define VARVAL( old, latest ) latest
#endif

#include "../sdk/data_crypt.h"
#include "ctx/ctx.hpp"
#include "menu/menu.hpp"
#include "valve/valve.hpp"
#include "local_player/local_player.hpp"
#include "render/render.h"
#include "hacks/hacks.hpp"
#include "hooks/hooks.hpp"
#include "render/blur/post_process.h"