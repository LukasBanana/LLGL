/*
 * D3D11RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RENDER_CONTEXT_H
#define LLGL_D3D11_RENDER_CONTEXT_H


#include <LLGL/RenderContext.h>
#include "../DXCommon/ComPtr.h"
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


class D3D11RenderContext final : public RenderContext
{

    public:

        D3D11RenderContext(
            IDXGIFactory*                       factory,
            const ComPtr<ID3D11Device>&         device,
            const ComPtr<ID3D11DeviceContext>&  context,
            const RenderContextDescriptor&      desc,
            const std::shared_ptr<Surface>&     surface
        );

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        inline const D3D11BackBuffer& GetBackBuffer() const
        {
            return backBuffer_;
        }

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        void CreateSwapChain(IDXGIFactory* factory, UINT samples);
        void CreateBackBuffer(const VideoModeDescriptor& videoModeDesc);
        void ResizeBackBuffer(const VideoModeDescriptor& videoModeDesc);

    private:

        ComPtr<ID3D11Device>        device_;
        ComPtr<ID3D11DeviceContext> context_;

        ComPtr<IDXGISwapChain>      swapChain_;
        UINT                        swapChainInterval_      = 0;
        DXGI_SAMPLE_DESC            swapChainSampleDesc_    = { 1, 0 };

        D3D11BackBuffer             backBuffer_;

        DXGI_FORMAT                 colorFormat_        = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT                 depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;

};


} // /namespace LLGL


#endif



// ================================================================================
