/*
 * D3D9BufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9BufferArray.h"
#include "D3D9VertexBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static std::vector<D3D9VertexBuffer*> GetD3D9VertexBuffers(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    std::vector<D3D9VertexBuffer*> buffers;
    buffers.reserve(numBuffers);
    for_range(i, numBuffers)
    {
        LLGL_ASSERT((bufferArray[i]->GetBindFlags() & BindFlags::VertexBuffer) != 0);
        buffers.push_back(LLGL_CAST(D3D9VertexBuffer*, bufferArray[i]));
    }
    return buffers;
}

D3D9BufferArray::D3D9BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray    { BindFlags::VertexBuffer                       },
    vertexBuffers_ { GetD3D9VertexBuffers(numBuffers, bufferArray) }
{
}


} // /namespace LLGL



// ================================================================================
