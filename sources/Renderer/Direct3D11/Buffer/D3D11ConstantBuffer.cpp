/*
 * D3D11ConstantBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ConstantBuffer.h"
#include "../../../Core/Helper.h"
#include "../../DXCommon/DXCore.h"
#include "../../Assertion.h"
#include <algorithm>


namespace LLGL
{


void D3D11ConstantBuffer::CreateResource(ID3D11Device* device, UINT bufferSize, const BufferUsage usage, const void* initialData)
{
    /* Setup descriptor and create constant buffer */
    CD3D11_BUFFER_DESC bufferDesc(bufferSize, D3D11_BIND_CONSTANT_BUFFER);

    if (usage == BufferUsage::Dynamic)
    {
        bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
        bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
    }

    usage_ = bufferDesc.Usage;

    hwBuffer.CreateResource(device, bufferDesc, initialData);
    bufferSize_ = bufferSize;
}

void D3D11ConstantBuffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    /* Validate parameters */
    if (dataSize + offset > bufferSize_)
        throw std::out_of_range(LLGL_ASSERT_INFO("size and offset out of range to update constant buffer subresource"));

    if (usage_ == D3D11_USAGE_DYNAMIC)
    {
        /* Update partial subresource by mapping buffer from GPU into CPU memory space */
        D3D11_MAPPED_SUBRESOURCE subresource;
        context->Map(hwBuffer.Get(), 0, (dataSize < bufferSize_ ? D3D11_MAP_WRITE : D3D11_MAP_WRITE_DISCARD), 0, &subresource);
        {
            ::memcpy(reinterpret_cast<char*>(subresource.pData) + offset, data, dataSize);
        }
        context->Unmap(hwBuffer.Get(), 0);
    }
    else
    {
        /* Update entire subresource */
        if (dataSize == bufferSize_)
            hwBuffer.UpdateSubresource(context, data);
        else
            throw std::out_of_range(LLGL_ASSERT_INFO("can not update D3D11 buffer partially when it is created with static usage"));
    }
}


} // /namespace LLGL



// ================================================================================
