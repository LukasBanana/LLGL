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
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    /* Store the pointer of each ID3D11Buffer, strides, and offsets inside the arrays */
    buffersAndBindingLocators_.resize(numBuffers * 2);
    stridesAndOffsets_.resize(numBuffers * 2);

    const std::uint32_t firstOffset = numBuffers;

    for (std::size_t i = 0; D3D11Buffer* next = NextArrayResource<D3D11Buffer>(numBuffers, bufferArray); ++i)
    {
        buffersAndBindingLocators_[i]               = next->GetNative();
        buffersAndBindingLocators_[i + firstOffset] = next->GetBindingLocator();
        stridesAndOffsets_[i]                       = next->GetStride();
        stridesAndOffsets_[i + firstOffset]         = 0;//next->GetOffset());
    }
}


} // /namespace LLGL



// ================================================================================
