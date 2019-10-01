/*
 * MTEncoderScheduler.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTEncoderScheduler.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTGraphicsPipeline.h"
#include "RenderState/MTComputePipeline.h"
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/Platform/Platform.h>
#include <algorithm>


namespace LLGL
{

    
void MTEncoderScheduler::Reset(id<MTLCommandBuffer> cmdBuffer)
{
    cmdBuffer_ = cmdBuffer;
    isRenderEncoderPaused_ = false;
    ResetRenderEncoderState();
}

void MTEncoderScheduler::Flush()
{
    if (renderEncoder_ != nil)
    {
        [renderEncoder_ endEncoding];
        renderEncoder_ = nil;
    }
    else if (computeEncoder_ != nil)
    {
        [computeEncoder_ endEncoding];
        computeEncoder_ = nil;
    }
    else if (blitEncoder_ != nil)
    {
        [blitEncoder_ endEncoding];
        blitEncoder_ = nil;
    }
}

id<MTLRenderCommandEncoder> MTEncoderScheduler::BindRenderEncoder(MTLRenderPassDescriptor* renderPassDesc, bool primaryRenderPass)
{
    Flush();
    renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];

    /* Store descriptor for primary render pass */
    if (primaryRenderPass)
        renderPassDesc_ = renderPassDesc;

    /* A new render command encoder forces all pipeline states to be reset */
    dirtyBits_.bits = ~0;

    return renderEncoder_;
}

id<MTLComputeCommandEncoder> MTEncoderScheduler::BindComputeEncoder()
{
    if (computeEncoder_ == nil)
    {
        Flush();
        computeEncoder_ = [cmdBuffer_ computeCommandEncoder];
    }
    return computeEncoder_;
}

id<MTLBlitCommandEncoder> MTEncoderScheduler::BindBlitEncoder()
{
    if (blitEncoder_ == nil)
    {
        Flush();
        blitEncoder_ = [cmdBuffer_ blitCommandEncoder];
    }
    return blitEncoder_;
}

void MTEncoderScheduler::PauseRenderEncoder()
{
    if (renderEncoder_ != nil && !isRenderEncoderPaused_)
        isRenderEncoderPaused_ = true;
}

void MTEncoderScheduler::ResumeRenderEncoder()
{
    if (isRenderEncoderPaused_)
    {
        auto renderPassDesc = CopyRenderPassDesc();
        {
            for (NSUInteger i = 0; i < 8; ++i)
                renderPassDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
            renderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
            renderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
        }
        BindRenderEncoder(renderPassDesc);
        isRenderEncoderPaused_ = false;
    }
}

MTLRenderPassDescriptor* MTEncoderScheduler::CopyRenderPassDesc()
{
    return (MTLRenderPassDescriptor*)[renderPassDesc_ copy];
}

static void Convert(MTLViewport& dst, const Viewport& src)
{
    const double scaling = 1.0;//2.0 for retina display
    dst.originX = static_cast<double>(src.x)*scaling;
    dst.originY = static_cast<double>(src.y)*scaling;
    dst.width   = static_cast<double>(src.width)*scaling;
    dst.height  = static_cast<double>(src.height)*scaling;
    dst.znear   = static_cast<double>(src.minDepth);
    dst.zfar    = static_cast<double>(src.maxDepth);
}

void MTEncoderScheduler::SetViewports(const Viewport* viewports, NSUInteger viewportCount)
{
    renderEncoderState_.viewportCount = std::min(viewportCount, NSUInteger(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));
    for (std::uint32_t i = 0; i < renderEncoderState_.viewportCount; ++i)
        Convert(renderEncoderState_.viewports[i], viewports[i]);
    dirtyBits_.viewports = 1;
}

static void Convert(MTLScissorRect& dst, const Scissor& scissor)
{
    dst.x       = static_cast<NSUInteger>(std::max(0, scissor.x));
    dst.y       = static_cast<NSUInteger>(std::max(0, scissor.y));
    dst.width   = static_cast<NSUInteger>(std::max(0, scissor.width));
    dst.height  = static_cast<NSUInteger>(std::max(0, scissor.height));
}

void MTEncoderScheduler::SetScissorRects(const Scissor* scissors, NSUInteger scissorCount)
{
    renderEncoderState_.scissorRectCount = std::min(scissorCount, NSUInteger(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));
    for (std::uint32_t i = 0; i < renderEncoderState_.scissorRectCount; ++i)
        Convert(renderEncoderState_.scissorRects[i], scissors[i]);
    dirtyBits_.scissors = 1;
}

void MTEncoderScheduler::SetVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset)
{
    renderEncoderState_.vertexBuffers[0]            = buffer;
    renderEncoderState_.vertexBufferOffsets[0]      = offset;
    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = 1;
    dirtyBits_.vertexBuffers = 1;
}

void MTEncoderScheduler::SetVertexBuffers(const id<MTLBuffer>* buffers, const NSUInteger* offsets, NSUInteger bufferCount)
{
    std::copy(buffers, buffers + bufferCount, renderEncoderState_.vertexBuffers);
    std::copy(offsets, offsets + bufferCount, renderEncoderState_.vertexBufferOffsets);

    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = bufferCount;

    dirtyBits_.vertexBuffers = 1;
}

void MTEncoderScheduler::SetGraphicsPipeline(MTGraphicsPipeline* graphicsPipeline)
{
    renderEncoderState_.graphicsPipeline = graphicsPipeline;
    dirtyBits_.graphicsPipeline = 1;
}

void MTEncoderScheduler::SetGraphicsResourceHeap(MTResourceHeap* resourceHeap)
{
    renderEncoderState_.resourceHeap = resourceHeap;
    dirtyBits_.resourceHeap = 1;
}

id<MTLRenderCommandEncoder> MTEncoderScheduler::GetRenderEncoderAndFlushRenderPass()
{
    if (dirtyBits_.bits != 0)
        SubmitRenderEncoderState();
    return GetRenderEncoder();
}


/*
 * ======= Private: =======
 */

void MTEncoderScheduler::SubmitRenderEncoderState()
{
    if (renderEncoder_ != nil)
    {
        if (renderEncoderState_.viewportCount > 0 && dirtyBits_.viewports != 0)
        {
            /* Bind viewports */
            #ifndef LLGL_OS_IOS //TODO: since iOS 12.0
            [renderEncoder_
                setViewports:   renderEncoderState_.viewports
                count:          renderEncoderState_.viewportCount
            ];
            #else
            [renderEncoder_ setViewport:renderEncoderState_.viewports[0]];
            #endif
        }
        if (renderEncoderState_.scissorRectCount > 0 && dirtyBits_.scissors != 0)
        {
            /* Bind scissor rectangles */
            #ifndef LLGL_OS_IOS //TODO: since iOS 12.0
            [renderEncoder_
                setScissorRects:    renderEncoderState_.scissorRects
                count:              renderEncoderState_.scissorRectCount
            ];
            #else
            [renderEncoder_ setScissorRect:renderEncoderState_.scissorRects[0]];
            #endif
        }
        if (renderEncoderState_.vertexBufferRange.length > 0 && dirtyBits_.vertexBuffers != 0)
        {
            /* Bind vertex buffers */
            [renderEncoder_
                setVertexBuffers:   renderEncoderState_.vertexBuffers
                offsets:            renderEncoderState_.vertexBufferOffsets
                withRange:          renderEncoderState_.vertexBufferRange
            ];
        }
        if (renderEncoderState_.graphicsPipeline != nullptr && dirtyBits_.graphicsPipeline != 0)
        {
            /* Bind graphics pipeline */
            renderEncoderState_.graphicsPipeline->Bind(renderEncoder_);
        }
        if (renderEncoderState_.resourceHeap != nullptr && dirtyBits_.resourceHeap != 0)
        {
            /* Bind resource heap */
            renderEncoderState_.resourceHeap->BindGraphicsResources(renderEncoder_);
        }

        /* Reset all dirty bits */
        dirtyBits_.bits = 0;
    }
}

void MTEncoderScheduler::ResetRenderEncoderState()
{
    renderEncoderState_.viewportCount               = 0;
    renderEncoderState_.scissorRectCount            = 0;
    renderEncoderState_.vertexBufferRange.length    = 0;
    renderEncoderState_.graphicsPipeline            = nullptr;
    renderEncoderState_.resourceHeap                = nullptr;
}


} // /namespace LLGL



// ================================================================================
