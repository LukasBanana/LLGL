/*
 * D3D11StorageBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StorageBuffer.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <algorithm>


namespace LLGL
{


static UINT GetBindFlags(const BufferDescriptor::StorageBuffer& desc)
{
    UINT flags = D3D11_BIND_SHADER_RESOURCE;

    if (IsRWBuffer(desc.storageType))
        flags |= D3D11_BIND_UNORDERED_ACCESS;

    return flags;
}

static UINT GetMiscFlags(const BufferDescriptor::StorageBuffer& desc)
{
    if (IsStructuredBuffer(desc.storageType))
        return D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    if (IsByteAddressBuffer(desc.storageType))
        return D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    return 0;
}

static UINT GetUAVFlags(const BufferDescriptor::StorageBuffer& desc)
{
    switch (desc.storageType)
    {
        case StorageBufferType::ByteAddressBuffer:          /* pass */
        case StorageBufferType::RWByteAddressBuffer:        return D3D11_BUFFER_UAV_FLAG_RAW;
        case StorageBufferType::AppendStructuredBuffer:     return D3D11_BUFFER_UAV_FLAG_APPEND;
        case StorageBufferType::ConsumeStructuredBuffer:    return D3D11_BUFFER_UAV_FLAG_COUNTER;
        default:                                            return 0;
    }
}

static DXGI_FORMAT GetResourceViewFormat(const BufferDescriptor::StorageBuffer& desc)
{
    /*
    D3D11_BUFFER_UAV_FLAG_RAW buffer flag for ByteAddressBuffer requires the UAV to have the DXGI_FORMAT_R32_TYPELESS format.
    -> see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476096(v=vs.85).aspx
    */
    if (IsTypedBuffer(desc.storageType))
        return D3D11Types::Map(desc.format);
    if (IsByteAddressBuffer(desc.storageType))
        return DXGI_FORMAT_R32_TYPELESS;
    return DXGI_FORMAT_UNKNOWN;
}

D3D11StorageBuffer::D3D11StorageBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer  { BufferType::Storage            },
    storageType_ { desc.storageBuffer.storageType }

{
    if (desc.storageBuffer.stride == 0)
        throw std::invalid_argument("storage buffer stride cannot be zero for a D3D11 resource view");

    /* Create D3D hardware buffer */
    D3D11_BUFFER_DESC bufferDesc;
    {
        bufferDesc.ByteWidth            = static_cast<UINT>(desc.size);
        bufferDesc.Usage                = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags            = GetBindFlags(desc.storageBuffer);
        bufferDesc.CPUAccessFlags       = 0;
        bufferDesc.MiscFlags            = GetMiscFlags(desc.storageBuffer);
        bufferDesc.StructureByteStride  = desc.storageBuffer.stride;
    }
    CreateResource(device, bufferDesc, initialData, desc.flags);

    /* Create either UAV or SRV */
    auto format         = GetResourceViewFormat(desc.storageBuffer);
    auto numElements    = static_cast<UINT>(desc.size) / desc.storageBuffer.stride;

    CreateSRV(device, format, 0, numElements);

    if (IsRWBuffer(GetStorageType()))
        CreateUAV(device, format, 0, numElements, GetUAVFlags(desc.storageBuffer));
}


/*
 * ======= Private: =======
 */

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
    auto hr = device->CreateShaderResourceView(GetNative(), &desc, srv_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resource-view (SRV) for storage buffer");
}

void D3D11StorageBuffer::CreateUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements, UINT flags)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        desc.Format                 = format;
        desc.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
        desc.Buffer.Flags           = flags;
    }
    auto hr = device->CreateUnorderedAccessView(GetNative(), &desc, uav_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}


} // /namespace LLGL



// ================================================================================
