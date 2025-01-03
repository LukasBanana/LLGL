/*
 * D3D12CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMMAND_BUFFER_H
#define LLGL_D3D12_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/Constants.h>
#include <cstddef>
#include "D3D12CommandContext.h"
#include "../D3D12Resource.h"
#include "../../DXCommon/ComPtr.h"
#include "../../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12CommandQueue;
class D3D12SwapChain;
class D3D12RenderTarget;
class D3D12RenderPass;
class D3D12Buffer;
class D3D12Texture;
class D3D12Sampler;
class D3D12PipelineLayout;
class D3D12PipelineState;
class D3D12SignatureFactory;

class D3D12CommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetDebugName(const char* name) override;

    public:

        // Executes all pending resource transitions and then the bundle.
        void ExecuteBundle(D3D12CommandContext& context);

        // Returns the command context of this command buffer.
        inline D3D12CommandContext& GetCommandContext()
        {
            return commandContext_;
        }

        // Returns the native ID3D12GraphicsCommandList object.
        inline ID3D12GraphicsCommandList* GetNative() const
        {
            return commandContext_.GetCommandList();
        }

        // Returns true if this is an immediate command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return (isImmediateSubmit_ != 0);
        }

        // Returns ture if this is a bundle command buffer.
        inline bool IsBundleCmdBuffer() const
        {
            return (isBundle_ != 0);
        }

    private:

        void CreateCommandContext(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetAndConvertViewports(std::uint32_t numViewports, const Viewport* viewports);
        void SetAndConvertScissorRects(std::uint32_t numScissors, const Scissor* scissors);
        void SetDefaultScissorRects(UINT numScissorRects);

        void BindRenderTarget(D3D12RenderTarget& renderTargetD3D);
        void BindSwapChain(D3D12SwapChain& swapChainD3D, std::uint32_t swapBufferIndex = LLGL_CURRENT_SWAP_INDEX);

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

        void CreateSOIndirectDrawArgBuffer(ID3D12Device* device);

        // Submits a resource transition. This is either scheduled with this command context or scheduled for later if this is a bundle.
        void SubmitTransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState);
        void SubmitTransitionResource(Resource& resource, D3D12_RESOURCE_STATES newState);

    private:

        D3D12CommandContext                     commandContext_;
        D3D12CommandQueue*                      commandQueue_                               = nullptr;
        const D3D12SignatureFactory*            cmdSignatureFactory_                        = nullptr;

        UINT                                    isImmediateSubmit_  : 1;
        UINT                                    isBundle_           : 1;

        D3D12_CPU_DESCRIPTOR_HANDLE             rtvDescHandle_                              = {};
        UINT                                    rtvDescSize_                                = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE             dsvDescHandle_                              = {};
        UINT                                    dsvDescSize_                                = 0;

        bool                                    scissorEnabled_                             = false;
        UINT                                    numDefaultScissorRects_                     = 0;
        UINT                                    numColorBuffers_                            = 0;
        UINT                                    currentColorBuffer_                         = 0;
        UINT                                    numSOBuffers_                               = 0;

        D3D12SwapChain*                         boundSwapChain_                             = nullptr;
        D3D12RenderTarget*                      boundRenderTarget_                          = nullptr;
        const D3D12PipelineLayout*              boundPipelineLayout_                        = nullptr;
        D3D12PipelineState*                     boundPipelineState_                         = nullptr;
        D3D12Buffer*                            boundSOBuffers_[LLGL_MAX_NUM_SO_BUFFERS]    = {};

        D3D12Buffer*                            soBufferIASlot0_                            = 0;
        D3D12Resource                           soDrawArgBuffer_;
        D3D12_RESOURCE_STATES                   soBufferStates_[LLGL_MAX_NUM_SO_BUFFERS]    = {};

        std::vector<D3D12ResourceTransition>    bundleResourceTransitions_;

};


} // /namespace LLGL


#endif



// ================================================================================
