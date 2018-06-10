/*
 * D3D12RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_CONTEXT_H
#define LLGL_D3D12_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <cstddef>
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12CommandBuffer;

class D3D12RenderContext : public RenderContext
{

    public:

        D3D12RenderContext(
            D3D12RenderSystem& renderSystem,
            RenderContextDescriptor desc,
            const std::shared_ptr<Surface>& surface
        );

        ~D3D12RenderContext();

        void Present() override;

        /* --- Extended functions --- */

        ID3D12Resource* GetCurrentColorBuffer();

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForCurrentRTV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForDSV() const;

        void SetCommandBuffer(D3D12CommandBuffer* commandBuffer);

        void TransitionRenderTarget(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

        bool HasMultiSampling() const;
        bool HasDepthBuffer() const;

        void SyncGPU();

    private:

        static const UINT maxNumBuffers = 3;

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        void CreateWindowSizeDependentResources(const VideoModeDescriptor& videoModeDesc);
        void CreateColorBufferRTVs(const VideoModeDescriptor& videoModeDesc);
        void CreateDepthStencil(const VideoModeDescriptor& videoModeDesc);
        void CreateDeviceResources();
        void UpdateFenceValues();

        void MoveToNextFrame();

        void ResolveRenderTarget(ID3D12GraphicsCommandList* commandList);

        D3D12RenderSystem&              renderSystem_;  // reference to its render system
        D3D12CommandBuffer*             commandBuffer_                  = nullptr;

        ComPtr<IDXGISwapChain3>         swapChain_;
        UINT                            swapChainInterval_              = 0;
        UINT                            swapChainSamples_               = 1;

        ComPtr<ID3D12DescriptorHeap>    rtvDescHeap_;
        UINT                            rtvDescSize_                    = 0;
        ComPtr<ID3D12DescriptorHeap>    dsvDescHeap_;

        ComPtr<ID3D12Resource>          colorBuffers_[maxNumBuffers];
        ComPtr<ID3D12Resource>          colorBuffersMS_[maxNumBuffers];
        DXGI_FORMAT                     colorBufferFormat_              = DXGI_FORMAT_B8G8R8A8_UNORM;

        ComPtr<ID3D12Resource>          depthStencil_;
        DXGI_FORMAT                     depthStencilFormat_             = DXGI_FORMAT_UNKNOWN;

        ComPtr<ID3D12CommandAllocator>  commandAllocs_[maxNumBuffers];
        UINT64                          fenceValues_[maxNumBuffers]     = { 0 };

        UINT                            numFrames_                      = 0;
        UINT                            currentFrame_                   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
