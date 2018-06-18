/*
 * MTCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTCommandBuffer.h"
#include "MTRenderContext.h"
#include "../CheckedCast.h"


namespace LLGL
{


MTCommandBuffer::MTCommandBuffer(id<MTLCommandQueue> commandQueue)
{
    commandBuffer_ = [commandQueue commandBuffer];
}

MTCommandBuffer::~MTCommandBuffer()
{
    [commandBuffer_ release];
}

/* ----- Configuration ----- */

void MTCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

void MTCommandBuffer::SetViewport(const Viewport& viewport)
{
}

void MTCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
}

void MTCommandBuffer::SetScissor(const Scissor& scissor)
{
}

void MTCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
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
    //todo
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
    //todo
}


/* ----- Pipeline States ----- */

void MTCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    //todo
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
