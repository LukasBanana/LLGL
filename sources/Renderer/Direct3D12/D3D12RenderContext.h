/*
 * D3D12RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_CONTEXT_H
#define LLGL_D3D12_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <cstddef>
#include "../ComPtr.h"
#include "../DXCommon/DXCore.h"

#include "RenderState/D3D12StateManager.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;

class D3D12RenderContext : public RenderContext
{

    public:

        D3D12RenderContext(
            D3D12RenderSystem& renderSystem,
            RenderContextDescriptor desc,
            const std::shared_ptr<Window>& window
        );

        void Present() override;

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        /* --- Extended functions --- */

        ID3D12Resource* GetCurrentRenderTarget();

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescHandle() const;

        void SetCommandList(ID3D12GraphicsCommandList* commandList);

    private:

        static const UINT maxNumBuffers = 3;

        void CreateWindowSizeDependentResources();

        //void SetupSwapChainInterval(const VsyncDescriptor& desc);

        void MoveToNextFrame();
        
        D3D12RenderSystem&                  renderSystem_;  // reference to its render system
        RenderContextDescriptor             desc_;

        ComPtr<IDXGISwapChain3>             swapChain_;
        UINT                                swapChainInterval_              = 0;

        ComPtr<ID3D12DescriptorHeap>        rtvDescHeap_;
        UINT                                rtvDescSize_                    = 0;

        ComPtr<ID3D12CommandAllocator>      commandAllocs_[maxNumBuffers];
        ComPtr<ID3D12Resource>              renderTargets_[maxNumBuffers];
        UINT64                              fenceValues_[maxNumBuffers]     = { 0 };

        ID3D12GraphicsCommandList*          commandList_                    = nullptr;

        UINT                                numFrames_                      = 0;
        UINT                                currentFrame_                   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
