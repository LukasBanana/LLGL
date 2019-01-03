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
#include "../CheckedCast.h"
#include <algorithm>


namespace LLGL
{


const std::uint32_t MTCommandBuffer::g_maxNumVertexBuffers;

MTCommandBuffer::MTCommandBuffer(id<MTLCommandQueue> cmdQueue) :
    cmdQueue_ { cmdQueue }
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
    ResetRenderEncoderState();
}

void MTCommandBuffer::End()
{
    // dummy
}

void MTCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    dstBufferMT.Write(dstOffset, data, dataSize);
}

void MTCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    auto& srcBufferMT = LLGL_CAST(MTBuffer&, srcBuffer);

    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer_ blitCommandEncoder];
    [blitEncoder
        copyFromBuffer:     srcBufferMT.GetNative()
        sourceOffset:       static_cast<NSUInteger>(srcOffset)
        toBuffer:           dstBufferMT.GetNative()
        destinationOffset:  static_cast<NSUInteger>(dstOffset)
        size:               static_cast<NSUInteger>(size)
    ];
    [blitEncoder endEncoding];
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

static void Convert(MTLViewport& dst, const Viewport& src)
{
    dst.originX = static_cast<double>(src.x);
    dst.originY = static_cast<double>(src.y);
    dst.width   = static_cast<double>(src.width);
    dst.height  = static_cast<double>(src.height);
    dst.znear   = static_cast<double>(src.minDepth);
    dst.zfar    = static_cast<double>(src.maxDepth);
}

void MTCommandBuffer::SetViewport(const Viewport& viewport)
{
    if (renderEncoder_ != nil)
    {
        MTLViewport viewportMT;
        Convert(viewportMT, viewport);
        [renderEncoder_ setViewport:viewportMT];
    }
    else
    {
        Convert(renderEncoderState_.viewports[0], viewport);
        renderEncoderState_.viewportCount = 1;
    }
}

void MTCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    if (renderEncoder_ != nil)
    {
        MTLViewport viewportsML[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
        
        numViewports = std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
        for (std::uint32_t i = 0; i < numViewports; ++i)
            Convert(viewportsML[i], viewports[i]);
        
        [renderEncoder_ setViewports:viewportsML count:(NSUInteger)numViewports];
    }
    else
    {
        renderEncoderState_.viewportCount = std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
        for (std::uint32_t i = 0; i < renderEncoderState_.viewportCount; ++i)
            Convert(renderEncoderState_.viewports[i], viewports[i]);
    }
}

static void Convert(MTLScissorRect& dst, const Scissor& scissor)
{
    dst.x       = static_cast<NSUInteger>(std::max(0, scissor.x));
    dst.y       = static_cast<NSUInteger>(std::max(0, scissor.y));
    dst.width   = static_cast<NSUInteger>(std::max(0, scissor.width));
    dst.height  = static_cast<NSUInteger>(std::max(0, scissor.height));
}

void MTCommandBuffer::SetScissor(const Scissor& scissor)
{
    if (renderEncoder_ != nil)
    {
        MTLScissorRect rect;
        Convert(rect, scissor);
        [renderEncoder_ setScissorRect:rect];
    }
    else
    {
        Convert(renderEncoderState_.scissorRects[0], scissor);
        renderEncoderState_.scissorRectCount = 1;
    }
}

void MTCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (renderEncoder_ != nil)
    {
        MTLScissorRect rects[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
        
        numScissors = std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
        for (std::uint32_t i = 0; i < numScissors; ++i)
            Convert(rects[i], scissors[i]);
        
        [renderEncoder_ setScissorRects:rects count:(NSUInteger)numScissors];
    }
    else
    {
        renderEncoderState_.scissorRectCount = std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
        for (std::uint32_t i = 0; i < renderEncoderState_.scissorRectCount; ++i)
            Convert(renderEncoderState_.scissorRects[i], scissors[i]);
    }
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
    if (renderEncoder_ != nil && renderPassDesc_ != nullptr && flags != 0)
    {
        /* End previous render pass */
        [renderEncoder_ endEncoding];
        
        /* Make new render pass descriptor with current clear values */
        auto renderPassDesc = (MTLRenderPassDescriptor*)[renderPassDesc_ copy];
        
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
        renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];
        [renderPassDesc release];
        
        SubmitRenderEncoderState();
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
    if (renderEncoder_ != nil && renderPassDesc_ != nullptr && numAttachments > 0)
    {
        /* End previous render pass */
        [renderEncoder_ endEncoding];
        
        /* Make new render pass descriptor with current clear values */
        auto renderPassDesc = (MTLRenderPassDescriptor*)[renderPassDesc_ copy];
        
        for (std::uint32_t i = 0; i < numAttachments; ++i)
            FillMTRenderPassDesc(renderPassDesc, attachments[i]);
        
        /* Begin with new render pass to clear buffers */
        renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];
        [renderPassDesc release];
        
        SubmitRenderEncoderState();
    }
}

/* ----- Input Assembly ------ */

void MTCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    if (renderEncoder_ != nil)
    {
        [renderEncoder_
            setVertexBuffer:    bufferMT.GetNative()
            offset:             0
            atIndex:            0
        ];
    }
    else
    {
        renderEncoderState_.vertexBuffer0               = bufferMT.GetNative();
        renderEncoderState_.vertexBuffers               = &(renderEncoderState_.vertexBuffer0);
        renderEncoderState_.vertexBufferOffset0         = 0;
        renderEncoderState_.vertexBufferOffsets         = &(renderEncoderState_.vertexBufferOffset0);
        renderEncoderState_.vertexBufferRange.location  = 0;
        renderEncoderState_.vertexBufferRange.length    = 1;
    }
}

void MTCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayMT = LLGL_CAST(MTBufferArray&, bufferArray);
    if (renderEncoder_ != nil)
    {
        [renderEncoder_
            setVertexBuffers:   bufferArrayMT.GetIDArray().data()
            offsets:            bufferArrayMT.GetOffsets().data()
            withRange:          NSMakeRange(0, static_cast<NSUInteger>(bufferArrayMT.GetIDArray().size()))
        ];
    }
    else
    {
        renderEncoderState_.vertexBuffers               = bufferArrayMT.GetIDArray().data();
        renderEncoderState_.vertexBufferOffsets         = bufferArrayMT.GetOffsets().data();
        renderEncoderState_.vertexBufferRange.location  = 0;
        renderEncoderState_.vertexBufferRange.length    = static_cast<NSUInteger>(bufferArrayMT.GetIDArray().size());
    }
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
    resourceHeapMT.Bind(renderEncoder_, computeEncoder_);
}

void MTCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    //todo
}

/* ----- Render Passes ----- */

void MTCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    renderPassDesc_ = nullptr;
    
    if (renderTarget.IsRenderContext())
    {
        /* Make this the current command buffer for the render context */
        auto& renderContextMT = LLGL_CAST(MTRenderContext&, renderTarget);
        renderContextMT.MakeCurrent(cmdBuffer_);
        
        /* Get next render pass descriptor from MetalKit view */
        MTKView* view = renderContextMT.GetMTKView();
        renderPassDesc_ = view.currentRenderPassDescriptor;
    }
    else
    {
        //TODO
    }
    
    /* Build render pass descriptor */
    //TODO...
    
    /* Get next render command encoder */
    if (renderPassDesc_ != nullptr)
    {
        renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc_];
        SubmitRenderEncoderState();
    }
}

void MTCommandBuffer::EndRenderPass()
{
    if (renderEncoder_ != nil)
    {
        [renderEncoder_ endEncoding];
        renderEncoder_ = nil;
        renderPassDesc_ = nullptr;
    }
}


/* ----- Pipeline States ----- */

void MTCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineMT = LLGL_CAST(MTGraphicsPipeline&, graphicsPipeline);
    
    /* Bind render states of graphics pipeline */
    if (renderEncoder_ != nil)
    {
        [renderEncoder_ setRenderPipelineState:graphicsPipelineMT.GetRenderPipelineState()];
        [renderEncoder_ setDepthStencilState:graphicsPipelineMT.GetDepthStencilState()];
    }
    else
    {
        renderEncoderState_.renderPipelineState = graphicsPipelineMT.GetRenderPipelineState();
        renderEncoderState_.depthStencilState   = graphicsPipelineMT.GetDepthStencilState();
    }
    
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
            drawPatches:            numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            indirectBuffer:         bufferMT.GetNative()
            indirectBufferOffset:   static_cast<NSUInteger>(offset)
        ];
    }
    else
    {
        [renderEncoder_
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
            [renderEncoder_
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
            [renderEncoder_
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
        [renderEncoder_
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
        [renderEncoder_
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
            [renderEncoder_
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
            [renderEncoder_
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
    MTLSize numGroups = { numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ };
    [computeEncoder_
        dispatchThreadgroups:   numGroups
        threadsPerThreadgroup:  numThreadsPerGroup_
    ];
}

void MTCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    [computeEncoder_
        dispatchThreadgroupsWithIndirectBuffer: bufferMT.GetNative()
        indirectBufferOffset:                   static_cast<NSUInteger>(offset)
        threadsPerThreadgroup:                  numThreadsPerGroup_
    ];
}

/* ----- Direct Resource Access ------ */

void MTCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    if ((stageFlags & StageFlags::VertexStage) != 0)
        [renderEncoder_ setVertexBuffer:bufferMT.GetNative() offset:0 atIndex:static_cast<NSUInteger>(slot)];
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        [renderEncoder_ setFragmentBuffer:bufferMT.GetNative() offset:0 atIndex:static_cast<NSUInteger>(slot)];
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
        [renderEncoder_
            setVertexTexture:   textureMT.GetNative()
            atIndex:            static_cast<NSUInteger>(slot)
        ];
    }
    if ((stageFlags & StageFlags::FragmentStage) != 0)
    {
        [renderEncoder_
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
        [renderEncoder_ setVertexSamplerState:samplerState atIndex:index];
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        [renderEncoder_ setFragmentSamplerState:samplerState atIndex:index];
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

void MTCommandBuffer::SubmitRenderEncoderState()
{
    if (renderEncoderState_.viewportCount > 0)
    {
        [renderEncoder_
            setViewports:   renderEncoderState_.viewports
            count:          renderEncoderState_.viewportCount
        ];
    }
    if (renderEncoderState_.scissorRectCount > 0)
    {
        [renderEncoder_
            setScissorRects:    renderEncoderState_.scissorRects
            count:              renderEncoderState_.scissorRectCount
        ];
    }
    if (renderEncoderState_.vertexBufferRange.length > 0)
    {
        [renderEncoder_
            setVertexBuffers:   renderEncoderState_.vertexBuffers
            offsets:            renderEncoderState_.vertexBufferOffsets
            withRange:          renderEncoderState_.vertexBufferRange
        ];
    }
    if (renderEncoderState_.renderPipelineState != nil)
    {
        [renderEncoder_ setRenderPipelineState:renderEncoderState_.renderPipelineState];
        [renderEncoder_ setDepthStencilState:renderEncoderState_.depthStencilState];
    }
}

void MTCommandBuffer::ResetRenderEncoderState()
{
    renderEncoderState_.viewportCount               = 0;
    renderEncoderState_.scissorRectCount            = 0;
    renderEncoderState_.vertexBufferRange.length    = 0;
    renderEncoderState_.renderPipelineState         = nil;
}

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
