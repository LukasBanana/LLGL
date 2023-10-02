/*
 * MTCommandBuffer.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTCommandBuffer.h"
#include "../MTSwapChain.h"
#include "../RenderState/MTGraphicsPSO.h"
#include "../RenderState/MTComputePSO.h"
#include "../RenderState/MTDescriptorCache.h"
#include "../RenderState/MTConstantsCache.h"
#include "../Buffer/MTBuffer.h"
#include "../Shader/MTShader.h"
#include "../../CheckedCast.h"
#include <algorithm>

#include <LLGL/Backend/Metal/NativeCommand.h>


namespace LLGL
{


static constexpr NSUInteger g_tessFactorBufferAlignment = (sizeof(MTLQuadTessellationFactorsHalf) * 256);

MTCommandBuffer::MTCommandBuffer(id<MTLDevice> device, long flags) :
    device_             { device                         },
    flags_              { flags                          },
    stagingBufferPools_ { { device, USHRT_MAX },
                          { device, USHRT_MAX },
                          { device, USHRT_MAX }          },
    tessFactorBuffer_   { device,
                          MTLResourceStorageModePrivate,
                          g_tessFactorBufferAlignment    }
{
    ResetRenderStates();
}

void MTCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    SetIndexStream(bufferMT.GetNative(), 0, bufferMT.IsIndexType16Bits());
}

void MTCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    SetIndexStream(bufferMT.GetNative(), static_cast<NSUInteger>(offset), (format == Format::R16UInt));
}


/*
 * ======= Protected: =======
 */

NSUInteger MTCommandBuffer::GetMaxLocalThreads(id<MTLComputePipelineState> computePSO) const
{
    return std::min(
        device_.maxThreadsPerThreadgroup.width,
        computePSO.maxTotalThreadsPerThreadgroup
    );
}

static NSUInteger GetTessFactorSize(const MTLPatchType patchType)
{
    switch (patchType)
    {
        case MTLPatchTypeNone:      return 0;
        case MTLPatchTypeTriangle:  return sizeof(MTLTriangleTessellationFactorsHalf);
        case MTLPatchTypeQuad:      return sizeof(MTLQuadTessellationFactorsHalf);
    }
    return 0;
}

void MTCommandBuffer::SetSwapChain(MTSwapChain* swapChainMT)
{
    boundSwapChain_ = swapChainMT;
}

void MTCommandBuffer::SetGraphicsPSORenderState(MTGraphicsPSO& graphicsPSO)
{
    /* Store current primitive type and tessellation data */
    boundPipelineState_     = &graphicsPSO;
    primitiveType_          = graphicsPSO.GetMTLPrimitiveType();
    numPatchControlPoints_  = graphicsPSO.GetNumPatchControlPoints();
    tessPipelineState_      = graphicsPSO.GetTessPipelineState();
    tessFactorSize_         = GetTessFactorSize(graphicsPSO.GetPatchType());
}

void MTCommandBuffer::SetComputePSORenderState(MTComputePSO& computePSO)
{
    /* Store work group size of shader program */
    boundPipelineState_ = &computePSO;
    if (const MTShader* computeShader = computePSO.GetComputeShader())
        threadsPerThreadgroup_ = computeShader->GetNumThreadsPerGroup();
}

void MTCommandBuffer::ResetRenderStates()
{
    numPatchControlPoints_  = 0;
    tessPipelineState_      = nil;
    boundSwapChain_         = nullptr;
    boundPipelineState_     = nullptr;
    currentStagingPool_     = (currentStagingPool_ + 1) % maxNumStagingPools;
}

void MTCommandBuffer::ResetStagingPool()
{
    stagingBufferPools_[currentStagingPool_].Reset();
}

void MTCommandBuffer::WriteStagingBuffer(
    const void*     data,
    NSUInteger      dataSize,
    id<MTLBuffer>&  outSrcBuffer,
    NSUInteger&     outSrcOffset)
{
    stagingBufferPools_[currentStagingPool_].Write(data, dataSize, outSrcBuffer, outSrcOffset);
}

MTKView* MTCommandBuffer::GetCurrentDrawableView() const
{
    return (boundSwapChain_ != nullptr ? boundSwapChain_->GetMTKView() : nullptr);
}

id<MTLBuffer> MTCommandBuffer::GetTessFactorBufferAndGrow(NSUInteger numPatchesAndInstances)
{
    tessFactorBuffer_.Grow(tessFactorSize_ * numPatchesAndInstances);
    return tessFactorBuffer_.GetNative();
}


/*
 * ======= Private: =======
 */

void MTCommandBuffer::SetIndexStream(id<MTLBuffer> indexBuffer, NSUInteger offset, bool indexType16Bits)
{
    indexBuffer_        = indexBuffer;
    indexBufferOffset_  = offset;
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
