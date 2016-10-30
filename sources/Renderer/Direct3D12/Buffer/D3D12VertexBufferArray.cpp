/*
 * D3D12VertexBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12VertexBufferArray.h"
#include "D3D12VertexBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12VertexBufferArray::D3D12VertexBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) :
    BufferArray( BufferType::Vertex )
{
    /* Store the strides and offests of each D3D12VertexBuffer inside the arrays */
    views_.reserve(numBuffers);
    while (auto next = NextArrayResource<D3D12VertexBuffer>(numBuffers, bufferArray))
        views_.push_back(next->GetView());
}


} // /namespace LLGL



// ================================================================================
