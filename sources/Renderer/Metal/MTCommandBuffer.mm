/*
 * MTCommandBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTCommandBuffer.h"
#include "MTRenderContext.h"
#include "MTTypes.h"
#include "Buffer/MTBuffer.h"
#include "Buffer/MTBufferArray.h"
#include "RenderState/MTGraphicsPipeline.h"
#include "RenderState/MTComputePipeline.h"
#include "RenderState/MTResourceHeap.h"
#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"
#include "Shader/MTShaderProgram.h"
#include "../CheckedCast.h"
#include <algorithm>
#include <limits.h>


namespace LLGL
{


static const MTLSize g_defaultNumThreadsPerGroup { 1, 1, 1 };

MTCommandBuffer::MTCommandBuffer(id<MTLDevice> device, id<MTLCommandQueue> cmdQueue) :
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

void MTCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    //TODO
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

void MTCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics pipeline with encoder scheduler */
    auto& graphicsPipelineMT = LLGL_CAST(MTGraphicsPipeline&, graphicsPipeline);
    encoderScheduler_.SetGraphicsPipeline(&graphicsPipelineMT);

    /* Store primitive type to subsequent draw commands */
    primitiveType_ = graphicsPipelineMT.GetMTLPrimitiveType();
}

void MTCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    /* Set compute pipeline with encoder scheduler */
    auto& computePipelineMT = LLGL_CAST(MTComputePipeline&, computePipeline);
    encoderScheduler_.BindComputeEncoder();
    computePipelineMT.Bind(encoderScheduler_.GetComputeEncoder());

    /* Store reference to work group size of shader program */
    if (auto shaderProgram = computePipelineMT.GetShaderProgram())
        numThreadsPerGroup_ = &(shaderProgram->GetNumThreadsPerGroup());
    else
        numThreadsPerGroup_ = &g_defaultNumThreadsPerGroup;
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
    encoderScheduler_.BindComputeEncoder();
    MTLSize numGroups = { numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ };
    [encoderScheduler_.GetComputeEncoder()
        dispatchThreadgroups:   numGroups
        threadsPerThreadgroup:  *numThreadsPerGroup_
    ];
}

void MTCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    encoderScheduler_.BindComputeEncoder();
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    [encoderScheduler_.GetComputeEncoder()
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


} // /namespace LLGL



// ================================================================================
