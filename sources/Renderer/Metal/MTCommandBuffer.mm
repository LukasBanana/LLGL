/*
 * MTCommandBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTCommandBuffer.h"
#include "MTRenderContext.h"
#include "MTTypes.h"
#include "Buffer/MTBuffer.h"
#include "Buffer/MTBufferArray.h"
#include "RenderState/MTGraphicsPSO.h"
#include "RenderState/MTComputePSO.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTBuiltinPSOFactory.h"
#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"
#include "Shader/MTShaderProgram.h"
#include "../CheckedCast.h"
#include <algorithm>
#include <limits.h>


namespace LLGL
{


/*
Minimum size for the "FillBuffer" command to use a GPU kernel.
for smaller buffers an emulated CPU copy operation will be used.
*/
static const NSUInteger g_minFillBufferForKernel = 64;

// Default value when no compute PSO is bound.
static const MTLSize g_defaultNumThreadsPerGroup { 1, 1, 1 };

MTCommandBuffer::MTCommandBuffer(id<MTLDevice> device, id<MTLCommandQueue> cmdQueue) :
    device_            { device            },
    cmdQueue_          { cmdQueue          },
    stagingBufferPool_ { device, USHRT_MAX }
{
    const NSUInteger maxCmdBuffers = 3;
    cmdBufferSemaphore_ = dispatch_semaphore_create(maxCmdBuffers);
}

MTCommandBuffer::~MTCommandBuffer()
{
    [cmdBuffer_ release];
}

/* ----- Encoding ----- */

void MTCommandBuffer::Begin()
{
    /* Wait until next command buffer becomes available */
    dispatch_semaphore_wait(cmdBufferSemaphore_, DISPATCH_TIME_FOREVER);

    /* Allocate new command buffer from command queue */
    cmdBuffer_ = [cmdQueue_ commandBuffer];

    /* Append complete handler to signal semaphore */
    __block dispatch_semaphore_t blockSemaphore = cmdBufferSemaphore_;
    [cmdBuffer_
        addCompletedHandler:^(id<MTLCommandBuffer> cmdBuffer)
        {
            dispatch_semaphore_signal(blockSemaphore);
        }
    ];

    /* Reset schedulers and pools */
    encoderScheduler_.Reset(cmdBuffer_);
    stagingBufferPool_.Reset();

    /* Reset references */
    numThreadsPerGroup_ = &g_defaultNumThreadsPerGroup;
}

void MTCommandBuffer::End()
{
    encoderScheduler_.Flush();
    PresentDrawables();
}

void MTCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    //TODO
}

/* ----- Blitting ----- */

void MTCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);

    /* Copy data to staging buffer */
    id<MTLBuffer> srcBuffer = nil;
    NSUInteger srcOffset = 0;

    stagingBufferPool_.Write(data, static_cast<NSUInteger>(dataSize), srcBuffer, srcOffset);

    /* Encode blit command to copy staging buffer region to destination buffer */
    encoderScheduler_.PauseRenderEncoder();
    {
        auto blitEncoder = encoderScheduler_.BindBlitEncoder();
        [blitEncoder
            copyFromBuffer:     srcBuffer
            sourceOffset:       srcOffset
            toBuffer:           dstBufferMT.GetNative()
            destinationOffset:  static_cast<NSUInteger>(dstOffset)
            size:               static_cast<NSUInteger>(dataSize)
        ];
    }
    encoderScheduler_.ResumeRenderEncoder();
}

void MTCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    encoderScheduler_.PauseRenderEncoder();
    {
        auto blitEncoder = encoderScheduler_.BindBlitEncoder();
        [blitEncoder
            copyFromBuffer:     srcBufferMT.GetNative()
            sourceOffset:       static_cast<NSUInteger>(srcOffset)
            toBuffer:           dstBufferMT.GetNative()
            destinationOffset:  static_cast<NSUInteger>(dstOffset)
            size:               static_cast<NSUInteger>(size)
        ];
    }
    encoderScheduler_.ResumeRenderEncoder();
}

void MTCommandBuffer::FillBuffer(
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
    if (fillSize == Constants::wholeSize)
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

void MTCommandBuffer::CopyTexture(
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

    encoderScheduler_.PauseRenderEncoder();
    {
        auto blitEncoder = encoderScheduler_.BindBlitEncoder();
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
    encoderScheduler_.ResumeRenderEncoder();
}

void MTCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    if ([textureMT.GetNative() mipmapLevelCount] > 1)
    {
        encoderScheduler_.PauseRenderEncoder();
        {
            auto blitEncoder = encoderScheduler_.BindBlitEncoder();
            [blitEncoder generateMipmapsForTexture:textureMT.GetNative()];
        }
        encoderScheduler_.ResumeRenderEncoder();
    }
}

void MTCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    if (subresource.numMipLevels > 1)
    {
        auto& textureMT = LLGL_CAST(MTTexture&, texture);

        // Create temporary subresource texture to generate MIP-maps only on that range
        id<MTLTexture> intermediateTexture = textureMT.CreateSubresourceView(subresource);

        encoderScheduler_.PauseRenderEncoder();
        {
            auto blitEncoder = encoderScheduler_.BindBlitEncoder();
            [blitEncoder generateMipmapsForTexture:intermediateTexture];
        }
        encoderScheduler_.ResumeRenderEncoder();

        [intermediateTexture release];
    }
}

/* ----- Viewport and Scissor ----- */

void MTCommandBuffer::SetViewport(const Viewport& viewport)
{
    encoderScheduler_.SetViewports(&viewport, 1u);
}

void MTCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    encoderScheduler_.SetViewports(viewports, numViewports);
}

void MTCommandBuffer::SetScissor(const Scissor& scissor)
{
    encoderScheduler_.SetScissorRects(&scissor, 1u);
}

void MTCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    encoderScheduler_.SetScissorRects(scissors, numScissors);
}

/* ----- Clear ----- */

static MTLClearColor ToMTLClearColor(const ColorRGBAf& color)
{
    return MTLClearColorMake(
        static_cast<double>(color.r),
        static_cast<double>(color.g),
        static_cast<double>(color.b),
        static_cast<double>(color.a)
    );
}

void MTCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearValue_.color = ToMTLClearColor(color);
}

void MTCommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depth = static_cast<double>(depth);
}

void MTCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.stencil = stencil;
}

void MTCommandBuffer::Clear(long flags)
{
    if (encoderScheduler_.GetRenderEncoder() != nil && flags != 0)
    {
        /* Make new render pass descriptor with current clear values */
        auto renderPassDesc = encoderScheduler_.CopyRenderPassDesc();

        if ((flags & ClearFlags::Color) != 0)
        {
            renderPassDesc.colorAttachments[0].loadAction   = MTLLoadActionClear;
            renderPassDesc.colorAttachments[0].clearColor   = clearValue_.color;
        }

        if ((flags & ClearFlags::Depth) != 0)
        {
            renderPassDesc.depthAttachment.loadAction       = MTLLoadActionClear;
            renderPassDesc.depthAttachment.clearDepth       = clearValue_.depth;
        }

        if ((flags & ClearFlags::Stencil) != 0)
        {
            renderPassDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
            renderPassDesc.stencilAttachment.clearStencil   = clearValue_.stencil;
        }

        /* Begin with new render pass to clear buffers */
        encoderScheduler_.BindRenderEncoder(renderPassDesc);
        [renderPassDesc release];
    }
}

// Fills the MTLRenderPassDescriptor object according to the secified attachment clear command
static void FillMTRenderPassDesc(MTLRenderPassDescriptor* renderPassDesc, const AttachmentClear& attachment)
{
    if ((attachment.flags & ClearFlags::Color) != 0)
    {
        /* Clear color buffer */
        auto colorBuffer = attachment.colorAttachment;
        renderPassDesc.colorAttachments[colorBuffer].loadAction = MTLLoadActionClear;
        renderPassDesc.colorAttachments[colorBuffer].clearColor = ToMTLClearColor(attachment.clearValue.color);
    }
    else if ((attachment.flags & ClearFlags::Depth) != 0)
    {
        /* Clear depth buffer */
        renderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
        renderPassDesc.depthAttachment.clearDepth = static_cast<double>(attachment.clearValue.depth);
    }
    else if ((attachment.flags & ClearFlags::Stencil) != 0)
    {
        /* Clear stencil buffer */
        renderPassDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
        renderPassDesc.stencilAttachment.clearStencil   = attachment.clearValue.stencil;
    }
}

void MTCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (encoderScheduler_.GetRenderEncoder() != nil && numAttachments > 0)
    {
        /* Make new render pass descriptor with current clear values */
        auto renderPassDesc = encoderScheduler_.CopyRenderPassDesc();

        for (std::uint32_t i = 0; i < numAttachments; ++i)
            FillMTRenderPassDesc(renderPassDesc, attachments[i]);

        /* Begin with new render pass to clear buffers */
        encoderScheduler_.BindRenderEncoder(renderPassDesc);
        [renderPassDesc release];
    }
}

/* ----- Input Assembly ------ */

void MTCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    encoderScheduler_.SetVertexBuffer(bufferMT.GetNative(), 0);
}

void MTCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayMT = LLGL_CAST(MTBufferArray&, bufferArray);
    encoderScheduler_.SetVertexBuffers(
        bufferArrayMT.GetIDArray().data(),
        bufferArrayMT.GetOffsets().data(),
        static_cast<NSUInteger>(bufferArrayMT.GetIDArray().size())
    );
}

void MTCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    indexBuffer_        = bufferMT.GetNative();
    indexBufferOffset_  = 0;
    SetIndexType(bufferMT.IsIndexType16Bits());
}

void MTCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    indexBuffer_        = bufferMT.GetNative();
    indexBufferOffset_  = static_cast<NSUInteger>(offset);
    SetIndexType(format == Format::R16UInt);
}

/* ----- Resources ----- */

void MTCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    encoderScheduler_.SetGraphicsResourceHeap(&resourceHeapMT);
}

void MTCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    encoderScheduler_.BindComputeEncoder();
    resourceHeapMT.BindComputeResources(encoderScheduler_.GetComputeEncoder());
}

void MTCommandBuffer::SetResource(Resource& resource, std::uint32_t slot, long /*bindFlags*/, long stageFlags)
{
    #if 0//TODO: store direct binding in <MTEncoderScheduler>
    switch (resource.GetResourceType())
    {
        case ResourceType::Undefined:
            break;
        case ResourceType::Buffer:
            SetBuffer(LLGL_CAST(MTBuffer&, resource), slot, stageFlags);
            break;
        case ResourceType::Texture:
            SetTexture(LLGL_CAST(MTTexture&, resource), slot, stageFlags);
            break;
        case ResourceType::Sampler:
            SetSampler(LLGL_CAST(MTSampler&, resource), slot, stageFlags);
            break;
    }
    #endif
}

void MTCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{
    //todo
}

/* ----- Render Passes ----- */

//TODO: process <clearValues>!!!
void MTCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    if (renderTarget.IsRenderContext())
    {
        /* Put current drawable into queue */
        auto& renderContextMT = LLGL_CAST(MTRenderContext&, renderTarget);
        QueueDrawable(renderContextMT.GetMTKView().currentDrawable);

        /* Get next render pass descriptor from MetalKit view */
        MTKView* view = renderContextMT.GetMTKView();
        encoderScheduler_.BindRenderEncoder(view.currentRenderPassDescriptor, true);
    }
    else
    {
        /* Get render pass descriptor from render target */
        auto& renderTargetMT = LLGL_CAST(MTRenderTarget&, renderTarget);
        encoderScheduler_.BindRenderEncoder(renderTargetMT.GetNative(), true);
    }
}

void MTCommandBuffer::EndRenderPass()
{
    encoderScheduler_.Flush();
}


/* ----- Pipeline States ----- */

void MTCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Set graphics pipeline with encoder scheduler */
    auto& pipelineStateMT = LLGL_CAST(MTPipelineState&, pipelineState);
    if (pipelineStateMT.IsGraphicsPSO())
    {
        /* Schedule graphics pipeline and store primitive type for draw commands */
        auto& graphicsPSO = LLGL_CAST(MTGraphicsPSO&, pipelineStateMT);
        encoderScheduler_.SetGraphicsPSO(&graphicsPSO);
        primitiveType_ = graphicsPSO.GetMTLPrimitiveType();
    }
    else
    {
        /* Set compute pipeline with encoder scheduler */
        auto& computePSO = LLGL_CAST(MTComputePSO&, pipelineStateMT);
        encoderScheduler_.BindComputeEncoder();
        computePSO.Bind(encoderScheduler_.GetComputeEncoder());

        /* Store reference to work group size of shader program */
        if (auto shaderProgram = computePSO.GetShaderProgram())
            numThreadsPerGroup_ = &(shaderProgram->GetNumThreadsPerGroup());
        else
            numThreadsPerGroup_ = &g_defaultNumThreadsPerGroup;
    }
}

void MTCommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    encoderScheduler_.SetBlendColor(color.Ptr());
}

void MTCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    encoderScheduler_.SetStencilRef(reference, stencilFace);
}

void MTCommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    // dummy
}

void MTCommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    // dummy
}

/* ----- Queries ----- */

void MTCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //todo
}

void MTCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //todo
}

void MTCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //todo
}

void MTCommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Stream Output ------ */

void MTCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    //todo
}

void MTCommandBuffer::EndStreamOutput()
{
    //todo
}

/* ----- Drawing ----- */

void MTCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawPatches:            numPatchControlPoints_
            patchStart:             static_cast<NSUInteger>(firstVertex) / numPatchControlPoints_
            patchCount:             static_cast<NSUInteger>(numVertices) / numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          1
            baseInstance:           0
        ];
    }
    else
    {
        [renderEncoder
            drawPrimitives: primitiveType_
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
        ];
    }
}

void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
            patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:                  1
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
        ];
    }
}

void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
            patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:                  1
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:          1
            baseVertex:             static_cast<NSUInteger>(vertexOffset)
            baseInstance:           0
        ];
    }
}

void MTCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawPatches:            numPatchControlPoints_
            patchStart:             static_cast<NSUInteger>(firstVertex) / numPatchControlPoints_
            patchCount:             static_cast<NSUInteger>(numVertices) / numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseInstance:           0
        ];
    }
    else
    {
        [renderEncoder
            drawPrimitives: primitiveType_
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
            instanceCount:  static_cast<NSUInteger>(numInstances)
            baseInstance:   0
        ];
    }
}

void MTCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawPatches:            numPatchControlPoints_
            patchStart:             static_cast<NSUInteger>(firstVertex) / numPatchControlPoints_
            patchCount:             static_cast<NSUInteger>(numVertices) / numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseInstance:           static_cast<NSUInteger>(firstInstance)
        ];
    }
    else
    {
        [renderEncoder
            drawPrimitives: primitiveType_
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
            instanceCount:  static_cast<NSUInteger>(numInstances)
            baseInstance:   static_cast<NSUInteger>(firstInstance)
        ];
    }
}

void MTCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
            patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:                  static_cast<NSUInteger>(numInstances)
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseVertex:             0
            baseInstance:           0
        ];
    }
}

void MTCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
            patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:                  static_cast<NSUInteger>(numInstances)
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseVertex:             static_cast<NSUInteger>(vertexOffset)
            baseInstance:           0
        ];
    }
}

void MTCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
            patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:                  static_cast<NSUInteger>(numInstances)
            baseInstance:                   static_cast<NSUInteger>(firstInstance)
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexCount:             static_cast<NSUInteger>(numIndices)
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
            instanceCount:          static_cast<NSUInteger>(numInstances)
            baseVertex:             static_cast<NSUInteger>(vertexOffset)
            baseInstance:           static_cast<NSUInteger>(firstInstance)
        ];
    }
}

void MTCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawPatches:            numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
    else
    {
        [renderEncoder
            drawPrimitives:         primitiveType_
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
}

void MTCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawPatches:            numPatchControlPoints_
                patchIndexBuffer:       nil
                patchIndexBufferOffset: 0
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
    else
    {
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawPrimitives:         primitiveType_
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
}

void MTCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder
            drawIndexedPatches:             numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  0
            indirectBuffer:                 bufferMT.GetNative()
            indirectBufferOffset:           static_cast<NSUInteger>(offset)
        ];
    }
    else
    {
        [renderEncoder
            drawIndexedPrimitives:  primitiveType_
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
}

void MTCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    auto renderEncoder = encoderScheduler_.GetRenderEncoderAndFlushRenderPass();
    if (numPatchControlPoints_ > 0)
    {
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawIndexedPatches:             numPatchControlPoints_
                patchIndexBuffer:               nil
                patchIndexBufferOffset:         0
                controlPointIndexBuffer:        indexBuffer_
                controlPointIndexBufferOffset:  0
                indirectBuffer:                 bufferMT.GetNative()
                indirectBufferOffset:           static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
    else
    {
        while (numCommands-- > 0)
        {
            [renderEncoder
                drawIndexedPrimitives:  primitiveType_
                indexType:              indexType_
                indexBuffer:            indexBuffer_
                indexBufferOffset:      0
                indirectBuffer:         bufferMT.GetNative()
                indirectBufferOffset:   static_cast<NSUInteger>(offset)
            ];
            offset += stride;
        }
    }
}

/* ----- Compute ----- */

void MTCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    auto computeEncoder = encoderScheduler_.BindComputeEncoder();
    MTLSize numGroups = { numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ };
    [computeEncoder
        dispatchThreadgroups:   numGroups
        threadsPerThreadgroup:  *numThreadsPerGroup_
    ];
}

void MTCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto computeEncoder = encoderScheduler_.BindComputeEncoder();
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    [computeEncoder
        dispatchThreadgroupsWithIndirectBuffer: bufferMT.GetNative()
        indirectBufferOffset:                   static_cast<NSUInteger>(offset)
        threadsPerThreadgroup:                  *numThreadsPerGroup_
    ];
}

/* ----- Debugging ----- */

void MTCommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef LLGL_DEBUG
    [cmdBuffer_ pushDebugGroup:[NSString stringWithUTF8String:name]];
    #endif // /LLGL_DEBUG
}

void MTCommandBuffer::PopDebugGroup()
{
    #ifdef LLGL_DEBUG
    [cmdBuffer_ popDebugGroup];
    #endif // /LLGL_DEBUG
}

/* ----- Extensions ----- */

void MTCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}


/*
 * ======= Private: =======
 */

void MTCommandBuffer::SetIndexType(bool indexType16Bits)
{
    if (indexType16Bits)
    {
        indexType_      = MTLIndexTypeUInt16;
        indexTypeSize_  = 2;
    }
    else
    {
        indexType_      = MTLIndexTypeUInt32;
        indexTypeSize_  = 4;
    }
}

void MTCommandBuffer::QueueDrawable(id<MTLDrawable> drawable)
{
    for (auto d : drawables_)
    {
        if (d == drawable)
            return;
    }
    drawables_.push_back(drawable);
}

void MTCommandBuffer::PresentDrawables()
{
    for (auto d : drawables_)
        [cmdBuffer_ presentDrawable:d];
    drawables_.clear();
}

#if 0//TODO: store direct binding in <MTEncoderScheduler>
void MTCommandBuffer::SetBuffer(MTBuffer& bufferMT, std::uint32_t slot, long stageFlags)
{
    if (auto renderEncoder = encoderScheduler_.GetRenderEncoder())
    {
        if ((stageFlags & StageFlags::VertexStage) != 0)
        {
            [renderEncoder
                setVertexBuffer:    bufferMT.GetNative()
                offset:             0
                atIndex:            static_cast<NSUInteger>(slot)
            ];
        }
        if ((stageFlags & StageFlags::FragmentStage) != 0)
        {
            [renderEncoder
                setFragmentBuffer:  bufferMT.GetNative()
                offset:             0
                atIndex:            static_cast<NSUInteger>(slot)
            ];
        }
    }
    else if (auto computeEncoder = encoderScheduler_.GetComputeEncoder())
    {
        if ((stageFlags & StageFlags::ComputeStage) != 0)
        {
            [computeEncoder
                setBuffer:  bufferMT.GetNative()
                offset:     0
                atIndex:    static_cast<NSUInteger>(slot)
            ];
        }
    }
}

void MTCommandBuffer::SetTexture(MTTexture& textureMT, std::uint32_t slot, long stageFlags)
{
    if (auto renderEncoder = encoderScheduler_.GetRenderEncoder())
    {
        if ((stageFlags & StageFlags::VertexStage) != 0)
        {
            [renderEncoder
                setVertexTexture:   textureMT.GetNative()
                atIndex:            static_cast<NSUInteger>(slot)
            ];
        }
        if ((stageFlags & StageFlags::FragmentStage) != 0)
        {
            [renderEncoder
                setFragmentTexture: textureMT.GetNative()
                atIndex:            static_cast<NSUInteger>(slot)
            ];
        }
    }
    else if (auto computeEncoder = encoderScheduler_.GetComputeEncoder())
    {
        if ((stageFlags & StageFlags::ComputeStage) != 0)
        {
            [computeEncoder
                setTexture: textureMT.GetNative()
                atIndex:    static_cast<NSUInteger>(slot)
            ];
        }
    }
}

void MTCommandBuffer::SetSampler(MTSampler& samplerMT, std::uint32_t slot, long stageFlags)
{
    if (auto renderEncoder  = encoderScheduler_.GetRenderEncoder())
    {
        if ((stageFlags & StageFlags::VertexStage) != 0)
        {
            [renderEncoder
                setVertexSamplerState:      samplerMT.GetNative()
                atIndex:                    static_cast<NSUInteger>(slot)
            ];
        }
        if ((stageFlags & StageFlags::FragmentStage) != 0)
        {
            [renderEncoder
                setFragmentSamplerState:    samplerMT.GetNative()
                atIndex:                    static_cast<NSUInteger>(slot)
            ];
        }
    }
    else if (auto computeEncoder = encoderScheduler_.GetComputeEncoder())
    {
        if ((stageFlags & StageFlags::ComputeStage) != 0)
        {
            [computeEncoder
                setSamplerState:    samplerMT.GetNative()
                atIndex:            static_cast<NSUInteger>(slot)
            ];
        }
    }
}
#endif

void MTCommandBuffer::FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value)
{
    encoderScheduler_.PauseRenderEncoder();
    {
        auto blitEncoder = encoderScheduler_.BindBlitEncoder();
        [blitEncoder fillBuffer:bufferMT.GetNative() range:range value:value];
    }
    encoderScheduler_.ResumeRenderEncoder();
}

void MTCommandBuffer::FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Use emulated fill command if buffer range is small enough to avoid having both a blit and compute encoder */
    if (range.length > g_minFillBufferForKernel)
        FillBufferByte4Accelerated(bufferMT, range, value);
    else
        FillBufferByte4Emulated(bufferMT, range, value);
}

void MTCommandBuffer::FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    /* Copy value into stack local buffer */
    std::uint32_t localBuffer[g_minFillBufferForKernel / sizeof(std::uint32_t)];
    std::fill(std::begin(localBuffer), std::end(localBuffer), value);

    /* Write clear value into first range of destination buffer */
    UpdateBuffer(bufferMT, range.location, localBuffer, range.length);
}

//TODO: manage binding of compute PSO in MTEncoderScheduler
void MTCommandBuffer::FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value)
{
    encoderScheduler_.PauseRenderEncoder();
    {
        auto computeEncoder = encoderScheduler_.BindComputeEncoder();

        /* Bind compute PSO with kernel to fill buffer */
        id<MTLComputePipelineState> pso = MTBuiltinPSOFactory::Get().GetComputePSO(MTBuiltinComputePSO::FillBufferByte4);
        [computeEncoder setComputePipelineState:pso];

        /* Bind destination buffer range and store clear value as input constant buffer */
        [computeEncoder setBuffer:bufferMT.GetNative() offset:range.location atIndex:0];
        [computeEncoder setBytes:&value length:sizeof(value) atIndex:1];

        /* Dispatch compute kernels */
        const NSUInteger numValues = range.length / sizeof(std::uint32_t);
        const NSUInteger maxLocalThreads = std::min(
            device_.maxThreadsPerThreadgroup.width,
            pso.maxTotalThreadsPerThreadgroup
        );

        if (@available(iOS 11.0, macOS 10.13, *))
        {
            /* Dispatch all threads with a single command and let Metal distribute the full and partial threadgroups */
            [computeEncoder
                dispatchThreads:        MTLSizeMake(numValues, 1, 1)
                threadsPerThreadgroup:  MTLSizeMake(std::min(numValues, maxLocalThreads), 1, 1)
            ];
        }
        else
        {
            /* Disaptch threadgroups with as many local threads as possible */
            const NSUInteger numThreadGroups = numValues / maxLocalThreads;
            if (numThreadGroups > 0)
            {
                [computeEncoder
                    dispatchThreadgroups:   MTLSizeMake(numThreadGroups, 1, 1)
                    threadsPerThreadgroup:  MTLSizeMake(maxLocalThreads, 1, 1)
                ];
            }

            /* Dispatch local threads for remaining range */
            const NSUInteger remainingValues = numValues % maxLocalThreads;
            if (remainingValues > 0)
            {
                [computeEncoder
                    dispatchThreadgroups:   MTLSizeMake(1, 1, 1)
                    threadsPerThreadgroup:  MTLSizeMake(remainingValues, 1, 1)
                ];
            }
        }
    }
    encoderScheduler_.ResumeRenderEncoder();
}


} // /namespace LLGL



// ================================================================================
