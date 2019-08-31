/*
 * VKBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKBufferArray.h"
#include "VKBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


VKBufferArray::VKBufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { bindFlags }
{
    /* Store the object of each VKBuffer inside the array and  */
    buffers_.reserve(numBuffers);
    offsets_.reserve(numBuffers);

    while (auto next = NextArrayResource<VKBuffer>(numBuffers, bufferArray))
    {
        buffers_.push_back(next->GetVkBuffer());
        offsets_.push_back(0);//next->GetOffset()
    }
}


} // /namespace LLGL



// ================================================================================
