/*
 * D3D11BufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11BufferArray.h"
#include "D3D11Buffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray { GetCombinedBindFlags(bufferViews) }
{
    /* Store the pointer of each ID3D11Buffer, strides, and offsets inside the arrays */
    buffersAndBindingLocators_.resize(bufferViews.size() * 2);
    stridesAndOffsets_.resize(bufferViews.size() * 2);

    const std::size_t secondBucketOffset = bufferViews.size();

    for_range(i, bufferViews.size())
    {
        const VertexBufferView& view = bufferViews[i];
        auto* bufferD3D = LLGL_CAST(D3D11Buffer*, view.buffer);
        buffersAndBindingLocators_[i]                       = bufferD3D;
        buffersAndBindingLocators_[i + secondBucketOffset]  = bufferD3D->GetBindingLocator();
        stridesAndOffsets_[i]                               = (view.stride > 0 ? view.stride : bufferD3D->GetStride());
        stridesAndOffsets_[i + secondBucketOffset]          = static_cast<UINT>(view.offset);
    }
}


} // /namespace LLGL



// ================================================================================
