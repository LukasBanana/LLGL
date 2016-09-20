/*
 * D3D11HardwareBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11HardwareBuffer.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


void D3D11HardwareBuffer::CreateResource(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData)
{
    /* Setup initial subresource data */
    D3D11_SUBRESOURCE_DATA subresourceData;
    
    if (initialData)
    {
        InitMemory(subresourceData);
        subresourceData.pSysMem = initialData;
    }

    /* Create new D3D11 hardware buffer */
    buffer_.Reset();
    auto hr = device->CreateBuffer(&desc, (initialData != nullptr ? &subresourceData : nullptr), &buffer_);
    DXThrowIfFailed(hr, "failed to create D3D11 buffer");
}

void D3D11HardwareBuffer::UpdateSubResource(ID3D11DeviceContext* context, const void* data, const D3D11_BOX& destBox, UINT srcRowPitch, UINT srcDepthPitch)
{
    context->UpdateSubresource(buffer_.Get(), 0, &destBox, data, srcRowPitch, srcDepthPitch);
}

void D3D11HardwareBuffer::UpdateSubResource(ID3D11DeviceContext* context, const void* data, UINT bufferSize, UINT offset)
{
    UpdateSubResource(context, data, { offset, 0, 0, offset + bufferSize, 1, 1 });

}


} // /namespace LLGL



// ================================================================================
