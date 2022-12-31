/*
 * NullBufferArray.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullBufferArray.h"
#include "NullBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include <LLGL/Misc/ForRange.h>


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
