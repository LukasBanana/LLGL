/*
 * MTMultiSubmitCommandBuffer.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTMultiSubmitCommandBuffer.h"
#include "MTCommand.h"
#include "MTCommandQueue.h"
#include "../MTSwapChain.h"
#include "../MTTypes.h"
#include "../Buffer/MTBuffer.h"
#include "../Buffer/MTBufferArray.h"
#include "../RenderState/MTGraphicsPSO.h"
#include "../RenderState/MTComputePSO.h"
#include "../RenderState/MTResourceHeap.h"
#include "../RenderState/MTBuiltinPSOFactory.h"
#include "../RenderState/MTDescriptorCache.h"
#include "../RenderState/MTConstantsCache.h"
#include "../Shader/MTShader.h"
#include "../Texture/MTTexture.h"
#include "../Texture/MTSampler.h"
#include "../Texture/MTRenderTarget.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <limits.h>


namespace LLGL
{


MTMultiSubmitCommandBuffer::MTMultiSubmitCommandBuffer(id<MTLDevice> device, const CommandBufferDescriptor& desc) :
    MTCommandBuffer       { device, desc.flags                                  },
    isSecondaryCmdBuffer_ { ((desc.flags & CommandBufferFlags::Secondary) != 0) }
{
}

MTMultiSubmitCommandBuffer::~MTMultiSubmitCommandBuffer()
{
    ReleaseIntermediateResources();
}

/* ----- Encoding ----- */

void MTMultiSubmitCommandBuffer::Begin()
{
    buffer_.Clear();
    lastOpcode_ = MTOpcodeNop;
    ResetRenderStates();
    ReleaseIntermediateResources();
}

void MTMultiSubmitCommandBuffer::End()
{
    /* Don't flush context nor present drawables in a secondary command buffer */
    if (!isSecondaryCmdBuffer_)
    {
        FlushContext();
        PresentDrawables();
    }
    buffer_.Pack();
}

void MTMultiSubmitCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    if (IsPrimary())
    {
        auto& commandBufferMT = LLGL_CAST(MTCommandBuffer&, secondaryCommandBuffer);
        if (commandBufferMT.IsMultiSubmitCmdBuffer() && !commandBufferMT.IsPrimary())
        {
            auto cmd = AllocCommand<MTCmdExecute>(MTOpcodeExecute);
            {
                cmd->commandBuffer = LLGL_CAST(MTMultiSubmitCommandBuffer*, &commandBufferMT);
            }
        }
    }
}

/* ----- Blitting ----- */

void MTMultiSubmitCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint64_t   dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);

    /* Copy data to staging buffer */
    id<MTLBuffer> srcBuffer = nil;
    NSUInteger srcOffset = 0;

    WriteStagingBuffer(data, static_cast<NSUInteger>(dataSize), srcBuffer, srcOffset);

    /* Encode blit command to copy staging buffer region to destination buffer */
    auto cmd = AllocCommand<MTCmdCopyBuffer>(MTOpcodeCopyBuffer);
    {
        cmd->sourceBuffer       = srcBuffer;
        cmd->sourceOffset       = srcOffset;
        cmd->destinationBuffer  = dstBufferMT.GetNative();
        cmd->destinationOffset  = static_cast<NSUInteger>(dstOffset);
        cmd->size               = static_cast<NSUInteger>(dataSize);
    }
}

void MTMultiSubmitCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    auto cmd = AllocCommand<MTCmdCopyBuffer>(MTOpcodeCopyBuffer);
    {
        cmd->sourceBuffer       = srcBufferMT.GetNative();
        cmd->sourceOffset       = static_cast<NSUInteger>(srcOffset);
        cmd->destinationBuffer  = dstBufferMT.GetNative();
        cmd->destinationOffset  = static_cast<NSUInteger>(dstOffset);
        cmd->size               = static_cast<NSUInteger>(size);
    }
}

void MTMultiSubmitCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcTextureMT = LLGL_CAST(MTTexture&, srcTexture);

    /* Determine actual row and layer strides */
    if (rowStride == 0)
        rowStride = static_cast<std::uint32_t>(srcTextureMT.GetBytesPerRow(srcRegion.extent.width));
    if (layerStride == 0)
        layerStride = rowStride * srcRegion.extent.height;

    /* Convert to Metal origin and size */
    MTLOrigin srcOrigin;
    MTTypes::Convert(srcOrigin, srcRegion.offset);

    MTLSize srcSize;
    MTTypes::Convert(srcSize, srcRegion.extent);

    /* Encode blit commands to copy texture form buffer */
    auto cmd = AllocCommand<MTCmdCopyBufferFromTexture>(MTOpcodeCopyBufferFromTexture);
    {
        cmd->sourceTexture              = srcTextureMT.GetNative();
        cmd->sourceSlice                = srcRegion.subresource.baseArrayLayer;
        cmd->sourceLevel                = srcRegion.subresource.baseMipLevel;
        cmd->sourceOrigin               = srcOrigin;
        cmd->sourceSize                 = srcSize;
        cmd->destinationBuffer          = dstBufferMT.GetNative();
        cmd->destinationOffset          = static_cast<NSUInteger>(dstOffset);
        cmd->destinationBytesPerRow     = rowStride;
        cmd->destinationBytesPerImage   = layerStride;
        cmd->layerCount                 = srcRegion.subresource.numArrayLayers;
    }
}

void MTMultiSubmitCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
#if 0 //TODO
    if (fillSize == 0)
        return;

    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);

    /* Check if native "fillBuffer" command can be used */
    const bool valueBytesAreEqual =
    (
        ((value >> 24) & 0x000000FF) == (value & 0x000000FF) &&
        ((value >> 16) & 0x000000FF) == (value & 0x000000FF) &&
        ((value >>  8) & 0x000000FF) == (value & 0x000000FF)
    );

    /* Determine buffer range for fill command */
    NSRange range;
    if (fillSize == LLGL_WHOLE_SIZE)
    {
        NSUInteger bufferSize = [dstBufferMT.GetNative() length];
        range = NSMakeRange(0, bufferSize);
    }
    else
    {
        range = NSMakeRange(
            static_cast<NSUInteger>(dstOffset),
            static_cast<NSUInteger>(fillSize)
        );
    }

    /* Fill with native command if all four bytes are equal, otherwise use blit and comput commands */
    if (valueBytesAreEqual)
        FillBufferByte1(dstBufferMT, range, static_cast<std::uint8_t>(value & 0x000000FF));
    else
        FillBufferByte4(dstBufferMT, range, value);
#endif
}

void MTMultiSubmitCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureMT = LLGL_CAST(MTTexture&, dstTexture);
    auto& srcTextureMT = LLGL_CAST(MTTexture&, srcTexture);

    MTLOrigin srcOrigin, dstOrigin;
    MTTypes::Convert(srcOrigin, srcLocation.offset);
    MTTypes::Convert(dstOrigin, dstLocation.offset);

    MTLSize srcSize;
    MTTypes::Convert(srcSize, extent);

    auto cmd = AllocCommand<MTCmdCopyTexture>(MTOpcodeCopyTexture);
    {
        cmd->sourceTexture      = srcTextureMT.GetNative();
        cmd->sourceSlice        = srcLocation.arrayLayer;
        cmd->sourceLevel        = srcLocation.mipLevel;
        cmd->sourceOrigin       = srcOrigin;
        cmd->sourceSize         = srcSize;
        cmd->destinationTexture = dstTextureMT.GetNative();
        cmd->destinationSlice   = dstLocation.arrayLayer;
        cmd->destinationLevel   = dstLocation.mipLevel;
        cmd->destinationOrigin  = dstOrigin;
    }
}

void MTMultiSubmitCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureMT = LLGL_CAST(MTTexture&, dstTexture);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    /* Determine actual row and layer strides */
    if (rowStride == 0)
        rowStride = static_cast<std::uint32_t>(dstTextureMT.GetBytesPerRow(dstRegion.extent.width));
    if (layerStride == 0)
        layerStride = rowStride * dstRegion.extent.height;

    /* Convert to Metal origin and size */
    MTLOrigin dstOrigin;
    MTTypes::Convert(dstOrigin, dstRegion.offset);

    MTLSize srcSize;
    MTTypes::Convert(srcSize, dstRegion.extent);

    /* Encode blit commands to copy texture form buffer */
    auto cmd = AllocCommand<MTCmdCopyTextureFromBuffer>(MTOpcodeCopyTextureFromBuffer);
    {
        cmd->sourceBuffer           = srcBufferMT.GetNative();
        cmd->sourceOffset           = static_cast<NSUInteger>(srcOffset);
        cmd->sourceBytesPerRow      = rowStride;
        cmd->sourceBytesPerImage    = layerStride;
        cmd->sourceSize             = srcSize;
        cmd->destinationTexture     = dstTextureMT.GetNative();
        cmd->destinationSlice       = dstRegion.subresource.baseArrayLayer;
        cmd->destinationLevel       = dstRegion.subresource.baseMipLevel;
        cmd->destinationOrigin      = dstOrigin;
        cmd->layerCount             = dstRegion.subresource.numArrayLayers;
    }
}

void MTMultiSubmitCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    if (dstRegion.extent.depth != 1 ||
        dstRegion.offset.x < 0      ||
        dstRegion.offset.y < 0      ||
        dstRegion.offset.z < 0)
    {
        return /*Out of bounds*/;
    }

    auto& dstTextureMT = LLGL_CAST(MTTexture&, dstTexture);
    id<MTLTexture> targetTexture = dstTextureMT.GetNative();

    /* Convert texture region and source origin */
    MTLOrigin srcOrigin = MTLOriginMake(
        static_cast<NSUInteger>(srcOffset.x),
        static_cast<NSUInteger>(srcOffset.y),
        0u
    );

    MTLOrigin dstOrigin;
    MTTypes::Convert(dstOrigin, dstRegion.offset);

    MTLSize srcSize;
    MTTypes::Convert(srcSize, dstRegion.extent);

    auto cmd = AllocCommand<MTCmdCopyTextureFromFramebuffer>(MTOpcodeCopyTextureFromFramebuffer);
    {
        cmd->sourceOrigin       = srcOrigin;
        cmd->sourceSize         = srcSize;
        cmd->destinationTexture = targetTexture;
        cmd->destinationSlice   = dstRegion.subresource.baseArrayLayer;
        cmd->destinationLevel   = dstRegion.subresource.baseMipLevel;
        cmd->destinationOrigin  = dstOrigin;
    }
}

void MTMultiSubmitCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    GenerateMipmapsForTexture(textureMT.GetNative());
}

void MTMultiSubmitCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    if (subresource.numMipLevels > 1)
    {
        // Create temporary subresource texture to generate MIP-maps only on that range
        id<MTLTexture> intermediateTexture = textureMT.CreateSubresourceView(subresource);
        intermediateTextures_.push_back(intermediateTexture);
        GenerateMipmapsForTexture(intermediateTexture);
    }
}

/* ----- Viewport and Scissor ----- */

void MTMultiSubmitCommandBuffer::SetViewport(const Viewport& viewport)
{
    MTMultiSubmitCommandBuffer::SetViewports(1, &viewport);
}

void MTMultiSubmitCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    auto cmd = AllocCommand<MTCmdSetViewports>(MTOpcodeSetViewports, sizeof(Viewport)*numViewports);
    {
        cmd->count = numViewports;
        ::memcpy(cmd + 1, viewports, sizeof(Viewport)*numViewports);
    }
}

void MTMultiSubmitCommandBuffer::SetScissor(const Scissor& scissor)
{
    MTMultiSubmitCommandBuffer::SetScissors(1, &scissor);
}

void MTMultiSubmitCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    auto cmd = AllocCommand<MTCmdSetScissorRects>(MTOpcodeSetScissorRects, sizeof(Scissor)*numScissors);
    {
        cmd->count = numScissors;
        ::memcpy(cmd + 1, scissors, sizeof(Scissor)*numScissors);
    }
}

/* ----- Input Assembly ------ */

//private
void MTMultiSubmitCommandBuffer::SetNativeVertexBuffers(NSUInteger count, const id<MTLBuffer>* buffers, const NSUInteger* offsets)
{
    auto cmd = AllocCommand<MTCmdSetVertexBuffers>(MTOpcodeSetVertexBuffers, (sizeof(id) + sizeof(NSUInteger))*count);
    {
        cmd->count = count;
        ::memcpy(cmd + 1, buffers, sizeof(id)*count);
        ::memcpy(reinterpret_cast<char*>(cmd + 1) + sizeof(id)*count, offsets, sizeof(NSUInteger)*count);
    }
}

void MTMultiSubmitCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    id<MTLBuffer> bufferId = bufferMT.GetNative();
    const NSUInteger bufferOffset = 0;
    SetNativeVertexBuffers(1, &bufferId, &bufferOffset);
}

void MTMultiSubmitCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayMT = LLGL_CAST(MTBufferArray&, bufferArray);
    SetNativeVertexBuffers(
        static_cast<NSUInteger>(bufferArrayMT.GetIDArray().size()),
        bufferArrayMT.GetIDArray().data(),
        bufferArrayMT.GetOffsets().data()
    );
}

//private
void MTMultiSubmitCommandBuffer::SetNativeIndexBuffer(id<MTLBuffer> buffer, NSUInteger offset, bool indexType16Bits)
{
    auto cmd = AllocCommand<MTCmdSetIndexBuffer>(MTOpcodeSetIndexBuffer);
    {
        cmd->buffer             = buffer;
        cmd->offset             = offset;
        cmd->indexType16Bits    = indexType16Bits;
    }
}

void MTMultiSubmitCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    SetNativeIndexBuffer(bufferMT.GetNative(), 0, bufferMT.IsIndexType16Bits());
}

void MTMultiSubmitCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    SetNativeIndexBuffer(bufferMT.GetNative(), static_cast<NSUInteger>(offset), (format == Format::R16UInt));
}

/* ----- Resources ----- */

void MTMultiSubmitCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    auto cmd = AllocCommand<MTCmdSetResourceHeap>(MTOpcodeSetResourceHeap);
    {
        cmd->resourceHeap   = &resourceHeapMT;
        cmd->descriptorSet  = descriptorSet;
    }
}

void MTMultiSubmitCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    auto cmd = AllocCommand<MTCmdSetResource>(MTOpcodeSetResource);
    {
        cmd->descriptor = descriptor;
        cmd->resource   = &resource;
    }
}

void MTMultiSubmitCommandBuffer::ResourceBarrier(
    std::uint32_t       /*numBuffers*/,
    Buffer* const *     /*buffers*/,
    std::uint32_t       /*numTextures*/,
    Texture* const *    /*textures*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void MTMultiSubmitCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        /* Put current drawable into queue */
        auto& swapChainMT = LLGL_CAST(MTSwapChain&, renderTarget);
        QueueDrawable(swapChainMT.GetMTKView());
    }

    /* Only allocate payload for clear values if a render pass was specified */
    auto cmd = AllocCommand<MTCmdBeginRenderPass>(MTOpcodeBeginRenderPass, sizeof(ClearValue)*(renderPass != nullptr ? numClearValues : 0));
    {
        cmd->renderTarget = &renderTarget;
        if (renderPass != nullptr)
        {
            cmd->renderPass     = LLGL_CAST(const MTRenderPass*, renderPass);
            cmd->numClearValues = numClearValues;
            ::memcpy(cmd + 1, clearValues, sizeof(ClearValue)*numClearValues);
        }
        else
        {
            cmd->renderPass     = nullptr;
            cmd->numClearValues = 0;
        }
    }
}

void MTMultiSubmitCommandBuffer::EndRenderPass()
{
    AllocOpcode(MTOpcodeEndRenderPass);
}

//TODO: support clearing all active attachments at once
void MTMultiSubmitCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    AttachmentClear attachmentClear;
    {
        attachmentClear.flags           = flags;
        attachmentClear.colorAttachment = 0;
        attachmentClear.clearValue      = clearValue;
    }
    MTMultiSubmitCommandBuffer::ClearAttachments(1, &attachmentClear);
}

// Fills the virtual command MTCmdClearRenderPass with the specified attachment clear values
static void FillCmdClearRenderPass(MTCmdClearRenderPass* cmd, const AttachmentClear& attachment)
{
    cmd->flags |= attachment.flags;

    if ((attachment.flags & ClearFlags::Color) != 0)
    {
        /* Clear color buffer */
        auto colorBuffers = reinterpret_cast<std::uint32_t*>(cmd + 1);
        auto clearColors = reinterpret_cast<MTLClearColor*>(colorBuffers + cmd->numAttachments);

        colorBuffers[cmd->numColorAttachments]  = attachment.colorAttachment;
        clearColors[cmd->numColorAttachments]   = MTTypes::ToMTLClearColor(attachment.clearValue.color);

        cmd->numColorAttachments++;
    }

    if ((attachment.flags & ClearFlags::Depth) != 0)
    {
        /* Clear depth buffer */
        cmd->clearDepth = static_cast<double>(attachment.clearValue.depth);
    }

    if ((attachment.flags & ClearFlags::Stencil) != 0)
    {
        /* Clear stencil buffer */
        cmd->clearStencil = attachment.clearValue.stencil;
    }
}

void MTMultiSubmitCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (numAttachments == 0 || attachments == nullptr)
        return;

    auto cmd = AllocCommand<MTCmdClearRenderPass>(MTOpcodeClearRenderPass, (sizeof(std::uint32_t) + sizeof(MTLClearColor))*numAttachments);
    {
        cmd->flags                  = 0;
        cmd->clearDepth             = 0.0;
        cmd->clearStencil           = 0u;
        cmd->numAttachments         = numAttachments;
        cmd->numColorAttachments    = 0u;
        for_range(i, numAttachments)
            FillCmdClearRenderPass(cmd, attachments[i]);
    }
}

/* ----- Pipeline States ----- */

void MTMultiSubmitCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto& pipelineStateMT = LLGL_CAST(MTPipelineState&, pipelineState);
    if (pipelineStateMT.IsGraphicsPSO())
    {
        /* Set graphics pipeline with encoder scheduler */
        auto cmd = AllocCommand<MTCmdSetGraphicsPSO>(MTOpcodeSetGraphicsPSO);
        cmd->graphicsPSO = LLGL_CAST(MTGraphicsPSO*, &pipelineStateMT);
    }
    else
    {
        /* Set compute pipeline with encoder scheduler */
        auto cmd = AllocCommand<MTCmdSetComputePSO>(MTOpcodeSetComputePSO);
        cmd->computePSO = LLGL_CAST(MTComputePSO*, &pipelineStateMT);
    }
}

void MTMultiSubmitCommandBuffer::SetBlendFactor(const float color[4])
{
    auto cmd = AllocCommand<MTCmdSetBlendColor>(MTOpcodeSetBlendColor);
    {
        cmd->blendColor[0] = color[0];
        cmd->blendColor[1] = color[1];
        cmd->blendColor[2] = color[2];
        cmd->blendColor[3] = color[3];
    }
}

void MTMultiSubmitCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    auto cmd = AllocCommand<MTCmdSetStencilRef>(MTOpcodeSetStencilRef);
    {
        cmd->ref    = reference;
        cmd->face   = stencilFace;
    }
}

void MTMultiSubmitCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    auto cmd = AllocCommand<MTCmdSetUniforms>(MTOpcodeSetUniforms, dataSize);
    {
        cmd->first      = first;
        cmd->dataSize   = dataSize;
        ::memcpy(cmd + 1, data, dataSize);
    }
}

/* ----- Queries ----- */

void MTMultiSubmitCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //todo
}

void MTMultiSubmitCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //todo
}

void MTMultiSubmitCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //todo
}

void MTMultiSubmitCommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Stream Output ------ */

void MTMultiSubmitCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    //todo
}

void MTMultiSubmitCommandBuffer::EndStreamOutput()
{
    //todo
}

/* ----- Drawing ----- */

void MTMultiSubmitCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    auto cmd = AllocCommand<MTCmdDraw>(MTOpcodeDraw);
    {
        cmd->vertexStart    = static_cast<NSUInteger>(firstVertex);
        cmd->vertexCount    = static_cast<NSUInteger>(numVertices);
        cmd->instanceCount  = 1;
        cmd->baseInstance   = 0;
    }
}

void MTMultiSubmitCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    auto cmd = AllocCommand<MTCmdDrawIndexed>(MTOpcodeDrawIndexed);
    {
        cmd->indexCount     = static_cast<NSUInteger>(numIndices);
        cmd->firstIndex     = static_cast<NSUInteger>(firstIndex);
        cmd->instanceCount  = 1;
        cmd->baseVertex     = 0;
        cmd->baseInstance   = 0;
    }
}

void MTMultiSubmitCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    MTMultiSubmitCommandBuffer::DrawIndexedInstanced(numIndices, /*numInstances:*/ 1, firstIndex, vertexOffset, /*firstInstance:*/ 0);
}

void MTMultiSubmitCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    MTMultiSubmitCommandBuffer::DrawInstanced(numVertices, firstVertex, numInstances, /*firstInstance:*/ 0);
}

void MTMultiSubmitCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    auto cmd = AllocCommand<MTCmdDraw>(MTOpcodeDraw);
    {
        cmd->vertexStart    = static_cast<NSUInteger>(firstVertex);
        cmd->vertexCount    = static_cast<NSUInteger>(numVertices);
        cmd->instanceCount  = static_cast<NSUInteger>(numInstances);
        cmd->baseInstance   = static_cast<NSUInteger>(firstInstance);
    }
}

void MTMultiSubmitCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    MTMultiSubmitCommandBuffer::DrawIndexedInstanced(numIndices, numInstances, firstIndex, /*vertexOffset:*/ 0, /*firstInstance:*/ 0);
}

void MTMultiSubmitCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    MTMultiSubmitCommandBuffer::DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, /*firstInstance:*/ 0);
}

void MTMultiSubmitCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    auto cmd = AllocCommand<MTCmdDrawIndexed>(MTOpcodeDrawIndexed);
    {
        cmd->indexCount     = static_cast<NSUInteger>(numIndices);
        cmd->firstIndex     = static_cast<NSUInteger>(firstIndex);
        cmd->instanceCount  = static_cast<NSUInteger>(numInstances);
        cmd->baseVertex     = static_cast<NSUInteger>(vertexOffset);
        cmd->baseInstance   = static_cast<NSUInteger>(firstInstance);
    }
}

#if 0 //TODO
[[noreturn]]
static void TrapIndirectPatchesNotSupported()
{
    LLGL_TRAP("tessellation with indirect arguments not supported in Metal backend yet");
}
#endif

//TODO: support patches with indirect arguments
void MTMultiSubmitCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
#if 0 //TODO
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        [renderEncoder
            drawPatches:            numPatchControlPoints
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
        #else
        TrapIndirectPatchesNotSupported();
        #endif
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawPrimitives:         GetPrimitiveType()
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
#endif
}

//TODO: support patches with indirect arguments
void MTMultiSubmitCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
#if 0 //TODO
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawPatches:            numPatchControlPoints
                patchIndexBuffer:       nil
                patchIndexBufferOffset: 0
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
        #else
        TrapIndirectPatchesNotSupported();
        #endif
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawPrimitives:         GetPrimitiveType()
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
#endif
}

//TODO: support patches with indirect arguments
void MTMultiSubmitCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
#if 0 //TODO
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        GetIndexBuffer()
            controlPointIndexBufferOffset:  0
            indirectBuffer:                 bufferMT.GetNative()
            indirectBufferOffset:           static_cast<NSUInteger>(offset)
        ];
        #else
        TrapIndirectPatchesNotSupported();
        #endif
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawIndexedPrimitives:  GetPrimitiveType()
            indexType:              GetIndexType()
            indexBuffer:            GetIndexBuffer()
            indexBufferOffset:      0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
#endif
}

//TODO: support patches with indirect arguments
void MTMultiSubmitCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
#if 0 //TODO
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawIndexedPatches:             numPatchControlPoints
                patchIndexBuffer:               nil
                patchIndexBufferOffset:         0
                controlPointIndexBuffer:        GetIndexBuffer()
                controlPointIndexBufferOffset:  0
                indirectBuffer:                 bufferMT.GetNative()
                indirectBufferOffset:           static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
        #else
        TrapIndirectPatchesNotSupported();
        #endif
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawIndexedPrimitives:  GetPrimitiveType()
                indexType:              GetIndexType()
                indexBuffer:            GetIndexBuffer()
                indexBufferOffset:      0
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
#endif
}

void MTMultiSubmitCommandBuffer::DrawStreamOutput()
{
    LLGL_TRAP("stream-outputs not supported");
}

/* ----- Compute ----- */

void MTMultiSubmitCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    auto cmd = AllocCommand<MTCmdDispatchThreads>(MTOpcodeDispatchThreadgroups);
    {
        cmd->threadgroups = MTLSizeMake(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
    }
}

void MTMultiSubmitCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto cmd = AllocCommand<MTCmdDispatchThreadsIndirect>(MTOpcodeDispatchThreadgroupsIndirect);
    {
        cmd->indirectBuffer         = bufferMT.GetNative();
        cmd->indirectBufferOffset   = static_cast<NSUInteger>(offset);
    }
}

/* ----- Debugging ----- */

void MTMultiSubmitCommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef LLGL_DEBUG
    const std::size_t nameLength = ::strlen(name);
    auto cmd = AllocCommand<MTCmdPushDebugGroup>(MTOpcodePushDebugGroup, nameLength + 1);
    {
        cmd->length = static_cast<NSUInteger>(nameLength);
        ::memcpy(cmd + 1, name, nameLength + 1);
    }
    #endif // /LLGL_DEBUG
}

void MTMultiSubmitCommandBuffer::PopDebugGroup()
{
    #ifdef LLGL_DEBUG
    AllocOpcode(MTOpcodePopDebugGroup);
    #endif // /LLGL_DEBUG
}

/* ----- Extensions ----- */

void MTMultiSubmitCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool MTMultiSubmitCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return false; // dummy - command is only allowed in immediate command buffer
}


/*
 * ======= Internal: =======
 */

bool MTMultiSubmitCommandBuffer::IsMultiSubmitCmdBuffer() const
{
    return true; // always true for MTMultiSubmitCommandBuffer
}


/*
 * ======= Private: =======
 */

void MTMultiSubmitCommandBuffer::QueueDrawable(MTKView* view)
{
    for (MTKView* v : views_)
    {
        if (v == view)
            return;
    }
    views_.push_back(view);
}

void MTMultiSubmitCommandBuffer::PresentDrawables()
{
    if (!views_.empty())
    {
        auto cmd = AllocCommand<MTCmdPresentDrawables>(MTOpcodePresentDrawables, sizeof(id)*views_.size());
        {
            cmd->count = static_cast<NSUInteger>(views_.size());
            ::memcpy(cmd + 1, views_.data(), sizeof(id)*views_.size());
        }
    }
    views_.clear();
}

#if 0 //TODO
void MTMultiSubmitCommandBuffer::FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value)
{
    context_.PauseRenderEncoder();
    {
        auto blitEncoder = context_.BindBlitEncoder();
        [blitEncoder fillBuffer:bufferMT.GetNative() range:range value:value];
    }
    context_.ResumeRenderEncoder();
}

void MTMultiSubmitCommandBuffer::FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Use emulated fill command if buffer range is small enough to avoid having both a blit and compute encoder */
    if (range.length > g_minFillBufferForKernel)
        FillBufferByte4Accelerated(bufferMT, range, value);
    else
        FillBufferByte4Emulated(bufferMT, range, value);
}

void MTMultiSubmitCommandBuffer::FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Copy value into stack local buffer */
    std::uint32_t localBuffer[g_minFillBufferForKernel / sizeof(std::uint32_t)];
    std::fill(std::begin(localBuffer), std::end(localBuffer), value);

    /* Write clear value into first range of destination buffer */
    UpdateBuffer(bufferMT, range.location, localBuffer, range.length);
}

//TODO: manage binding of compute PSO in MTCommandContext
void MTMultiSubmitCommandBuffer::FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    context_.PauseRenderEncoder();
    {
        auto computeEncoder = context_.BindComputeEncoder();

        /* Bind compute PSO with kernel to fill buffer */
        id<MTLComputePipelineState> pso = MTBuiltinPSOFactory::Get().GetComputePSO(MTBuiltinComputePSO::FillBufferByte4);
        [computeEncoder setComputePipelineState:pso];

        /* Bind destination buffer range and store clear value as input constant buffer */
        [computeEncoder setBuffer:bufferMT.GetNative() offset:range.location atIndex:0];
        [computeEncoder setBytes:&value length:sizeof(value) atIndex:1];

        /* Dispatch compute kernels */
        const NSUInteger numValues = range.length / sizeof(std::uint32_t);
        DispatchThreads1D(computeEncoder, pso, numValues);
    }
    context_.ResumeRenderEncoder();
}
#endif //TODO

void MTMultiSubmitCommandBuffer::GenerateMipmapsForTexture(id<MTLTexture> texture)
{
    if ([texture mipmapLevelCount] > 1)
    {
        auto cmd = AllocCommand<MTCmdGenerateMipmaps>(MTOpcodeGenerateMipmaps);
        {
            cmd->texture = texture;
        }
    }
}

void MTMultiSubmitCommandBuffer::FlushContext()
{
    AllocOpcode(MTOpcodeFlush);
}

void MTMultiSubmitCommandBuffer::ReleaseIntermediateResources()
{
    for (id<MTLTexture> tex : intermediateTextures_)
        [tex release];
    intermediateTextures_.clear();
}

void MTMultiSubmitCommandBuffer::AllocOpcode(const MTOpcode opcode)
{
    /* Redundant single-opcode instructions can be ignored (such as MTOpcodeFlush) */
    if (lastOpcode_ != opcode)
    {
        buffer_.AllocOpcode(opcode);
        lastOpcode_ = opcode;
    }
}

template <typename TCommand>
TCommand* MTMultiSubmitCommandBuffer::AllocCommand(const MTOpcode opcode, std::size_t payloadSize)
{
    lastOpcode_ = opcode;
    return buffer_.AllocCommand<TCommand>(opcode, payloadSize);
}


} // /namespace LLGL



// ================================================================================
