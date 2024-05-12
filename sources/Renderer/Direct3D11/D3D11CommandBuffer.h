/*
 * D3D11CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_BUFFER_H
#define LLGL_D3D11_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"
#include "Direct3D11.h"
#include <dxgi.h>
#include <vector>
#include <cstddef>


namespace LLGL
{


class D3D11Buffer;
class D3D11StateManager;
class D3D11BindingTable;
class D3D11RenderTarget;
class D3D11SwapChain;
class D3D11RenderPass;
class D3D11PipelineState;
class D3D11PipelineLayout;
class D3D11ConstantsCache;
class D3D11RenderTargetHandles;

class D3D11CommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D11CommandBuffer(
            ID3D11Device*                               device,
            const ComPtr<ID3D11DeviceContext>&          context,
            const std::shared_ptr<D3D11StateManager>&   stateMngr,
            const CommandBufferDescriptor&              desc
        );

    public:

        /* ----- Internal ----- */

        // Calls OMSetRenderTargets and stores the references to these resource views.
        void SetRenderTargets(const D3D11RenderTargetHandles& renderTargetHandles);
        void SetRenderTargetsNull();

        // Calls ClearState() on a deferred device context and discard a partially built command list.
        void ClearStateAndResetDeferredCommandList();

        // Returns the native command list for deferred contexts or null if there is none.
        inline ID3D11CommandList* GetDeferredCommandList() const
        {
            return commandList_.Get();
        }

        // Returns true if this is a secondary command buffer that can be executed within a primary command buffer.
        inline bool IsSecondaryCmdBuffer() const
        {
            return isSecondaryCmdBuffer_;
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

        void ResolveAndUnbindRenderTarget();

        void ClearAttachmentsWithRenderPass(
            const D3D11RenderPass&  renderPassD3D,
            std::uint32_t           numClearValues,
            const ClearValue*       clearValues
        );

        std::uint32_t ClearColorBuffers(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

        void ClearWithIntermediateUAV(ID3D11Buffer* buffer, UINT offset, UINT size, const UINT (&valuesVec4)[4]);

        // Creates a copy of this buffer as ByteAddressBuffer; 'size' must be a multiple of 4.
        void CreateByteAddressBufferR32Typeless(
            ID3D11Device*               device,
            ID3D11DeviceContext*        context,
            ID3D11Buffer**              bufferOutput,
            ID3D11ShaderResourceView**  srvOutput,
            ID3D11UnorderedAccessView** uavOutput,
            UINT                        size,
            D3D11_USAGE                 usage           = D3D11_USAGE_DEFAULT
        );

        void FlushGraphicsResourceBindingCache();
        void FlushComputeResourceBindingCache();

        void ResetBindingStates();

    private:

        // Device object to create on-demand objects like temporary SRVs and UAVs
        ID3D11Device*                       device_                 = nullptr;

        // Primary D3D11 context for most commands
        ComPtr<ID3D11DeviceContext>         context_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        // Extended D3D11 context to bind constant-buffer ranges (Direct3D 11.1)
        ComPtr<ID3D11DeviceContext1>        context1_;
        #endif

        ComPtr<ID3D11CommandList>           commandList_;

        bool                                hasDeferredContext_     = false;
        bool                                isSecondaryCmdBuffer_   = false;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3DUserDefinedAnnotation>   annotation_;
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
