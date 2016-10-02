/*
 * D3D11BufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BufferArray.h"
#include "D3D11Buffer.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D11BufferArray::D3D11BufferArray(const BufferType type, unsigned int numBuffers, Buffer* const * bufferArray) :
    BufferArray( type )
{
    /* Store the pointer of each ID3D11Buffer inside the array */
    for (buffers_.reserve(numBuffers); numBuffers > 0; --numBuffers)
    {
        auto bufferD3D = LLGL_CAST(D3D11Buffer*, (*bufferArray));
        buffers_.push_back(bufferD3D->Get());
        ++bufferArray;
    }
}


} // /namespace LLGL



// ================================================================================
