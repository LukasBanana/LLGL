/*
 * D3D11Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Buffer.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11Buffer::D3D11Buffer(const BufferType type) :
    Buffer( type )
{
}

D3D11Buffer::D3D11Buffer(const BufferType type, ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData) :
    Buffer( type )
{
    CreateResource(device, desc, initialData);
}

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    CD3D11_BOX destBox(offset, 0, 0, offset + dataSize, 1, 1);
    context->UpdateSubresource(buffer_.Get(), 0, &destBox, data, 0, 0);
}

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data)
{
    context->UpdateSubresource(buffer_.Get(), 0, nullptr, data, 0, 0);
}

static bool HasReadAccess(const BufferCPUAccess access)
{
    return (access != BufferCPUAccess::WriteOnly);
}

static bool HasWriteAccess(const BufferCPUAccess access)
{
    return (access != BufferCPUAccess::ReadOnly);
}

void* D3D11Buffer::Map(ID3D11DeviceContext* context, const BufferCPUAccess access)
{
    HRESULT hr = 0;
    D3D11_MAPPED_SUBRESOURCE mapppedSubresource;

    if (cpuAccessBuffer_)
    {
        /* On read access -> copy storage buffer to CPU-access buffer */
        if (HasReadAccess(access))
            context->CopyResource(cpuAccessBuffer_.Get(), Get());

        /* Map CPU-access buffer */
        hr = context->Map(cpuAccessBuffer_.Get(), 0, D3D11Types::Map(access), 0, &mapppedSubresource);
    }
    else
    {
        /* Map buffer */
        hr = context->Map(Get(), 0, D3D11Types::Map(access), 0, &mapppedSubresource);
    }

    return (SUCCEEDED(hr) ? mapppedSubresource.pData : nullptr);
}

void D3D11Buffer::Unmap(ID3D11DeviceContext* context, const BufferCPUAccess access)
{
    if (cpuAccessBuffer_)
    {
        /* Unmap CPU-access buffer */
        context->Unmap(cpuAccessBuffer_.Get(), 0);

        /* On write access -> copy CPU-access buffer to storage buffer */
        if (HasWriteAccess(access))
            context->CopyResource(Get(), cpuAccessBuffer_.Get());
    }
    else
    {
        /* Unmap buffer */
        context->Unmap(Get(), 0);
    }
}


/*
 * ======= Protected: =======
 */

void D3D11Buffer::CreateResource(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData)
{
    /* Setup initial subresource data */
    D3D11_SUBRESOURCE_DATA subresourceData;
    
    if (initialData)
    {
        InitMemory(subresourceData);
        subresourceData.pSysMem = initialData;
    }

    /* Create new D3D11 hardware buffer */
    auto hr = device->CreateBuffer(&desc, (initialData != nullptr ? &subresourceData : nullptr), buffer_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 buffer");
}

void D3D11Buffer::CreateCPUAccessBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& gpuBufferDesc, UINT cpuAccessFlags)
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

D3D11_USAGE D3D11Buffer::GetUsageForCPUAccessFlags(UINT cpuAccessFlags) const
{
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476106(v=vs.85).aspx
    if ((cpuAccessFlags & D3D11_CPU_ACCESS_READ) != 0)
        return D3D11_USAGE_STAGING;
    else if ((cpuAccessFlags & D3D11_CPU_ACCESS_WRITE) != 0)
        return D3D11_USAGE_DYNAMIC;
    else
        return D3D11_USAGE_DEFAULT;
}

UINT D3D11Buffer::GetCPUAccessFlags(long bufferFlags) const
{
    UINT flags = 0;

    if ((bufferFlags & BufferFlags::MapReadAccess) != 0)
        flags |= D3D11_CPU_ACCESS_READ;
    if ((bufferFlags & BufferFlags::MapWriteAccess) != 0)
        flags |= D3D11_CPU_ACCESS_WRITE;

    return flags;
}


} // /namespace LLGL



// ================================================================================
