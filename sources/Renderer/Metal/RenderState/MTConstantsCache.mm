/*
 * MTConstantsCache.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTConstantsCache.h"
#include "../Shader/MTShader.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <string.h>


namespace LLGL
{


void MTConstantsCache::Reset(const MTConstantsCacheLayout* layout)
{
    if (layout != nullptr)
    {
        /* Store references to constants map and constant buffer layouts */
        constantsMap_       = layout->GetConstantsMap();
        constantBuffers_    = layout->GetConstantBuffers();

        /* Resize cache if necessary */
        std::size_t cacheSize = layout->GetConstantsDataSize();
        if (constants_.size() < cacheSize)
        {
            cacheSize = GrowStrategyAddHalf::Grow(cacheSize);
            constants_.resize(cacheSize, UninitializeTag{});
        }
    }
    else
    {
        /* Clear references to constants map and constant buffer layouts */
        constantsMap_       = {};
        constantBuffers_    = {};
    }
    Reset();
}

void MTConstantsCache::Reset()
{
    dirtyBits_.bits = 0xFF;
}

void MTConstantsCache::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    for (const char* bytes = static_cast<const char*>(data); first < constantsMap_.size() && dataSize > 0; ++first)
    {
        /* Early exit even ignoring the dirty bits since out-of-bounds is undefined behavior */
        const ConstantLocation& constant = constantsMap_[first];
        if (constant.size > dataSize)
            return /*Out of bounds*/;

        /* Update constant data for each shader stage */
        for_range(i, MTShaderStage_CountPerPSO)
        {
            if (constant.offsetPerStage[i] != MTConstantsCacheLayout::invalidOffset)
                ::memcpy(constants_.get() + constant.offsetPerStage[i], bytes, constant.size);
        }

        /* Move to next uniform */
        dataSize -= constant.size;
        bytes += constant.size;
    }
    Reset();
}

void MTConstantsCache::FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder)
{
    if (dirtyBits_.graphics != 0)
        FlushGraphicsResourcesForced(renderEncoder);
}

void MTConstantsCache::FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder)
{
    for (const ConstantBuffer& constantBuffer : constantBuffers_)
    {
        if ((constantBuffer.stages & StageFlags::VertexStage) != 0)
        {
            [renderEncoder
                setVertexBytes: constants_.get() + constantBuffer.offset
                length:         constantBuffer.size
                atIndex:        constantBuffer.index
            ];
        }
        if ((constantBuffer.stages & StageFlags::FragmentStage) != 0)
        {
            [renderEncoder
                setFragmentBytes:   constants_.get() + constantBuffer.offset
                length:             constantBuffer.size
                atIndex:            constantBuffer.index
            ];
        }
    }
    dirtyBits_.graphics = 0;
}

void MTConstantsCache::FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder)
{
    if (dirtyBits_.compute != 0)
        FlushComputeResourcesForced(computeEncoder);
}

void MTConstantsCache::FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder)
{
    for (const ConstantBuffer& constantBuffer : constantBuffers_)
    {
        if ((constantBuffer.stages & StageFlags::ComputeStage) != 0)
        {
            [computeEncoder
                setBytes:   constants_.get() + constantBuffer.offset
                length:     constantBuffer.size
                atIndex:    constantBuffer.index
            ];
        }
    }
    dirtyBits_.compute = 0;
}


} // /namespace LLGL



// ================================================================================
