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
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


MTBufferArray::MTBufferArray(ArrayView<BufferLocation> bufferLocations) :
    BufferArray { GetCombinedBindFlags(bufferLocations) }
{
    /* Store id<MTLBuffer> of each buffer object inside the array */
    idArray_.resize(bufferLocations.size());
    offsets_.resize(bufferLocations.size());

    for_range(i, bufferLocations.size())
    {
        const BufferLocation& location = bufferLocations[i];
        auto* bufferMT = LLGL_CAST(MTBuffer, location.buffer);
        idArray_[i] = bufferMT->GetNative();
        offsets_[i] = static_cast<NSUInteger>(location.offset);
    }
}


} // /namespace LLGL



// ================================================================================
