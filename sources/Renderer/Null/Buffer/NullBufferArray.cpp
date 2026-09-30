/*
 * NullBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullBufferArray.h"
#include "NullBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static std::vector<NullBuffer*> GetNullBuffers(ArrayView<VertexBufferView> bufferViews)
{
    std::vector<NullBuffer*> buffers;
    buffers.resize(bufferViews.size());
    for_range(i, bufferViews.size())
        buffers[i] = LLGL_CAST(NullBuffer*, bufferViews[i].buffer);
    return buffers;
}

static std::vector<std::uint64_t> GetBufferOffsets(ArrayView<VertexBufferView> bufferViews)
{
    std::vector<std::uint64_t> offsets;
    offsets.resize(bufferViews.size());
    for_range(i, bufferViews.size())
        offsets[i] = bufferViews[i].offset;
    return offsets;
}

NullBufferArray::NullBufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray { GetCombinedBindFlags(bufferViews) },
    buffers     { GetNullBuffers(bufferViews)       },
    offsets     { GetBufferOffsets(bufferViews)     }
{
}


} // /namespace LLGL



// ================================================================================
