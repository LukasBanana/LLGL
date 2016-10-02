/*
 * D3D11VertexBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11VertexBufferArray.h"
#include "D3D11VertexBuffer.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D11VertexBufferArray::D3D11VertexBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) :
    D3D11BufferArray( BufferType::Vertex, numBuffers, bufferArray )
{
    /* Store the pointer of each ID3D11Buffer inside the array */
    for (strides_.reserve(numBuffers), offsets_.reserve(numBuffers); numBuffers > 0; --numBuffers)
    {
        auto vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer*, (*bufferArray));
        strides_.push_back(vertexBufferD3D->GetStride());
        offsets_.push_back(0);//vertexBufferD3D->GetOffset());
        ++bufferArray;
    }
}


} // /namespace LLGL



// ================================================================================
