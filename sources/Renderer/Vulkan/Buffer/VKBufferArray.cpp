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
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


VKBufferArray::VKBufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray { GetCombinedBindFlags(bufferViews) }
{
    /* Store the object of each VKBuffer inside the array and  */
    buffers_.resize(bufferViews.size());
    offsets_.resize(bufferViews.size());

    for_range(i, bufferViews.size())
    {
        const VertexBufferView& view = bufferViews[i];
        auto* bufferVK = LLGL_CAST(VKBuffer*, view.buffer);

        buffers_[i] = bufferVK->GetVkBuffer();
        offsets_[i] = static_cast<VkDeviceSize>(view.offset);
    }
}


} // /namespace LLGL



// ================================================================================
