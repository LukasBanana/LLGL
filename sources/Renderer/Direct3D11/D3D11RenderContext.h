/*
 * D3D11RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_RENDER_CONTEXT_H__
#define __LLGL_D3D11_RENDER_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <cstddef>
#include "../ComPtr.h"
#include "../DXCommon/DXCore.h"
#include <vector>
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


// Container structure for all D3D11 back buffer objects.
struct D3D11BackBuffer
{
    ComPtr<ID3D11Texture2D>         colorBuffer;
    ComPtr<ID3D11RenderTargetView>  rtv;
    ComPtr<ID3D11Texture2D>         depthStencil;
    ComPtr<ID3D11DepthStencilView>  dsv;
};


class D3D11RenderSystem;
class D3D11StateManager;
class D3D11RenderTarget;

class D3D11RenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        D3D11RenderContext(
            D3D11RenderSystem& renderSystem,
            D3D11StateManager& stateMngr,
            const ComPtr<ID3D11DeviceContext>& context,
            RenderContextDescriptor desc,
            const std::shared_ptr<Window>& window
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        /* ----- Extended internal functions ----- */

        inline const D3D11BackBuffer& GetBackBuffer() const
        {
            return backBuffer_;
        }

    private:

        void CreateSwapChain();
        void CreateBackBuffer(UINT width, UINT height);
        void ResizeBackBuffer(UINT width, UINT height);

        D3D11RenderSystem&          renderSystem_;  // reference to its render system
        D3D11StateManager&          stateMngr_;
        RenderContextDescriptor     desc_;
        
        ComPtr<ID3D11DeviceContext> context_;

        ComPtr<IDXGISwapChain>      swapChain_;
        UINT                        swapChainInterval_  = 0;

        D3D11BackBuffer             backBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
