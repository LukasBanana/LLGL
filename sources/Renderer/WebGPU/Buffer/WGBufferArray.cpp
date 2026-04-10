/*
 * WGBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGBufferArray.h"
#include "WGBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


WGBufferArray::WGBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    wgpuBuffers_.resize(numBuffers);
    for (std::size_t i = 0; WGBuffer* next = NextArrayResource<WGBuffer>(numBuffers, bufferArray); ++i)
        wgpuBuffers_[i] = next->GetNative();
}


} // /namespace LLGL



// ================================================================================
