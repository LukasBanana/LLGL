/*
 * D3D11StreamOutputBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StreamOutputBufferArray.h"
#include "D3D11StreamOutputBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11StreamOutputBufferArray::D3D11StreamOutputBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) :
    D3D11BufferArray( BufferType::StreamOutput, numBuffers, bufferArray )
{
    /* Store the offsets of each D3D11StreamOutputBuffer inside the array */
    offsets_.reserve(numBuffers);

    while (auto next = NextArrayResource<D3D11StreamOutputBuffer>(numBuffers, bufferArray))
        offsets_.push_back(next->GetOffset());
}


} // /namespace LLGL



// ================================================================================
