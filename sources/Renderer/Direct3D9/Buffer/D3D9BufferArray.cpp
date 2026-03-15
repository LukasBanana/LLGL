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


static std::vector<D3D9BufferArray::D3DBufferAndStride> GetD3DVertexBuffersAndStrides(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    std::vector<D3D9BufferArray::D3DBufferAndStride> buffers;
    buffers.reserve(numBuffers);
    for_range(i, numBuffers)
    {
        LLGL_ASSERT((bufferArray[i]->GetBindFlags() & BindFlags::VertexBuffer) != 0);
        D3D9VertexBuffer* vertexBufferD3D = LLGL_CAST(D3D9VertexBuffer*, bufferArray[i]);
        buffers.push_back({ vertexBufferD3D->GetNative(), vertexBufferD3D->GetStride() });
    }
    return buffers;
}

D3D9BufferArray::D3D9BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray              { BindFlags::VertexBuffer                                },
    nativeBuffersAndStrides_ { GetD3DVertexBuffersAndStrides(numBuffers, bufferArray) }
{
}


} // /namespace LLGL



// ================================================================================
