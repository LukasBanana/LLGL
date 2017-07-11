/*
 * D3D11StorageBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StorageBuffer.h"
#include "../D3D11Types.h"
#include "../../../Core/Helper.h"
#include "../../DXCommon/DXCore.h"
#include "../../Assertion.h"
#include <algorithm>


namespace LLGL
{


D3D11StorageBuffer::D3D11StorageBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer { BufferType::Storage }
{
    if (desc.storageBuffer.stride == 0)
        throw std::invalid_argument("storage buffer stride can not be zero for a D3D11 resource view");

    storageType_ = desc.storageBuffer.storageType;

    /* Create D3D hardware buffer */
    D3D11_BUFFER_DESC bufferDesc;
    {
        bufferDesc.ByteWidth            = desc.size;
        bufferDesc.Usage                = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags            = GetBindFlags();
        bufferDesc.CPUAccessFlags       = 0;
        bufferDesc.MiscFlags            = GetMiscFlags();
        bufferDesc.StructureByteStride  = desc.storageBuffer.stride;
    }
    CreateResource(device, bufferDesc, initialData, desc.flags);

    /* Create either UAV or SRV */
    auto format         = GetFormat(desc.storageBuffer.vectorType);
    auto numElements    = desc.size / desc.storageBuffer.stride;

    CreateSRV(device, format, 0, numElements);

    if (HasUAV())
        CreateUAV(device, format, 0, numElements);
}

bool D3D11StorageBuffer::HasUAV() const
{
    return ( storageType_ >= StorageBufferType::RWBuffer );
}

bool D3D11StorageBuffer::IsTyped() const
{
    return ( storageType_ == StorageBufferType::Buffer ||
             storageType_ == StorageBufferType::RWBuffer );
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

UINT D3D11StorageBuffer::GetBindFlags() const
{
    UINT flags = D3D11_BIND_SHADER_RESOURCE;
    
    if (HasUAV())
        flags |= D3D11_BIND_UNORDERED_ACCESS;

    return flags;
}

UINT D3D11StorageBuffer::GetMiscFlags() const
{
    if (IsStructured())
        return D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    else if (IsByteAddressable())
        return D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    else
        return 0;
}

UINT D3D11StorageBuffer::GetUAVFlags() const
{
    if (IsByteAddressable())
        return D3D11_BUFFER_UAV_FLAG_RAW;
    if (storageType_ == StorageBufferType::AppendStructuredBuffer)
        return D3D11_BUFFER_UAV_FLAG_APPEND;
    if (storageType_ == StorageBufferType::ConsumeStructuredBuffer)
        return D3D11_BUFFER_UAV_FLAG_COUNTER;
    return 0;
}

DXGI_FORMAT D3D11StorageBuffer::GetFormat(const VectorType vectorType) const
{
    /*
    D3D11_BUFFER_UAV_FLAG_RAW buffer flag for ByteAddressBuffer requires the UAV to have the DXGI_FORMAT_R32_TYPELESS format.
    -> see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476096(v=vs.85).aspx
    */
    if (IsTyped())
        return D3D11Types::Map(vectorType);
    if (IsByteAddressable())
        return DXGI_FORMAT_R32_TYPELESS;
    return DXGI_FORMAT_UNKNOWN;
}

void D3D11StorageBuffer::CreateSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements)
{
    /* Initialize descriptor and create SRV */
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    {
        desc.Format                 = format;
        desc.ViewDimension          = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
    }
    auto hr = device->CreateShaderResourceView(Get(), &desc, srv_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resource-view (SRV) for storage buffer");
}

void D3D11StorageBuffer::CreateUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        desc.Format                 = format;
        desc.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
        desc.Buffer.Flags           = GetUAVFlags();
    }
    auto hr = device->CreateUnorderedAccessView(Get(), &desc, uav_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}


} // /namespace LLGL



// ================================================================================
