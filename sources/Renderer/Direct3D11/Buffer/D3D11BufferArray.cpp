/*
 * D3D11BufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BufferArray.h"
#include "D3D11Buffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(const BufferType type, unsigned int numBuffers, Buffer* const * bufferArray) :
    BufferArray { type }
{
    /* Store the pointer of each ID3D11Buffer inside the array */
    buffers_.reserve(numBuffers);
    while (auto next = NextArrayResource<D3D11Buffer>(numBuffers, bufferArray))
        buffers_.push_back(next->Get());
}


} // /namespace LLGL



// ================================================================================
