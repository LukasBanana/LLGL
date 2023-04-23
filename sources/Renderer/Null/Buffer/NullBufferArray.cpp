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


static std::vector<NullBuffer*> GetNullBuffers(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    std::vector<NullBuffer*> buffers;
    buffers.reserve(numBuffers);
    for_range(i, numBuffers)
        buffers.push_back(LLGL_CAST(NullBuffer*, bufferArray[i]));
    return buffers;
}

NullBufferArray::NullBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) },
    buffers     { GetNullBuffers(numBuffers, bufferArray)       }
{
}


} // /namespace LLGL



// ================================================================================
