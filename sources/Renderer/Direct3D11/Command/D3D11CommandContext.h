/*
 * D3D11CommandContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_CONTEXT_H
#define LLGL_D3D11_COMMAND_CONTEXT_H


#include "../Direct3D11.h"
#include "../RenderState/D3D11StateManager.h"
#include <LLGL/CommandBufferFlags.h>


namespace LLGL
{


class D3D11Buffer;
class D3D11BufferArray;
class D3D11Texture;
class D3D11Sampler;
class D3D11ResourceHeap;
class D3D11BindingTable;
class D3D11PipelineLayout;
class D3D11PipelineState;
class D3D11ConstantsCache;
class D3D11SwapChain;
class D3D11RenderTarget;
class D3D11RenderTargetHandles;

class D3D11CommandContext
{

    public:

        D3D11CommandContext(
            const ComPtr<ID3D11DeviceContext>&          context,
            const std::shared_ptr<D3D11StateManager>&   stateMngr
        );

        void ResetBindingStates();

        void BindSwapChainRenderTargets(D3D11SwapChain& swapChainD3D);
        void BindOffscreenRenderTargets(D3D11RenderTarget& renderTargetD3D);

        void ResolveAndUnbindRenderTargets();

        void ClearFramebufferViewsSimple(long flags, const ClearValue& clearValue);
        void ClearFramebufferViewsIndexed(std::uint32_t numAttachments, const AttachmentClear* attachments);
        void ClearFramebufferViewsOrdered(
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            const std::uint8_t* colorBuffers,
            UINT                depthStencilClearFlags
        );

        void SetVertexBuffer(D3D11Buffer& bufferD3D);
        void SetVertexBufferArray(D3D11BufferArray& bufferArrayD3D);

        void SetIndexBuffer(D3D11Buffer& bufferD3D, DXGI_FORMAT format, UINT offset);

        HRESULT SetResourceHeap(D3D11ResourceHeap& resourceHeapD3D, std::uint32_t descriptorSet);
        HRESULT SetResource(std::uint32_t descriptor, Resource& resource);

        void SetPipelineState(D3D11PipelineState* pipelineStateD3D);

        void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize);

        void Draw(UINT vertexCount, UINT startVertexLocation);
        void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation);
        void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation);
        void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation);

        void DrawInstancedIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs);
        void DrawInstancedIndirectN(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs, UINT numCommands, UINT stride);

        void DrawIndexedInstancedIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs);
        void DrawIndexedInstancedIndirectN(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs, UINT numCommands, UINT stride);

        void DrawAuto();

        void Dispatch(UINT numWorkGroupsX, UINT numWorkGroupsY, UINT numWorkGroupsZ);
        void DispatchIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs);

    public:

        // Returns the native D3D11 device context.
        inline ID3D11DeviceContext* GetNative() const
        {
            return context_.Get();
        }

        // Returns a pointer to the state manager for this command context.
        inline D3D11StateManager* GetStateManagerPtr() const
        {
            return stateMngr_.get();
        }

        // Returns the state manager for this command context.
        inline D3D11StateManager& GetStateManager() const
        {
            return *stateMngr_;
        }

        // Returns the binding table.
        inline D3D11BindingTable& GetBindingTable() const
        {
            return stateMngr_->GetBindingTable();
        }

        // Returns the currently bound swap-chain.
        inline D3D11SwapChain* GetBoundSwapChain() const
        {
            return boundSwapChain_;
        }

        // Returns the currently bound (offscreen) render target.
        inline D3D11RenderTarget* GetBoundRenderTarget() const
        {
            return boundRenderTarget_;
        }

    private:

        // Wrapper structure for the framebuffer resource views.
        struct D3D11FramebufferView
        {
            UINT                            numRenderTargetViews    = 0;
            ID3D11RenderTargetView* const * renderTargetViews       = nullptr;
            ID3D11DepthStencilView*         depthStencilView        = nullptr;
        };

    private:

        // Calls OMSetRenderTargets and stores the references to these resource views.
        void SetRenderTargets(const D3D11RenderTargetHandles& renderTargetHandles);
        void SetRenderTargetsNull();

        std::uint32_t ClearColorBuffers(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

        void FlushGraphicsResourceBindingCache();
        void FlushComputeResourceBindingCache();

    private:

        // Primary D3D11 context for most commands
        ComPtr<ID3D11DeviceContext>         context_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        // Extended D3D11 context to bind constant-buffer ranges (Direct3D 11.1)
        ComPtr<ID3D11DeviceContext1>        context1_;
        #endif

        std::shared_ptr<D3D11StateManager>  stateMngr_;
        D3D11BindingTable*                  bindingTable_           = nullptr;

        D3D11FramebufferView                framebufferView_;
        D3D11RenderTarget*                  boundRenderTarget_      = nullptr;
        D3D11SwapChain*                     boundSwapChain_         = nullptr;
        const D3D11PipelineLayout*          boundPipelineLayout_    = nullptr;
        D3D11PipelineState*                 boundPipelineState_     = nullptr;
        D3D11ConstantsCache*                boundConstantsCache_    = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
