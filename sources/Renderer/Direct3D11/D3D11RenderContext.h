/*
 * D3D11RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RENDER_CONTEXT_H
#define LLGL_D3D11_RENDER_CONTEXT_H


#include <LLGL/RenderContext.h>
#include "../ComPtr.h"
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


class D3D11RenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        D3D11RenderContext(
            IDXGIFactory* factory,
            const ComPtr<ID3D11Device>& device,
            const ComPtr<ID3D11DeviceContext>& context,
            const RenderContextDescriptor& desc,
            const std::shared_ptr<Surface>& surface
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

        void CreateSwapChain(IDXGIFactory* factory);
        void CreateBackBuffer(UINT width, UINT height);
        void ResizeBackBuffer(UINT width, UINT height);

        RenderContextDescriptor     desc_;
        
        ComPtr<ID3D11Device>        device_;
        ComPtr<ID3D11DeviceContext> context_;

        ComPtr<IDXGISwapChain>      swapChain_;
        UINT                        swapChainInterval_  = 0;

        D3D11BackBuffer             backBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
