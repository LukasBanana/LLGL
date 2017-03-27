/*
 * D3D11StorageBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StorageBufferArray.h"
#include "D3D11StorageBuffer.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11StorageBufferArray::D3D11StorageBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) :
    D3D11BufferArray( BufferType::Storage, numBuffers, bufferArray )
{
    auto next = NextArrayResource<D3D11StorageBuffer>(numBuffers, bufferArray);

    if (next)
    {
        if (next->HasUAV())
        {
            /* Store the SRV pointer, UAV pointer, and intial count of each D3D11StorageBuffer inside the arrays */
            do
            {
                unorderedViews_.push_back(next->GetUAV());
                resourceViews_.push_back(next->GetSRV());
                initialCounts_.push_back(next->GetInitialCount());
            }
            while (next = NextArrayResource<D3D11StorageBuffer>(numBuffers, bufferArray));
        }
        else
        {
            /* Store the SRV pointer of each D3D11StorageBuffer inside the array */
            do
            {
                resourceViews_.push_back(next->GetSRV());
            }
            while (next = NextArrayResource<D3D11StorageBuffer>(numBuffers, bufferArray));
        }
    }
}

bool D3D11StorageBufferArray::HasUAV() const
{
    return (!unorderedViews_.empty());
}


} // /namespace LLGL



// ================================================================================
