/*
 * MTBufferArray.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTBufferArray.h"
#include "MTBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


MTBufferArray::MTBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    /* Store id<MTLBuffer> of each buffer object inside the array */
    idArray_.reserve(numBuffers);
    offsets_.reserve(numBuffers);
    while (MTBuffer* next = NextArrayResource<MTBuffer>(numBuffers, bufferArray))
    {
        idArray_.push_back(next->GetNative());
        offsets_.push_back(0);
    }
}


} // /namespace LLGL



// ================================================================================
