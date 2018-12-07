/*
 * D3D11Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Buffer.h"
#include "../D3D11Types.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


D3D11Buffer::D3D11Buffer(long bindFlags) :
    Buffer { bindFlags }
{
}

D3D11Buffer::D3D11Buffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    Buffer { desc.bindFlags }
{
    CreateNativeBuffer(device, desc, initialData);
}

static D3D11_MAP GetD3DMapWrite(bool mapPartial)
{
    return (mapPartial ? D3D11_MAP_WRITE : D3D11_MAP_WRITE_DISCARD);
}

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    /* Validate parameters */
    LLGL_ASSERT_RANGE(dataSize + offset, GetSize());

    if (GetUsage() == D3D11_USAGE_DYNAMIC)
    {
        /* Update partial subresource by mapping buffer from GPU into CPU memory space */
        D3D11_MAPPED_SUBRESOURCE subresource;
        context->Map(GetNative(), 0, GetD3DMapWrite(dataSize < GetSize()), 0, &subresource);
        {
            ::memcpy(reinterpret_cast<char*>(subresource.pData) + offset, data, dataSize);
        }
        context->Unmap(GetNative(), 0);
    }
    else
    {
        if ((GetBindFlags() & BindFlags::ConstantBuffer) != 0)
        {
            /* Update entire subresource */
            if (dataSize == GetSize())
                context->UpdateSubresource(buffer_.Get(), 0, nullptr, data, 0, 0);
            else
                throw std::out_of_range(LLGL_ASSERT_INFO("cannot update D3D11 buffer partially when it is created with static usage"));
        }
        else
        {
            /* Update sub region of buffer */
            const D3D11_BOX destBox { offset, 0, 0, offset + dataSize, 1, 1 };
            context->UpdateSubresource(buffer_.Get(), 0, &destBox, data, 0, 0);
        }
    }
}

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data)
{
    context->UpdateSubresource(buffer_.Get(), 0, nullptr, data, 0, 0);
}

static bool HasReadAccess(const CPUAccess access)
{
    return (access != CPUAccess::WriteOnly);
}

static bool HasWriteAccess(const CPUAccess access)
{
    return (access != CPUAccess::ReadOnly);
}

void* D3D11Buffer::Map(ID3D11DeviceContext* context, const CPUAccess access)
{
    HRESULT hr = 0;
    D3D11_MAPPED_SUBRESOURCE mapppedSubresource;

    if (cpuAccessBuffer_)
    {
        /* On read access -> copy storage buffer to CPU-access buffer */
        if (HasReadAccess(access))
            context->CopyResource(cpuAccessBuffer_.Get(), GetNative());

        /* Map CPU-access buffer */
        hr = context->Map(cpuAccessBuffer_.Get(), 0, D3D11Types::Map(access), 0, &mapppedSubresource);
    }
    else
    {
        /* Map buffer */
        hr = context->Map(GetNative(), 0, D3D11Types::Map(access), 0, &mapppedSubresource);
    }

    return (SUCCEEDED(hr) ? mapppedSubresource.pData : nullptr);
}

void D3D11Buffer::Unmap(ID3D11DeviceContext* context, const CPUAccess access)
{
    if (cpuAccessBuffer_)
    {
        /* Unmap CPU-access buffer */
        context->Unmap(cpuAccessBuffer_.Get(), 0);

        /* On write access -> copy CPU-access buffer to storage buffer */
        if (HasWriteAccess(access))
            context->CopyResource(GetNative(), cpuAccessBuffer_.Get());
    }
    else
    {
        /* Unmap buffer */
        context->Unmap(GetNative(), 0);
    }
}


/*
 * ======= Protected: =======
 */

#if 0 // TODO: replace by "CreateNativeBuffer"
D3D11_BUFFER_DESC D3D11Buffer::GetNativeBufferDesc(const BufferDescriptor& desc, UINT bindFlags) const
{
    /* Initialize native buffer descriptor */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = static_cast<UINT>(desc.size);
        descD3D.Usage               = D3D11_USAGE_DEFAULT;
        descD3D.BindFlags           = bindFlags;
        descD3D.CPUAccessFlags      = 0;
        descD3D.MiscFlags           = 0;
        descD3D.StructureByteStride = 0;
    }

    /* Apply additional flags */
    if ((desc.flags & BufferFlags::IndirectBinding) != 0)
        descD3D.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

    return descD3D;
}

void D3D11Buffer::CreateResource(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData, long bufferFlags)
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

    /* Create CPU access buffer (if required) */
    auto cpuAccessFlags = GetCPUAccessFlags(bufferFlags);
    if (cpuAccessFlags != 0)
        CreateCPUAccessBuffer(device, desc, cpuAccessFlags);
}
#endif // /TODO

static UINT GetD3DBufferSize(const BufferDescriptor& desc)
{
    auto size = static_cast<UINT>(desc.size);
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        return GetAlignedSize(size, 16u);
    else
        return size;
}

static UINT GetD3DBindFlags(long bindFlags)
{
    UINT flagsD3D = 0;

    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        flagsD3D |= D3D11_BIND_VERTEX_BUFFER;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        flagsD3D |= D3D11_BIND_INDEX_BUFFER;
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        flagsD3D |= D3D11_BIND_CONSTANT_BUFFER;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flagsD3D |= D3D11_BIND_STREAM_OUTPUT;
    if ((bindFlags & BindFlags::SampleBuffer) != 0)
        flagsD3D |= D3D11_BIND_SHADER_RESOURCE;
    if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
        flagsD3D |= D3D11_BIND_UNORDERED_ACCESS;

    return flagsD3D;
}

static UINT GetD3DCPUAccessFlagsForMiscFlags(long miscFlags)
{
    UINT flagsD3D = 0;

    if ((miscFlags & MiscFlags::DynamicUsage) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_WRITE;

    return flagsD3D;
}

static UINT GetD3DMiscFlags(const BufferDescriptor& desc)
{
    UINT flagsD3D = 0;

    if ((desc.bindFlags & BindFlags::IndirectBuffer) != 0)
        flagsD3D |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

    if (IsStructuredBuffer(desc.storageBuffer.storageType))
        flagsD3D |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    else if (IsByteAddressBuffer(desc.storageBuffer.storageType))
        flagsD3D |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    return flagsD3D;
}

static UINT GetD3DCPUAccessFlags(long cpuAccessFlags)
{
    UINT flagsD3D = 0;

    if ((cpuAccessFlags & CPUAccessFlags::Read) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_READ;
    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
        flagsD3D |= D3D11_CPU_ACCESS_WRITE;

    return flagsD3D;
}

static D3D11_USAGE GetD3DUsageForCPUAccessFlags(UINT cpuAccessFlags)
{
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476106(v=vs.85).aspx
    if ((cpuAccessFlags & D3D11_CPU_ACCESS_READ) != 0)
        return D3D11_USAGE_STAGING;
    else if ((cpuAccessFlags & D3D11_CPU_ACCESS_WRITE) != 0)
        return D3D11_USAGE_DYNAMIC;
    else
        return D3D11_USAGE_DEFAULT;
}

void D3D11Buffer::CreateNativeBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData)
{
    /* Initialize native buffer descriptor */
    auto cpuAccessFlags = GetD3DCPUAccessFlagsForMiscFlags(desc.miscFlags);

    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = GetD3DBufferSize(desc);
        descD3D.Usage               = GetD3DUsageForCPUAccessFlags(cpuAccessFlags);
        descD3D.BindFlags           = GetD3DBindFlags(desc.bindFlags);
        descD3D.CPUAccessFlags      = cpuAccessFlags;
        descD3D.MiscFlags           = GetD3DMiscFlags(desc);
        descD3D.StructureByteStride = desc.storageBuffer.stride;
    }

    if (initialData)
    {
        /* Create native D3D11 buffer with initial subresource data */
        D3D11_SUBRESOURCE_DATA subresourceData;
        {
            subresourceData.pSysMem             = initialData;
            subresourceData.SysMemPitch         = 0;
            subresourceData.SysMemSlicePitch    = 0;
        }
        auto hr = device->CreateBuffer(&descD3D, &subresourceData, buffer_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11Buffer");
    }
    else
    {
        /* Create native D3D11 buffer */
        auto hr = device->CreateBuffer(&descD3D, nullptr, buffer_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11Buffer");
    }

    /* Create CPU access buffer (if required) */
    if (desc.cpuAccessFlags != 0)
        CreateCPUAccessBuffer(device, desc);

    /* Store buffer creation attributes */
    size_   = descD3D.ByteWidth;
    stride_ = desc.vertexBuffer.format.stride;
    format_ = D3D11Types::Map(desc.indexBuffer.format.GetDataType());
    usage_  = descD3D.Usage;
}

void D3D11Buffer::CreateCPUAccessBuffer(ID3D11Device* device, const BufferDescriptor& desc)
{
    auto cpuAccessFlags = GetD3DCPUAccessFlags(desc.cpuAccessFlags);

    /* Create new D3D11 hardware buffer (for CPU access) */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = static_cast<UINT>(desc.size);
        descD3D.Usage               = GetD3DUsageForCPUAccessFlags(cpuAccessFlags);
        descD3D.BindFlags           = 0;
        descD3D.CPUAccessFlags      = cpuAccessFlags;
        descD3D.MiscFlags           = 0;
        descD3D.StructureByteStride = desc.storageBuffer.stride;
    }
    auto hr = device->CreateBuffer(&descD3D, nullptr, cpuAccessBuffer_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for CPU-access buffer");
}


} // /namespace LLGL



// ================================================================================
