/*
 * D3D11BufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BufferArray.h"
#include "D3D11VertexBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(const BufferType type, std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { type }
{
    /* Store the pointer of each ID3D11Buffer, strides, and offests inside the arrays */
    buffers_.reserve(numBuffers);
    strides_.reserve(numBuffers);
    offsets_.reserve(numBuffers);

    while (auto next = NextArrayResource<D3D11VertexBuffer>(numBuffers, bufferArray))
    {
        buffers_.push_back(next->GetNative());
        strides_.push_back(next->GetStride());
        offsets_.push_back(0);//next->GetOffset());
    }
}

UINT D3D11BufferArray::GetCount() const
{
    return static_cast<UINT>(buffers_.size());
}

ID3D11Buffer* const * D3D11BufferArray::GetBuffers() const
{
    return buffers_.data();
}

const UINT* D3D11BufferArray::GetStrides() const
{
    return strides_.data();
}

const UINT* D3D11BufferArray::GetOffsets() const
{
    return offsets_.data();
}


} // /namespace LLGL



// ================================================================================
