/*
 * MTBufferArray.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTBufferArray.h"
#include "MTBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


MTBufferArray::MTBufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { bindFlags }
{
    /* Store id<MTLBuffer> of each buffer object inside the array */
    idArray_.reserve(numBuffers);
    offsets_.reserve(numBuffers);
    while (auto next = NextArrayResource<MTBuffer>(numBuffers, bufferArray))
    {
        idArray_.push_back(next->GetNative());
        offsets_.push_back(0);
    }
}


} // /namespace LLGL



// ================================================================================
