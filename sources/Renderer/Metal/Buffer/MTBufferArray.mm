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


MTBufferArray::MTBufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray { GetCombinedBindFlags(bufferViews) }
{
    /* Store id<MTLBuffer> of each buffer object inside the array */
    idArray_.resize(bufferViews.size());
    offsets_.resize(bufferViews.size());

    for_range(i, bufferViews.size())
    {
        const VertexBufferView& view = bufferViews[i];
        auto* bufferMT = LLGL_CAST(MTBuffer*, view.buffer);
        idArray_[i] = bufferMT->GetNative();
        offsets_[i] = static_cast<NSUInteger>(view.offset);
    }
}


} // /namespace LLGL



// ================================================================================
