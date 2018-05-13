/*
 * D3D11VertexBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11VertexBufferArray.h"
#include "D3D11VertexBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11VertexBufferArray::D3D11VertexBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    D3D11BufferArray { BufferType::Vertex, numBuffers, bufferArray }
{
    /* Store the strides and offests of each D3D11VertexBuffer inside the arrays */
    strides_.reserve(numBuffers);
    offsets_.reserve(numBuffers);

    while (auto next = NextArrayResource<D3D11VertexBuffer>(numBuffers, bufferArray))
    {
        strides_.push_back(next->GetStride());
        offsets_.push_back(0);//next->GetOffset());
    }
}


} // /namespace LLGL



// ================================================================================
