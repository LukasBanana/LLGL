/*
 * D3D12CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandBuffer.h"
#include "D3D12SignatureFactory.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12RenderContext.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../../TextureUtils.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"

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

#include "../D3DX12/d3dx12.h"
#include <pix.h>

#include <algorithm>
#include <codecvt>
#include <limits.h>


namespace LLGL
{


D3D12CommandBuffer::D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc) :
    cmdSignatureFactory_ { &(renderSystem.GetSignatureFactory())           },
    stagingBufferPool_   { renderSystem.GetDevice().GetNative(), USHRT_MAX }
{
    CreateCommandContext(renderSystem, desc);
}

void D3D12CommandBuffer::SetName(const char* name)
{
    D3D12SetObjectName(commandList_, name);
}

/* ----- Encoding ----- */

void D3D12CommandBuffer::Begin()
{
    /* Reset command list using the next command allocator */
    commandContext_.Reset();
    stagingBufferPool_.Reset();
}

void D3D12CommandBuffer::End()
{
    /* Close command context and reset intermediate states */
    commandContext_.Close();
    numBoundScissorRects_ = 0;
}

void D3D12CommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D12CommandBuffer&, deferredCommandBuffer);
    commandList_->ExecuteBundle(cmdBufferD3D.GetNative());
}

/* ----- Blitting ----- */

void D3D12CommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    stagingBufferPool_.WriteStaged(commandContext_, dstBufferD3D.GetResource(), dstOffset, data, dataSize);
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

    commandContext_.TransitionResource(dstBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    {
        commandList_->CopyBufferRegion(dstBufferD3D.GetNative(), dstOffset, srcBufferD3D.GetNative(), srcOffset, size);
    }
    commandContext_.TransitionResource(dstBufferD3D.GetResource(), dstBufferD3D.GetResource().usageState);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), srcBufferD3D.GetResource().usageState, true);
}

//TODO: incomplete for unaligned row strides
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
        rowStride = GetMemoryFootprint(srcTextureD3D.GetFormat(), srcExtent.width);
        alignedRowStride = GetAlignedSize<UINT>(rowStride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    }

    const D3D12_TEXTURE_COPY_LOCATION   dstLocationD3D  = srcTextureD3D.CalcCopyLocation(dstBufferD3D, dstOffset, srcExtent, alignedRowStride);
    const D3D12_TEXTURE_COPY_LOCATION   srcLocationD3D  = srcTextureD3D.CalcCopyLocation(srcLocation);
    const D3D12_BOX                     srcBox          = srcTextureD3D.CalcRegion(srcRegion.offset, srcExtent);

    commandContext_.TransitionResource(dstBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    {
        if (alignedRowStride != rowStride)
        {
            /* Copy each row individually due to unalgined row pitch */
            for (UINT z = 0; z < srcExtent.depth; ++z)
            {
                for (UINT y = 0; y < srcExtent.height; ++y)
                {
                    commandList_->CopyTextureRegion(
                        &dstLocationD3D,    // pDst
                        0,                  // DstX
                        0,                  // DstY
                        0,                  // DstZ
                        &srcLocationD3D,    // pSrc
                        &CD3DX12_BOX(       // pSrcBox
                            srcBox.left, srcBox.top + y, srcBox.front + z,
                            srcBox.right, srcBox.top + y + 1, srcBox.front + z + 1
                        )
                    );
                }
            }
        }
        else
        {
            /* Copy entire texture region from buffer  */
            commandList_->CopyTextureRegion(
                &dstLocationD3D,    // pDst
                0,                  // DstX
                0,                  // DstY
                0,                  // DstZ
                &srcLocationD3D,    // pSrc
                &srcBox             // pSrcBox
            );
        }
    }
    commandContext_.TransitionResource(dstBufferD3D.GetResource(), dstBufferD3D.GetResource().usageState);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), srcTextureD3D.GetResource().usageState, true);
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
    if (fillSize == Constants::wholeSize)
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
    const D3D12_TEXTURE_COPY_LOCATION srcLocationD3D = srcTextureD3D.CalcCopyLocation(dstLocation);

    const D3D12_BOX srcBox = srcTextureD3D.CalcRegion(srcLocation.offset, extent);

    commandContext_.TransitionResource(dstTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    {
        commandList_->CopyTextureRegion(
            &dstLocationD3D,                            // pDst
            static_cast<UINT>(dstLocation.offset.x),    // DstX
            static_cast<UINT>(dstLocation.offset.y),    // DstY
            static_cast<UINT>(dstLocation.offset.z),    // DstZ
            &srcLocationD3D,                            // pSrc
            &srcBox                                     // pSrcBox
        );
    }
    commandContext_.TransitionResource(dstTextureD3D.GetResource(), dstTextureD3D.GetResource().usageState);
    commandContext_.TransitionResource(srcTextureD3D.GetResource(), srcTextureD3D.GetResource().usageState, true);
}

//TODO: incomplete for unaligned row strides
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
        rowStride = GetMemoryFootprint(dstTextureD3D.GetFormat(), dstExtent.width);
        alignedRowStride = GetAlignedSize<UINT>(rowStride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    }

    const D3D12_TEXTURE_COPY_LOCATION   dstLocationD3D  = dstTextureD3D.CalcCopyLocation(dstLocation);
    const D3D12_TEXTURE_COPY_LOCATION   srcLocationD3D  = dstTextureD3D.CalcCopyLocation(srcBufferD3D, srcOffset, dstExtent, alignedRowStride);
    const D3D12_BOX                     srcBox          = dstTextureD3D.CalcRegion(Offset3D{}, dstExtent);

    commandContext_.TransitionResource(dstTextureD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    {
        if (alignedRowStride != rowStride)
        {
            /* Copy each row individually due to unalgined row pitch */
            for (UINT z = 0; z < dstExtent.depth; ++z)
            {
                for (UINT y = 0; y < dstExtent.height; ++y)
                {
                    commandList_->CopyTextureRegion(
                        &dstLocationD3D,                            // pDst
                        static_cast<UINT>(dstRegion.offset.x),      // DstX
                        static_cast<UINT>(dstRegion.offset.y) + y,  // DstY
                        static_cast<UINT>(dstRegion.offset.z) + z,  // DstZ
                        &srcLocationD3D,                            // pSrc
                        &CD3DX12_BOX(                               // pSrcBox
                            srcBox.left, srcBox.top + y, srcBox.front + z,
                            srcBox.right, srcBox.top + y + 1, srcBox.front + z + 1
                        )
                    );
                }
            }
        }
        else
        {
            /* Copy entire texture region from buffer  */
            commandList_->CopyTextureRegion(
                &dstLocationD3D,                        // pDst
                static_cast<UINT>(dstRegion.offset.x),  // DstX
                static_cast<UINT>(dstRegion.offset.y),  // DstY
                static_cast<UINT>(dstRegion.offset.z),  // DstZ
                &srcLocationD3D,                        // pSrc
                &srcBox                                 // pSrcBox
            );
        }
    }
    commandContext_.TransitionResource(dstTextureD3D.GetResource(), dstTextureD3D.GetResource().usageState);
    commandContext_.TransitionResource(srcBufferD3D.GetResource(), srcBufferD3D.GetResource().usageState, true);
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
    D3D12CommandBuffer::SetViewports(1, &viewport);
}

void D3D12CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    /* Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if (IsCompatibleToD3DViewport())
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        commandList_->RSSetViewports(numViewports, reinterpret_cast<const D3D12_VIEWPORT*>(viewports));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D12_VIEWPORT viewportsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewports[i];
            auto& dest = viewportsD3D[i];

            dest.TopLeftX   = src.x;
            dest.TopLeftY   = src.y;
            dest.Width      = src.width;
            dest.Height     = src.height;
            dest.MinDepth   = src.minDepth;
            dest.MaxDepth   = src.maxDepth;
        }

        commandList_->RSSetViewports(numViewports, viewportsD3D);
    }

    /* If scissor test is disabled, update remaining scissor rectangles to default value */
    if (!scissorEnabled_)
        SetScissorRectsToDefault(numViewports);
}

void D3D12CommandBuffer::SetScissor(const Scissor& scissor)
{
    D3D12CommandBuffer::SetScissors(1, &scissor);
}

void D3D12CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    numScissors = std::min(numScissors, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    D3D12_RECT scissorsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    for (std::uint32_t i = 0; i < numScissors; ++i)
    {
        const auto& src = scissors[i];
        auto& dest = scissorsD3D[i];

        dest.left   = src.x;
        dest.top    = src.y;
        dest.right  = src.x + src.width;
        dest.bottom = src.y + src.height;
    }

    commandList_->RSSetScissorRects(numScissors, scissorsD3D);
}

/* ----- Clear ----- */

void D3D12CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearValue_.color = color;
}

void D3D12CommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depth = depth;
}

void D3D12CommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.stencil = (stencil & 0xff);
}

static D3D12_CLEAR_FLAGS GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_STENCIL;

    return static_cast<D3D12_CLEAR_FLAGS>(clearFlagsDSV);
}

void D3D12CommandBuffer::Clear(long flags)
{
    if (rtvDescHandle_.ptr != 0)
    {
        /* Clear color buffers */
        if ((flags & ClearFlags::Color) != 0)
        {
            auto rtvDescHandle = rtvDescHandle_;
            for (UINT i = 0; i < numColorBuffers_; ++i)
            {
                commandList_->ClearRenderTargetView(rtvDescHandle, clearValue_.color.Ptr(), 0, nullptr);
                rtvDescHandle.ptr += rtvDescSize_;
            }
        }
    }

    if (dsvDescHandle_.ptr != 0)
    {
        /* Clear depth-stencil buffer */
        if (auto clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            commandList_->ClearDepthStencilView(
                dsvDescHandle_,
                clearFlagsDSV,
                clearValue_.depth,
                static_cast<UINT8>(clearValue_.stencil),
                0,
                nullptr
            );
        }
    }
}

void D3D12CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for (std::uint32_t i = 0; i < numAttachments; ++i)
    {
        const auto& clearOp = attachments[i];

        if (rtvDescHandle_.ptr != 0)
        {
            /* Clear color buffers */
            if ((clearOp.flags & ClearFlags::Color) != 0)
            {
                auto rtvDescHandle = rtvDescHandle_;
                rtvDescHandle.ptr += (rtvDescSize_ * clearOp.colorAttachment);
                commandList_->ClearRenderTargetView(rtvDescHandle, clearOp.clearValue.color.Ptr(), 0, nullptr);
            }
        }

        if (dsvDescHandle_.ptr != 0)
        {
            /* Clear depth-stencil buffer */
            if (auto clearFlagsDSV = GetClearFlagsDSV(clearOp.flags))
            {
                commandList_->ClearDepthStencilView(
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
    commandList_->IASetVertexBuffers(0, 1, &(bufferD3D.GetVertexBufferView()));
}

void D3D12CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D12BufferArray&, bufferArray);
    commandList_->IASetVertexBuffers(
        0,
        static_cast<UINT>(bufferArrayD3D.GetVertexBufferViews().size()),
        bufferArrayD3D.GetVertexBufferViews().data()
    );
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->IASetIndexBuffer(&(bufferD3D.GetIndexBufferView()));
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    auto indexBufferView = bufferD3D.GetIndexBufferView();
    if (indexBufferView.SizeInBytes > offset)
    {
        /* Update buffer location and size by offset, and override format */
        indexBufferView.BufferLocation  += offset;
        indexBufferView.SizeInBytes     -= static_cast<UINT>(offset);
        indexBufferView.Format          = D3D12Types::Map(format);
        commandList_->IASetIndexBuffer(&indexBufferView);
    }
}

/* ----- Resources ----- */

void D3D12CommandBuffer::SetResourceHeap(
    ResourceHeap&           resourceHeap,
    std::uint32_t           firstSet,
    const PipelineBindPoint bindPoint)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D12ResourceHeap&, resourceHeap);

    /* Get descriptor heaps */
    auto heapCount = resourceHeapD3D.GetNumDescriptorHeaps();
    if (heapCount > 0)
    {
        /* Bind descriptor heaps */
        auto descHeaps = resourceHeapD3D.GetDescriptorHeaps();
        commandContext_.SetDescriptorHeaps(heapCount, descHeaps);

        auto handleStrides = resourceHeapD3D.GetDescriptorHandleStrides();

        /* Bind root descriptor tables to graphics pipeline */
        for (UINT i = 0; i < heapCount; ++i)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descHeaps[i]->GetGPUDescriptorHandleForHeapStart();

            gpuDescHandle.ptr += handleStrides[i] * firstSet;

            if (resourceHeapD3D.HasGraphicsDescriptors() && bindPoint != PipelineBindPoint::Compute)
                commandList_->SetGraphicsRootDescriptorTable(i, gpuDescHandle);
            if (resourceHeapD3D.HasComputeDescriptors() && bindPoint != PipelineBindPoint::Graphics)
                commandList_->SetComputeRootDescriptorTable(i, gpuDescHandle);
        }

        /* Insert resource barriers for the specified descriptor set */
        resourceHeapD3D.InsertResourceBarriers(commandList_, firstSet);
    }
}

void D3D12CommandBuffer::SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long stageFlags)
{
    //TODOL: use "SetGraphicsRootShaderResourceView" etc.
}

void D3D12CommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{

    const D3D12_STREAM_OUTPUT_BUFFER_VIEW nullViews[1] = {};
    commandList_->SOSetTargets(0, 1, nullViews);

    // dummy
}

/* ----- Render Passes ----- */

void D3D12CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    boundRenderTarget_ = &(renderTarget);

    /* Bind render target/context */
    if (renderTarget.IsRenderContext())
        BindRenderContext(LLGL_CAST(D3D12RenderContext&, renderTarget));
    else
        BindRenderTarget(LLGL_CAST(D3D12RenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto renderPassD3D = LLGL_CAST(const D3D12RenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassD3D, numClearValues, clearValues);
    }
}

void D3D12CommandBuffer::EndRenderPass()
{
    if (boundRenderTarget_)
    {
        if (boundRenderTarget_->IsRenderContext())
        {
            auto renderContextD3D = LLGL_CAST(D3D12RenderContext*, boundRenderTarget_);
            renderContextD3D->ResolveRenderTarget(commandContext_);
        }
        else
        {
            auto renderTargetD3D = LLGL_CAST(D3D12RenderTarget*, boundRenderTarget_);
            renderTargetD3D->ResolveRenderTarget(commandContext_);
        }
        boundRenderTarget_ = nullptr;
    }
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

        /* Scissor rectangle must be updated (if scissor test is disabled) */
        scissorEnabled_ = graphicsPSO.IsScissorEnabled();
        if (!scissorEnabled_ && commandList_->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
            SetScissorRectsToDefault(graphicsPSO.NumDefaultScissorRects());
    }
    else
    {
        /* Bind compute PSO */
        auto& computePSO = LLGL_CAST(D3D12ComputePSO&, pipelineState);
        computePSO.Bind(commandContext_);
    }
}

void D3D12CommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    commandList_->OMSetBlendFactor(color.Ptr());
}

void D3D12CommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    commandList_->OMSetStencilRef(reference);
}

void D3D12CommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    D3D12CommandBuffer::SetUniforms(location, 1, data, dataSize);
}

void D3D12CommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    //TODO
}

/* ----- Queries ----- */

void D3D12CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    queryHeapD3D.Begin(commandList_, query);
}

void D3D12CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    queryHeapD3D.End(commandList_, query);
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
        queryHeapD3D.FlushDirtyRange(commandList_);

    /* Set specified query as predicate */
    commandList_->SetPredication(
        queryHeapD3D.GetResultResource(),
        queryHeapD3D.GetAlignedBufferOffest(query),
        GetDXPredicateOp(mode)
    );
}

void D3D12CommandBuffer::EndRenderCondition()
{
    commandList_->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
}

/* ----- Stream Output ------ */

void D3D12CommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferViews[LLGL_MAX_NUM_SO_BUFFERS];
    D3D12Buffer* buffersD3D[LLGL_MAX_NUM_SO_BUFFERS];

    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    /* Store native buffer views and transition resources */
    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        auto bufferD3D = LLGL_CAST(D3D12Buffer*, buffers[i]);
        buffersD3D[i] = bufferD3D;
        soBufferViews[i] = bufferD3D->GetSOBufferView();
        commandContext_.TransitionResource(bufferD3D->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    }
    commandContext_.FlushResourceBarrieres();

    /* Reset counter values in buffers by copying from a static zero-initialized buffer to the stream-output targets */
    const auto srcBufferView = D3D12BufferConstantsPool::Get().FetchConstants(D3D12BufferConstants::ZeroUInt64);

    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        commandList_->CopyBufferRegion(
            buffersD3D[i]->GetNative(),
            buffersD3D[i]->GetBufferSize(),
            srcBufferView.resource,
            srcBufferView.offset,
            srcBufferView.size
        );
    }

    /* Transition resources to stream-output */
    for (std::uint32_t i = 0; i < numBuffers; ++i)
        commandContext_.TransitionResource(buffersD3D[i]->GetResource(), D3D12_RESOURCE_STATE_STREAM_OUT);
    commandContext_.FlushResourceBarrieres();

    /* Set active stream-output targets */
    commandList_->SOSetTargets(0, numBuffers, soBufferViews);
}

void D3D12CommandBuffer::EndStreamOutput()
{
    const D3D12_STREAM_OUTPUT_BUFFER_VIEW soBufferViewsNull[LLGL_MAX_NUM_SO_BUFFERS] = {};
    commandList_->SOSetTargets(0, LLGL_MAX_NUM_SO_BUFFERS, soBufferViewsNull);
}

/* ----- Drawing ----- */

void D3D12CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    commandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numIndices, 1, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numIndices, 1, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        cmdSignatureFactory_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    while (numCommands-- > 0)
    {
        commandList_->ExecuteIndirect(
            cmdSignatureFactory_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
        );
        offset += stride;
    }
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        cmdSignatureFactory_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    while (numCommands-- > 0)
    {
        commandList_->ExecuteIndirect(
            cmdSignatureFactory_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
        );
        offset += stride;
    }
}

/* ----- Compute ----- */

void D3D12CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    commandList_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D12CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        cmdSignatureFactory_->GetSignatureDispatchIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

/* ----- Debugging ----- */

void D3D12CommandBuffer::PushDebugGroup(const char* name)
{
    std::wstring nameWStr = ToUTF16String(name);
    PIXBeginEvent(commandList_, 0, nameWStr.c_str());
}

void D3D12CommandBuffer::PopDebugGroup()
{
    PIXEndEvent(commandList_);
}

/* ----- Extensions ----- */

void D3D12CommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Internal ----- */

void D3D12CommandBuffer::Execute()
{
    commandContext_.Execute();
}


/*
 * ======= Private: =======
 */

static D3D12_COMMAND_LIST_TYPE GetD3DCommandListType(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & CommandBufferFlags::DeferredSubmit) != 0)
        return D3D12_COMMAND_LIST_TYPE_BUNDLE;
    else
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

void D3D12CommandBuffer::CreateCommandContext(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc)
{
    auto& device = renderSystem.GetDevice();

    /* Create command context and store reference to command list */
    auto commandQueueD3D = LLGL_CAST(D3D12CommandQueue*, renderSystem.GetCommandQueue());
    commandContext_.Create(device, *commandQueueD3D, GetD3DCommandListType(desc), desc.numNativeBuffers, true);
    commandList_ = commandContext_.GetCommandList();

    /* Store increment size for descriptor heaps */
    rtvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void D3D12CommandBuffer::SetScissorRectsToDefault(UINT numScissorRects)
{
    numScissorRects = std::min(numScissorRects, UINT(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    if (numScissorRects > numBoundScissorRects_)
    {
        /* Set scissor to render target resolution */
        D3D12_RECT scissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (UINT i = 0; i < numScissorRects; ++i)
        {
            scissorRects[i].left    = 0;
            scissorRects[i].top     = 0;
            scissorRects[i].right   = std::numeric_limits<LONG>::max();
            scissorRects[i].bottom  = std::numeric_limits<LONG>::max();
        }

        commandList_->RSSetScissorRects(numScissorRects, scissorRects);

        /* Store new number of bound scissor rectangles */
        numBoundScissorRects_ = numScissorRects;
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
        commandList_->OMSetRenderTargets(numColorBuffers_, &rtvDescHandle_, TRUE, &dsvDescHandle_);
    else
        commandList_->OMSetRenderTargets(numColorBuffers_, &rtvDescHandle_, TRUE, nullptr);
}

void D3D12CommandBuffer::BindRenderContext(D3D12RenderContext& renderContextD3D)
{
    /* Indicate that the back buffer will be used as render target */
    commandContext_.TransitionResource(
        renderContextD3D.GetCurrentColorBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        true
    );

    /* Set current back buffer as RTV and optional DSV */
    numColorBuffers_ = 1;

    rtvDescHandle_ = renderContextD3D.GetCPUDescriptorHandleForRTV();
    dsvDescHandle_ = renderContextD3D.GetCPUDescriptorHandleForDSV();

    if (dsvDescHandle_.ptr != 0)
        commandList_->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, &dsvDescHandle_);
    else
        commandList_->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, nullptr);
}

void D3D12CommandBuffer::ClearAttachmentsWithRenderPass(
    const D3D12RenderPass&  renderPassD3D,
    std::uint32_t           numClearValues,
    const ClearValue*       clearValues,
    UINT                    numRects,
    const D3D12_RECT*       rects)
{
    auto* colorBuffers = renderPassD3D.GetClearColorAttachments();

    /* Clear color attachments */
    std::uint32_t idx = 0;
    if (rtvDescHandle_.ptr != 0)
    {
        /* Clear active RTVs with specified clear values */
        ClearRenderTargetViews(colorBuffers, numClearValues, clearValues, idx, numRects, rects);
    }
    else
    {
        /* Fast forward to end of list */
        for (std::uint32_t i = 0; i < numClearValues; ++i)
        {
            if (colorBuffers[i] != 0xFF)
                ++idx;
            else
                return;
        }
    }

    /* Clear depth-stencil attachment */
    if (dsvDescHandle_.ptr != 0)
    {
        /* Clear active DSV with specified clear value */
        if (auto clearFlagsDSV = renderPassD3D.GetClearFlagsDSV())
            ClearDepthStencilView(clearFlagsDSV, numClearValues, clearValues, idx, numRects, rects);
    }
}

void D3D12CommandBuffer::ClearRenderTargetViews(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx,
    UINT                numRects,
    const D3D12_RECT*   rects)
{
    numClearValues = std::min(numClearValues, numColorBuffers_);

    std::uint32_t i = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = {};

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        const auto targetIndex = colorBuffers[i];
        if (targetIndex != 0xFF)
        {
            /* Clear RTV at specified color buffer offset */
            if (targetIndex < numColorBuffers_)
            {
                rtvDescHandle.ptr = rtvDescHandle_.ptr + rtvDescSize_ * targetIndex;
                commandList_->ClearRenderTargetView(rtvDescHandle, clearValues[idx].color.Ptr(), numRects, rects);
            }
            ++idx;
        }
        else
            return;
    }

    /* Use default clear values */
    for (; i < numColorBuffers_; ++i)
    {
        /* Check if attachment list has ended */
        const auto targetIndex = colorBuffers[i];
        if (targetIndex != 0xFF)
        {
            /* Clear RTV at specified color buffer offset */
            if (targetIndex < numColorBuffers_)
            {
                rtvDescHandle.ptr = rtvDescHandle_.ptr + rtvDescSize_ * targetIndex;
                commandList_->ClearRenderTargetView(rtvDescHandle, clearValue_.color.Ptr(), numRects, rects);
            }
        }
        else
            return;
    }
}

void D3D12CommandBuffer::ClearDepthStencilView(
    D3D12_CLEAR_FLAGS   clearFlags,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       idx,
    UINT                numRects,
    const D3D12_RECT*   rects)
{
    /* Get clear values */
    FLOAT depth;
    UINT8 stencil;

    if (idx < numClearValues)
    {
        depth   = clearValues[idx].depth;
        stencil = static_cast<UINT8>(clearValues[idx].stencil & 0xff);
    }
    else
    {
        depth   = clearValue_.depth;
        stencil = static_cast<UINT8>(clearValue_.stencil);
    }

    /* Clear depth-stencil view */
    commandList_->ClearDepthStencilView(dsvDescHandle_, clearFlags, depth, stencil, numRects, rects);
}


} // /namespace LLGL



// ================================================================================
