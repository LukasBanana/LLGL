/*
 * D3D11BufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BufferArray.h"
#include "D3D11Buffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { bindFlags }
{
    /* Store the pointer of each ID3D11Buffer, strides, and offests inside the arrays */
    buffers_.resize(numBuffers);
    stridesAndOffsets_.resize(numBuffers * 2);

    offsetStart_ = numBuffers;

    for (std::size_t i = 0; auto next = NextArrayResource<D3D11Buffer>(numBuffers, bufferArray); ++i)
    {
        buffers_[i]                             = next->GetNative();
        stridesAndOffsets_[i]                   = next->GetStride();
        stridesAndOffsets_[i + offsetStart_]    = 0;//next->GetOffset());
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
    return stridesAndOffsets_.data();
}

const UINT* D3D11BufferArray::GetOffsets() const
{
    return (stridesAndOffsets_.data() + offsetStart_);
}


} // /namespace LLGL



// ================================================================================
