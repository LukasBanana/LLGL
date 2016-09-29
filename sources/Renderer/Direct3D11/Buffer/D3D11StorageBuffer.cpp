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


void D3D11StorageBuffer::CreateResource(
    ID3D11Device* device, UINT bufferSize, const BufferUsage usage, const StorageBufferType type, const void* initialData)
{
    type_ = type;

    /* Setup descriptor and create storage buffer */
    UINT bindFlags = (IsUAV() ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE);

    CD3D11_BUFFER_DESC bufferDesc(bufferSize, bindFlags);

    if (usage == BufferUsage::Dynamic)
    {
        bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
        bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    }

    if (IsStructured())
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    else if (IsByteAddressable())
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    /* Create D3D hardware buffer */
    hwBuffer.CreateResource(device, bufferDesc, initialData);

    /* Create either SRV or UAV */
    if (IsUAV())
        CreateUAV(device);
    else
        CreateSRV(device);
}

bool D3D11StorageBuffer::IsUAV() const
{
    return ( type_ >= StorageBufferType::RWBuffer );
}

bool D3D11StorageBuffer::IsStructured() const
{
    return ( type_ == StorageBufferType::StructuredBuffer       ||
             type_ == StorageBufferType::RWStructuredBuffer     ||
             type_ == StorageBufferType::AppendStructuredBuffer ||
             type_ == StorageBufferType::ConsumeStructuredBuffer );
}

bool D3D11StorageBuffer::IsByteAddressable() const
{
    return ( type_ == StorageBufferType::ByteAddressBuffer ||
             type_ == StorageBufferType::RWByteAddressBuffer );
}


/*
 * ======= Private: =======
 */

void D3D11StorageBuffer::CreateUAV(ID3D11Device* device)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        //desc.Format = ;

    }
    auto hr = device->CreateUnorderedAccessView(hwBuffer.Get(), &desc, &uav_);
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}

void D3D11StorageBuffer::CreateSRV(ID3D11Device* device)
{
    /* Initialize descriptor and create SRV */
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    {


    }
    auto hr = device->CreateShaderResourceView(hwBuffer.Get(), &desc, &srv_);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resource-view (SRV) for storage buffer");
}


} // /namespace LLGL



// ================================================================================
