/*
 * D3D12BufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12BufferArray.h"
#include "D3D12Buffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12BufferArray::D3D12BufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { bindFlags }
{
    /* Store the strides and offests of each D3D12VertexBuffer inside the arrays */
    vertexBufferViews_.reserve(numBuffers);
    while (auto next = NextArrayResource<D3D12Buffer>(numBuffers, bufferArray))
        vertexBufferViews_.push_back(next->GetVertexBufferView());
}


} // /namespace LLGL



// ================================================================================
