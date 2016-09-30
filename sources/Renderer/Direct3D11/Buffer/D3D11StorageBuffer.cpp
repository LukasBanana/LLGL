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


D3D11StorageBuffer_::D3D11StorageBuffer_(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer( BufferType::Storage )
{
    storageType_ = desc.storageBufferDesc.storageType;

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
        CreateUAV(device);
    else
        CreateSRV(device);
}

bool D3D11StorageBuffer_::IsUAV() const
{
    return ( storageType_ >= StorageBufferType::RWBuffer );
}

bool D3D11StorageBuffer_::IsStructured() const
{
    return ( storageType_ == StorageBufferType::StructuredBuffer       ||
             storageType_ == StorageBufferType::RWStructuredBuffer     ||
             storageType_ == StorageBufferType::AppendStructuredBuffer ||
             storageType_ == StorageBufferType::ConsumeStructuredBuffer );
}

bool D3D11StorageBuffer_::IsByteAddressable() const
{
    return ( storageType_ == StorageBufferType::ByteAddressBuffer ||
             storageType_ == StorageBufferType::RWByteAddressBuffer );
}


/*
 * ======= Private: =======
 */

void D3D11StorageBuffer_::CreateUAV(ID3D11Device* device)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        //desc.Format = ;

    }
    auto hr = device->CreateUnorderedAccessView(Get(), &desc, &uav_);
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}

void D3D11StorageBuffer_::CreateSRV(ID3D11Device* device)
{
    /* Initialize descriptor and create SRV */
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    {


    }
    auto hr = device->CreateShaderResourceView(Get(), &desc, &srv_);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resource-view (SRV) for storage buffer");
}


} // /namespace LLGL



// ================================================================================
