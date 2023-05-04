#ifdef _WIN32
#include <d3d9.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
#else
#include "imgui/GL/gl3w.h"
#endif

#include "imgui/imgui.h"

// shaders are build during compilation and header files are created
#include "shaders/build/blur_x.h"
#include "shaders/build/blur_y.h"
#include "shaders/build/monochrome.h"

#include "post_process.h"

static IDirect3DDevice9* device; // DO NOT RELEASE!

[[nodiscard]] static IDirect3DTexture9* createTexture( int width, int height ) noexcept
{ 
    IDirect3DTexture9* texture;
    device->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr );
    return texture;
}

static void copyBackbufferToTexture( IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering ) noexcept
{ 
    if( ComPtr<IDirect3DSurface9> backBuffer; device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, backBuffer.GetAddressOf( ) ) == D3D_OK ) { 
        if( ComPtr<IDirect3DSurface9> surface; texture->GetSurfaceLevel( 0, surface.GetAddressOf( ) ) == D3D_OK )
            device->StretchRect( backBuffer.Get( ), nullptr, surface.Get( ), nullptr, filtering );
    }
}

static void setRenderTarget( IDirect3DTexture9* rtTexture ) noexcept
{ 
    if( ComPtr<IDirect3DSurface9> surface; rtTexture->GetSurfaceLevel( 0, surface.GetAddressOf( ) ) == D3D_OK )
        device->SetRenderTarget( 0, surface.Get( ) );
}

class ShaderProgram { 
public:
    ~ShaderProgram( )
    { 

    }

    void use( float uniform, int location ) const noexcept
    { 
        device->SetPixelShader( pixelShader.Get( ) );
        const float params[4] = { uniform };
        device->SetPixelShaderConstantF( location, params, 1 );
    }

    void init( const BYTE* pixelShaderSrc ) noexcept
    { 
        if( initialized )
            return;
        initialized = true;

        device->CreatePixelShader( reinterpret_cast<const DWORD*> ( pixelShaderSrc ), pixelShader.GetAddressOf( ) );
    }

private:
    ComPtr<IDirect3DPixelShader9> pixelShader;
    bool initialized = false;
};

class blur_effect { 
public:
    static void draw( ImDrawList* drawList, float alpha ) noexcept
    { 
        instance( )._draw( drawList, alpha );
    }
    static void draw( ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, float alpha ) noexcept
    { 
        instance( )._draw( drawList, p_min, p_max, alpha );
    }

    static void clear_textures( ) noexcept
    { 
        if( instance( ).blurTexture1 ) { 
            instance( ).blurTexture1->Release( );
            instance( ).blurTexture1 = nullptr;
        }
        if( instance( ).blurTexture2 ) { 
            instance( ).blurTexture2->Release( );
            instance( ).blurTexture2 = nullptr;
        }
    }
private:
    IDirect3DSurface9* rtBackup = nullptr;
    IDirect3DTexture9* blurTexture1 = nullptr;
    IDirect3DTexture9* blurTexture2 = nullptr;

    ShaderProgram blurShaderX;
    ShaderProgram blurShaderY;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    static constexpr auto blurDownsample = 2;

    blur_effect( ) = default;
    blur_effect( const blur_effect& ) = delete;

    ~blur_effect( )
    { 
        if( rtBackup )
            rtBackup->Release( );
        if( blurTexture1 )
            blurTexture1->Release( );
        if( blurTexture2 )
            blurTexture2->Release( );
    }

    static blur_effect& instance( ) noexcept
    { 
        static blur_effect blur_effect;
        return blur_effect;
    }

    static void begin( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._begin( ); }
    static void firstPass( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._firstPass( ); }
    static void secondPass( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._secondPass( ); }
    static void end( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._end( ); }

    void createTextures( ) noexcept
    { 
        if( const auto [width, height] = ImGui::GetIO( ).DisplaySize; backbufferWidth != static_cast<int> ( width ) || backbufferHeight != static_cast<int> ( height ) ) { 
            clear_textures( );
            backbufferWidth = static_cast<int> ( width );
            backbufferHeight = static_cast<int> ( height );
        }

        if( !blurTexture1 )
            blurTexture1 = createTexture( backbufferWidth / blurDownsample, backbufferHeight / blurDownsample );
        if( !blurTexture2 )
            blurTexture2 = createTexture( backbufferWidth / blurDownsample, backbufferHeight / blurDownsample );
    }

    void createShaders( ) noexcept
    { 
        blurShaderX.init( blur_x );
        blurShaderY.init( blur_y );
    }

    void _begin( ) noexcept
    { 
        device->GetRenderTarget( 0, &rtBackup );

        copyBackbufferToTexture( blurTexture1, D3DTEXF_LINEAR );

        device->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
        device->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
        device->SetRenderState( D3DRS_SCISSORTESTENABLE, false );

        const D3DMATRIX projection{ { { 
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f / ( backbufferWidth / blurDownsample ), 1.0f / ( backbufferHeight / blurDownsample ), 0.0f, 1.0f
        }} };
        device->SetVertexShaderConstantF( 0, &projection.m[0][0], 4 );
    }

    void _firstPass( ) noexcept
    { 
        blurShaderX.use( 1.0f / ( backbufferWidth / blurDownsample ), 0 );
        setRenderTarget( blurTexture2 );
    }

    void _secondPass( ) noexcept
    { 
        blurShaderY.use( 1.0f / ( backbufferHeight / blurDownsample ), 0 );
        setRenderTarget( blurTexture1 );
    }

    void _end( ) noexcept
    { 
        device->SetRenderTarget( 0, rtBackup );
        rtBackup->Release( );

        device->SetPixelShader( nullptr );
        device->SetRenderState( D3DRS_SCISSORTESTENABLE, true );
    }

    void _draw( ImDrawList* drawList, float alpha ) noexcept
    { 
        createTextures( );
        createShaders( );

        if( !blurTexture1 || !blurTexture2 )
            return;

        drawList->AddCallback( &begin, nullptr );
        for( int i = 0; i < 8; ++i ) { 
            drawList->AddCallback( &firstPass, nullptr );
            drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture1 ), { -1.0f, -1.0f }, { 1.0f, 1.0f } );
            drawList->AddCallback( &secondPass, nullptr );
            drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture2 ), { -1.0f, -1.0f }, { 1.0f, 1.0f } );
        }
        drawList->AddCallback( &end, nullptr );
        drawList->AddCallback( ImDrawCallback_ResetRenderState, nullptr );

        drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture1 ), { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32( 255, 255, 255, 255 * alpha ) );
    }

    void _draw( ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, float alpha ) noexcept
    { 
        createTextures( );
        createShaders( );

        if( !blurTexture1 || !blurTexture2 )
            return;

        drawList->AddCallback( &begin, nullptr );
        for( int i = 0; i < 8; ++i ) { 
            drawList->AddCallback( &firstPass, nullptr );
            drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture1 ), p_min, p_max, { 0.0f, 0.0f } );
            drawList->AddCallback( &secondPass, nullptr );
            drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture2 ), p_min, p_max, { 0.0f, 0.0f } );
        }
        drawList->AddCallback( &end, nullptr );
        drawList->AddCallback( ImDrawCallback_ResetRenderState, nullptr );

        drawList->AddImage( reinterpret_cast<ImTextureID> ( blurTexture1 ), p_min, p_max, ImVec2( backbufferWidth * 1.f, backbufferHeight * 1.f ), ImVec2( backbufferWidth * 1.f, backbufferHeight * 1.f ), IM_COL32( 255, 255, 255, 255 * alpha ) );
    }
};

class mono_chrome_effect { 
public:
    static void draw( ImDrawList* drawList, float amount ) noexcept
    { 
        instance( ).amount = amount;
        instance( )._draw( drawList );
    }

    static void clear_texture( ) noexcept
    { 
        instance( )._clear_texture( );
    }

private:
    IDirect3DTexture9* texture = nullptr;

    ShaderProgram shader;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    float amount = 0.0f;

    mono_chrome_effect( ) = default;
    mono_chrome_effect( const mono_chrome_effect& ) = delete;

    ~mono_chrome_effect( )
    { 
        if( texture )
            texture->Release( );
    }

    static mono_chrome_effect& instance( ) noexcept
    { 
        static mono_chrome_effect mono_chrome_effect;
        return mono_chrome_effect;
    }

    void _clear_texture( ) noexcept
    { 
        if( texture ) { 
            texture->Release( );
            texture = nullptr;
        }
    }

    static void begin( const ImDrawList*, const ImDrawCmd* cmd ) noexcept { instance( )._begin( ); }
    static void end( const ImDrawList*, const ImDrawCmd* cmd ) noexcept { instance( )._end( ); }

    void createTexture( ) noexcept
    { 
        if( const auto [width, height] = ImGui::GetIO( ).DisplaySize; backbufferWidth != static_cast<int> ( width ) || backbufferHeight != static_cast<int> ( height ) ) { 
            clear_texture( );
            backbufferWidth = static_cast<int> ( width );
            backbufferHeight = static_cast<int> ( height );
        }

        if( !texture )
            texture = ::createTexture( backbufferWidth, backbufferHeight );
    }

    void createShaders( ) noexcept
    { 
        shader.init( monochrome );
    }

    void _begin( ) noexcept
    { 
        copyBackbufferToTexture( texture, D3DTEXF_NONE );

        device->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
        device->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

        const D3DMATRIX projection{ { { 
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f / ( backbufferWidth ), 1.0f / ( backbufferHeight ), 0.0f, 1.0f
        }} };
        device->SetVertexShaderConstantF( 0, &projection.m[0][0], 4 );
        shader.use( amount, 0 );
    }

    void _end( ) noexcept
    { 
        device->SetPixelShader( nullptr );
    }

    void _draw( ImDrawList* drawList ) noexcept
    { 
        createTexture( );
        createShaders( );
        if( !texture )
            return;

        drawList->AddCallback( &begin, nullptr );
        drawList->AddImage( reinterpret_cast<ImTextureID> ( texture ), { -1.0f, -1.0f }, { 1.0f, 1.0f } );
        drawList->AddCallback( &end, nullptr );
        drawList->AddCallback( ImDrawCallback_ResetRenderState, nullptr );
    }

};

void post_process::set_device_next( IDirect3DDevice9* device ) noexcept
{ 
    ::device = device;
}

void post_process::clear_textures( ) noexcept
{ 
    blur_effect::clear_textures( );
}

void post_process::on_reset( ) noexcept
{ 
    blur_effect::clear_textures( );
}

void post_process::perform_full_screen_blur( ImDrawList* drawList, float alpha ) noexcept
{ 
    blur_effect::draw( drawList, alpha );
}

void post_process::perform_blur( ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, float alpha ) noexcept
{ 
    blur_effect::draw( drawList, p_min, p_max, alpha );
}

void post_process::perform_full_screen_mono_chrome( ImDrawList* drawList, float amount ) noexcept
{ 
    mono_chrome_effect::draw( drawList, amount );
}
