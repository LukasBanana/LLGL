/*
 * D3D12BufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12BufferArray.h"
#include "D3D12Buffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


D3D12BufferArray::D3D12BufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray { GetCombinedBindFlags(bufferViews) }
{
    /* Store the strides and offsets of each D3D12VertexBuffer inside the arrays */
    vertexBufferViews_.resize(bufferViews.size());
    resourceRefs_.resize(bufferViews.size());

    for_range(i, bufferViews.size())
    {
        const VertexBufferView& view = bufferViews[i];
        auto* bufferD3D = LLGL_CAST(D3D12Buffer*, view.buffer);
        vertexBufferViews_[i] = bufferD3D->GetVertexBufferView();
        vertexBufferViews_[i].BufferLocation += view.offset;
        vertexBufferViews_[i].SizeInBytes -= static_cast<UINT>(view.offset);
        if (view.stride > 0)
            vertexBufferViews_[i].StrideInBytes = view.stride;
        resourceRefs_[i] = &(bufferD3D->GetResource());
    }
}


} // /namespace LLGL



// ================================================================================
