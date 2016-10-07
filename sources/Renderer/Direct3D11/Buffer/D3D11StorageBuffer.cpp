/*
 * D3D11StorageBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StorageBuffer.h"
#include "../../../Core/Helper.h"
#include "../../DXCommon/DXCore.h"
#include "../../Assertion.h"
#include <algorithm>


namespace LLGL
{


D3D11StorageBuffer::D3D11StorageBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer( BufferType::Storage )
{
    storageType_ = desc.storageBuffer.storageType;

    /* Setup descriptor and create storage buffer */
    UINT bindFlags = (IsUAV() ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE);

    CD3D11_BUFFER_DESC bufferDesc(desc.size, bindFlags);

    if (desc.usage == BufferUsage::Dynamic)
    {
        bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
        bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    }

    if (IsStructured())
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    else if (IsByteAddressable())
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    /* Create D3D hardware buffer */
    CreateResource(device, bufferDesc, initialData);

    /* Create either SRV or UAV */
    if (IsUAV())
        CreateUAV(device, 0, desc.storageBuffer.elements);
    else
        CreateSRV(device, 0, desc.storageBuffer.elements);
}

bool D3D11StorageBuffer::IsUAV() const
{
    return ( storageType_ >= StorageBufferType::RWBuffer );
}

bool D3D11StorageBuffer::IsStructured() const
{
    return ( storageType_ == StorageBufferType::StructuredBuffer       ||
             storageType_ == StorageBufferType::RWStructuredBuffer     ||
             storageType_ == StorageBufferType::AppendStructuredBuffer ||
             storageType_ == StorageBufferType::ConsumeStructuredBuffer );
}

bool D3D11StorageBuffer::IsByteAddressable() const
{
    return ( storageType_ == StorageBufferType::ByteAddressBuffer ||
             storageType_ == StorageBufferType::RWByteAddressBuffer );
}


/*
 * ======= Private: =======
 */

void D3D11StorageBuffer::CreateUAV(ID3D11Device* device, UINT firstElement, UINT numElements)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        desc.Format                 = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
        desc.Buffer.Flags           = 0;

        if (IsByteAddressable())
        {
            /*
            D3D11_BUFFER_UAV_FLAG_RAW buffer flag for ByteAddressBuffer requires the UAV to have the DXGI_FORMAT_R32_TYPELESS format.
            -> see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476096(v=vs.85).aspx
            */
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
        }
        else if (storageType_ == StorageBufferType::AppendStructuredBuffer)
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
        else if (storageType_ == StorageBufferType::ConsumeStructuredBuffer)
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
    }
    auto hr = device->CreateUnorderedAccessView(Get(), &desc, &uav_);
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}

void D3D11StorageBuffer::CreateSRV(ID3D11Device* device, UINT firstElement, UINT numElements)
{
    /* Initialize descriptor and create SRV */
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    {
        desc.Format                 = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension          = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
    }
    auto hr = device->CreateShaderResourceView(Get(), &desc, &srv_);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resource-view (SRV) for storage buffer");
}


} // /namespace LLGL



// ================================================================================
