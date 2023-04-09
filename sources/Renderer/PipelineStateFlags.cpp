/*
 * PipelineStateFlags.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


LLGL_EXPORT bool IsPrimitiveTopologyPatches(const PrimitiveTopology primitiveTopology)
{
    return (primitiveTopology >= PrimitiveTopology::Patches1 && primitiveTopology <= PrimitiveTopology::Patches32);
}

LLGL_EXPORT bool IsPrimitiveTopologyStrip(const PrimitiveTopology primitiveTopology)
{
    return
    (
        primitiveTopology == PrimitiveTopology::LineStrip               ||
        primitiveTopology == PrimitiveTopology::LineStripAdjacency      ||
        primitiveTopology == PrimitiveTopology::TriangleStrip           ||
        primitiveTopology == PrimitiveTopology::TriangleStripAdjacency
    );
}

LLGL_EXPORT std::uint32_t GetPrimitiveTopologyPatchSize(const PrimitiveTopology primitiveTopology)
{
    if (IsPrimitiveTopologyPatches(primitiveTopology))
        return (static_cast<std::uint32_t>(primitiveTopology) - static_cast<std::uint32_t>(PrimitiveTopology::Patches1) + 1);
    else
        return 0;
}


} // /namespace LLGL



// ================================================================================
