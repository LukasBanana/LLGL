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
    D3D11Buffer( BufferType::Storage )
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
    CreateResource(device, bufferDesc, initialData);

    /* Create either UAV or SRV */
    auto numElements = desc.size / desc.storageBuffer.stride;

    CreateSRV(device, 0, numElements);

    if (HasUAV())
        CreateUAV(device, 0, numElements);

    /* Create CPU access buffer (if required) */
    //if (???)
        CreateCPUAccessBuffer(device, bufferDesc, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
}

static bool HasReadAccess(const BufferCPUAccess access)
{
    return (access != BufferCPUAccess::WriteOnly);
}

static bool HasWriteAccess(const BufferCPUAccess access)
{
    return (access != BufferCPUAccess::ReadOnly);
}

void* D3D11StorageBuffer::Map(ID3D11DeviceContext* context, const BufferCPUAccess access)
{
    /* On read access -> copy storage buffer to CPU-access buffer */
    if (HasReadAccess(access))
        context->CopyResource(cpuAccessBuffer_.Get(), Get());

    /* Map CPU-access buffer */
    D3D11_MAPPED_SUBRESOURCE mapppedSubresource;
    auto hr = context->Map(cpuAccessBuffer_.Get(), 0, D3D11Types::Map(access), 0, &mapppedSubresource);
    return (SUCCEEDED(hr) ? mapppedSubresource.pData : nullptr);
}

void D3D11StorageBuffer::Unmap(ID3D11DeviceContext* context, const BufferCPUAccess access)
{
    /* Unmap CPU-access buffer */
    context->Unmap(cpuAccessBuffer_.Get(), 0);

    /* On write access -> copy CPU-access buffer to storage buffer */
    if (HasWriteAccess(access))
        context->CopyResource(Get(), cpuAccessBuffer_.Get());
}

bool D3D11StorageBuffer::HasUAV() const
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

UINT D3D11StorageBuffer::GetBindFlags() const
{
    return (HasUAV() ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE);
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
    auto hr = device->CreateUnorderedAccessView(Get(), &desc, uav_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 unordered-acces-view (UAV) for storage buffer");
}

void D3D11StorageBuffer::CreateCPUAccessBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& gpuBufferDesc, UINT cpuAccessFlags)
{
    D3D11_BUFFER_DESC desc;
    {
        desc.ByteWidth              = gpuBufferDesc.ByteWidth;
        desc.Usage                  = GetUsageForCPUAccessFlags(cpuAccessFlags);
        desc.BindFlags              = 0;
        desc.CPUAccessFlags         = cpuAccessFlags;
        desc.MiscFlags              = 0;
        desc.StructureByteStride    = gpuBufferDesc.StructureByteStride;
    }
    auto hr = device->CreateBuffer(&desc, nullptr, cpuAccessBuffer_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 CPU-access buffer for storage buffer");
}


} // /namespace LLGL



// ================================================================================
