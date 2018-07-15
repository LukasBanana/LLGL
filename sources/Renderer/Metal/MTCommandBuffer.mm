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


const std::uint32_t MTCommandBuffer::g_maxNumViewportsAndScissors;
const std::uint32_t MTCommandBuffer::g_maxNumVertexBuffers;

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
        MTLViewport viewportsML[MTCommandBuffer::g_maxNumViewportsAndScissors];
        
        numViewports = std::min(numViewports, MTCommandBuffer::g_maxNumViewportsAndScissors);
        for (std::uint32_t i = 0; i < numViewports; ++i)
            Convert(viewportsML[i], viewports[i]);
        
        [renderEncoder_ setViewports:viewportsML count:(NSUInteger)numViewports];
    }
    else
    {
        renderEncoderState_.viewportCount = std::min(numViewports, MTCommandBuffer::g_maxNumViewportsAndScissors);
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
        MTLScissorRect rects[MTCommandBuffer::g_maxNumViewportsAndScissors];
        
        numScissors = std::min(numScissors, MTCommandBuffer::g_maxNumViewportsAndScissors);
        for (std::uint32_t i = 0; i < numScissors; ++i)
            Convert(rects[i], scissors[i]);
        
        [renderEncoder_ setScissorRects:rects count:(NSUInteger)numScissors];
    }
    else
    {
        renderEncoderState_.scissorRectCount = std::min(numScissors, MTCommandBuffer::g_maxNumViewportsAndScissors);
        for (std::uint32_t i = 0; i < renderEncoderState_.scissorRectCount; ++i)
            Convert(renderEncoderState_.scissorRects[i], scissors[i]);
    }
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
    indexBuffer_ = bufferMT.GetNative();
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
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    if ((stageFlags & StageFlags::VertexStage) != 0)
        [renderEncoder_ setVertexBuffer:bufferMT.GetNative() offset:0 atIndex:static_cast<NSUInteger>(slot)];
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        [renderEncoder_ setFragmentBuffer:bufferMT.GetNative() offset:0 atIndex:static_cast<NSUInteger>(slot)];
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
    /* Get native MTLSamplerState object */
    auto& samplerMT = LLGL_CAST(MTSampler&, sampler);
    
    auto samplerState   = samplerMT.GetNative();
    auto index          = static_cast<NSUInteger>(slot);
    
    if ((stageFlags & StageFlags::VertexStage) != 0)
        [renderEncoder_ setVertexSamplerState:samplerState atIndex:index];
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        [renderEncoder_ setFragmentSamplerState:samplerState atIndex:index];
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
    MTLRenderPassDescriptor* renderPassDesc = nullptr;
    
    if (renderTarget.IsRenderContext())
    {
        /* Make this the current command buffer for the render context */
        auto& renderContextMT = LLGL_CAST(MTRenderContext&, renderTarget);
        renderContextMT.MakeCurrent(cmdBuffer_);
        
        /* Get next render pass descriptor from MetalKit view */
        MTKView* view = renderContextMT.GetMTKView();
        renderPassDesc = view.currentRenderPassDescriptor;
    }
    else
    {
        //TODO
    }
    
    /* Build render pass descriptor */
    //TODO...
    
    /* Get next render command encoder */
    if (renderPassDesc != nullptr)
    {
        renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];
        SubmitRenderEncoderState();
    }
}

void MTCommandBuffer::EndRenderPass()
{
    if (renderEncoder_ != nil)
    {
        [renderEncoder_ endEncoding];
        renderEncoder_ = nil;
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

/* ----- Extended functions ----- */

void MTCommandBuffer::NextCommandBuffer(id<MTLCommandQueue> cmdQueue)
{
    cmdBuffer_ = [cmdQueue commandBuffer];
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


} // /namespace LLGL



// ================================================================================
