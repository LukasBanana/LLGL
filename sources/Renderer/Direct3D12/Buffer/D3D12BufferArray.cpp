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
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D12BufferArray::D3D12BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    /* Store the strides and offsets of each D3D12VertexBuffer inside the arrays */
    vertexBufferViews_.reserve(numBuffers);
    resourceRefs_.reserve(numBuffers);
    while (D3D12Buffer* next = NextArrayResource<D3D12Buffer>(numBuffers, bufferArray))
    {
        vertexBufferViews_.push_back(next->GetVertexBufferView());
        resourceRefs_.push_back(&(next->GetResource()));
    }
}


} // /namespace LLGL



// ================================================================================
