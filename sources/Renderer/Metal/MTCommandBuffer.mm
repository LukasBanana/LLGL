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
#include "RenderState/MTResourceHeap.h"
#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"
#include "../CheckedCast.h"
#include <algorithm>
#include <limits.h>


namespace LLGL
{


MTCommandBuffer::MTCommandBuffer(id<MTLDevice> device, id<MTLCommandQueue> cmdQueue) :
    cmdQueue_          { cmdQueue          },
    stagingBufferPool_ { device, USHRT_MAX }
{
}

MTCommandBuffer::~MTCommandBuffer()
{
    [cmdBuffer_ release];
}

/* ----- Encoding ----- */

void MTCommandBuffer::Begin()
{
    cmdBuffer_ = [cmdQueue_ commandBuffer];
    encoderScheduler_.Reset(cmdBuffer_);
    stagingBufferPool_.Reset();
}

void MTCommandBuffer::End()
{
    // dummy
}

void MTCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    
    /* Copy data to staging buffer */
    id<MTLBuffer> srcBuffer = nil;
    NSUInteger srcOffset = 0;
    
    stagingBufferPool_.Write(data, static_cast<NSUInteger>(dataSize), srcBuffer, srcOffset);
    
    /* Encode blit command to copy staging buffer region to destination buffer */
    auto blitEncoder = encoderScheduler_.BindBlitEncoder();
    [blitEncoder
        copyFromBuffer:     srcBuffer
        sourceOffset:       srcOffset
        toBuffer:           dstBufferMT.GetNative()
        destinationOffset:  static_cast<NSUInteger>(dstOffset)
        size:               static_cast<NSUInteger>(dataSize)
    ];
}

void MTCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    auto blitEncoder = encoderScheduler_.BindBlitEncoder();
    [blitEncoder
        copyFromBuffer:     srcBufferMT.GetNative()
        sourceOffset:       static_cast<NSUInteger>(srcOffset)
        toBuffer:           dstBufferMT.GetNative()
        destinationOffset:  static_cast<NSUInteger>(dstOffset)
        size:               static_cast<NSUInteger>(size)
    ];
}

void MTCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    //TODO
}

/* ----- Configuration ----- */

void MTCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
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
        
        //SubmitRenderEncoderState();
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
        
        //SubmitRenderEncoderState();
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

/* ----- Stream Output Buffers ------ */

void MTCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    //todo
}

void MTCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    //todo
}

void MTCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    //todo
}

void MTCommandBuffer::EndStreamOutput()
{
    //todo
}

/* ----- Resource Heaps ----- */

void MTCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    resourceHeapMT.BindGraphicsResources(encoderScheduler_.GetRenderEncoder());
}

void MTCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    encoderScheduler_.BindComputeEncoder();
    auto& resourceHeapMT = LLGL_CAST(MTResourceHeap&, resourceHeap);
    resourceHeapMT.BindComputeResources(encoderScheduler_.GetComputeEncoder());
}

/* ----- Render Passes ----- */

void MTCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    if (renderTarget.IsRenderContext())
    {
        /* Make this the current command buffer for the render context */
        auto& renderContextMT = LLGL_CAST(MTRenderContext&, renderTarget);
        renderContextMT.MakeCurrent(cmdBuffer_);
        
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
    
    //SubmitRenderEncoderState();
}

void MTCommandBuffer::EndRenderPass()
{
    encoderScheduler_.Flush();
}


/* ----- Pipeline States ----- */

void MTCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineMT = LLGL_CAST(MTGraphicsPipeline&, graphicsPipeline);

    encoderScheduler_.SetRenderPipelineState(graphicsPipelineMT.GetRenderPipelineState());
    encoderScheduler_.SetDepthStencilState(graphicsPipelineMT.GetDepthStencilState());

    /* Store primitive type to subsequent draw commands */
    primitiveType_ = graphicsPipelineMT.GetMTLPrimitiveType();
}

void MTCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
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

/* ----- Drawing ----- */

void MTCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
            drawPrimitives: primitiveType_
            vertexStart:    static_cast<NSUInteger>(firstVertex)
            vertexCount:    static_cast<NSUInteger>(numVertices)
        ];
    }
}

void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            drawPatches:            numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
    else
    {
        [encoderScheduler_.GetRenderEncoder()
            drawPrimitives:         primitiveType_
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
}

void MTCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    if (numPatchControlPoints_ > 0)
    {
        while (numCommands-- > 0)
        {
            [encoderScheduler_.GetRenderEncoder()
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
            [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        [encoderScheduler_.GetRenderEncoder()
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
        [encoderScheduler_.GetRenderEncoder()
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
    if (numPatchControlPoints_ > 0)
    {
        while (numCommands-- > 0)
        {
            [encoderScheduler_.GetRenderEncoder()
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
            [encoderScheduler_.GetRenderEncoder()
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
        threadsPerThreadgroup:  numThreadsPerGroup_
    ];
}

void MTCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    encoderScheduler_.BindComputeEncoder();
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    [encoderScheduler_.GetComputeEncoder()
        dispatchThreadgroupsWithIndirectBuffer: bufferMT.GetNative()
        indirectBufferOffset:                   static_cast<NSUInteger>(offset)
        threadsPerThreadgroup:                  numThreadsPerGroup_
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

/* ----- Direct Resource Access ------ */

void MTCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    if ((stageFlags & StageFlags::VertexStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setVertexBuffer:    bufferMT.GetNative()
            offset:             0
            atIndex:            static_cast<NSUInteger>(slot)
        ];
    }
    if ((stageFlags & StageFlags::FragmentStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setFragmentBuffer:  bufferMT.GetNative()
            offset:             0
            atIndex:            static_cast<NSUInteger>(slot)
        ];
    }
}

void MTCommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    //todo
}

void MTCommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    //todo
}

void MTCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long stageFlags)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    if ((stageFlags & StageFlags::VertexStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setVertexTexture:   textureMT.GetNative()
            atIndex:            static_cast<NSUInteger>(slot)
        ];
    }
    if ((stageFlags & StageFlags::FragmentStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setFragmentTexture: textureMT.GetNative()
            atIndex:            static_cast<NSUInteger>(slot)
        ];
    }
}

void MTCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags)
{
    /* Get native MTLSamplerState object */
    auto& samplerMT = LLGL_CAST(MTSampler&, sampler);
    
    auto samplerState   = samplerMT.GetNative();
    auto index          = static_cast<NSUInteger>(slot);
    
    if ((stageFlags & StageFlags::VertexStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setVertexSamplerState:      samplerState
            atIndex:                    index
        ];
    }
    if ((stageFlags & StageFlags::FragmentStage) != 0)
    {
        [encoderScheduler_.GetRenderEncoder()
            setFragmentSamplerState:    samplerState
            atIndex:                    index
        ];
    }
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


} // /namespace LLGL



// ================================================================================
