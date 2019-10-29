/*
 * MTEncoderScheduler.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTEncoderScheduler.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTGraphicsPSO.h"
#include "RenderState/MTComputePSO.h"
#include <LLGL/PipelineStateFlags.h>
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

void MTEncoderScheduler::SetGraphicsPSO(MTGraphicsPSO* pipelineState)
{
    if (pipelineState != nullptr)
    {
        renderEncoderState_.graphicsPSO         = pipelineState;
        renderEncoderState_.blendColorDynamic   = pipelineState->IsBlendColorDynamic();
        renderEncoderState_.stencilRefDynamic   = pipelineState->IsStencilRefDynamic();
        dirtyBits_.graphicsPipeline = 1;
    }
}

void MTEncoderScheduler::SetGraphicsResourceHeap(MTResourceHeap* resourceHeap)
{
    renderEncoderState_.resourceHeap = resourceHeap;
    dirtyBits_.resourceHeap = 1;
}

void MTEncoderScheduler::SetBlendColor(const float* blendColor)
{
    renderEncoderState_.blendColor[0] = blendColor[0];
    renderEncoderState_.blendColor[1] = blendColor[1];
    renderEncoderState_.blendColor[2] = blendColor[2];
    renderEncoderState_.blendColor[3] = blendColor[3];
    dirtyBits_.blendColor = 1;
}

void MTEncoderScheduler::SetStencilRef(std::uint32_t ref, const StencilFace face)
{
    switch (face)
    {
        case StencilFace::FrontAndBack:
            renderEncoderState_.stencilFrontRef = ref;
            renderEncoderState_.stencilBackRef = ref;
            break;
        case StencilFace::Front:
            renderEncoderState_.stencilFrontRef = ref;
            break;
        case StencilFace::Back:
            renderEncoderState_.stencilBackRef = ref;
            break;
    }
    dirtyBits_.stencilRef = 1;
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
            if (@available(macOS 10.13, iOS 12.0, *))
            {
                [renderEncoder_
                    setViewports:   renderEncoderState_.viewports
                    count:          renderEncoderState_.viewportCount
                ];
            }
            else
                [renderEncoder_ setViewport:renderEncoderState_.viewports[0]];
        }
        if (renderEncoderState_.scissorRectCount > 0 && dirtyBits_.scissors != 0)
        {
            /* Bind scissor rectangles */
            if (@available(macOS 10.13, iOS 12.0, *))
            {
                [renderEncoder_
                    setScissorRects:    renderEncoderState_.scissorRects
                    count:              renderEncoderState_.scissorRectCount
                ];
            }
            else
                [renderEncoder_ setScissorRect:renderEncoderState_.scissorRects[0]];
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
        if (renderEncoderState_.graphicsPSO != nullptr && dirtyBits_.graphicsPipeline != 0)
        {
            /* Bind graphics pipeline */
            renderEncoderState_.graphicsPSO->Bind(renderEncoder_);
        }
        if (renderEncoderState_.resourceHeap != nullptr && dirtyBits_.resourceHeap != 0)
        {
            /* Bind resource heap */
            renderEncoderState_.resourceHeap->BindGraphicsResources(renderEncoder_);
        }
        if (renderEncoderState_.blendColorDynamic && dirtyBits_.blendColor != 0)
        {
            /* Set blend color */
            [renderEncoder_
                setBlendColorRed:   renderEncoderState_.blendColor[0]
                green:              renderEncoderState_.blendColor[1]
                blue:               renderEncoderState_.blendColor[2]
                alpha:              renderEncoderState_.blendColor[3]
            ];
        }
        if (renderEncoderState_.stencilRefDynamic && dirtyBits_.stencilRef != 0)
        {
            /* Set stencil reference */
            if (renderEncoderState_.stencilFrontRef != renderEncoderState_.stencilBackRef)
            {
                [renderEncoder_
                    setStencilFrontReferenceValue:  renderEncoderState_.stencilFrontRef
                    backReferenceValue:             renderEncoderState_.stencilBackRef
                ];
            }
            else
                [renderEncoder_ setStencilReferenceValue:renderEncoderState_.stencilFrontRef];
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
    renderEncoderState_.graphicsPSO                 = nullptr;
    renderEncoderState_.resourceHeap                = nullptr;
}


} // /namespace LLGL



// ================================================================================
