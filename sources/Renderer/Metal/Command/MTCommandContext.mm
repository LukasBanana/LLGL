/*
 * MTCommandContext.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTCommandContext.h"
#include "../RenderState/MTDescriptorCache.h"
#include "../RenderState/MTConstantsCache.h"
#include "../RenderState/MTResourceHeap.h"
#include "../RenderState/MTGraphicsPSO.h"
#include "../RenderState/MTComputePSO.h"
#include "../../../Core/Assertion.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


void MTCommandContext::Reset()
{
    /* Reset all dirty bits */
    renderDirtyBits_.bits = ~0;
    isRenderEncoderPaused_ = false;
    ResetRenderEncoderState();
    ResetComputeEncoderState();
}

void MTCommandContext::Reset(id<MTLCommandBuffer> cmdBuffer)
{
    Reset();
    cmdBuffer_ = cmdBuffer;
}

void MTCommandContext::Flush()
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

id<MTLRenderCommandEncoder> MTCommandContext::BindRenderEncoder(MTLRenderPassDescriptor* renderPassDesc, bool isPrimaryRenderPass)
{
    LLGL_ASSERT_PTR(renderPassDesc);

    Flush();
    renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];

    /* Store descriptor for primary render pass */
    if (isPrimaryRenderPass)
        renderPassDesc_ = renderPassDesc;

    /* A new render command encoder forces all pipeline states to be reset */
    renderDirtyBits_.bits = ~0;

    /* Invalidate descriptor and constant caches */
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.Reset();
    if (!constantsCache_.IsEmpty())
        constantsCache_.Reset();

    return renderEncoder_;
}

id<MTLComputeCommandEncoder> MTCommandContext::BindComputeEncoder()
{
    if (computeEncoder_ == nil)
    {
        Flush();
        computeEncoder_ = [cmdBuffer_ computeCommandEncoder];

        /* A new compute command encoder forces all pipeline states to be reset */
        computeDirtyBits_.bits = ~0;

        /* Invalidate descriptor and constant caches */
        if (!descriptorCache_.IsEmpty())
            descriptorCache_.Reset();
        if (!constantsCache_.IsEmpty())
            constantsCache_.Reset();
    }
    return computeEncoder_;
}

id<MTLBlitCommandEncoder> MTCommandContext::BindBlitEncoder()
{
    if (blitEncoder_ == nil)
    {
        Flush();
        blitEncoder_ = [cmdBuffer_ blitCommandEncoder];
    }
    return blitEncoder_;
}

void MTCommandContext::PauseRenderEncoder()
{
    if (renderEncoder_ != nil && !isRenderEncoderPaused_)
        isRenderEncoderPaused_ = true;
}

void MTCommandContext::ResumeRenderEncoder()
{
    if (isRenderEncoderPaused_)
    {
        /* Bind new render command encoder with previous render pass */
        auto renderPassDesc = CopyRenderPassDesc();
        {
            for_range(i, 8u)
                renderPassDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
            renderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
            renderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
        }
        BindRenderEncoder(renderPassDesc);
        [renderPassDesc release];
        isRenderEncoderPaused_ = false;
    }
}

MTLRenderPassDescriptor* MTCommandContext::CopyRenderPassDesc()
{
    return (MTLRenderPassDescriptor*)[renderPassDesc_ copy];
}

static void ConvertMTLViewport(MTLViewport& dst, const Viewport& src)
{
    const double scaling = 1.0;//2.0 for retina display
    dst.originX = static_cast<double>(src.x)*scaling;
    dst.originY = static_cast<double>(src.y)*scaling;
    dst.width   = static_cast<double>(src.width)*scaling;
    dst.height  = static_cast<double>(src.height)*scaling;
    dst.znear   = static_cast<double>(src.minDepth);
    dst.zfar    = static_cast<double>(src.maxDepth);
}

void MTCommandContext::SetViewports(const Viewport* viewports, NSUInteger viewportCount)
{
    renderEncoderState_.viewportCount = std::min(viewportCount, NSUInteger(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));
    for_range(i, renderEncoderState_.viewportCount)
        ConvertMTLViewport(renderEncoderState_.viewports[i], viewports[i]);
    renderDirtyBits_.viewports = 1;
}

static void Convert(MTLScissorRect& dst, const Scissor& scissor)
{
    dst.x       = static_cast<NSUInteger>(std::max(0, scissor.x));
    dst.y       = static_cast<NSUInteger>(std::max(0, scissor.y));
    dst.width   = static_cast<NSUInteger>(std::max(0, scissor.width));
    dst.height  = static_cast<NSUInteger>(std::max(0, scissor.height));
}

void MTCommandContext::SetScissorRects(const Scissor* scissors, NSUInteger scissorCount)
{
    renderEncoderState_.scissorRectCount = std::min(scissorCount, NSUInteger(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));
    for_range(i, renderEncoderState_.scissorRectCount)
        Convert(renderEncoderState_.scissorRects[i], scissors[i]);
    renderDirtyBits_.scissors = 1;
}

void MTCommandContext::SetVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset)
{
    renderEncoderState_.vertexBuffers[0]            = buffer;
    renderEncoderState_.vertexBufferOffsets[0]      = offset;
    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = 1;
    renderDirtyBits_.vertexBuffers = 1;
}

void MTCommandContext::SetVertexBuffers(const id<MTLBuffer>* buffers, const NSUInteger* offsets, NSUInteger bufferCount)
{
    ::memcpy(renderEncoderState_.vertexBuffers, buffers, sizeof(id) * bufferCount);
    ::memcpy(renderEncoderState_.vertexBufferOffsets, offsets, sizeof(NSUInteger) * bufferCount);

    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = bufferCount;

    renderDirtyBits_.vertexBuffers = 1;
}

void MTCommandContext::SetGraphicsPSO(MTGraphicsPSO* pipelineState)
{
    if (pipelineState != nullptr && renderEncoderState_.graphicsPSO != pipelineState)
    {
        renderEncoderState_.graphicsPSO         = pipelineState;
        renderEncoderState_.blendColorDynamic   = pipelineState->IsBlendColorDynamic();
        renderEncoderState_.stencilRefDynamic   = pipelineState->IsStencilRefDynamic();
        renderDirtyBits_.graphicsPSO = 1;
        descriptorCache_.Reset(pipelineState->GetPipelineLayout());
        constantsCache_.Reset(pipelineState->GetConstantsCacheLayout());
    }
}

void MTCommandContext::SetGraphicsResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet)
{
    renderEncoderState_.graphicsResourceHeap    = resourceHeap;
    renderEncoderState_.graphicsResourceSet     = descriptorSet;
    renderDirtyBits_.graphicsResourceHeap       = 1;
}

void MTCommandContext::SetBlendColor(const float blendColor[4])
{
    renderEncoderState_.blendColor[0] = blendColor[0];
    renderEncoderState_.blendColor[1] = blendColor[1];
    renderEncoderState_.blendColor[2] = blendColor[2];
    renderEncoderState_.blendColor[3] = blendColor[3];
    renderDirtyBits_.blendColor = 1;
}

void MTCommandContext::SetStencilRef(std::uint32_t ref, const StencilFace face)
{
    switch (face)
    {
        case StencilFace::FrontAndBack:
            renderEncoderState_.stencilFrontRef   = ref;
            renderEncoderState_.stencilBackRef    = ref;
            break;
        case StencilFace::Front:
            renderEncoderState_.stencilFrontRef   = ref;
            break;
        case StencilFace::Back:
            renderEncoderState_.stencilBackRef    = ref;
            break;
    }
    renderDirtyBits_.stencilRef = 1;
}

void MTCommandContext::SetComputePSO(MTComputePSO* pipelineState)
{
    if (pipelineState != nullptr && computeEncoderState_.computePSO != pipelineState)
    {
        computeEncoderState_.computePSO = pipelineState;
        computeDirtyBits_.computePSO    = 1;
        descriptorCache_.Reset(pipelineState->GetPipelineLayout());
        constantsCache_.Reset(pipelineState->GetConstantsCacheLayout());
    }
}

void MTCommandContext::SetComputeResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet)
{
    computeEncoderState_.computeResourceHeap    = resourceHeap;
    computeEncoderState_.computeResourceSet     = descriptorSet;
    computeDirtyBits_.computeResourceHeap       = 1;
}

void MTCommandContext::RebindResourceHeap(id<MTLComputeCommandEncoder> computeEncoder)
{
    if (computeEncoderState_.computeResourceHeap != nullptr)
    {
        computeEncoderState_.computeResourceHeap->BindComputeResources(
            computeEncoder,
            computeEncoderState_.computeResourceSet
        );
    }
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.FlushComputeResourcesForced(computeEncoder);
    if (!constantsCache_.IsEmpty())
        constantsCache_.FlushComputeResourcesForced(computeEncoder);
}

id<MTLRenderCommandEncoder> MTCommandContext::FlushAndGetRenderEncoder()
{
    if (renderDirtyBits_.bits != 0)
        SubmitRenderEncoderState();
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.FlushGraphicsResources(GetRenderEncoder());
    if (!constantsCache_.IsEmpty())
        constantsCache_.FlushGraphicsResources(GetRenderEncoder());
    return GetRenderEncoder();
}

id<MTLComputeCommandEncoder> MTCommandContext::FlushAndGetComputeEncoder()
{
    /* Always compute encoder here, because there is no section like with Begin/EndRenderPass */
    BindComputeEncoder();
    if (computeDirtyBits_.bits != 0)
        SubmitComputeEncoderState();
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.FlushComputeResources(GetComputeEncoder());
    if (!constantsCache_.IsEmpty())
        constantsCache_.FlushComputeResources(GetComputeEncoder());
    return GetComputeEncoder();
}


/*
 * ======= Private: =======
 */

void MTCommandContext::SubmitRenderEncoderState()
{
    if (renderEncoder_ == nil)
        return;

    if (renderEncoderState_.viewportCount > 0 && renderDirtyBits_.viewports != 0)
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
    if (renderEncoderState_.scissorRectCount > 0 && renderDirtyBits_.scissors != 0)
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
    if (renderEncoderState_.vertexBufferRange.length > 0 && renderDirtyBits_.vertexBuffers != 0)
    {
        /* Bind vertex buffers */
        [renderEncoder_
            setVertexBuffers:   renderEncoderState_.vertexBuffers
            offsets:            renderEncoderState_.vertexBufferOffsets
            withRange:          renderEncoderState_.vertexBufferRange
        ];
    }
    if (renderEncoderState_.graphicsPSO != nullptr && renderDirtyBits_.graphicsPSO != 0)
    {
        /* Bind graphics pipeline */
        renderEncoderState_.graphicsPSO->Bind(renderEncoder_);
    }
    if (renderEncoderState_.graphicsResourceHeap != nullptr && renderDirtyBits_.graphicsResourceHeap != 0)
    {
        /* Bind resource heap */
        renderEncoderState_.graphicsResourceHeap->BindGraphicsResources(
            renderEncoder_,
            renderEncoderState_.graphicsResourceSet
        );
    }
    if (renderEncoderState_.blendColorDynamic && renderDirtyBits_.blendColor != 0)
    {
        /* Set blend color */
        [renderEncoder_
            setBlendColorRed:   renderEncoderState_.blendColor[0]
            green:              renderEncoderState_.blendColor[1]
            blue:               renderEncoderState_.blendColor[2]
            alpha:              renderEncoderState_.blendColor[3]
        ];
    }
    if (renderEncoderState_.stencilRefDynamic && renderDirtyBits_.stencilRef != 0)
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
    renderDirtyBits_.bits = 0;
}

void MTCommandContext::ResetRenderEncoderState()
{
    renderEncoderState_.viewportCount             = 0;
    renderEncoderState_.scissorRectCount          = 0;
    renderEncoderState_.vertexBufferRange.length  = 0;
    renderEncoderState_.graphicsPSO               = nullptr;
    renderEncoderState_.graphicsResourceHeap      = nullptr;
}

void MTCommandContext::SubmitComputeEncoderState()
{
    if (computeEncoder_ == nil)
        return;

    if (computeEncoderState_.computePSO != nullptr && computeEncoderState_.computePSO != 0)
    {
        /* Bind compute pipeline */
        computeEncoderState_.computePSO->Bind(computeEncoder_);
    }
    if (computeEncoderState_.computeResourceHeap != nullptr && computeEncoderState_.computeResourceHeap != 0)
    {
        /* Bind resource heap */
        computeEncoderState_.computeResourceHeap->BindComputeResources(
            computeEncoder_,
            computeEncoderState_.computeResourceSet
        );
    }

    /* Reset all dirty bits */
    computeDirtyBits_.bits = 0;
}

void MTCommandContext::ResetComputeEncoderState()
{
    computeEncoderState_.computeResourceHeap = nullptr;
}


} // /namespace LLGL



// ================================================================================
