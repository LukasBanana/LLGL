/*
 * D3D11SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SWAP_CHAIN_H
#define LLGL_D3D11_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>
#include "../DXCommon/ComPtr.h"
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


class D3D11RenderSystem;

class D3D11SwapChain final : public SwapChain
{

    public:

        D3D11SwapChain(
            IDXGIFactory*                       factory,
            const ComPtr<ID3D11Device>&         device,
            D3D11RenderSystem&                  renderSystem,
            const SwapChainDescriptor&          desc,
            const std::shared_ptr<Surface>&     surface
        );

        void SetDebugName(const char* name) override;

        void Present() override;

        std::uint32_t GetCurrentSwapIndex() const override;
        std::uint32_t GetNumSwapBuffers() const override;
        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        // Copyies a subresource region from the backbuffer (color or depth-stencil) into the destination resource.
        HRESULT CopySubresourceRegion(
            ID3D11DeviceContext*    context,
            ID3D11Resource*         dstResource,
            UINT                    dstSubresource,
            UINT                    dstX,
            UINT                    dstY,
            UINT                    dstZ,
            const D3D11_BOX&        srcBox,
            DXGI_FORMAT             format
        );

        inline ID3D11RenderTargetView* const * GetRenderTargetViews() const
        {
            return renderTargetView_.GetAddressOf();
        }

        inline ID3D11DepthStencilView* GetDepthStencilView() const
        {
            return depthStencilView_.Get();
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetPresentSyncInterval(UINT syncInterval);

        void CreateSwapChain(IDXGIFactory* factory, const Extent2D& resolution, std::uint32_t samples, std::uint32_t swapBuffers);
        void CreateBackBuffer();
        void ResizeBackBuffer(const Extent2D& resolution);

        void StoreDebugNames(std::string (&debugNames)[4]);
        void RestoreDebugNames(const std::string (&debugNames)[4]);
        
        void CheckTearingSupport(IDXGIFactory* factory);

    private:

        ComPtr<ID3D11Device>            device_;
        D3D11RenderSystem&              renderSystem_;

        ComPtr<IDXGISwapChain>          swapChain_;
        UINT                            swapChainInterval_      = 0;
        DXGI_SAMPLE_DESC                swapChainSampleDesc_    = { 1, 0 };

        ComPtr<ID3D11Texture2D>         colorBuffer_;
        ComPtr<ID3D11RenderTargetView>  renderTargetView_;
        ComPtr<ID3D11Texture2D>         depthBuffer_;
        ComPtr<ID3D11DepthStencilView>  depthStencilView_;

        DXGI_FORMAT                     colorFormat_            = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT                     depthStencilFormat_     = DXGI_FORMAT_UNKNOWN;

        bool                            hasDebugName_           = false;
        bool                            tearingSupported_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
