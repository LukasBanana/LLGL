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
#include "RenderState/MTGraphicsPipeline.h"
#include "Texture/MTTexture.h"
#include "../CheckedCast.h"
#include <algorithm>


namespace LLGL
{


static const std::uint32_t g_maxNumViewportsAndScissors = 32;

MTCommandBuffer::MTCommandBuffer(id<MTLCommandQueue> commandQueue) :
    cmdQueue_ { commandQueue }
{
}

MTCommandBuffer::~MTCommandBuffer()
{
    [cmdBuffer_ release];
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
    MTLViewport viewportMT;
    Convert(viewportMT, viewport);
    [renderEncoder_ setViewport:viewportMT];
}

void MTCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    MTLViewport viewportsML[g_maxNumViewportsAndScissors];
    
    numViewports = std::min(numViewports, g_maxNumViewportsAndScissors);
    for (std::uint32_t i = 0; i < numViewports; ++i)
        Convert(viewportsML[i], viewports[i]);
    
    [renderEncoder_ setViewports:viewportsML count:(NSUInteger)numViewports];
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
    MTLScissorRect rect;
    Convert(rect, scissor);
    [renderEncoder_ setScissorRect:rect];
}

void MTCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    MTLScissorRect rects[g_maxNumViewportsAndScissors];
    
    numScissors = std::min(numScissors, g_maxNumViewportsAndScissors);
    for (std::uint32_t i = 0; i < numScissors; ++i)
        Convert(rects[i], scissors[i]);
    
    [renderEncoder_ setScissorRects:rects count:(NSUInteger)numScissors];
}

/* ----- Clear ----- */

void MTCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    //todo
}

void MTCommandBuffer::SetClearDepth(float depth)
{
    //todo
}

void MTCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    //todo
}

void MTCommandBuffer::Clear(long flags)
{
    //todo
}

void MTCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //todo
}

/* ----- Input Assembly ------ */

void MTCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    [renderEncoder_ setVertexBuffer:bufferMT.GetNative() offset:0 atIndex:0];
}

void MTCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    //todo
}

void MTCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    //todo
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

/* ----- Constant Buffers ------ */

void MTCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    //todo
}

/* ----- Storage Buffers ----- */

void MTCommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    //todo
}

/* ----- Textures ----- */

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

/* ----- Samplers ----- */

void MTCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags)
{
    //todo
}

/* ----- Resource Heaps ----- */

void MTCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    //todo
}

void MTCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    //todo
}

/* ----- Render Targets ----- */

void MTCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void MTCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextMT = LLGL_CAST(MTRenderContext&, renderContext);
    
    /* Get next command buffer */
    cmdBuffer_ = [cmdQueue_ commandBuffer];
    
    /* Get next render pass descriptor from MetalKit view */
    MTKView* view = renderContextMT.GetMTKView();
    MTLRenderPassDescriptor* renderPassDesc = view.currentRenderPassDescriptor;
    
    if (renderPassDesc != nullptr)
    {
        /* Get next render command encoder */
        renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];
        renderContextMT.MakeCurrent(cmdBuffer_, renderEncoder_);
    }
}


/* ----- Pipeline States ----- */

void MTCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineMT = LLGL_CAST(MTGraphicsPipeline&, graphicsPipeline);
    graphicsPipelineMT.Bind(renderEncoder_);
    primitiveType_ = graphicsPipelineMT.GetMTLPrimitiveType();
}

void MTCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

void MTCommandBuffer::BeginQuery(Query& query)
{
    //todo
}

void MTCommandBuffer::EndQuery(Query& query)
{
    //todo
}

bool MTCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    //todo
    return false;
}

bool MTCommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    //todo
    return false;
}

void MTCommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
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
            patchStart:             ((NSUInteger)firstVertex) / numPatchControlPoints_
            patchCount:             ((NSUInteger)numVertices) / numPatchControlPoints_
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
            vertexStart:    (NSUInteger)firstVertex
            vertexCount:    (NSUInteger)numVertices
        ];
    }
}

void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder_
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     ((NSUInteger)firstIndex) / numPatchControlPoints_
            patchCount:                     ((NSUInteger)numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:                  1
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder_
            drawIndexedPrimitives:  primitiveType_
            indexCount:             (NSUInteger)numIndices
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexTypeSize_ * ((NSUInteger)firstIndex)
        ];
    }
}

void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder_
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     ((NSUInteger)firstIndex) / numPatchControlPoints_
            patchCount:                     ((NSUInteger)numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:                  1
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder_
            drawIndexedPrimitives:  primitiveType_
            indexCount:             (NSUInteger)numIndices
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:          1
            baseVertex:             (NSInteger)vertexOffset
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
            patchStart:             ((NSUInteger)firstVertex) / numPatchControlPoints_
            patchCount:             ((NSUInteger)numVertices) / numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          (NSUInteger)numInstances
            baseInstance:           0
        ];
    }
    else
    {
        [renderEncoder_
            drawPrimitives: primitiveType_
            vertexStart:    (NSUInteger)firstVertex
            vertexCount:    (NSUInteger)numVertices
            instanceCount:  (NSUInteger)numInstances
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
            patchStart:             ((NSUInteger)firstVertex) / numPatchControlPoints_
            patchCount:             ((NSUInteger)numVertices) / numPatchControlPoints_
            patchIndexBuffer:       nil
            patchIndexBufferOffset: 0
            instanceCount:          (NSUInteger)numInstances
            baseInstance:           (NSUInteger)firstInstance
        ];
    }
    else
    {
        [renderEncoder_
            drawPrimitives: primitiveType_
            vertexStart:    (NSUInteger)firstVertex
            vertexCount:    (NSUInteger)numVertices
            instanceCount:  (NSUInteger)numInstances
            baseInstance:   (NSUInteger)firstInstance
        ];
    }
}

void MTCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    if (numPatchControlPoints_ > 0)
    {
        [renderEncoder_
            drawIndexedPatches:             numPatchControlPoints_
            patchStart:                     ((NSUInteger)firstIndex) / numPatchControlPoints_
            patchCount:                     ((NSUInteger)numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:                  (NSUInteger)numInstances
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder_
            drawIndexedPrimitives:  primitiveType_
            indexCount:             (NSUInteger)numIndices
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:          (NSUInteger)numInstances
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
            patchStart:                     ((NSUInteger)firstIndex) / numPatchControlPoints_
            patchCount:                     ((NSUInteger)numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:                  (NSUInteger)numInstances
            baseInstance:                   0
        ];
    }
    else
    {
        [renderEncoder_
            drawIndexedPrimitives:  primitiveType_
            indexCount:             (NSUInteger)numIndices
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:          (NSUInteger)numInstances
            baseVertex:             (NSInteger)vertexOffset
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
            patchStart:                     ((NSUInteger)firstIndex) / numPatchControlPoints_
            patchCount:                     ((NSUInteger)numIndices) / numPatchControlPoints_
            patchIndexBuffer:               nil
            patchIndexBufferOffset:         0
            controlPointIndexBuffer:        indexBuffer_
            controlPointIndexBufferOffset:  indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:                  (NSUInteger)numInstances
            baseInstance:                   (NSUInteger)firstInstance
        ];
    }
    else
    {
        [renderEncoder_
            drawIndexedPrimitives:  primitiveType_
            indexCount:             (NSUInteger)numIndices
            indexType:              indexType_
            indexBuffer:            indexBuffer_
            indexBufferOffset:      indexTypeSize_ * ((NSUInteger)firstIndex)
            instanceCount:          (NSUInteger)numInstances
            baseVertex:             (NSInteger)vertexOffset
            baseInstance:           (NSUInteger)firstInstance
        ];
    }
}

/* ----- Compute ----- */

void MTCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    //todo
}



/*
 * ======= Private: =======
 */



} // /namespace LLGL



// ================================================================================
