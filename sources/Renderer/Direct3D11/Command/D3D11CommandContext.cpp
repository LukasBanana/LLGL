/*
 * D3D11CommandContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11CommandContext.h"
#include "../D3D11SwapChain.h"
#include "../D3D11Types.h"
#include "../D3D11ResourceFlags.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../ResourceUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/TypeInfo.h>
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/Assertion.h"
#include "../../TextureUtils.h"
#include <algorithm>

#include "../RenderState/D3D11StateManager.h"
#include "../RenderState/D3D11PipelineState.h"
#include "../RenderState/D3D11PipelineLayout.h"
#include "../RenderState/D3D11ConstantsCache.h"
#include "../RenderState/D3D11QueryHeap.h"
#include "../RenderState/D3D11ResourceType.h"
#include "../RenderState/D3D11ResourceHeap.h"
#include "../RenderState/D3D11RenderPass.h"

#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11BufferArray.h"
#include "../Buffer/D3D11BufferWithRV.h"

#include "../Texture/D3D11Texture.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11RenderTarget.h"
#include "../Texture/D3D11MipGenerator.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>


namespace LLGL
{


D3D11CommandContext::D3D11CommandContext(
    const ComPtr<ID3D11DeviceContext>&          context,
    const std::shared_ptr<D3D11StateManager>&   stateMngr)
:
    context_      { context                         },
    stateMngr_    { stateMngr                       },
    bindingTable_ { &(stateMngr->GetBindingTable()) }
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    context_->QueryInterface(IID_PPV_ARGS(&context1_));
    #endif
}

void D3D11CommandContext::ResetBindingStates()
{
    boundRenderTarget_      = nullptr;
    boundSwapChain_         = nullptr;
    boundPipelineLayout_    = nullptr;
    boundPipelineState_     = nullptr;
    boundConstantsCache_    = nullptr;
}

void D3D11CommandContext::BindSwapChainRenderTargets(D3D11SwapChain& swapChainD3D)
{
    SetRenderTargets(swapChainD3D.GetRenderTargetHandles());
    boundSwapChain_ = &swapChainD3D;
}

void D3D11CommandContext::BindOffscreenRenderTargets(D3D11RenderTarget& renderTargetD3D)
{
    SetRenderTargets(renderTargetD3D.GetRenderTargetHandles());
    boundRenderTarget_ = &renderTargetD3D;
}

void D3D11CommandContext::ResolveAndUnbindRenderTargets()
{
    /* Set RTV list and DSV in framebuffer view */
    SetRenderTargetsNull();

    if (boundRenderTarget_ != nullptr)
    {
        boundRenderTarget_->ResolveSubresources(context_.Get());
        boundRenderTarget_ = nullptr;
    }
    else if (boundSwapChain_ != nullptr)
    {
        boundSwapChain_->ResolveSubresources(context_.Get());
        boundSwapChain_ = nullptr;
    }
}

static UINT GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D11_CLEAR_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D11_CLEAR_STENCIL;

    return clearFlagsDSV;
}

void D3D11CommandContext::ClearFramebufferViewsSimple(long flags, const ClearValue& clearValue)
{
    /* Clear color buffers */
    if ((flags & ClearFlags::Color) != 0)
    {
        for_range(i, framebufferView_.numRenderTargetViews)
            context_->ClearRenderTargetView(framebufferView_.renderTargetViews[i], clearValue.color);
    }

    /* Clear depth-stencil buffer */
    if (framebufferView_.depthStencilView != nullptr)
    {
        if (auto clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            context_->ClearDepthStencilView(
                framebufferView_.depthStencilView,
                clearFlagsDSV,
                clearValue.depth,
                static_cast<UINT8>(clearValue.stencil & 0xFF)
            );
        }
    }
}

void D3D11CommandContext::ClearFramebufferViewsIndexed(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Clear color attachment */
            if (attachments->colorAttachment < framebufferView_.numRenderTargetViews)
            {
                context_->ClearRenderTargetView(
                    framebufferView_.renderTargetViews[attachments->colorAttachment],
                    attachments->clearValue.color
                );
            }
        }
        else if (framebufferView_.depthStencilView != nullptr)
        {
            /* Clear depth and stencil buffer simultaneously */
            if (auto clearFlagsDSV = GetClearFlagsDSV(attachments->flags))
            {
                context_->ClearDepthStencilView(
                    framebufferView_.depthStencilView,
                    clearFlagsDSV,
                    attachments->clearValue.depth,
                    static_cast<UINT8>(attachments->clearValue.stencil & 0xff)
                );
            }
        }
    }
}

void D3D11CommandContext::ClearFramebufferViewsOrdered(
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    const std::uint8_t* colorBuffers,
    UINT                depthStencilClearFlags)
{
    /* Clear color attachments */
    const std::uint32_t clearValueIndex = ClearColorBuffers(colorBuffers, numClearValues, clearValues);

    /* Clear depth-stencil attachment */
    if (framebufferView_.depthStencilView != nullptr && depthStencilClearFlags != 0)
    {
        /* Get clear values */
        FLOAT depth     = 1.0f;
        UINT8 stencil   = 0;

        if (clearValueIndex < numClearValues)
        {
            depth   = clearValues[clearValueIndex].depth;
            stencil = static_cast<UINT8>(clearValues[clearValueIndex].stencil & 0xff);
        }

        /* Clear depth-stencil view */
        context_->ClearDepthStencilView(framebufferView_.depthStencilView, depthStencilClearFlags, depth, stencil);
    }
}

/* ----- Input Assembly ------ */

void D3D11CommandContext::SetVertexBuffer(D3D11Buffer& bufferD3D)
{
    bindingTable_->SetVertexBuffer(
        0,
        bufferD3D.GetNative(),
        bufferD3D.GetStride(),
        0,
        bufferD3D.GetBindingLocator()
    );
}

void D3D11CommandContext::SetVertexBufferArray(D3D11BufferArray& bufferArrayD3D)
{
    bindingTable_->SetVertexBuffers(
        0,
        bufferArrayD3D.GetCount(),
        bufferArrayD3D.GetBuffers(),
        bufferArrayD3D.GetStrides(),
        bufferArrayD3D.GetOffsets(),
        bufferArrayD3D.GetBindingLocators()
    );
}

void D3D11CommandContext::SetIndexBuffer(D3D11Buffer& bufferD3D, DXGI_FORMAT format, UINT offset)
{
    bindingTable_->SetIndexBuffer(
        bufferD3D.GetNative(),
        format,
        offset,
        bufferD3D.GetBindingLocator()
    );
}

/* ----- Resources ----- */

HRESULT D3D11CommandContext::SetResourceHeap(D3D11ResourceHeap& resourceHeapD3D, std::uint32_t descriptorSet)
{
    if (boundPipelineState_ == nullptr)
        return E_POINTER;

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (context1_.Get() != nullptr)
    {
        if (boundPipelineState_->IsGraphicsPSO())
            resourceHeapD3D.BindForGraphicsPipeline1(context1_.Get(), *bindingTable_, descriptorSet);
        else
            resourceHeapD3D.BindForComputePipeline1(context1_.Get(), *bindingTable_, descriptorSet);
    }
    else
    #endif // /LLGL_D3D11_ENABLE_FEATURELEVEL
    {
        if (boundPipelineState_->IsGraphicsPSO())
            resourceHeapD3D.BindForGraphicsPipeline(context_.Get(), *bindingTable_, descriptorSet);
        else
            resourceHeapD3D.BindForComputePipeline(context_.Get(), *bindingTable_, descriptorSet);
    }

    return S_OK;
}

HRESULT D3D11CommandContext::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (boundPipelineLayout_ == nullptr)
        return E_POINTER;

    const auto& bindingList = boundPipelineLayout_->GetBindings();
    if (!(descriptor < bindingList.size()))
        return E_INVALIDARG;

    const D3D11PipelineResourceBinding& binding = bindingList[descriptor];

    switch (binding.type)
    {
        case D3DResourceType_CBV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11Buffer&, resource);
            ID3D11Buffer* cbv[] = { bufferD3D.GetNative() };
            stateMngr_->SetConstantBuffers(binding.slot, 1, cbv, binding.stageFlags);
        }
        break;

        case D3DResourceType_BufferSRV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, resource);
            ID3D11ShaderResourceView* srv[] = { bufferD3D.GetSRV() };
            D3D11BindingLocator* locator[] = { bufferD3D.GetBindingLocator() };
            bindingTable_->SetShaderResourceViews(binding.slot, 1, srv, locator, nullptr, binding.stageFlags);
        }
        break;

        case D3DResourceType_BufferUAV:
        {
            auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, resource);
            ID3D11UnorderedAccessView* uav[] = { bufferD3D.GetUAV() };
            const UINT initialCounts[] = { bufferD3D.GetInitialCount() };
            D3D11BindingLocator* locator[] = { bufferD3D.GetBindingLocator() };
            bindingTable_->SetUnorderedAccessViews(binding.slot, 1, uav, initialCounts, locator, nullptr, binding.stageFlags);
        }
        break;

        case D3DResourceType_TextureSRV:
        {
            auto& textureD3D = LLGL_CAST(D3D11Texture&, resource);
            ID3D11ShaderResourceView* srv[] = { textureD3D.GetSRV() };
            D3D11BindingLocator* locator[] = { textureD3D.GetBindingLocator() };
            bindingTable_->SetShaderResourceViews(binding.slot, 1, srv, locator, nullptr, binding.stageFlags);
        }
        break;

        case D3DResourceType_TextureUAV:
        {
            auto& textureD3D = LLGL_CAST(D3D11Texture&, resource);
            ID3D11UnorderedAccessView* uav[] = { textureD3D.GetUAV() };
            const UINT initialCounts[] = { 0 };
            D3D11BindingLocator* locator[] = { textureD3D.GetBindingLocator() };
            bindingTable_->SetUnorderedAccessViews(binding.slot, 1, uav, initialCounts, locator, nullptr, binding.stageFlags);
        }
        break;

        case D3DResourceType_Sampler:
        {
            /* Set sampler state object to all shader stages */
            auto& samplerD3D = LLGL_CAST(D3D11Sampler&, resource);
            ID3D11SamplerState* samplerStates[] = { samplerD3D.GetNative() };
            stateMngr_->SetSamplers(binding.slot, 1, samplerStates, binding.stageFlags);
        }
        break;
    }

    return S_OK;
}

/* ----- Pipeline States ----- */

void D3D11CommandContext::SetPipelineState(D3D11PipelineState* pipelineStateD3D)
{
    LLGL_ASSERT_PTR(pipelineStateD3D);
    if (boundPipelineState_ != pipelineStateD3D)
    {
        boundPipelineState_ = pipelineStateD3D;
        boundPipelineState_->Bind(*stateMngr_);
        boundPipelineLayout_ = boundPipelineState_->GetPipelineLayout();
        boundConstantsCache_ = boundPipelineState_->GetConstantsCache();
        if (boundConstantsCache_ != nullptr)
            boundConstantsCache_->Reset();
    }
}

void D3D11CommandContext::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->SetUniforms(first, data, dataSize);
}

/* ----- Drawing ----- */

void D3D11CommandContext::Draw(UINT vertexCount, UINT startVertexLocation)
{
    FlushGraphicsResourceBindingCache();
    context_->Draw(vertexCount, startVertexLocation);
}

void D3D11CommandContext::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
    FlushGraphicsResourceBindingCache();
    context_->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void D3D11CommandContext::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
    FlushGraphicsResourceBindingCache();
    context_->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void D3D11CommandContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
    FlushGraphicsResourceBindingCache();
    context_->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void D3D11CommandContext::DrawInstancedIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs)
{
    FlushGraphicsResourceBindingCache();
    context_->DrawInstancedIndirect(bufferForArgs, alignedByteOffsetForArgs);
}

void D3D11CommandContext::DrawInstancedIndirectN(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs, UINT numCommands, UINT stride)
{
    FlushGraphicsResourceBindingCache();
    while (numCommands-- > 0)
    {
        context_->DrawInstancedIndirect(bufferForArgs, alignedByteOffsetForArgs);
        alignedByteOffsetForArgs += stride;
    }
}

void D3D11CommandContext::DrawIndexedInstancedIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs)
{
    FlushGraphicsResourceBindingCache();
    context_->DrawIndexedInstancedIndirect(bufferForArgs, alignedByteOffsetForArgs);
}

void D3D11CommandContext::DrawIndexedInstancedIndirectN(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs, UINT numCommands, UINT stride)
{
    FlushGraphicsResourceBindingCache();
    while (numCommands-- > 0)
    {
        context_->DrawIndexedInstancedIndirect(bufferForArgs, alignedByteOffsetForArgs);
        alignedByteOffsetForArgs += stride;
    }
}

void D3D11CommandContext::DrawAuto()
{
    FlushGraphicsResourceBindingCache();
    context_->DrawAuto();
}

/* ----- Compute ----- */

void D3D11CommandContext::Dispatch(UINT numWorkGroupsX, UINT numWorkGroupsY, UINT numWorkGroupsZ)
{
    FlushComputeResourceBindingCache();
    context_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D11CommandContext::DispatchIndirect(ID3D11Buffer* bufferForArgs, UINT alignedByteOffsetForArgs)
{
    FlushComputeResourceBindingCache();
    context_->DispatchIndirect(bufferForArgs, alignedByteOffsetForArgs);
}


/*
 * ======= Private: =======
 */

void D3D11CommandContext::SetRenderTargets(const D3D11RenderTargetHandles& renderTargetHandles)
{
    /* Set output-merger render target views */
    bindingTable_->SetRenderTargets(
        renderTargetHandles.GetNumRenderTargetViews(),
        renderTargetHandles.GetRenderTargetViews(),
        renderTargetHandles.GetDepthStencilView(),
        renderTargetHandles.GetRenderTargetLocators(),
        renderTargetHandles.GetRenderTargetSubresourceRanges(),
        renderTargetHandles.GetDepthStencilLocator()
    );

    /* Store new render-target configuration */
    framebufferView_.numRenderTargetViews   = renderTargetHandles.GetNumRenderTargetViews();
    framebufferView_.renderTargetViews      = renderTargetHandles.GetRenderTargetViews();
    framebufferView_.depthStencilView       = renderTargetHandles.GetDepthStencilView();
}

void D3D11CommandContext::SetRenderTargetsNull()
{
    bindingTable_->SetRenderTargets(0, nullptr, nullptr, nullptr, nullptr, nullptr);
    framebufferView_.numRenderTargetViews   = 0;
    framebufferView_.renderTargetViews      = nullptr;
    framebufferView_.depthStencilView       = nullptr;
}

std::uint32_t D3D11CommandContext::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    numClearValues = std::min(numClearValues, framebufferView_.numRenderTargetViews);

    std::uint32_t clearValueIndex = 0;

    /* Use specified clear values */
    for_range(i, numClearValues)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        context_->ClearRenderTargetView(
            framebufferView_.renderTargetViews[colorBuffers[i]],
            clearValues[clearValueIndex].color
        );
        ++clearValueIndex;
    }

    /* Use default clear values */
    const float defaultClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for_subrange(i, numClearValues, framebufferView_.numRenderTargetViews)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] == 0xFF)
            return clearValueIndex;

        context_->ClearRenderTargetView(
            framebufferView_.renderTargetViews[colorBuffers[i]],
            defaultClearColor
        );
    }

    return clearValueIndex;
}

void D3D11CommandContext::FlushGraphicsResourceBindingCache()
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->Flush(*stateMngr_);
    bindingTable_->FlushOutputMergerUAVs();
}

void D3D11CommandContext::FlushComputeResourceBindingCache()
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->Flush(*stateMngr_);
}


} // /namespace LLGL



// ================================================================================
