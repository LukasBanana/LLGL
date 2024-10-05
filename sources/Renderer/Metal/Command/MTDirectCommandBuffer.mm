/*
 * MTDirectCommandBuffer.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTDirectCommandBuffer.h"
#include "MTMultiSubmitCommandBuffer.h"
#include "MTCommandExecutor.h"
#include "MTCommandQueue.h"
#include "../MTSwapChain.h"
#include "../MTTypes.h"
#include "../Buffer/MTBuffer.h"
#include "../Buffer/MTBufferArray.h"
#include "../RenderState/MTGraphicsPSO.h"
#include "../RenderState/MTComputePSO.h"
#include "../RenderState/MTQueryHeap.h"
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

#include <LLGL/Backend/Metal/NativeHandle.h>
#include <LLGL/Backend/Metal/NativeCommand.h>


namespace LLGL
{


/*
Minimum size for the "FillBuffer" command to use a GPU kernel.
for smaller buffers an emulated CPU copy operation will be used.
*/
static const NSUInteger g_minFillBufferForKernel = 64;

MTDirectCommandBuffer::MTDirectCommandBuffer(id<MTLDevice> device, MTCommandQueue& cmdQueue, const CommandBufferDescriptor& desc) :
    MTCommandBuffer    { device, desc.flags },
    cmdQueue_          { cmdQueue           },
    context_           { device             }
{
    cmdBufferSemaphore_ = dispatch_semaphore_create(MTCommandBuffer::maxNumCommandBuffersInFlight);
}

/* ----- Encoding ----- */

void MTDirectCommandBuffer::Begin()
{
    /*
    If the previous encoded command buffer was submitted to the command queue,
    we have to wait until our in-flight resources (such as staging buffer pool) become available again.
    */
    if (!cmdBufferDirty_)
    {
        dispatch_semaphore_wait(cmdBufferSemaphore_, DISPATCH_TIME_FOREVER);
        cmdBufferDirty_ = true;
    }

    /* Allocate new command buffer from command queue */
    cmdBuffer_ = [cmdQueue_.GetNative() commandBuffer];

    /* Append complete handler to signal semaphore */
    __block dispatch_semaphore_t blockSemaphore = cmdBufferSemaphore_;
    [cmdBuffer_
        addCompletedHandler:^(id<MTLCommandBuffer> cmdBuffer)
        {
            dispatch_semaphore_signal(blockSemaphore);
        }
    ];

    /* Reset schedulers and pools */
    context_.Reset(cmdBuffer_);
    ResetStagingPool();
}

void MTDirectCommandBuffer::End()
{
    context_.Flush();
    PresentDrawables();

    /* Commit native buffer right after encoding for immediate command buffers */
    if (IsImmediateCmdBuffer())
    {
        MarkSubmitted();
        cmdQueue_.SubmitCommandBuffer(GetNative());
    }

    ResetRenderStates();
}

void MTDirectCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    if (IsPrimary())
    {
        auto& commandBufferMT = LLGL_CAST(MTCommandBuffer&, secondaryCommandBuffer);
        if (commandBufferMT.IsMultiSubmitCmdBuffer() && !commandBufferMT.IsPrimary())
        {
            auto& multiSubmitCommandBufferMT = LLGL_CAST(MTMultiSubmitCommandBuffer&, commandBufferMT);
            ExecuteMTMultiSubmitCommandBuffer(multiSubmitCommandBufferMT, context_);
        }
    }
}

/* ----- Blitting ----- */

void MTDirectCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);

    /* Copy data to staging buffer */
    id<MTLBuffer> srcBuffer = nil;
    NSUInteger srcOffset = 0;

    WriteStagingBuffer(data, static_cast<NSUInteger>(dataSize), srcBuffer, srcOffset);

    /* Encode blit command to copy staging buffer region to destination buffer */
    auto blitEncoder = context_.BindBlitEncoder();
    [blitEncoder
        copyFromBuffer:     srcBuffer
        sourceOffset:       srcOffset
        toBuffer:           dstBufferMT.GetNative()
        destinationOffset:  static_cast<NSUInteger>(dstOffset)
        size:               static_cast<NSUInteger>(dataSize)
    ];
}

void MTDirectCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    auto blitEncoder = context_.BindBlitEncoder();
    [blitEncoder
        copyFromBuffer:     srcBufferMT.GetNative()
        sourceOffset:       static_cast<NSUInteger>(srcOffset)
        toBuffer:           dstBufferMT.GetNative()
        destinationOffset:  static_cast<NSUInteger>(dstOffset)
        size:               static_cast<NSUInteger>(size)
    ];
}

void MTDirectCommandBuffer::CopyBufferFromTexture(
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
    auto blitEncoder = context_.BindBlitEncoder();
    for_range(arrayLayer, srcRegion.subresource.numArrayLayers)
    {
        [blitEncoder
            copyFromTexture:            srcTextureMT.GetNative()
            sourceSlice:                srcRegion.subresource.baseArrayLayer + arrayLayer
            sourceLevel:                srcRegion.subresource.baseMipLevel
            sourceOrigin:               srcOrigin
            sourceSize:                 srcSize
            toBuffer:                   dstBufferMT.GetNative()
            destinationOffset:          static_cast<NSUInteger>(dstOffset)
            destinationBytesPerRow:     rowStride
            destinationBytesPerImage:   layerStride
        ];
        dstOffset += layerStride;
    }
}

void MTDirectCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
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
}

void MTDirectCommandBuffer::CopyTexture(
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

    auto blitEncoder = context_.BindBlitEncoder();
    [blitEncoder
        copyFromTexture:    srcTextureMT.GetNative()
        sourceSlice:        srcLocation.arrayLayer
        sourceLevel:        srcLocation.mipLevel
        sourceOrigin:       srcOrigin
        sourceSize:         srcSize
        toTexture:          dstTextureMT.GetNative()
        destinationSlice:   dstLocation.arrayLayer
        destinationLevel:   dstLocation.mipLevel
        destinationOrigin:  dstOrigin
    ];
}

void MTDirectCommandBuffer::CopyTextureFromBuffer(
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
    auto blitEncoder = context_.BindBlitEncoder();
    for_range(arrayLayer, dstRegion.subresource.numArrayLayers)
    {
        [blitEncoder
            copyFromBuffer:         srcBufferMT.GetNative()
            sourceOffset:           static_cast<NSUInteger>(srcOffset)
            sourceBytesPerRow:      rowStride
            sourceBytesPerImage:    layerStride
            sourceSize:             srcSize
            toTexture:              dstTextureMT.GetNative()
            destinationSlice:       dstRegion.subresource.baseArrayLayer + arrayLayer
            destinationLevel:       dstRegion.subresource.baseMipLevel
            destinationOrigin:      dstOrigin
        ];
        srcOffset += layerStride;
    }
}

void MTDirectCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    id<MTLTexture> drawableTexture = GetCurrentDrawableTexture();
    if (drawableTexture == nil)
        return /*No drawable source texture*/;

    if (dstRegion.extent.depth != 1 ||
        dstRegion.offset.x < 0      ||
        dstRegion.offset.y < 0      ||
        dstRegion.offset.z < 0)
    {
        return /*Out of bounds*/;
    }

    auto& dstTextureMT = LLGL_CAST(MTTexture&, dstTexture);
    id<MTLTexture> targetTexture = dstTextureMT.GetNative();

    /* Source and target texture formats must match for 'copyFromTexture', so create texture view on mismatch */
    id<MTLTexture> sourceTexture;
    const bool isTextureViewRequired = ([drawableTexture pixelFormat] != [targetTexture pixelFormat]);
    if (isTextureViewRequired)
        sourceTexture = [drawableTexture newTextureViewWithPixelFormat:[targetTexture pixelFormat]];
    else
        sourceTexture = drawableTexture;

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

    auto blitEncoder = context_.BindBlitEncoder();
    [blitEncoder
        copyFromTexture:    sourceTexture
        sourceSlice:        0
        sourceLevel:        0
        sourceOrigin:       srcOrigin
        sourceSize:         srcSize
        toTexture:          targetTexture
        destinationSlice:   dstRegion.subresource.baseArrayLayer
        destinationLevel:   dstRegion.subresource.baseMipLevel
        destinationOrigin:  dstOrigin
    ];

    /* Decrement reference counter for temporary texture */
    if (isTextureViewRequired)
        [sourceTexture release];
}

void MTDirectCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    if ([textureMT.GetNative() mipmapLevelCount] > 1)
    {
        auto blitEncoder = context_.BindBlitEncoder();
        [blitEncoder generateMipmapsForTexture:textureMT.GetNative()];
    }
}

void MTDirectCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    if (subresource.numMipLevels > 1)
    {
        auto& textureMT = LLGL_CAST(MTTexture&, texture);

        // Create temporary subresource texture to generate MIP-maps only on that range
        id<MTLTexture> intermediateTexture = textureMT.CreateSubresourceView(subresource);

        auto blitEncoder = context_.BindBlitEncoder();
        [blitEncoder generateMipmapsForTexture:intermediateTexture];

        [intermediateTexture release];
    }
}

/* ----- Viewport and Scissor ----- */

void MTDirectCommandBuffer::SetViewport(const Viewport& viewport)
{
    context_.SetViewports(&viewport, 1u);
}

void MTDirectCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    context_.SetViewports(viewports, numViewports);
}

void MTDirectCommandBuffer::SetScissor(const Scissor& scissor)
{
    context_.SetScissorRects(&scissor, 1u);
}

void MTDirectCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    context_.SetScissorRects(scissors, numScissors);
}

/* ----- Input Assembly ------ */

void MTDirectCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    context_.SetVertexBuffer(bufferMT.GetNative(), 0);
}

void MTDirectCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayMT = LLGL_CAST(MTBufferArray&, bufferArray);
    context_.SetVertexBuffers(
        bufferArrayMT.GetIDArray().data(),
        bufferArrayMT.GetOffsets().data(),
        static_cast<NSUInteger>(bufferArrayMT.GetIDArray().size())
    );
}

void MTDirectCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    context_.SetIndexStream(bufferMT.GetNative(), 0, bufferMT.IsIndexType16Bits());
}

void MTDirectCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    context_.SetIndexStream(bufferMT.GetNative(), static_cast<NSUInteger>(offset), (format == Format::R16UInt));
}

/* ----- Resources ----- */

void MTDirectCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    MTPipelineState* boundPipelineState = context_.GetBoundPipelineState();
    if (boundPipelineState == nullptr)
        return /*Invalid state*/;

    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    if (boundPipelineState->IsGraphicsPSO())
    {
        if (resourceHeapMT.HasGraphicsResources())
            context_.SetGraphicsResourceHeap(&resourceHeapMT, descriptorSet);
    }
    else
    {
        if (resourceHeapMT.HasComputeResources())
            context_.SetComputeResourceHeap(&resourceHeapMT, descriptorSet);
    }
}

void MTDirectCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    context_.SetResource(descriptor, resource);
}

/* ----- Render Passes ----- */

void MTDirectCommandBuffer::BeginRenderPass(
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
        QueueDrawable(swapChainMT.GetMTKView().currentDrawable);
    }

    /* Get next render pass descriptor from MetalKit view */
    auto* renderPassMT = LLGL_CAST(const MTRenderPass*, renderPass);
    context_.BeginRenderPass(&renderTarget, renderPassMT, numClearValues, clearValues);
}

void MTDirectCommandBuffer::EndRenderPass()
{
    context_.EndRenderPass();
}

void MTDirectCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if ((flags & ClearFlags::All) == 0)
        return; // nothing to do

    /* Make new render pass descriptor with current clear values */
    MTLRenderPassDescriptor* renderPassDesc = context_.CopyRenderPassDesc();

    if ((flags & ClearFlags::Color) != 0)
    {
        renderPassDesc.colorAttachments[0].loadAction   = MTLLoadActionClear;
        renderPassDesc.colorAttachments[0].clearColor   = MTTypes::ToMTLClearColor(clearValue.color);
    }

    if ((flags & ClearFlags::Depth) != 0)
    {
        renderPassDesc.depthAttachment.loadAction       = MTLLoadActionClear;
        renderPassDesc.depthAttachment.clearDepth       = static_cast<double>(clearValue.depth);
    }

    if ((flags & ClearFlags::Stencil) != 0)
    {
        renderPassDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
        renderPassDesc.stencilAttachment.clearStencil   = clearValue.stencil;
    }

    /* Begin with new render pass to clear buffers */
    context_.UpdateRenderPass(renderPassDesc);
    [renderPassDesc release];
}

// Fills the MTLRenderPassDescriptor object according to the specified attachment clear command
static void FillMTRenderPassDesc(MTLRenderPassDescriptor* renderPassDesc, const AttachmentClear& attachment)
{
    if ((attachment.flags & ClearFlags::Color) != 0)
    {
        /* Clear color buffer */
        const std::uint32_t colorBufferIndex = attachment.colorAttachment;
        renderPassDesc.colorAttachments[colorBufferIndex].loadAction = MTLLoadActionClear;
        renderPassDesc.colorAttachments[colorBufferIndex].clearColor = MTTypes::ToMTLClearColor(attachment.clearValue.color);
    }
    
    if ((attachment.flags & ClearFlags::Depth) != 0)
    {
        /* Clear depth buffer */
        renderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
        renderPassDesc.depthAttachment.clearDepth = static_cast<double>(attachment.clearValue.depth);
    }

    if ((attachment.flags & ClearFlags::Stencil) != 0)
    {
        /* Clear stencil buffer */
        renderPassDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
        renderPassDesc.stencilAttachment.clearStencil   = attachment.clearValue.stencil;
    }
}

void MTDirectCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (numAttachments == 0 || attachments == nullptr)
        return; // nothing to do

    /* Make new render pass descriptor with current clear values */
    auto renderPassDesc = context_.CopyRenderPassDesc();

    for_range(i, numAttachments)
        FillMTRenderPassDesc(renderPassDesc, attachments[i]);

    /* Begin with new render pass to clear buffers */
    context_.UpdateRenderPass(renderPassDesc);
    [renderPassDesc release];
}

/* ----- Pipeline States ----- */

void MTDirectCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto& pipelineStateMT = LLGL_CAST(MTPipelineState&, pipelineState);
    if (pipelineStateMT.IsGraphicsPSO())
    {
        /* Set graphics pipeline with encoder scheduler */
        auto& graphicsPSO = LLGL_CAST(MTGraphicsPSO&, pipelineStateMT);
        context_.SetGraphicsPSO(&graphicsPSO);
    }
    else
    {
        /* Set compute pipeline with encoder scheduler */
        auto& computePSO = LLGL_CAST(MTComputePSO&, pipelineStateMT);
        context_.SetComputePSO(&computePSO);
    }
}

void MTDirectCommandBuffer::SetBlendFactor(const float color[4])
{
    context_.SetBlendColor(color);
}

void MTDirectCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    context_.SetStencilRef(reference, stencilFace);
}

void MTDirectCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    context_.SetUniforms(first, data, dataSize);
}

/* ----- Queries ----- */

void MTDirectCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapMT = LLGL_CAST(MTQueryHeap&, queryHeap);
    const MTLVisibilityResultMode mode = queryHeapMT.GetVisibilityResultMode();
    if (mode != MTLVisibilityResultModeDisabled && query < queryHeapMT.GetNumQueries())
    {
        const NSUInteger offset = queryHeapMT.GetStride() * query;
        context_.SetVisibilityBuffer(queryHeapMT.GetNative(), queryHeapMT.GetVisibilityResultMode(), offset);
    }
}

void MTDirectCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapMT = LLGL_CAST(MTQueryHeap&, queryHeap);
    const MTLVisibilityResultMode mode = queryHeapMT.GetVisibilityResultMode();
    if (mode != MTLVisibilityResultModeDisabled && query < queryHeapMT.GetNumQueries())
        context_.SetVisibilityBuffer(nil, MTLVisibilityResultModeDisabled, 0);
}

void MTDirectCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //todo
}

void MTDirectCommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Stream Output ------ */

void MTDirectCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    //todo
}

void MTDirectCommandBuffer::EndStreamOutput()
{
    //todo
}

/* ----- Drawing ----- */

void MTDirectCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        const NSUInteger firstPatch = (static_cast<NSUInteger>(firstVertex) / numPatchControlPoints);
        const NSUInteger numPatches = (static_cast<NSUInteger>(numVertices) / numPatchControlPoints);

        auto renderEncoder = context_.DispatchTessellationAndGetRenderEncoder(numPatches);
        [renderEncoder
            drawPatches:            numPatchControlPoints
            patchStart:             firstPatch
            patchCount:             numPatches
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          1
            baseInstance:           0
        ];
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawPrimitives: context_.GetPrimitiveType()
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
        ];
    }
}

void MTDirectCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        const NSUInteger firstPatch = (static_cast<NSUInteger>(firstIndex) / numPatchControlPoints);
        const NSUInteger numPatches = (static_cast<NSUInteger>(numIndices) / numPatchControlPoints);

        auto renderEncoder = context_.DispatchTessellationAndGetRenderEncoder(numPatches);
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints
            patchStart:                     firstPatch
            patchCount:                     numPatches
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        context_.GetIndexBuffer()
            controlPointIndexBufferOffset:  context_.GetIndexBufferOffset(static_cast<NSUInteger>(firstIndex))
            instanceCount:                  1
            baseInstance:                   0
        ];
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawIndexedPrimitives:  context_.GetPrimitiveType()
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              context_.GetIndexType()
            indexBuffer:            context_.GetIndexBuffer()
            indexBufferOffset:      context_.GetIndexBufferOffset(static_cast<NSUInteger>(firstIndex))
        ];
    }
}

void MTDirectCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    MTDirectCommandBuffer::DrawIndexedInstanced(numIndices, /*numInstances:*/ 1, firstIndex, vertexOffset, /*firstInstance:*/ 0);
}

void MTDirectCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        const NSUInteger firstPatch = (static_cast<NSUInteger>(firstVertex) / numPatchControlPoints);
        const NSUInteger numPatches = (static_cast<NSUInteger>(numVertices) / numPatchControlPoints);

        auto renderEncoder = context_.DispatchTessellationAndGetRenderEncoder(numPatches, numInstances);
        [renderEncoder
            drawPatches:            numPatchControlPoints
            patchStart:             firstPatch
            patchCount:             numPatches
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseInstance:           0
        ];
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawPrimitives: context_.GetPrimitiveType()
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
            instanceCount:  static_cast<NSUInteger>(numInstances)
        ];
    }
}

void MTDirectCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        const NSUInteger firstPatch = (static_cast<NSUInteger>(firstVertex) / numPatchControlPoints);
        const NSUInteger numPatches = (static_cast<NSUInteger>(numVertices) / numPatchControlPoints);

        auto renderEncoder = context_.DispatchTessellationAndGetRenderEncoder(numPatches, numInstances);
        [renderEncoder
            drawPatches:            numPatchControlPoints
            patchStart:             firstPatch
            patchCount:             numPatches
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseInstance:           static_cast<NSUInteger>(firstInstance)
        ];
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawPrimitives: context_.GetPrimitiveType()
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
            instanceCount:  static_cast<NSUInteger>(numInstances)
            baseInstance:   static_cast<NSUInteger>(firstInstance)
        ];
    }
}

void MTDirectCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    MTDirectCommandBuffer::DrawIndexedInstanced(numIndices, numInstances, firstIndex, /*vertexOffset:*/ 0, /*firstInstance:*/ 0);
}

void MTDirectCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    MTDirectCommandBuffer::DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, /*firstInstance:*/ 0);
}

void MTDirectCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        const NSUInteger firstPatch = (static_cast<NSUInteger>(firstIndex) / numPatchControlPoints);
        const NSUInteger numPatches = (static_cast<NSUInteger>(numIndices) / numPatchControlPoints);

        auto renderEncoder = context_.DispatchTessellationAndGetRenderEncoder(numPatches, numInstances);
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints
            patchStart:                     firstPatch
            patchCount:                     numPatches
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        context_.GetIndexBuffer()
            controlPointIndexBufferOffset:  context_.GetIndexBufferOffset(static_cast<NSUInteger>(firstIndex))
            instanceCount:                  static_cast<NSUInteger>(numInstances)
            baseInstance:                   static_cast<NSUInteger>(firstInstance)
        ];
    }
    else
    {
        auto renderEncoder = context_.FlushAndGetRenderEncoder();
        [renderEncoder
            drawIndexedPrimitives:  context_.GetPrimitiveType()
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              context_.GetIndexType()
            indexBuffer:            context_.GetIndexBuffer()
            indexBufferOffset:      context_.GetIndexBufferOffset(static_cast<NSUInteger>(firstIndex))
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseVertex:             static_cast<NSUInteger>(vertexOffset)
            baseInstance:           static_cast<NSUInteger>(firstInstance)
        ];
    }
}

[[noreturn]]
static void TrapIndirectPatchesNotSupported()
{
    LLGL_TRAP("tessellation with indirect arguments not supported in Metal backend yet");
}

//TODO: support patches with indirect arguments
void MTDirectCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
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
            drawPrimitives:         context_.GetPrimitiveType()
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
}

//TODO: support patches with indirect arguments
void MTDirectCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
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
                drawPrimitives:         context_.GetPrimitiveType()
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
}

//TODO: support patches with indirect arguments
void MTDirectCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        context_.GetIndexBuffer()
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
            drawIndexedPrimitives:  context_.GetPrimitiveType()
            indexType:              context_.GetIndexType()
            indexBuffer:            context_.GetIndexBuffer()
            indexBufferOffset:      0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
}

//TODO: support patches with indirect arguments
void MTDirectCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    const NSUInteger numPatchControlPoints = context_.GetNumPatchControlPoints();
    if (numPatchControlPoints > 0)
    {
        #if 0 //TODO
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawIndexedPatches:             numPatchControlPoints
                patchIndexBuffer:               nil
                patchIndexBufferOffset:         0
                controlPointIndexBuffer:        context_.GetIndexBuffer()
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
                drawIndexedPrimitives:  context_.GetPrimitiveType()
                indexType:              context_.GetIndexType()
                indexBuffer:            context_.GetIndexBuffer()
                indexBufferOffset:      0
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
}

void MTDirectCommandBuffer::DrawStreamOutput()
{
    LLGL_TRAP("stream-outputs not supported");
}

/* ----- Compute ----- */

void MTDirectCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    auto computeEncoder = context_.FlushAndGetComputeEncoder();
    [computeEncoder
        dispatchThreadgroups:   MTLSizeMake(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ)
        threadsPerThreadgroup:  context_.GetThreadsPerThreadgroup()
    ];
}

void MTDirectCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto computeEncoder = context_.FlushAndGetComputeEncoder();
    [computeEncoder
        dispatchThreadgroupsWithIndirectBuffer: bufferMT.GetNative()
        indirectBufferOffset:                   static_cast<NSUInteger>(offset)
        threadsPerThreadgroup:                  context_.GetThreadsPerThreadgroup()
    ];
}

/* ----- Debugging ----- */

void MTDirectCommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef LLGL_DEBUG
    [cmdBuffer_ pushDebugGroup:[NSString stringWithUTF8String:name]];
    #endif // /LLGL_DEBUG
}

void MTDirectCommandBuffer::PopDebugGroup()
{
    #ifdef LLGL_DEBUG
    [cmdBuffer_ popDebugGroup];
    #endif // /LLGL_DEBUG
}

/* ----- Extensions ----- */

void MTDirectCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    if (nativeCommand != nullptr && nativeCommandSize == sizeof(Metal::NativeCommand))
    {
        const auto* nativeCommandMT = reinterpret_cast<const Metal::NativeCommand*>(nativeCommand);
        ExecuteNativeMTCommand(*nativeCommandMT, context_);
    }
}

bool MTDirectCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Metal::CommandBufferNativeHandle))
    {
        auto* nativeHandleMT = reinterpret_cast<Metal::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleMT->commandBuffer = cmdBuffer_;
        [nativeHandleMT->commandBuffer retain];
        return true;
    }
    return false;
}


/*
 * ======= Internal: =======
 */

bool MTDirectCommandBuffer::IsMultiSubmitCmdBuffer() const
{
    return false; // always false for MTDirectCommandBuffer
}

void MTDirectCommandBuffer::MarkSubmitted()
{
    /* Reset dirty bit now that it has been submitted to the command queue */
    cmdBufferDirty_ = false;
}


/*
 * ======= Private: =======
 */

void MTDirectCommandBuffer::QueueDrawable(id<MTLDrawable> drawable)
{
    for (id<MTLDrawable> d : drawables_)
    {
        if (d == drawable)
            return;
    }
    drawables_.push_back(drawable);
}

void MTDirectCommandBuffer::PresentDrawables()
{
    for (id<MTLDrawable> d : drawables_)
        [cmdBuffer_ presentDrawable:d];
    drawables_.clear();
}

id<MTLTexture> MTDirectCommandBuffer::GetCurrentDrawableTexture() const
{
    if (MTKView* view = context_.GetCurrentDrawableView())
    {
        id<CAMetalDrawable> drawable = [view currentDrawable];
        if (drawable != nil)
            return [drawable texture];
    }
    return nil;
}

void MTDirectCommandBuffer::FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value)
{
    auto blitEncoder = context_.BindBlitEncoder();
    [blitEncoder fillBuffer:bufferMT.GetNative() range:range value:value];
}

void MTDirectCommandBuffer::FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Use emulated fill command if buffer range is small enough to avoid having both a blit and compute encoder */
    if (range.length > g_minFillBufferForKernel)
        FillBufferByte4Accelerated(bufferMT, range, value);
    else
        FillBufferByte4Emulated(bufferMT, range, value);
}

void MTDirectCommandBuffer::FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Copy value into stack local buffer */
    std::uint32_t localBuffer[g_minFillBufferForKernel / sizeof(std::uint32_t)];
    std::fill(std::begin(localBuffer), std::end(localBuffer), value);

    /* Write clear value into first range of destination buffer */
    UpdateBuffer(bufferMT, range.location, localBuffer, range.length);
}

//TODO: manage binding of compute PSO in MTCommandContext
void MTDirectCommandBuffer::FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
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
    context_.DispatchThreads1D(computeEncoder, pso, numValues);
}


} // /namespace LLGL



// ================================================================================
