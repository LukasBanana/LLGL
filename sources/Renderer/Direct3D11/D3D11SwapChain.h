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


class D3D11CommandBuffer;

class D3D11SwapChain final : public SwapChain
{

    public:

        D3D11SwapChain(
            IDXGIFactory*                   factory,
            const ComPtr<ID3D11Device>&     device,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface
        );

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetCurrentSwapIndex() const override;
        std::uint32_t GetNumSwapBuffers() const override;
        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        // Binds the framebuffer view of this swap-chain and stores a references to this command buffer.
        void BindFramebufferView(D3D11CommandBuffer* commandBuffer);

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetPresentSyncInterval(UINT syncInterval);

        void CreateSwapChain(IDXGIFactory* factory, const Extent2D& resolution, std::uint32_t samples, std::uint32_t swapBuffers);
        void CreateBackBuffer();
        void ResizeBackBuffer(const Extent2D& resolution);

    private:

        ComPtr<ID3D11Device>            device_;

        ComPtr<IDXGISwapChain>          swapChain_;
        UINT                            swapChainInterval_      = 0;
        DXGI_SAMPLE_DESC                swapChainSampleDesc_    = { 1, 0 };

        ComPtr<ID3D11Texture2D>         colorBuffer_;
        ComPtr<ID3D11RenderTargetView>  renderTargetView_;
        ComPtr<ID3D11Texture2D>         depthBuffer_;
        ComPtr<ID3D11DepthStencilView>  depthStencilView_;

        DXGI_FORMAT                     colorFormat_            = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT                     depthStencilFormat_     = DXGI_FORMAT_UNKNOWN;

        D3D11CommandBuffer*             bindingCommandBuffer_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
