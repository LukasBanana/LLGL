/*
 * VKBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKBufferArray.h"
#include "VKBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


VKBufferArray::VKBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    /* Store the object of each VKBuffer inside the array and  */
    buffers_.reserve(numBuffers);
    offsets_.reserve(numBuffers);

    while (VKBuffer* next = NextArrayResource<VKBuffer>(numBuffers, bufferArray))
    {
        buffers_.push_back(next->GetVkBuffer());
        offsets_.push_back(0);//next->GetOffset()
    }
}


} // /namespace LLGL



// ================================================================================
