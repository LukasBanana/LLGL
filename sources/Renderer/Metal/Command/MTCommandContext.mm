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
#include "../RenderState/MTRenderPass.h"
#include "../Shader/MTShader.h"
#include "../MTSwapChain.h"
#include "../Texture/MTRenderTarget.h"
#include "../../../Core/Assertion.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/TypeInfo.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


static constexpr NSUInteger g_tessFactorBufferAlignment = (sizeof(MTLQuadTessellationFactorsHalf) * 256);

MTCommandContext::MTCommandContext(id<MTLDevice> device) :
    tessFactorBuffer_    { device,
                           MTLResourceStorageModePrivate,
                           g_tessFactorBufferAlignment           },
    maxThreadgroupSizeX_ { device.maxThreadsPerThreadgroup.width }
{
}

void MTCommandContext::Reset()
{
    /* Reset all dirty bits */
    renderDirtyBits_        = ~0u;
    computeDirtyBits_       = ~0u;
    isRenderEncoderPaused_  = false;
    boundSwapChain_         = nullptr;
    ResetRenderEncoderState();
    ResetComputeEncoderState();
    ResetContextState();
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

void MTCommandContext::BeginRenderPass(
    RenderTarget*       renderTarget,
    const MTRenderPass* renderPassMT,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    LLGL_ASSERT_PTR(renderTarget);
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        /* Get next render pass descriptor from MetalKit view */
        auto* swapChainMT = LLGL_CAST(MTSwapChain*, renderTarget);
        if (renderPassMT != nullptr)
            BeginRenderPassWithDescriptor(swapChainMT->GetAndUpdateNativeRenderPass(*renderPassMT, numClearValues, clearValues), swapChainMT);
        else
            BeginRenderPassWithDescriptor(swapChainMT->GetNativeRenderPass(), swapChainMT);
    }
    else
    {
        /* Get render pass descriptor from render target */
        auto* renderTargetMT = LLGL_CAST(MTRenderTarget*, renderTarget);
        if (renderPassMT != nullptr)
            BeginRenderPassWithDescriptor(renderTargetMT->GetAndUpdateNativeRenderPass(*renderPassMT, numClearValues, clearValues), nullptr);
        else
            BeginRenderPassWithDescriptor(renderTargetMT->GetNativeRenderPass(), nullptr);
    }
}

void MTCommandContext::UpdateRenderPass(MTLRenderPassDescriptor* renderPassDesc)
{
    LLGL_ASSERT_PTR(renderPassDesc);
    if (contextState_.isInsideRenderPass)
        BindRenderEncoderWithDescriptor(renderPassDesc);
}

void MTCommandContext::EndRenderPass()
{
    if (contextState_.isInsideRenderPass)
    {
        Flush();
        contextState_.isInsideRenderPass = false;
        [renderPassDesc_ release];
    }
}

id<MTLRenderCommandEncoder> MTCommandContext::BindRenderEncoder()
{
    /* Resume render encoder if we are inside a render pass */
    if (contextState_.isInsideRenderPass && contextState_.encoderState != MTEncoderState::Render)
        ResumeRenderEncoder();

    if (renderEncoder_ == nil)
        BindRenderEncoderWithDescriptor(renderPassDesc_);

    return renderEncoder_;
}

id<MTLComputeCommandEncoder> MTCommandContext::BindComputeEncoder()
{
    /* Pause render encoder if we are inside a render pass */
    if (contextState_.isInsideRenderPass && contextState_.encoderState == MTEncoderState::Render)
        PauseRenderEncoder();

    if (computeEncoder_ == nil)
    {
        Flush();
        computeEncoder_ = [cmdBuffer_ computeCommandEncoder];

        /* A new compute command encoder forces all pipeline states to be reset */
        computeDirtyBits_ = ~0;

        /* Invalidate descriptor and constant caches */
        if (!descriptorCache_.IsEmpty())
            descriptorCache_.Reset();
        if (!constantsCache_.IsEmpty())
            constantsCache_.Reset();

        /* Store compute encoder mode */
        contextState_.encoderState = MTEncoderState::Compute;
    }

    return computeEncoder_;
}

id<MTLBlitCommandEncoder> MTCommandContext::BindBlitEncoder()
{
    /* Pause render encoder if we are inside a render pass */
    if (contextState_.isInsideRenderPass && contextState_.encoderState == MTEncoderState::Render)
        PauseRenderEncoder();

    if (blitEncoder_ == nil)
    {
        Flush();
        blitEncoder_ = [cmdBuffer_ blitCommandEncoder];

        /* Store blit encoder mode */
        contextState_.encoderState = MTEncoderState::Blit;
    }

    return blitEncoder_;
}

id<MTLRenderCommandEncoder> MTCommandContext::FlushAndGetRenderEncoder()
{
    BindRenderEncoder();

    /* Flush render encoder state */
    if (renderDirtyBits_ != 0)
        SubmitRenderEncoderState();
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.FlushGraphicsResources(GetRenderEncoder());
    if (!constantsCache_.IsEmpty())
        constantsCache_.FlushGraphicsResources(GetRenderEncoder());

    return GetRenderEncoder();
}

id<MTLComputeCommandEncoder> MTCommandContext::FlushAndGetComputeEncoder()
{
    BindComputeEncoder();

    /* Flush compute encoder state */
    if (computeDirtyBits_ != 0)
        SubmitComputeEncoderState();
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.FlushComputeResources(GetComputeEncoder());
    if (!constantsCache_.IsEmpty())
        constantsCache_.FlushComputeResources(GetComputeEncoder());

    return GetComputeEncoder();
}

MTLRenderPassDescriptor* MTCommandContext::CopyRenderPassDesc()
{
    return (MTLRenderPassDescriptor*)[renderPassDesc_ copy];
}

void MTCommandContext::DispatchThreads1D(
    id<MTLComputeCommandEncoder>    computeEncoder,
    id<MTLComputePipelineState>     computePSO,
    NSUInteger                      numThreads)
{
    const NSUInteger maxLocalThreads = GetMaxLocalThreads(computePSO);
    LLGL_ASSERT(maxLocalThreads > 0);

    if (@available(iOS 11.0, macOS 10.13, *))
    {
        /* Dispatch all threads with a single command and let Metal distribute the full and partial threadgroups */
        [computeEncoder
            dispatchThreads:        MTLSizeMake(numThreads, 1, 1)
            threadsPerThreadgroup:  MTLSizeMake(std::min(numThreads, maxLocalThreads), 1, 1)
        ];
    }
    else
    {
        /* Dispatch threadgroups with as many local threads as possible */
        const NSUInteger numThreadGroups = numThreads / maxLocalThreads;
        if (numThreadGroups > 0)
        {
            [computeEncoder
                dispatchThreadgroups:   MTLSizeMake(numThreadGroups, 1, 1)
                threadsPerThreadgroup:  MTLSizeMake(maxLocalThreads, 1, 1)
            ];
        }

        /* Dispatch local threads for remaining range */
        const NSUInteger remainingValues = numThreads % maxLocalThreads;
        if (remainingValues > 0)
        {
            [computeEncoder
                dispatchThreadgroups:   MTLSizeMake(1, 1, 1)
                threadsPerThreadgroup:  MTLSizeMake(remainingValues, 1, 1)
            ];
        }
    }
}

id<MTLRenderCommandEncoder> MTCommandContext::DispatchTessellationAndGetRenderEncoder(NSUInteger numPatches, NSUInteger numInstances)
{
    /* Ensure internal tessellation factor buffer is large enough */
    const NSUInteger numPatchesAndInstances = numPatches * numInstances;
    id<MTLBuffer> tessFactorBuffer = GetTessFactorBufferAndGrow(numPatchesAndInstances);

    if (contextState_.tessPipelineState != nil)
    {
        /* Encode kernel dispatch to generate tessellation factors for each patch */
        auto computeEncoder = BindComputeEncoder();

        /* Rebind resource heap to bind compute stage resources to the new compute command encoder */
        RebindResourceHeap(computeEncoder);

        /* Dispatch kernel to generate patch tessellation factors */
        [computeEncoder setComputePipelineState: contextState_.tessPipelineState];
        [computeEncoder
            setBuffer:  tessFactorBuffer
            offset:     0
            atIndex:    this->bindingTable.tessFactorBufferSlot
        ];

        DispatchThreads1D(computeEncoder, contextState_.tessPipelineState, numPatchesAndInstances);
    }

    /* Get render command encoder and set tessellation factor buffer */
    id<MTLRenderCommandEncoder> renderEncoder = FlushAndGetRenderEncoder();

    [renderEncoder
        setTessellationFactorBuffer:    tessFactorBuffer
        offset:                         0
        instanceStride:                 numPatches * sizeof(MTLQuadTessellationFactorsHalf)
    ];

    return renderEncoder;
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
    renderDirtyBits_ |= DirtyBit_Viewports;
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
    renderDirtyBits_ |= DirtyBit_Scissors;
}

void MTCommandContext::SetVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset)
{
    renderEncoderState_.vertexBuffers[0]            = buffer;
    renderEncoderState_.vertexBufferOffsets[0]      = offset;
    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = 1;
    renderDirtyBits_ |= DirtyBit_VertexBuffers;
}

void MTCommandContext::SetVertexBuffers(const id<MTLBuffer>* buffers, const NSUInteger* offsets, NSUInteger bufferCount)
{
    ::memcpy(renderEncoderState_.vertexBuffers, buffers, sizeof(id) * bufferCount);
    ::memcpy(renderEncoderState_.vertexBufferOffsets, offsets, sizeof(NSUInteger) * bufferCount);

    renderEncoderState_.vertexBufferRange.location  = 0;
    renderEncoderState_.vertexBufferRange.length    = bufferCount;

    renderDirtyBits_ |= DirtyBit_VertexBuffers;
}

static NSUInteger GetTessFactorSizeForPatchType(const MTLPatchType patchType)
{
    switch (patchType)
    {
        case MTLPatchTypeNone:      return 0;
        case MTLPatchTypeTriangle:  return sizeof(MTLTriangleTessellationFactorsHalf);
        case MTLPatchTypeQuad:      return sizeof(MTLQuadTessellationFactorsHalf);
    }
    return 0;
}

void MTCommandContext::SetGraphicsPSO(MTGraphicsPSO* pipelineState)
{
    if (pipelineState != nullptr && renderEncoderState_.graphicsPSO != pipelineState)
    {
        renderDirtyBits_ |= DirtyBit_GraphicsPSO;

        renderEncoderState_.graphicsPSO             = pipelineState;
        renderEncoderState_.blendColorDynamic       = pipelineState->IsBlendColorDynamic();
        renderEncoderState_.stencilRefDynamic       = pipelineState->IsStencilRefDynamic();
        renderEncoderState_.isScissorTestEnabled    = pipelineState->HasScissorTest();

        const bool hasStaticViewportAndScissor = pipelineState->GetStaticState(
            renderEncoderState_.viewports,
            renderEncoderState_.viewportCount,
            renderEncoderState_.scissorRects,
            renderEncoderState_.scissorRectCount
        );
        if (hasStaticViewportAndScissor)
            renderDirtyBits_ |= (DirtyBit_Viewports | DirtyBit_Scissors);

        descriptorCache_.Reset(pipelineState->GetPipelineLayout());
        constantsCache_.Reset(pipelineState->GetConstantsCacheLayout());

        /* Cache context state */
        contextState_.boundPipelineState    = pipelineState;
        contextState_.primitiveType         = pipelineState->GetMTLPrimitiveType();
        contextState_.numPatchControlPoints = pipelineState->GetNumPatchControlPoints();
        contextState_.tessPipelineState     = pipelineState->GetTessPipelineState();
        contextState_.tessFactorSize        = GetTessFactorSizeForPatchType(pipelineState->GetPatchType());
    }
}

void MTCommandContext::SetGraphicsResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet)
{
    renderEncoderState_.graphicsResourceHeap    = resourceHeap;
    renderEncoderState_.graphicsResourceSet     = descriptorSet;
    renderDirtyBits_ |= DirtyBit_GraphicsResourceHeap;
}

void MTCommandContext::SetBlendColor(const float blendColor[4])
{
    renderEncoderState_.blendColor[0] = blendColor[0];
    renderEncoderState_.blendColor[1] = blendColor[1];
    renderEncoderState_.blendColor[2] = blendColor[2];
    renderEncoderState_.blendColor[3] = blendColor[3];
    renderDirtyBits_ |= DirtyBit_BlendColor;
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
    renderDirtyBits_ |= DirtyBit_StencilRef;
}

void MTCommandContext::SetVisibilityBuffer(id<MTLBuffer> buffer, MTLVisibilityResultMode mode, NSUInteger offset)
{
    if (buffer != nil)
    {
        /* Check if a new render pass must be started with the new visibility buffer */
        if (contextState_.visBuffer != buffer)
        {
            /* Start new render pass to bind visibility buffer */
            MTLRenderPassDescriptor* renderPassDesc = CopyRenderPassDesc();
            renderPassDesc.visibilityResultBuffer = buffer;
            UpdateRenderPass(renderPassDesc);
            [renderPassDesc release];
            contextState_.visBuffer = buffer;
        }

        /* Check if visibility mode or offset must be updated in the render command encoder */
        if (renderEncoderState_.visResultMode != mode ||
            renderEncoderState_.visResultOffset != offset)
        {
            renderEncoderState_.visResultMode   = mode;
            renderEncoderState_.visResultOffset = offset;
            renderDirtyBits_ |= DirtyBit_VisibilityResultMode;
        }
    }
    else if (renderEncoderState_.visResultMode != MTLVisibilityResultModeDisabled)
    {
        /* Disable visibility result mode but don't bother starting a new render pass */
        renderEncoderState_.visResultMode = MTLVisibilityResultModeDisabled;
        renderDirtyBits_ |= DirtyBit_VisibilityResultMode;
    }
}

void MTCommandContext::SetComputePSO(MTComputePSO* pipelineState)
{
    if (pipelineState != nullptr && computeEncoderState_.computePSO != pipelineState)
    {
        computeEncoderState_.computePSO = pipelineState;
        computeDirtyBits_ |= DirtyBit_ComputePSO;
        descriptorCache_.Reset(pipelineState->GetPipelineLayout());
        constantsCache_.Reset(pipelineState->GetConstantsCacheLayout());

        /* Cache context state */
        contextState_.boundPipelineState = pipelineState;
        if (const MTShader* computeShader = pipelineState->GetComputeShader())
            contextState_.threadsPerThreadgroup = computeShader->GetNumThreadsPerGroup();
    }
}

void MTCommandContext::SetComputeResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet)
{
    computeEncoderState_.computeResourceHeap    = resourceHeap;
    computeEncoderState_.computeResourceSet     = descriptorSet;
    computeDirtyBits_ |= DirtyBit_ComputeResourceHeap;
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

void MTCommandContext::SetIndexStream(id<MTLBuffer> indexBuffer, NSUInteger offset, bool indexType16Bits)
{
    contextState_.indexBuffer       = indexBuffer;
    contextState_.indexBufferOffset = offset;
    if (indexType16Bits)
    {
        contextState_.indexType     = MTLIndexTypeUInt16;
        contextState_.indexTypeSize = 2;
    }
    else
    {
        contextState_.indexType     = MTLIndexTypeUInt32;
        contextState_.indexTypeSize = 4;
    }
}

id<MTLBuffer> MTCommandContext::GetTessFactorBufferAndGrow(NSUInteger numPatchesAndInstances)
{
    tessFactorBuffer_.Grow(contextState_.tessFactorSize * numPatchesAndInstances);
    return tessFactorBuffer_.GetNative();
}

MTKView* MTCommandContext::GetCurrentDrawableView() const
{
    return (boundSwapChain_ != nullptr ? boundSwapChain_->GetMTKView() : nullptr);
}


/*
 * ======= Private: =======
 */

void MTCommandContext::BeginRenderPassWithDescriptor(MTLRenderPassDescriptor* renderPassDesc, MTSwapChain* swapChainMT)
{
    LLGL_ASSERT_PTR(renderPassDesc);

    if (!contextState_.isInsideRenderPass)
    {
        renderPassDesc_ = (MTLRenderPassDescriptor*)[renderPassDesc copy];
        contextState_.isInsideRenderPass = true;
        boundSwapChain_ = swapChainMT;
    }
}

void MTCommandContext::BindRenderEncoderWithDescriptor(MTLRenderPassDescriptor* renderPassDesc)
{
    Flush();
    renderEncoder_ = [cmdBuffer_ renderCommandEncoderWithDescriptor:renderPassDesc];

    /* A new render command encoder forces all pipeline states to be reset */
    renderDirtyBits_ = ~0;

    /* Invalidate descriptor and constant caches */
    if (!descriptorCache_.IsEmpty())
        descriptorCache_.Reset();
    if (!constantsCache_.IsEmpty())
        constantsCache_.Reset();

    /* Store render encoder mode */
    contextState_.encoderState = MTEncoderState::Render;
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
        for_range(i, 8u)
        {
            if (renderPassDesc_.colorAttachments[i].texture != nil)
            {
                renderPassDesc_.colorAttachments[i].loadAction = MTLLoadActionLoad;
                //renderPassDesc_.colorAttachments[i].storeAction = MTLStoreActionStore;
            }
            else
                break;
        }
        if (renderPassDesc_.depthAttachment.texture != nil)
            renderPassDesc_.depthAttachment.loadAction = MTLLoadActionLoad;
        if (renderPassDesc_.stencilAttachment != nil)
            renderPassDesc_.stencilAttachment.loadAction = MTLLoadActionLoad;
        isRenderEncoderPaused_ = false;
    }
}

void MTCommandContext::SubmitRenderEncoderState()
{
    if (renderEncoder_ == nil)
        return;

    if (renderEncoderState_.viewportCount > 0 && (renderDirtyBits_ & DirtyBit_Viewports) != 0)
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
    if (renderEncoderState_.isScissorTestEnabled && renderEncoderState_.scissorRectCount > 0 && (renderDirtyBits_ & DirtyBit_Scissors) != 0)
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
    if (renderEncoderState_.vertexBufferRange.length > 0 && (renderDirtyBits_ & DirtyBit_VertexBuffers) != 0)
    {
        /* Bind vertex buffers */
        [renderEncoder_
            setVertexBuffers:   renderEncoderState_.vertexBuffers
            offsets:            renderEncoderState_.vertexBufferOffsets
            withRange:          renderEncoderState_.vertexBufferRange
        ];
    }
    if (renderEncoderState_.graphicsPSO != nullptr && (renderDirtyBits_ & DirtyBit_GraphicsPSO) != 0)
    {
        /* Bind graphics pipeline */
        renderEncoderState_.graphicsPSO->Bind(renderEncoder_);
    }
    if (renderEncoderState_.graphicsResourceHeap != nullptr && (renderDirtyBits_ & DirtyBit_GraphicsResourceHeap) != 0)
    {
        /* Bind resource heap */
        renderEncoderState_.graphicsResourceHeap->BindGraphicsResources(
            renderEncoder_,
            renderEncoderState_.graphicsResourceSet
        );
    }
    if (renderEncoderState_.blendColorDynamic && (renderDirtyBits_ & DirtyBit_BlendColor) != 0)
    {
        /* Set blend color */
        [renderEncoder_
            setBlendColorRed:   renderEncoderState_.blendColor[0]
            green:              renderEncoderState_.blendColor[1]
            blue:               renderEncoderState_.blendColor[2]
            alpha:              renderEncoderState_.blendColor[3]
        ];
    }
    if (renderEncoderState_.stencilRefDynamic && (renderDirtyBits_ & DirtyBit_StencilRef) != 0)
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
    if (contextState_.visBuffer != nil && (renderDirtyBits_ & DirtyBit_VisibilityResultMode) != 0)
    {
        /* Set visibility result mode and offset */
        [renderEncoder_
            setVisibilityResultMode:    renderEncoderState_.visResultMode
            offset:                     renderEncoderState_.visResultOffset
        ];
    }

    /* Reset all dirty bits */
    renderDirtyBits_ = 0;
}

void MTCommandContext::ResetRenderEncoderState()
{
    renderEncoderState_.viewportCount               = 0;
    renderEncoderState_.scissorRectCount            = 0;
    renderEncoderState_.vertexBufferRange.length    = 0;
    renderEncoderState_.graphicsPSO                 = nullptr;
    renderEncoderState_.graphicsResourceHeap        = nullptr;
    renderEncoderState_.visResultMode               = MTLVisibilityResultModeDisabled;
    renderEncoderState_.visResultOffset             = 0;
}

void MTCommandContext::SubmitComputeEncoderState()
{
    if (computeEncoder_ == nil)
        return;

    if (computeEncoderState_.computePSO != nullptr && (computeDirtyBits_ & DirtyBit_ComputePSO) != 0)
    {
        /* Bind compute pipeline */
        computeEncoderState_.computePSO->Bind(computeEncoder_);
    }
    if (computeEncoderState_.computeResourceHeap != nullptr && (computeDirtyBits_ & DirtyBit_ComputeResourceHeap) != 0)
    {
        /* Bind resource heap */
        computeEncoderState_.computeResourceHeap->BindComputeResources(
            computeEncoder_,
            computeEncoderState_.computeResourceSet
        );
    }

    /* Reset all dirty bits */
    computeDirtyBits_ = 0;
}

void MTCommandContext::ResetComputeEncoderState()
{
    computeEncoderState_.computePSO             = nullptr;
    computeEncoderState_.computeResourceHeap    = nullptr;
}

void MTCommandContext::ResetContextState()
{
    contextState_.encoderState          = MTEncoderState::None;
    contextState_.numPatchControlPoints = 0;
    contextState_.tessPipelineState     = nil;
    contextState_.boundPipelineState    = nullptr;
    contextState_.visBuffer             = nil;
}

NSUInteger MTCommandContext::GetMaxLocalThreads(id<MTLComputePipelineState> computePSO) const
{
    return std::min<NSUInteger>(
        maxThreadgroupSizeX_,
        computePSO.maxTotalThreadsPerThreadgroup
    );
}


} // /namespace LLGL



// ================================================================================
