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


void D3D11ConstantBuffer::CreateResource(ID3D11Device* device, UINT bufferSize, const void* initialData)
{
    hwBuffer.CreateResource(device, CD3D11_BUFFER_DESC(bufferSize, D3D11_BIND_CONSTANT_BUFFER), initialData);
    bufferSize_ = bufferSize;

    /* Allocate intermediate buffer */
    intermediateBuffer_.resize(bufferSize_);

    if (initialData)
    {
        auto byteAlignedData = reinterpret_cast<const char*>(initialData);
        std::copy(byteAlignedData, byteAlignedData + bufferSize, intermediateBuffer_.data());
    }
}

void D3D11ConstantBuffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    /* Validate parameters */
    if (dataSize + offset > bufferSize_)
        throw std::out_of_range(LLGL_ASSERT_INFO("size and offset out of range to update constant buffer subresource"));
    
    /* Copy data into intermediate buffer */
    auto byteAlignedData = reinterpret_cast<const char*>(data);
    std::copy(byteAlignedData, byteAlignedData + dataSize, intermediateBuffer_.data() + offset);
    
    /*
    Update entire subresource, because constant buffers can not be updated partially,
    except they are mapped, which requires the buffer to be created with CPU write access.
    */
    hwBuffer.UpdateSubresource(context, intermediateBuffer_.data());
}


} // /namespace LLGL



// ================================================================================
