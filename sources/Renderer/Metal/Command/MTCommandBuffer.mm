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


MTCommandBuffer::MTCommandBuffer(id<MTLDevice> device, long flags) :
    device_             { device                },
    flags_              { flags                 },
    stagingBufferPools_ { { device, USHRT_MAX },
                          { device, USHRT_MAX },
                          { device, USHRT_MAX } }
{
    ResetRenderStates();
}


/*
 * ======= Protected: =======
 */

void MTCommandBuffer::ResetRenderStates()
{
    currentStagingPool_ = (currentStagingPool_ + 1) % MTCommandBuffer::maxNumCommandBuffersInFlight;
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


} // /namespace LLGL



// ================================================================================
