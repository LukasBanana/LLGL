/*
 * D3D12CommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12CommandBuffer.h"
#include "D3D12SignatureFactory.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12SwapChain.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../../TextureUtils.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/CompilerExtensions.h"

#include "../Buffer/D3D12Buffer.h"
#include "../Buffer/D3D12BufferArray.h"
#include "../Buffer/D3D12BufferConstantsPool.h"

#include "../Texture/D3D12Texture.h"
#include "../Texture/D3D12RenderTarget.h"
#include "../Texture/D3D12MipGenerator.h"

#include "../RenderState/D3D12ResourceHeap.h"
#include "../RenderState/D3D12RenderPass.h"
#include "../RenderState/D3D12QueryHeap.h"
#include "../RenderState/D3D12GraphicsPSO.h"
#include "../RenderState/D3D12ComputePSO.h"

#include "../Shader/D3D12BuiltinShaderFactory.h"

#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Backend/Direct3D12/NativeHandle.h>

#include "../D3DX12/d3dx12.h"

// Only include PIX if we build with MSVC as MSYS2 does not provide this header
#ifdef _MSC_VER
#include <pix.h>
#endif

#include <algorithm>


namespace LLGL
{


D3D12CommandBuffer::D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc) :
    cmdSignatureFactory_ { &(renderSystem.GetSignatureFactory())                         },
    isImmediateSubmit_   { ((desc.flags & CommandBufferFlags::ImmediateSubmit) != 0)     },
    isBundle_            { ((desc.flags & CommandBufferFlags::Secondary) != 0)           },
    commandQueue_        { LLGL_CAST(D3D12CommandQueue*, renderSystem.GetCommandQueue()) }
{
    CreateCommandContext(renderSystem, desc);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
    CreateSOIndirectDrawArgBuffer(renderSystem.GetDXDevice());
}

void D3D12CommandBuffer::SetDebugName(const char* name)
{
    D3D12SetObjectName(GetNative(), name);
}

/* ----- Encoding ----- */

void D3D12CommandBuffer::Begin()
{
    /* Reset bundle resource transitions before startinga new recording */
    bundleResourceTransitions_.clear();

    /* Reset command list using the next command allocator */
    commandContext_.Reset(*commandQueue_);
}

void D3D12CommandBuffer::End()
{
    /* Close command context and reset intermediate states */
    commandContext_.Close();

    /* Clear references to bound pipeline objects */
    ResetBindingStates();

    /* Execute command list right after encoding for immediate command buffers */
    if (IsImmediateCmdBuffer())
        commandQueue_->SubmitCommandContext(commandContext_);
}

void D3D12CommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D12CommandBuffer&, secondaryCommandBuffer);
    cmdBufferD3D.ExecuteBundle(commandContext_);
}

/* ----- Blitting ----- */

void D3D12CommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    commandContext_.UpdateSubresource(dstBufferD3D.GetResource(), dstOffset, data, dataSize);
}

void D3D12CommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    auto& srcBufferD3D = LLGL_CAST(D3D12Buffer&, srcBuffer);

    const D3D12_RESOURCE_STATES oldDstBufferState = dstBufferD3D.GetResource().currentState;
    const D3D12_RESOURCE_STATES oldSrcBufferState = srcBufferD3D.GetResource().currentState;

    commandContext_.TransitionResource(dstBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    {
        commandContext_.FlushResourceBarriers();
        GetNative()->CopyBufferRegion(dstBufferD3D.GetNative(), dstOffset, srcBufferD3D.GetNative(), srcOffset, size);
    }
    commandContext_.TransitionResource(dstBufferD3D.GetResource(), oldDstBufferState);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), oldSrcBufferState);
}

void D3D12CommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    auto& srcTextureD3D = LLGL_CAST(D3D12Texture&, srcTexture);

    const TextureLocation   srcLocation { srcRegion.offset, srcRegion.subresource.baseArrayLayer, srcRegion.subresource.baseMipLevel };
    const Extent3D          srcExtent   { CalcTextureExtent(srcTexture.GetType(), srcRegion.extent, srcRegion.subresource.numArrayLayers) };

    /* Determine actual buffer row stride and required row stride */
    UINT alignedRowStride = rowStride;
    if (rowStride == 0)
    {
        rowStride = static_cast<std::uint32_t>(GetMemoryFootprint(srcTextureD3D.GetFormat(), srcExtent.width));
        alignedRowStride = GetAlignedSize<UINT>(rowStride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    }

    const D3D12_TEXTURE_COPY_LOCATION   srcLocationD3D  = srcTextureD3D.CalcCopyLocation(srcLocation);
    const D3D12_BOX                     srcBox          = srcTextureD3D.CalcRegion(srcRegion.offset, srcExtent);

    const D3D12_RESOURCE_STATES oldDstBufferState = dstBufferD3D.GetResource().currentState;
    const D3D12_RESOURCE_STATES oldSrcTextureState = srcTextureD3D.GetResource().currentState;

    commandContext_.TransitionResource(dstBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    {
        if (dstOffset % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT != 0 || (alignedRowStride != rowStride && (srcExtent.height > 1 || srcExtent.depth > 1)))
        {
            /* Copy texture region into intermediate buffer with correct row stride */
            const UINT64 alignedBufferSize = GetAlignedImageSize<UINT64>(srcExtent, rowStride, alignedRowStride);
            ID3D12Resource* alignedBuffer = commandContext_.AllocIntermediateBuffer(alignedBufferSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

            /* Copy entire region from source texture into intermediate buffer */
            const D3D12_TEXTURE_COPY_LOCATION dstLocationD3D = srcTextureD3D.CalcCopyLocation(alignedBuffer, 0, srcExtent, alignedRowStride);
            commandContext_.FlushResourceBarriers();
            GetNative()->CopyTextureRegion(
                &dstLocationD3D,    // pDst
                0,                  // DstX
                0,                  // DstY
                0,                  // DstZ
                &srcLocationD3D,    // pSrc
                &srcBox             // pSrcBox
            );

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

            /* Copy each row individually from intermediate buffer into destination buffer due to unalgined row pitch */
            commandContext_.FlushResourceBarriers();
            UINT64 alignedOffset = 0;
            for_range(z, srcExtent.depth)
            {
                for_range(y, srcExtent.height)
                {
                    GetNative()->CopyBufferRegion(dstBufferD3D.GetNative(), dstOffset, alignedBuffer, alignedOffset, rowStride);
                    alignedOffset += alignedRowStride;
                    dstOffset += rowStride;
                }
            }

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);
        }
        else
        {
            /* Copy entire region from source texture into destination buffer */
            const D3D12_TEXTURE_COPY_LOCATION dstLocationD3D = srcTextureD3D.CalcCopyLocation(dstBufferD3D.GetNative(), dstOffset, srcExtent, alignedRowStride);
            commandContext_.FlushResourceBarriers();
            GetNative()->CopyTextureRegion(
                &dstLocationD3D,    // pDst
                0,                  // DstX
                0,                  // DstY
                0,                  // DstZ
                &srcLocationD3D,    // pSrc
                &srcBox             // pSrcBox
            );
        }
    }
    commandContext_.TransitionResource(dstBufferD3D.GetResource(), oldDstBufferState);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), oldSrcTextureState);
}

void D3D12CommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);

    /* Copy value to 4D vector to be used with native D3D12 clear functions */
    UINT valuesVec4[4] = { value, value, value, value };

    /* Clamp range to buffer size if whole buffer is meant to be filled */
    if (fillSize == LLGL_WHOLE_SIZE)
    {
        dstOffset   = 0;
        fillSize    = dstBufferD3D.GetBufferSize();
    }

    /* Clear buffer subresource with R32UInt format */
    dstBufferD3D.ClearSubresourceUInt(commandContext_, DXGI_FORMAT_R32_UINT, sizeof(UINT), dstOffset, fillSize, valuesVec4);
}

void D3D12CommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureD3D = LLGL_CAST(D3D12Texture&, dstTexture);
    auto& srcTextureD3D = LLGL_CAST(D3D12Texture&, srcTexture);

    const D3D12_TEXTURE_COPY_LOCATION dstLocationD3D = dstTextureD3D.CalcCopyLocation(dstLocation);
    const D3D12_TEXTURE_COPY_LOCATION srcLocationD3D = srcTextureD3D.CalcCopyLocation(srcLocation);

    const D3D12_BOX srcBox = srcTextureD3D.CalcRegion(srcLocation.offset, extent);

    const D3D12_RESOURCE_STATES oldDstTextureState = dstTextureD3D.GetResource().currentState;
    const D3D12_RESOURCE_STATES oldSrcTextureState = srcTextureD3D.GetResource().currentState;

    commandContext_.TransitionResource(dstTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    {
        commandContext_.FlushResourceBarriers();
        GetNative()->CopyTextureRegion(
            &dstLocationD3D,                            // pDst
            static_cast<UINT>(dstLocation.offset.x),    // DstX
            static_cast<UINT>(dstLocation.offset.y),    // DstY
            static_cast<UINT>(dstLocation.offset.z),    // DstZ
            &srcLocationD3D,                            // pSrc
            &srcBox                                     // pSrcBox
        );
    }
    commandContext_.TransitionResource(dstTextureD3D.GetResource(), oldDstTextureState);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), oldSrcTextureState);
}

void D3D12CommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureD3D = LLGL_CAST(D3D12Texture&, dstTexture);
    auto& srcBufferD3D = LLGL_CAST(D3D12Buffer&, srcBuffer);

    const TextureLocation   dstLocation { dstRegion.offset, dstRegion.subresource.baseArrayLayer, dstRegion.subresource.baseMipLevel };
    const Extent3D          dstExtent   { CalcTextureExtent(dstTexture.GetType(), dstRegion.extent, dstRegion.subresource.numArrayLayers) };

    /* Determine actual buffer row stride and required row stride */
    UINT alignedRowStride = rowStride;
    if (rowStride == 0)
    {
        rowStride = static_cast<std::uint32_t>(GetMemoryFootprint(dstTextureD3D.GetFormat(), dstExtent.width));
        alignedRowStride = GetAlignedSize<UINT>(rowStride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    }

    const D3D12_TEXTURE_COPY_LOCATION   dstLocationD3D  = dstTextureD3D.CalcCopyLocation(dstLocation);
    const D3D12_BOX                     srcBox          = dstTextureD3D.CalcRegion(Offset3D{}, dstExtent);

    const D3D12_RESOURCE_STATES oldDstTextureState = dstTextureD3D.GetResource().currentState;
    const D3D12_RESOURCE_STATES oldSrcBufferState = srcBufferD3D.GetResource().currentState;

    commandContext_.TransitionResource(dstTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    {
        if (srcOffset % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT != 0 || (alignedRowStride != rowStride && (dstExtent.height > 1 || dstExtent.depth > 1)))
        {
            /* Copy texture region into intermediate buffer with correct row stride */
            const UINT64 alignedBufferSize = GetAlignedImageSize<UINT64>(dstExtent, rowStride, alignedRowStride);
            ID3D12Resource* alignedBuffer = commandContext_.AllocIntermediateBuffer(alignedBufferSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

            /* Copy each row individually from intermediate buffer into destination texture due to unalgined row pitch */
            commandContext_.FlushResourceBarriers();
            UINT64 alignedOffset = 0;
            for_range(z, dstExtent.depth)
            {
                for_range(y, dstExtent.height)
                {
                    GetNative()->CopyBufferRegion(alignedBuffer, alignedOffset, srcBufferD3D.GetNative(), srcOffset, rowStride);
                    alignedOffset += alignedRowStride;
                    srcOffset += rowStride;
                }
            }

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

            /* Copy entire region from intermediate buffer into destination texture */
            const D3D12_TEXTURE_COPY_LOCATION srcLocationD3D = dstTextureD3D.CalcCopyLocation(alignedBuffer, 0, dstExtent, alignedRowStride);
            commandContext_.FlushResourceBarriers();
            GetNative()->CopyTextureRegion(
                &dstLocationD3D,                        // pDst
                static_cast<UINT>(dstRegion.offset.x),  // DstX
                static_cast<UINT>(dstRegion.offset.y),  // DstY
                static_cast<UINT>(dstRegion.offset.z),  // DstZ
                &srcLocationD3D,                        // pSrc
                &srcBox                                 // pSrcBox
            );

            commandContext_.TransitionBarrier(alignedBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);
        }
        else
        {
            /* Copy entire region from source buffer into destination texture */
            const D3D12_TEXTURE_COPY_LOCATION srcLocationD3D = dstTextureD3D.CalcCopyLocation(srcBufferD3D.GetNative(), srcOffset, dstExtent, alignedRowStride);
            commandContext_.FlushResourceBarriers();
            GetNative()->CopyTextureRegion(
                &dstLocationD3D,                        // pDst
                static_cast<UINT>(dstRegion.offset.x),  // DstX
                static_cast<UINT>(dstRegion.offset.y),  // DstY
                static_cast<UINT>(dstRegion.offset.z),  // DstZ
                &srcLocationD3D,                        // pSrc
                &srcBox                                 // pSrcBox
            );
        }
    }
    commandContext_.TransitionResource(dstTextureD3D.GetResource(), oldDstTextureState);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), oldSrcBufferState);
}

void D3D12CommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    if (dstRegion.extent.depth != 1 ||
        dstRegion.offset.x < 0      ||
        dstRegion.offset.y < 0      ||
        dstRegion.offset.z < 0)
    {
        return /*E_INVALIDARG*/;
    }

    auto& dstTextureD3D = LLGL_CAST(D3D12Texture&, dstTexture);

    D3D12Resource&  dstResource     = dstTextureD3D.GetResource();
    UINT            dstSubresource  = dstTextureD3D.CalcSubresource(dstRegion.subresource.baseMipLevel, dstRegion.subresource.baseArrayLayer);
    UINT            dstX            = static_cast<UINT>(dstRegion.offset.x);
    UINT            dstY            = static_cast<UINT>(dstRegion.offset.y);
    UINT            dstZ            = static_cast<UINT>(dstRegion.offset.z);

    const D3D12_BOX srcBox
    {
        static_cast<UINT>(srcOffset.x),
        static_cast<UINT>(srcOffset.y),
        0u,
        static_cast<UINT>(srcOffset.x) + dstRegion.extent.width,
        static_cast<UINT>(srcOffset.y) + dstRegion.extent.height,
        1u,
    };

    if (boundSwapChain_)
        boundSwapChain_->CopySubresourceRegion(commandContext_, dstResource, dstSubresource, dstX, dstY, dstZ, currentColorBuffer_, srcBox, dstTextureD3D.GetDXFormat());
    #if 0 //TODO
    else if (boundRenderTarget_)
        boundRenderTarget_->CopySubresourceRegion();
    #endif
}

void D3D12CommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);
    D3D12MipGenerator::Get().GenerateMips(commandContext_, textureD3D, textureD3D.GetWholeSubresource());
}

void D3D12CommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);
    D3D12MipGenerator::Get().GenerateMips(commandContext_, textureD3D, subresource);
}

/* ----- Viewport and Scissor ----- */

// Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted
static constexpr bool IsCompatibleToD3DViewport()
{
    return
    (
        sizeof(D3D12_VIEWPORT)             == sizeof(Viewport)             &&
        offsetof(D3D12_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
        offsetof(D3D12_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
        offsetof(D3D12_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
        offsetof(D3D12_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
        offsetof(D3D12_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
        offsetof(D3D12_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth)
    );
}

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
    SetAndConvertViewports(1, &viewport);
}

void D3D12CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
    SetAndConvertViewports(numViewports, viewports);
}

void D3D12CommandBuffer::SetScissor(const Scissor& scissor)
{
    SetAndConvertScissorRects(1, &scissor);
}

void D3D12CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    numScissors = std::min(numScissors, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
    SetAndConvertScissorRects(numScissors, scissors);
}

/* ----- Clear ----- */

static D3D12_CLEAR_FLAGS GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_STENCIL;

    return static_cast<D3D12_CLEAR_FLAGS>(clearFlagsDSV);
}

void D3D12CommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if (rtvDescHandle_.ptr != 0)
    {
        /* Clear color buffers */
        if ((flags & ClearFlags::Color) != 0)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = rtvDescHandle_;
            for_range(i, numColorBuffers_)
            {
                GetNative()->ClearRenderTargetView(rtvDescHandle, clearValue.color, 0, nullptr);
                rtvDescHandle.ptr += rtvDescSize_;
            }
        }
    }

    if (dsvDescHandle_.ptr != 0)
    {
        /* Clear depth-stencil buffer */
        if (D3D12_CLEAR_FLAGS clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            GetNative()->ClearDepthStencilView(
                dsvDescHandle_,
                clearFlagsDSV,
                clearValue.depth,
                static_cast<UINT8>(clearValue.stencil & 0xFF),
                0,
                nullptr
            );
        }
    }
}

void D3D12CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for_range(i, numAttachments)
    {
        const AttachmentClear& clearOp = attachments[i];

        if (rtvDescHandle_.ptr != 0)
        {
            /* Clear color buffers */
            if ((clearOp.flags & ClearFlags::Color) != 0)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = rtvDescHandle_;
                rtvDescHandle.ptr += (rtvDescSize_ * clearOp.colorAttachment);
                GetNative()->ClearRenderTargetView(rtvDescHandle, clearOp.clearValue.color, 0, nullptr);
            }
        }

        if (dsvDescHandle_.ptr != 0)
        {
            /* Clear depth-stencil buffer */
            if (D3D12_CLEAR_FLAGS clearFlagsDSV = GetClearFlagsDSV(clearOp.flags))
            {
                GetNative()->ClearDepthStencilView(
                    dsvDescHandle_,
                    clearFlagsDSV,
                    clearOp.clearValue.depth,
                    static_cast<UINT8>(clearOp.clearValue.stencil),
                    0,
                    nullptr
                );
            }
        }
    }
}

/* ----- Buffers ------ */

void D3D12CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    SubmitTransitionResource(bufferD3D.GetResource(), bufferD3D.GetResource().usageState);
    GetNative()->IASetVertexBuffers(0, 1, &(bufferD3D.GetVertexBufferView()));

    if ((bufferD3D.GetBindFlags() & BindFlags::StreamOutputBuffer) != 0)
        soBufferIASlot0_ = &bufferD3D;
}

void D3D12CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D12BufferArray&, bufferArray);

    for (D3D12Resource* resource : bufferArrayD3D.GetResourceRefs())
        SubmitTransitionResource(*resource, resource->usageState);

    GetNative()->IASetVertexBuffers(
        0,
        static_cast<UINT>(bufferArrayD3D.GetVertexBufferViews().size()),
        bufferArrayD3D.GetVertexBufferViews().data()
    );
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    SubmitTransitionResource(bufferD3D.GetResource(), bufferD3D.GetResource().usageState);
    commandContext_.SetIndexBuffer(bufferD3D.GetIndexBufferView());
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    D3D12_INDEX_BUFFER_VIEW indexBufferView = bufferD3D.GetIndexBufferView();
    if (indexBufferView.SizeInBytes > offset)
    {
        SubmitTransitionResource(bufferD3D.GetResource(), bufferD3D.GetResource().usageState);

        /* Update buffer location and size by offset, and override format */
        indexBufferView.BufferLocation  += offset;
        indexBufferView.SizeInBytes     -= static_cast<UINT>(offset);
        indexBufferView.Format          = DXTypes::ToDXGIFormat(format);
        commandContext_.SetIndexBuffer(indexBufferView);
    }
}

/* ----- Resources ----- */

void D3D12CommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    if (boundPipelineLayout_ == nullptr || boundPipelineState_ == nullptr)
        return /*E_POINTER*/;

    auto& resourceHeapD3D = LLGL_CAST(D3D12ResourceHeap&, resourceHeap);

    /* Copy descriptors for specified set into shader-visible descriptor heap */
    for_range(i, 2)
    {
        const auto heapType = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
        if (resourceHeapD3D.GetDescriptorHeap(heapType) != nullptr)
        {
            /* Copies the entire set of descriptors from the non-shader-visible heap to the global shader-visible heap */
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = commandContext_.CopyDescriptorsForStaging(
                heapType,
                resourceHeapD3D.GetCPUDescriptorHandleForHeapStart(heapType, descriptorSet),
                0,
                resourceHeapD3D.GetNumDescriptorsPerSet(heapType)
            );

            /* Bind descriptor table to root parameter */
            const UINT rootParamIndex = boundPipelineLayout_->GetRootParameterIndices().rootParamDescriptorHeaps[i];
            if (boundPipelineState_->IsGraphicsPSO())
                GetNative()->SetGraphicsRootDescriptorTable(rootParamIndex, gpuDescHandle);
            else
                GetNative()->SetComputeRootDescriptorTable(rootParamIndex, gpuDescHandle);
        }
    }

    /* Insert resource barriers for the specified descriptor set */
    resourceHeapD3D.TransitionResources(commandContext_, descriptorSet);
    resourceHeapD3D.InsertUAVBarriers(commandContext_, descriptorSet);
}

/*
Returns the virtual GPU address of the specified D3D12 resource. This function is only used for buffer resources.
See https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12resource-getgpuvirtualaddress
*/
static D3D12_GPU_VIRTUAL_ADDRESS GetD3DResourceGPUAddr(Resource& resource)
{
    /* GetGPUVirtualAddress() is only useful for buffers */
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        D3D12Buffer& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
        return bufferD3D.GetNative()->GetGPUVirtualAddress();
    }
    return 0;
}

void D3D12CommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (boundPipelineLayout_ == nullptr)
        return /*E_POINTER*/;

    if (!(descriptor < boundPipelineLayout_->GetNumBindings()))
        return /*E_INVALIDARG*/;

    const D3D12DescriptorHeapLocation& descriptorLocation = boundPipelineLayout_->GetDescriptorMap()[descriptor];

    const D3D12DescriptorLocation& rootParameterLocation = boundPipelineLayout_->GetRootParameterMap()[descriptor];
    if (rootParameterLocation.type != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
    {
        /* Transition resource into target state */
        SubmitTransitionResource(resource, rootParameterLocation.state); //TODO: should this be flushed immediately for root parameters?

        /* Set resource as root parameter */
        D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddr = GetD3DResourceGPUAddr(resource);
        if (gpuVirtualAddr != 0)
        {
            /* Root parameter can only be raw or structured buffers, so only handle CBV, SRV, and UAV */
            if (boundPipelineState_ != nullptr && boundPipelineState_->IsGraphicsPSO())
                commandContext_.SetGraphicsRootParameter(rootParameterLocation.index, rootParameterLocation.type, gpuVirtualAddr);
            else
                commandContext_.SetComputeRootParameter(rootParameterLocation.index, rootParameterLocation.type, gpuVirtualAddr);
        }
    }
    else
    {
        /* Transition resource into target state and bind resource with staging descriptor heap */
        SubmitTransitionResource(resource, descriptorLocation.state);
        commandContext_.EmplaceDescriptorForStaging(resource, descriptorLocation);
    }

    commandContext_.SetResourceUAVBarrier(resource, descriptorLocation);
}

void D3D12CommandBuffer::ResourceBarrier(
    std::uint32_t       numBuffers,
    Buffer* const *     buffers,
    std::uint32_t       numTextures,
    Texture* const *    textures)
{
    for_range(i, numBuffers)
    {
        if (buffers[i] != nullptr)
        {
            auto* bufferD3D = LLGL_CAST(D3D12Buffer*, buffers[i]);
            commandContext_.UAVBarrier(bufferD3D->GetResource().Get());
        }
    }

    for_range(i, numTextures)
    {
        if (textures[i] != nullptr)
        {
            auto* textureD3D = LLGL_CAST(D3D12Texture*, textures[i]);
            commandContext_.UAVBarrier(textureD3D->GetResource().Get());
        }
    }
}

/* ----- Render Passes ----- */

void D3D12CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       swapBufferIndex)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        /* Bind swap chain */
        boundSwapChain_     = LLGL_CAST(D3D12SwapChain*, &renderTarget);
        boundRenderTarget_  = nullptr;

        BindSwapChain(*boundSwapChain_, swapBufferIndex);
    }
    else
    {
        /* Bind render target */
        boundSwapChain_     = nullptr;
        boundRenderTarget_  = LLGL_CAST(D3D12RenderTarget*, &renderTarget);

        BindRenderTarget(*boundRenderTarget_);
    }

    /* Clear attachments */
    if (renderPass)
    {
        auto* renderPassD3D = LLGL_CAST(const D3D12RenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassD3D, numClearValues, clearValues);
    }
}

void D3D12CommandBuffer::EndRenderPass()
{
    /* Resolve multi-sampled subresources of previously bound render target */
    if (boundSwapChain_ != nullptr)
        boundSwapChain_->ResolveSubresources(commandContext_, currentColorBuffer_);
    else if (boundRenderTarget_ != nullptr)
        boundRenderTarget_->ResolveSubresources(commandContext_);
}

/* ----- Pipeline States ----- */

void D3D12CommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Bind pipeline state to command context */
    auto& pipelineStateD3D = LLGL_CAST(D3D12PipelineState&, pipelineState);
    if (pipelineStateD3D.IsGraphicsPSO())
    {
        /* Bind graphics PSO */
        auto& graphicsPSO = LLGL_CAST(D3D12GraphicsPSO&, pipelineState);
        graphicsPSO.Bind(commandContext_);
        boundPipelineState_ = &graphicsPSO;

        /* Scissor rectangle must be updated (if scissor test is disabled) */
        scissorEnabled_ = graphicsPSO.IsScissorEnabled();
        if (scissorEnabled_)
        {
            /* Invalidate previously bound default scissor rectangles */
            numDefaultScissorRects_ = 0;
        }
        else
        {
            if (GetNative()->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
                SetDefaultScissorRects(graphicsPSO.NumDefaultScissorRects());
        }
    }
    else
    {
        /* Bind compute PSO */
        auto& computePSO = LLGL_CAST(D3D12ComputePSO&, pipelineState);
        computePSO.Bind(commandContext_);
        boundPipelineState_ = &computePSO;
    }

    /* Keep reference to pipeline layout */
    boundPipelineLayout_ = pipelineStateD3D.GetPipelineLayout();

    if (boundPipelineLayout_ != nullptr)
    {
        /* Prepare staging descriptor heaps for bound pipeline layout */
        commandContext_.SetStagingDescriptorHeaps(
            boundPipelineLayout_->GetDescriptorHeapSetLayout(),
            boundPipelineLayout_->GetRootParameterIndices()
        );
        commandContext_.ResetUAVBarriers(boundPipelineLayout_->GetNumUAVBarriers());
    }
    else
    {
        /* Reset staging descriptor layout to avoid undefined behavior in next Flush*StagingDescriptorTables() call */
        commandContext_.SetStagingDescriptorHeaps({}, {});
        commandContext_.ResetUAVBarriers(0);
    }
}

void D3D12CommandBuffer::SetBlendFactor(const float color[4])
{
    GetNative()->OMSetBlendFactor(color);
}

void D3D12CommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    GetNative()->OMSetStencilRef(reference);
}

void D3D12CommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    /* Data size must be a multiple of 4 bytes */
    if (dataSize == 0 || dataSize % 4 != 0 || data == nullptr)
        return /*E_INVALIDARG*/;

    /* Check if a valid pipeline layout and PSO is bound and validate uniform  */
    if (boundPipelineLayout_ == nullptr || boundPipelineState_ == nullptr)
        return /*E_POINTER*/;

    const std::uint32_t                             dataSizeInWords = dataSize / 4;
    const std::uint32_t                             maxNumUniforms  = boundPipelineLayout_->GetNumUniforms();
    const std::vector<D3D12RootConstantLocation>&   rootConstantMap = boundPipelineState_->GetRootConstantMap();

    for (auto words = reinterpret_cast<const UINT*>(data), wordsEnd = words + dataSizeInWords; words != wordsEnd; ++first)
    {
        if (first >= maxNumUniforms)
            return /*E_INVALIDARG*/;

        const D3D12RootConstantLocation& rootConstantLocation = rootConstantMap[first];
        UINT wordOffset = rootConstantLocation.wordOffset;
        for_range(i, rootConstantLocation.num32BitValues)
        {
            const D3D12Constant value{ words[i] };
            if (boundPipelineState_->IsGraphicsPSO())
                commandContext_.SetGraphicsConstant(rootConstantLocation.index, value, wordOffset);
            else
                commandContext_.SetComputeConstant(rootConstantLocation.index, value, wordOffset);
            ++wordOffset;
        }
        words += rootConstantLocation.num32BitValues;
    }
}

/* ----- Queries ----- */

void D3D12CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    queryHeapD3D.Begin(GetNative(), query);
}

void D3D12CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    queryHeapD3D.End(GetNative(), query);
}

static D3D12_PREDICATION_OP GetDXPredicateOp(const RenderConditionMode mode)
{
    if (mode >= RenderConditionMode::WaitInverted)
        return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;
    else
        return D3D12_PREDICATION_OP_EQUAL_ZERO;
}

void D3D12CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);

    /* Flush query result data if it was marked as dirty */
    if (queryHeapD3D.InsideDirtyRange(query, 1))
        queryHeapD3D.FlushDirtyRange(GetNative());

    /* Set specified query as predicate */
    GetNative()->SetPredication(
        queryHeapD3D.GetResultResource(),
        queryHeapD3D.GetAlignedBufferOffest(query),
        GetDXPredicateOp(mode)
    );
}

void D3D12CommandBuffer::EndRenderCondition()
{
    GetNative()->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
}

/* ----- Stream Output ------ */

void D3D12CommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferViews[LLGL_MAX_NUM_SO_BUFFERS];

    numSOBuffers_ = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    /* Store native buffer views and transition resources */
    for_range(i, numSOBuffers_)
    {
        auto* bufferD3D = LLGL_CAST(D3D12Buffer*, buffers[i]);
        boundSOBuffers_[i] = bufferD3D;
        soBufferViews[i] = bufferD3D->GetSOBufferView();
        soBufferStates_[i] = bufferD3D->GetResource().currentState;
        commandContext_.TransitionResource(bufferD3D->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    }
    commandContext_.FlushResourceBarriers();

    /* Reset counter values in buffers by copying from a static zero-initialized buffer to the stream-output targets */
    const D3D12BufferConstantsView srcBufferView = D3D12BufferConstantsPool::Get().FetchConstantsView(D3D12BufferConstants::ZeroUInt64);

    for_range(i, numSOBuffers_)
    {
        GetNative()->CopyBufferRegion(
            boundSOBuffers_[i]->GetNative(),
            boundSOBuffers_[i]->GetStreamOutputCounterOffset(),
            srcBufferView.resource,
            srcBufferView.offset,
            srcBufferView.size
        );
    }

    /* Transition resources to stream-output */
    for_range(i, numSOBuffers_)
        commandContext_.TransitionResource(boundSOBuffers_[i]->GetResource(), D3D12_RESOURCE_STATE_STREAM_OUT);
    commandContext_.FlushResourceBarriers();

    /* Set active stream-output targets */
    GetNative()->SOSetTargets(0, numSOBuffers_, soBufferViews);
}

void D3D12CommandBuffer::EndStreamOutput()
{
    /* Unbind SO targets */
    const D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferViewsNull[LLGL_MAX_NUM_SO_BUFFERS] = {};
    GetNative()->SOSetTargets(0, LLGL_MAX_NUM_SO_BUFFERS, soBufferViewsNull);

    /* Transition resources back to their common usage */
    for_range(i, numSOBuffers_)
        commandContext_.TransitionResource(boundSOBuffers_[i]->GetResource(), soBufferStates_[i]);
    commandContext_.FlushResourceBarriers();
}

/* ----- Drawing ----- */

void D3D12CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    commandContext_.DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    commandContext_.DrawIndexedInstanced(numIndices, 1, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandContext_.DrawIndexedInstanced(numIndices, 1, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    commandContext_.DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    commandContext_.DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    commandContext_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandContext_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    commandContext_.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandContext_.TransitionResource(bufferD3D.GetResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset);
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandContext_.TransitionResource(bufferD3D.GetResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    if likely(stride == sizeof(D3D12_DRAW_ARGUMENTS))
    {
        /* Encode indirect draw with pre-defined command stride */
        commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndirect(), numCommands, bufferD3D.GetNative(), offset);
    }
    else
    {
        /* Encode indirect draw individually with custom stride */
        while (numCommands-- > 0)
        {
            commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset);
            offset += stride;
        }
    }
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandContext_.TransitionResource(bufferD3D.GetResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset);
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandContext_.TransitionResource(bufferD3D.GetResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    if likely(stride == sizeof(D3D12_DRAW_INDEXED_ARGUMENTS))
    {
        /* Encode indirect draw with pre-defined command stride */
        commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndexedIndirect(), numCommands, bufferD3D.GetNative(), offset);
    }
    else
    {
        /* Encode indirect draw individually with custom stride */
        while (numCommands-- > 0)
        {
            commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset);
            offset += stride;
        }
    }
}

/*
D3D12 stream-outputs only write out the fill buffer size. This cannot be used directly as indirect draw arguments for three reasons:
 1. It's a UINT64 instead of the required UINT (see D3D12_DRAW_ARGUMENTS::VertexCountPerInstance).
 2. It's a byte size instead of a vertex count (as opposed to D3D11).
 3. It does not contain any other draw arguments such as instance count - this could be compensated by providing these constants at buffer creation time.
For these reasons, this function dispatches a single compute shader invocation to write out the draw arguments as follows:
D3D12_DRAW_ARGUMENTS {
  VertexCountPerInstance = SOFillBufferSize / VertexStride
  InstanceCount          = 1
  StartVertexLocation    = 0
  StartInstanceLocation  = 0
}
*/
void D3D12CommandBuffer::DrawStreamOutput()
{
    if unlikely(soBufferIASlot0_ == nullptr)
        return /*E_INVALIDARG*/;

    ID3D12PipelineState* soDrawArgsPSO = nullptr;
    ID3D12RootSignature* soDrawArgsRootSig = nullptr;
    D3D12BuiltinShaderFactory::Get().GetBulitinPSO(D3D12BuiltinPSO::StreamOutputDrawArgsCS, soDrawArgsPSO, soDrawArgsRootSig);

    ID3D12GraphicsCommandList* commandList = commandContext_.GetCommandList();
    ID3D12PipelineState* oldGraphicsPSO = commandContext_.GetCurrentPipelineState();

    /* Copy stream-output fill buffer size into draw argument buffer */
    commandContext_.TransitionResource(soBufferIASlot0_->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandContext_.TransitionResource(soDrawArgBuffer_, D3D12_RESOURCE_STATE_COPY_DEST, true);
    commandList->CopyBufferRegion(soDrawArgBuffer_.Get(), 0, soBufferIASlot0_->GetNative(), soBufferIASlot0_->GetStreamOutputCounterOffset(), sizeof(UINT64));

    /* Generate indirect draw arguments with a single compute shader invocation */
    commandContext_.TransitionResource(soDrawArgBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
    commandContext_.SetComputeRootSignature(soDrawArgsRootSig);
    commandList->SetPipelineState(soDrawArgsPSO);
    commandList->SetComputeRoot32BitConstant(0, soBufferIASlot0_->GetStride(), 0);
    commandList->SetComputeRootUnorderedAccessView(1, soDrawArgBuffer_.Get()->GetGPUVirtualAddress());
    commandContext_.Dispatch(1, 1, 1);

    /* Submit indirect draw command with previous graphics PSO */
    commandList->SetPipelineState(oldGraphicsPSO);
    commandContext_.UAVBarrier(soDrawArgBuffer_.Get());
    commandContext_.TransitionResource(soDrawArgBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    commandContext_.DrawIndirect(cmdSignatureFactory_->GetSignatureDrawIndirect(), 1, soDrawArgBuffer_.Get(), 0);
}

/* ----- Compute ----- */

void D3D12CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    commandContext_.Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D12CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandContext_.TransitionResource(bufferD3D.GetResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    commandContext_.DispatchIndirect(cmdSignatureFactory_->GetSignatureDispatchIndirect(), 1, bufferD3D.GetNative(), offset);
}

/* ----- Debugging ----- */

void D3D12CommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef _MSC_VER
    PIXBeginEvent(GetNative(), 0, name);
    #endif
}

void D3D12CommandBuffer::PopDebugGroup()
{
    #ifdef _MSC_VER
    PIXEndEvent(GetNative());
    #endif
}

/* ----- Extensions ----- */

void D3D12CommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool D3D12CommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D12::CommandBufferNativeHandle))
    {
        auto* nativeHandleD3D = reinterpret_cast<Direct3D12::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleD3D->commandList = GetNative();
        nativeHandleD3D->commandList->AddRef();
        return true;
    }
    return false;
}

/* ----- Internal ----- */

void D3D12CommandBuffer::ExecuteBundle(D3D12CommandContext& context)
{
    /* First transition all resource states with the command context, since bundles cannot execute those themselves */
    for (const D3D12ResourceTransition& transition : bundleResourceTransitions_)
        context.TransitionResource(*(transition.resource), transition.newState);

    /* Now execute the bundle natively */
    context.ExecuteBundle(commandContext_);
}


/*
 * ======= Private: =======
 */

static D3D12_COMMAND_LIST_TYPE GetD3DCommandListType(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & CommandBufferFlags::Secondary) != 0)
        return D3D12_COMMAND_LIST_TYPE_BUNDLE;
    else
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

static UINT GetNumCommandAllocators(const CommandBufferDescriptor& desc)
{
    constexpr UINT numAllocatorsDefault = 3;
    if (desc.numNativeBuffers == 0)
    {
        /* Return default number of allocators and only 1 for multi-submit command buffers as it's expected they won't be encoded more than once */
        if ((desc.flags & CommandBufferFlags::MultiSubmit) != 0)
            return 1;
        else
            return numAllocatorsDefault;
    }
    return desc.numNativeBuffers;
}

void D3D12CommandBuffer::CreateCommandContext(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc)
{
    auto& device = renderSystem.GetDevice();

    /* Create command context and store reference to command list */
    const bool initialClose         = true;
    const bool cacheResourceStates  = ((desc.flags & (CommandBufferFlags::ImmediateSubmit | CommandBufferFlags::Secondary)) == 0);
    commandContext_.Create(device, GetD3DCommandListType(desc), GetNumCommandAllocators(desc), desc.minStagingPoolSize, initialClose, cacheResourceStates);

    /* Store increment size for descriptor heaps */
    rtvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void D3D12CommandBuffer::SetAndConvertViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    /* Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if (IsCompatibleToD3DViewport())
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        GetNative()->RSSetViewports(numViewports, reinterpret_cast<const D3D12_VIEWPORT*>(viewports));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D12_VIEWPORT viewportsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for_range(i, numViewports)
        {
            const Viewport& src = viewports[i];
            D3D12_VIEWPORT& dst = viewportsD3D[i];

            dst.TopLeftX   = src.x;
            dst.TopLeftY   = src.y;
            dst.Width      = src.width;
            dst.Height     = src.height;
            dst.MinDepth   = src.minDepth;
            dst.MaxDepth   = src.maxDepth;
        }

        GetNative()->RSSetViewports(numViewports, viewportsD3D);
    }

    /* If scissor test is disabled, set scissor rectangles to default value alongside viewports */
    if (!scissorEnabled_)
        SetDefaultScissorRects(numViewports);
}

void D3D12CommandBuffer::SetAndConvertScissorRects(std::uint32_t numScissors, const Scissor* scissors)
{
    D3D12_RECT scissorsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    for_range(i, numScissors)
    {
        const Scissor&  src = scissors[i];
        D3D12_RECT&     dst = scissorsD3D[i];

        dst.left   = src.x;
        dst.top    = src.y;
        dst.right  = src.x + src.width;
        dst.bottom = src.y + src.height;
    }

    GetNative()->RSSetScissorRects(numScissors, scissorsD3D);

    /* Invalidate previously bound default scissor rectangles */
    numDefaultScissorRects_ = 0;
}

static constexpr D3D12_RECT g_maxViewportBoundsD3D =
{
    D3D12_VIEWPORT_BOUNDS_MIN, D3D12_VIEWPORT_BOUNDS_MIN, D3D12_VIEWPORT_BOUNDS_MAX, D3D12_VIEWPORT_BOUNDS_MAX
};

static constexpr D3D12_RECT g_maxViewportBoundsD3DArray[] =
{
    g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D,
    g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D,
    g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D,
    g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D, g_maxViewportBoundsD3D,
};

static_assert(
    sizeof(g_maxViewportBoundsD3DArray)/sizeof(g_maxViewportBoundsD3DArray[0]) == D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
    "g_maxViewportBoundsD3DArray must have D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE elements"
);

void D3D12CommandBuffer::SetDefaultScissorRects(UINT numScissorRects)
{
    numScissorRects = std::min(numScissorRects, UINT(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
    if (numScissorRects > numDefaultScissorRects_)
    {
        /* Set scissor to maximum viewport boundary and store new number of scissor rectangles to avoid unnecessary updates */
        GetNative()->RSSetScissorRects(numScissorRects, g_maxViewportBoundsD3DArray);
        numDefaultScissorRects_ = numScissorRects;
    }
}

void D3D12CommandBuffer::BindRenderTarget(D3D12RenderTarget& renderTargetD3D)
{
    /* Transition resources to state ready for output merger */
    renderTargetD3D.TransitionToOutputMerger(commandContext_);

    /* Set current back buffer as RTV and optional DSV */
    numColorBuffers_ = renderTargetD3D.GetNumColorAttachments();

    rtvDescHandle_ = renderTargetD3D.GetCPUDescriptorHandleForRTV();
    dsvDescHandle_ = renderTargetD3D.GetCPUDescriptorHandleForDSV();

    if (dsvDescHandle_.ptr != 0)
        GetNative()->OMSetRenderTargets(numColorBuffers_, &rtvDescHandle_, TRUE, &dsvDescHandle_);
    else
        GetNative()->OMSetRenderTargets(numColorBuffers_, &rtvDescHandle_, TRUE, nullptr);
}

void D3D12CommandBuffer::BindSwapChain(D3D12SwapChain& swapChainD3D, std::uint32_t swapBufferIndex)
{
    /* Translate swap-index into actual D3D color buffer index */
    currentColorBuffer_ = swapChainD3D.TranslateSwapIndex(swapBufferIndex);

    /* Indicate that the back buffer will be used as render target */
    commandContext_.TransitionResource(
        swapChainD3D.GetCurrentColorBuffer(currentColorBuffer_),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        true
    );

    /* Set current back buffer as RTV and optional DSV */
    numColorBuffers_ = 1;

    rtvDescHandle_ = swapChainD3D.GetCPUDescriptorHandleForRTV(currentColorBuffer_);
    dsvDescHandle_ = swapChainD3D.GetCPUDescriptorHandleForDSV();

    if (dsvDescHandle_.ptr != 0)
        GetNative()->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, &dsvDescHandle_);
    else
        GetNative()->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, nullptr);
}

std::uint32_t D3D12CommandBuffer::ClearAttachmentsWithRenderPass(
    const D3D12RenderPass&  renderPassD3D,
    std::uint32_t           numClearValues,
    const ClearValue*       clearValues,
    UINT                    numRects,
    const D3D12_RECT*       rects)
{
    std::uint32_t clearValueIndex = 0;

    const std::uint8_t* colorBuffers        = renderPassD3D.GetClearColorAttachments();
    const std::uint32_t numColorClearValues = std::min(numClearValues, numColorBuffers_);

    /* Clear color attachments */
    if (rtvDescHandle_.ptr != 0)
    {
        /* Clear active RTVs with specified clear values */
        clearValueIndex = ClearRenderTargetViews(colorBuffers, numColorClearValues, clearValues, clearValueIndex, numRects, rects);
    }

    /* Clear depth-stencil attachment */
    if (dsvDescHandle_.ptr != 0)
    {
        /* Fast forward to end of list of color clear values */
        while (clearValueIndex < numColorClearValues && colorBuffers[clearValueIndex] != 0xFF)
            ++clearValueIndex;

        /* Clear active DSV with specified clear value */
        if (D3D12_CLEAR_FLAGS clearFlagsDSV = renderPassD3D.GetClearFlagsDSV())
            ClearDepthStencilView(clearFlagsDSV, numClearValues, clearValues, clearValueIndex, numRects, rects);
    }

    return clearValueIndex;
}

std::uint32_t D3D12CommandBuffer::ClearRenderTargetViews(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       clearValueIndex,
    UINT                numRects,
    const D3D12_RECT*   rects)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = {};

    /* Use specified clear values */
    for_range(i, numClearValues)
    {
        /* Check if attachment list has ended */
        const std::uint8_t targetIndex = colorBuffers[i];
        if (targetIndex == 0xFF)
            return clearValueIndex;

        /* Clear RTV at specified color buffer offset */
        if (targetIndex < numColorBuffers_)
        {
            rtvDescHandle.ptr = rtvDescHandle_.ptr + rtvDescSize_ * targetIndex;
            GetNative()->ClearRenderTargetView(rtvDescHandle, clearValues[clearValueIndex].color, numRects, rects);
        }

        ++clearValueIndex;
    }

    /* Use default clear values */
    const FLOAT defaultClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for_subrange(i, numClearValues, numColorBuffers_)
    {
        /* Check if attachment list has ended */
        const std::uint8_t targetIndex = colorBuffers[i];
        if (targetIndex == 0xFF)
            return clearValueIndex;

        /* Clear RTV at specified color buffer offset */
        if (targetIndex < numColorBuffers_)
        {
            rtvDescHandle.ptr = rtvDescHandle_.ptr + rtvDescSize_ * targetIndex;
            GetNative()->ClearRenderTargetView(rtvDescHandle, defaultClearColor, numRects, rects);
        }
    }

    return clearValueIndex;
}

void D3D12CommandBuffer::ClearDepthStencilView(
    D3D12_CLEAR_FLAGS   clearFlags,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       clearValueIndex,
    UINT                numRects,
    const D3D12_RECT*   rects)
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
    GetNative()->ClearDepthStencilView(dsvDescHandle_, clearFlags, depth, stencil, numRects, rects);
}

void D3D12CommandBuffer::ResetBindingStates()
{
    numDefaultScissorRects_ = 0;
    numSOBuffers_           = 0;
    soBufferIASlot0_        = nullptr;
    boundRenderTarget_      = nullptr;
    boundSwapChain_         = nullptr;
    boundPipelineLayout_    = nullptr;
    boundPipelineState_     = nullptr;
}

void D3D12CommandBuffer::CreateSOIndirectDrawArgBuffer(ID3D12Device* device)
{
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_DRAW_ARGUMENTS), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, // Buffers are effectively created in D3D12_RESOURCE_STATE_COMMON state
        nullptr,
        IID_PPV_ARGS(soDrawArgBuffer_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 indirect draw argument buffer");
    soDrawArgBuffer_.native->SetName(L"LLGL.SODrawArgBuffer");
}

void D3D12CommandBuffer::SubmitTransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState)
{
    if (IsBundleCmdBuffer())
        bundleResourceTransitions_.push_back({ &resource, newState });
    else
        commandContext_.TransitionResource(resource, newState);
}

void D3D12CommandBuffer::SubmitTransitionResource(Resource& resource, D3D12_RESOURCE_STATES newState)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            SubmitTransitionResource(bufferD3D.GetResource(), newState);
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            if ((newState & (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) != 0 &&
                IsDepthOrStencilFormat(textureD3D.GetBaseFormat()))
            {
                /* Depth-stencil SRVs must also be transitioned into DEPTH_READ state */
                SubmitTransitionResource(textureD3D.GetResource(), newState | D3D12_RESOURCE_STATE_DEPTH_READ);
            }
            else
                SubmitTransitionResource(textureD3D.GetResource(), newState);
        }
        break;

        default:
        break;
    }
}


} // /namespace LLGL



// ================================================================================
