/*
 * D3D12SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_SWAP_CHAIN_H
#define LLGL_D3D12_SWAP_CHAIN_H


#include <LLGL/Window.h>
#include <LLGL/SwapChain.h>
#include <cstddef>
#include "D3D12Resource.h"
#include "RenderState/D3D12Fence.h"
#include "RenderState/D3D12RenderPass.h"
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12CommandQueue;
class D3D12CommandBuffer;
class D3D12CommandContext;

class D3D12SwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D12SwapChain(
            D3D12RenderSystem&              renderSystem,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface
        );

        ~D3D12SwapChain();

        UINT TranslateSwapIndex(std::uint32_t swapBufferIndex) const;

        // Returns the native color buffer resource from the swap-chain that is currently being used.
        D3D12Resource& GetCurrentColorBuffer(UINT colorBuffer);

        void ResolveSubresources(D3D12CommandContext& commandContext, UINT colorBuffer);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForRTV(UINT colorBuffer) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForDSV() const;

        bool HasMultiSampling() const;
        bool HasDepthBuffer() const;

        // Copyies a subresource region from the backbuffer (color or depth-stencil) into the destination resource.
        HRESULT CopySubresourceRegion(
            D3D12CommandContext&    context,
            D3D12Resource&          dstResource,
            UINT                    dstSubresource,
            UINT                    dstX,
            UINT                    dstY,
            UINT                    dstZ,
            UINT                    srcColorBuffer,
            const D3D12_BOX&        srcBox,
            DXGI_FORMAT             format
        );

    private:

        static constexpr UINT maxNumColorBuffers    = 3;
        static constexpr UINT numDebugNames         = maxNumColorBuffers*2 + 1;

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetPresentSyncInterval(UINT syncInterval);

        void CreateDescriptorHeaps(const D3D12Device& device, std::uint32_t samples);

        HRESULT CreateResolutionDependentResources(const Extent2D& resolution);
        void CreateColorBufferRTVs(ID3D12Device* device, const Extent2D& resolution);
        void CreateDepthStencil(ID3D12Device* device, const Extent2D& resolution);

        void MoveToNextFrame();

        void StoreDebugNames(std::string (&debugNames)[D3D12SwapChain::numDebugNames]);
        void RestoreDebugNames(const std::string (&debugNames)[D3D12SwapChain::numDebugNames]);

    private:

        D3D12RenderSystem&              renderSystem_;  // reference to its render system
        D3D12CommandQueue*              commandQueue_                           = nullptr;
        D3D12RenderPass                 defaultRenderPass_;

        ComPtr<IDXGISwapChain3>         swapChainDXGI_;
        DXGI_SAMPLE_DESC                sampleDesc_                             = { 1, 0 };
        UINT                            syncInterval_                           = 0;

        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        UINT                            rtvDescSize_                            = 0;
        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;

        D3D12Resource                   colorBuffers_[maxNumColorBuffers];
        D3D12Resource                   colorBuffersMS_[maxNumColorBuffers];
        DXGI_FORMAT                     colorFormat_                            = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM;

        D3D12Resource                   depthStencil_;
        DXGI_FORMAT                     depthStencilFormat_                     = DXGI_FORMAT_UNKNOWN;

        UINT64                          frameFenceValues_[maxNumColorBuffers]   = {};
        D3D12NativeFence                frameFence_;

        UINT                            numColorBuffers_                        = 0;
        UINT                            currentColorBuffer_                     = 0;

        bool                            hasDebugName_                           = false;
        bool                            tearingSupported_                       = false;
        bool                            windowedMode_                           = false;

};


} // /namespace LLGL


#endif



// ================================================================================
