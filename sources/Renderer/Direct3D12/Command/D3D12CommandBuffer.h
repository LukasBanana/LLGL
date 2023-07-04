/*
 * D3D12CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMMAND_BUFFER_H
#define LLGL_D3D12_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <cstddef>
#include "D3D12CommandContext.h"
#include "../Buffer/D3D12StagingBufferPool.h"
#include "../../DXCommon/ComPtr.h"
#include "../../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12SwapChain;
class D3D12RenderTarget;
class D3D12RenderPass;
class D3D12Buffer;
class D3D12Texture;
class D3D12Sampler;
class D3D12PipelineLayout;
class D3D12PipelineState;
class D3D12SignatureFactory;
struct D3D12Resource;

class D3D12CommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetName(const char* name) override;

    public:

        // Executes this command buffer.
        void Execute();

        // Returns the native ID3D12GraphicsCommandList object.
        inline ID3D12GraphicsCommandList* GetNative() const
        {
            return commandList_;
        }

        // Returns true if this is an immediate command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return immediateSubmit_;
        }

    private:

        void CreateCommandContext(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetScissorRectsToDefault(UINT numScissorRects);

        void BindRenderTarget(D3D12RenderTarget& renderTargetD3D);
        void BindSwapChain(D3D12SwapChain& swapChainD3D, std::uint32_t swapBufferIndex = Constants::currentSwapIndex);

        std::uint32_t ClearAttachmentsWithRenderPass(
            const D3D12RenderPass&  renderPassD3D,
            std::uint32_t           numClearValues,
            const ClearValue*       clearValues,
            UINT                    numRects        = 0,
            const D3D12_RECT*       rects           = nullptr
        );

        std::uint32_t ClearRenderTargetViews(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t       clearValueIndex,
            UINT                numRects,
            const D3D12_RECT*   rects
        );

        void ClearDepthStencilView(
            D3D12_CLEAR_FLAGS   clearFlags,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t       clearValueIndex,
            UINT                numRects,
            const D3D12_RECT*   rects
        );

        void ResetBindingStates();

    private:

        D3D12CommandContext             commandContext_;
        ID3D12GraphicsCommandList*      commandList_            = nullptr;
        const D3D12SignatureFactory*    cmdSignatureFactory_    = nullptr;

        bool                            immediateSubmit_        = false;

        D3D12_CPU_DESCRIPTOR_HANDLE     rtvDescHandle_          = {};
        UINT                            rtvDescSize_            = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE     dsvDescHandle_          = {};
        UINT                            dsvDescSize_            = 0;

        bool                            scissorEnabled_         = false;
        UINT                            numBoundScissorRects_   = 0;
        UINT                            numColorBuffers_        = 0;
        UINT                            currentColorBuffer_     = 0;

        D3D12SwapChain*                 boundSwapChain_         = nullptr;
        D3D12RenderTarget*              boundRenderTarget_      = nullptr;
        const D3D12PipelineLayout*      boundPipelineLayout_    = nullptr;
        D3D12PipelineState*             boundPipelineState_     = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
