/*
 * D3D12RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_CONTEXT_H
#define LLGL_D3D12_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
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

class D3D12RenderContext final : public RenderContext
{

    public:

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        D3D12RenderContext(
            D3D12RenderSystem&              renderSystem,
            const RenderContextDescriptor&  desc,
            const std::shared_ptr<Surface>& surface
        );

        ~D3D12RenderContext();

        // Returns the native color buffer resource from the swap-chain that is currently being used.
        D3D12Resource& GetCurrentColorBuffer();

        void ResolveRenderTarget(D3D12CommandContext& commandContext);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForRTV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForDSV() const;

        bool HasMultiSampling() const;
        bool HasDepthBuffer() const;

        void SyncGPU();

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        void QueryDeviceParameters(const D3D12Device& device, std::uint32_t samples);

        void CreateWindowSizeDependentResources(const VideoModeDescriptor& videoModeDesc);
        void CreateColorBufferRTVs(const VideoModeDescriptor& videoModeDesc);
        void CreateDepthStencil(const VideoModeDescriptor& videoModeDesc);

        void MoveToNextFrame();

    private:

        static const UINT g_maxSwapChainSize = 3;

        D3D12RenderSystem&              renderSystem_;  // reference to its render system
        D3D12CommandQueue*              commandQueue_                           = nullptr;
        D3D12RenderPass                 defaultRenderPass_;

        ComPtr<IDXGISwapChain3>         swapChain_;
        UINT                            swapChainInterval_                      = 0;
        DXGI_SAMPLE_DESC                swapChainSampleDesc_                    = { 1, 0 };

        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        UINT                            rtvDescSize_                            = 0;
        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;

        D3D12Resource                   colorBuffers_[g_maxSwapChainSize];
        D3D12Resource                   colorBuffersMS_[g_maxSwapChainSize];
        DXGI_FORMAT                     colorFormat_                            = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM;

        D3D12Resource                   depthStencil_;
        DXGI_FORMAT                     depthStencilFormat_                     = DXGI_FORMAT_UNKNOWN;

        UINT64                          frameFenceValues_[g_maxSwapChainSize]   = {};
        D3D12Fence                      frameFence_;

        UINT                            numFrames_                              = 0;
        UINT                            currentFrame_                           = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
