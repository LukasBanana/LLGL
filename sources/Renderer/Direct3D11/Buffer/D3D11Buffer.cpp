/*
 * D3D11Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11Buffer.h"
#include "../D3D11Types.h"
#include "../D3D11ResourceFlags.h"
#include "../D3D11ObjectUtils.h"
#include "../../ResourceUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/Direct3D11/NativeHandle.h>


namespace LLGL
{


// Returns true if the specified buffer descriptors requires an intermediate buffer for CPU-access.
static bool NeedsIntermediateCpuAccessBuffer(const BufferDescriptor& desc)
{
    return (desc.cpuAccessFlags != 0);
}

D3D11Buffer::D3D11Buffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    Buffer          { desc.bindFlags                       },
    bindingLocator_ { ResourceType::Buffer, desc.bindFlags }
{
    CreateGpuBuffer(device, desc, initialData);

    if (NeedsIntermediateCpuAccessBuffer(desc))
        CreateCpuAccessBuffer(device, DXGetCPUAccessFlags(desc.cpuAccessFlags), desc.stride);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

bool D3D11Buffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleD3D = GetTypedNativeHandle<Direct3D11::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleD3D->deviceChild = buffer_.Get();
        nativeHandleD3D->deviceChild->AddRef();
        return true;
    }
    return false;
}

void D3D11Buffer::SetDebugName(const char* name)
{
    D3D11SetObjectName(GetNative(), name);
    if (cpuAccessBuffer_)
        D3D11SetObjectNameSubscript(cpuAccessBuffer_.Get(), name, ".CPUAccessBuffer");
}

BufferDescriptor D3D11Buffer::GetDesc() const
{
    /* Get native buffer descriptor and convert */
    D3D11_BUFFER_DESC nativeDesc;
    GetNative()->GetDesc(&nativeDesc);

    BufferDescriptor bufferDesc;
    bufferDesc.size         = nativeDesc.ByteWidth;
    bufferDesc.bindFlags    = GetBindFlags();

    if (cpuAccessBuffer_)
    {
        /* Convert CPU access flags from secondary buffer */
        D3D11_BUFFER_DESC cpuAccessDesc;
        cpuAccessBuffer_->GetDesc(&cpuAccessDesc);
        if ((cpuAccessDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) != 0)
            bufferDesc.cpuAccessFlags |= CPUAccessFlags::Read;
        if ((cpuAccessDesc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) != 0)
            bufferDesc.cpuAccessFlags |= CPUAccessFlags::Write;
    }

    if (nativeDesc.Usage == D3D11_USAGE_DYNAMIC)
        bufferDesc.miscFlags |= MiscFlags::DynamicUsage;

    return bufferDesc;
}

void D3D11Buffer::WriteSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    /* Validate parameters */
    LLGL_ASSERT_RANGE(dataSize + offset, GetSize());

    /* Discard previous content if the entire resource will be updated */
    const bool isWholeBufferUpdated = (offset == 0 && dataSize == GetSize());

    if (GetDXUsage() == D3D11_USAGE_DYNAMIC)
    {
        if (isWholeBufferUpdated)
        {
            /* Update partial subresource by mapping buffer from GPU into CPU memory space */
            D3D11_MAPPED_SUBRESOURCE mappedSubresource;
            if (SUCCEEDED(context->Map(GetNative(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
            {
                ::memcpy(reinterpret_cast<char*>(mappedSubresource.pData) + offset, data, dataSize);
                context->Unmap(GetNative(), 0);
            }
        }
        else
        {
            /* D3D11 buffers with D3D11_USAGE_DYNAMIC cannot be mapped with D3D11_MAP_WRITE, so we have to copy the input data into an intermediate buffer */
            WriteWithSubresourceCopyWithCpuAccess(context, data, dataSize, offset);
        }
    }
    else
    {
        if (isWholeBufferUpdated)
        {
            /* Update entire subresource */
            LLGL_ASSERT(dataSize == GetSize(), "cannot update D3D11 buffer partially when it is created with static usage");
            context->UpdateSubresource(GetNative(), 0, nullptr, data, 0, 0);
        }
        else if ((GetBindFlags() & BindFlags::ConstantBuffer) != 0)
        {
            /* D3D11 constant buffers cannot use UpdateSubresource for partial updates, so we have to copy the input data into an intermediate buffer */
            WriteWithSubresourceCopyWithCpuAccess(context, data, dataSize, offset);
        }
        else
        {
            /* Update subresource region of buffer */
            const D3D11_BOX dstBox{ offset, 0, 0, offset + dataSize, 1, 1 };
            context->UpdateSubresource(GetNative(), 0, &dstBox, data, 0, 0);
        }
    }
}

void D3D11Buffer::ReadSubresource(ID3D11DeviceContext* context, void* data, UINT dataSize, UINT offset)
{
    //NOTE:
    //  At the moment, the internal CPU access buffer is always created with D3D11_USAGE_DEFAULT, so don't need to check for D3D11_USAGE_STAGING.
    #if 0
    if (cpuAccessBuffer_)
    {
        D3D11_BUFFER_DESC cpuAccessBufferDesc;
        cpuAccessBuffer_->GetDesc(&cpuAccessBufferDesc);
        if (cpuAccessBufferDesc.Usage == D3D11_USAGE_STAGING && (cpuAccessBufferDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) != 0)
            ReadFromStagingBuffer(context, cpuAccessBuffer_.Get(), offset, data, dataSize, offset);
        else
            ReadFromSubresourceCopyWithCpuAccess(context, data, dataSize, offset);
    }
    else
    #endif
        ReadFromSubresourceCopyWithCpuAccess(context, data, dataSize, offset);
}

static D3D11_MAP GetCPUAccessTypeForUsage(D3D11_USAGE usage, CPUAccess access)
{
    /* D3D11_MAP_WRITE_DISCARD can only be used for buffers with D3D11_USAGE_DYNAMIC usage */
    if (access == CPUAccess::WriteDiscard && usage != D3D11_USAGE_DYNAMIC)
        return D3D11_MAP_WRITE;
    else
        return D3D11Types::Map(access);
}

void* D3D11Buffer::Map(ID3D11DeviceContext* context, const CPUAccess access, UINT offset, UINT length)
{
    if (offset + length > GetSize())
        return nullptr;

    HRESULT hr = 0;
    D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};

    if (cpuAccessBuffer_)
    {
        /* On read access -> copy storage buffer to CPU-access buffer */
        if (HasReadAccess(access))
        {
            if (offset == 0 && length == GetSize())
            {
                /* Copy entire resource */
                context->CopyResource(cpuAccessBuffer_.Get(), GetNative());
            }
            else
            {
                /* Copy subresource */
                const D3D11_BOX srcRange{ offset, 0, 0, offset + length, 1, 1 };
                context->CopySubresourceRegion(cpuAccessBuffer_.Get(), 0, offset, 0, 0, GetNative(), 0, &srcRange);
            }
        }

        /* Mark dirty-bit if write access is used */
        if (HasWriteAccess(access))
        {
            mappedWriteRange_[0] = offset;
            mappedWriteRange_[1] = offset + length;
        }

        /* Map intermediate CPU-access buffer */
        hr = context->Map(cpuAccessBuffer_.Get(), 0, GetCPUAccessTypeForUsage(D3D11_USAGE_DEFAULT, access), 0, &mappedSubresource);
    }
    else
    {
        /* Map primary buffer */
        hr = context->Map(GetNative(), 0, GetCPUAccessTypeForUsage(usage_, access), 0, &mappedSubresource);
    }

    return (SUCCEEDED(hr) ? mappedSubresource.pData : nullptr);
}

void D3D11Buffer::Unmap(ID3D11DeviceContext* context)
{
    if (cpuAccessBuffer_)
    {
        /* Unmap CPU-access buffer */
        context->Unmap(cpuAccessBuffer_.Get(), 0);

        /* On write access -> copy CPU-access buffer to storage buffer */
        if (mappedWriteRange_[0] < mappedWriteRange_[1])
        {
            const D3D11_BOX srcRange{ mappedWriteRange_[0], 0, 0, mappedWriteRange_[1], 1, 1 };
            context->CopySubresourceRegion(GetNative(), 0, mappedWriteRange_[0], 0, 0, cpuAccessBuffer_.Get(), 0, &srcRange);
            mappedWriteRange_[0] = 0;
            mappedWriteRange_[1] = 0;
        }
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

static UINT GetD3DBufferSize(const BufferDescriptor& desc)
{
    auto size = static_cast<UINT>(desc.size);
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        return GetAlignedSize(size, 16u);
    else
        return size;
}

void D3D11Buffer::CreateGpuBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData)
{
    /* Initialize native buffer descriptor */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = GetD3DBufferSize(desc);
        descD3D.Usage               = DXGetBufferUsage(desc);
        descD3D.BindFlags           = DXGetBufferBindFlags(desc.bindFlags);
        descD3D.CPUAccessFlags      = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags           = DXGetBufferMiscFlags(desc);
        descD3D.StructureByteStride = desc.stride;
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
        HRESULT hr = device->CreateBuffer(&descD3D, &subresourceData, buffer_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11Buffer");
    }
    else
    {
        /* Create native D3D11 buffer */
        HRESULT hr = device->CreateBuffer(&descD3D, nullptr, buffer_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11Buffer");
    }

    /* Store buffer creation attributes */
    size_   = descD3D.ByteWidth;
    stride_ = (desc.vertexAttribs.empty() ? 0 : desc.vertexAttribs.front().stride);
    format_ = DXTypes::ToDXGIFormat(desc.format);
    usage_  = descD3D.Usage;
}

void D3D11Buffer::CreateCpuAccessBuffer(ID3D11Device* device, UINT cpuAccessFlags, UINT stride)
{
    /* Create new D3D11 hardware buffer (for CPU access) */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = GetSize();
        descD3D.Usage               = D3D11_USAGE_DEFAULT;
        descD3D.BindFlags           = D3D11_BIND_SHADER_RESOURCE; // Must have either SRV or UAV bind flags for D3D11_USAGE_DEFAULT
        descD3D.CPUAccessFlags      = cpuAccessFlags;
        descD3D.MiscFlags           = 0;
        descD3D.StructureByteStride = stride;
    }
    HRESULT hr = device->CreateBuffer(&descD3D, nullptr, cpuAccessBuffer_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for CPU-access buffer");
}

void D3D11Buffer::ReadFromStagingBuffer(
    ID3D11DeviceContext*    context,
    ID3D11Buffer*           stagingBuffer,
    UINT                    stagingBufferOffset,
    void*                   data,
    UINT                    dataSize,
    UINT                    srcOffset)
{
    /* Copy memory range from GPU buffer to CPU-access buffer */
    const D3D11_BOX srcRange{ srcOffset, 0, 0, srcOffset + dataSize, 1, 1 };
    context->CopySubresourceRegion(stagingBuffer, 0, stagingBufferOffset, 0, 0, GetNative(), 0, &srcRange);

    /* Map CPU-access buffer to read data */
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    if (SUCCEEDED(context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource)))
    {
        /* Copy mapped memory to output data */
        const char* mappedSubresourceWithOffset = reinterpret_cast<const char*>(mappedSubresource.pData) + stagingBufferOffset;
        ::memcpy(data, mappedSubresourceWithOffset, dataSize);
        context->Unmap(stagingBuffer, 0);
    }
}

void D3D11Buffer::ReadFromSubresourceCopyWithCpuAccess(
    ID3D11DeviceContext*    context,
    void*                   data,
    UINT                    dataSize,
    UINT                    srcOffset)
{
    /* Get device this D3D buffer was created with */
    ComPtr<ID3D11Device> device;
    GetNative()->GetDevice(device.GetAddressOf());

    /* Create intermediate staging buffer */
    D3D11_BUFFER_DESC stagingBufferDesc;
    {
        stagingBufferDesc.ByteWidth           = dataSize;
        stagingBufferDesc.Usage               = D3D11_USAGE_STAGING;
        stagingBufferDesc.BindFlags           = 0;
        stagingBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_READ;
        stagingBufferDesc.MiscFlags           = 0;
        stagingBufferDesc.StructureByteStride = 0;
    }
    ComPtr<ID3D11Buffer> stagingBuffer;
    HRESULT hr = device->CreateBuffer(&stagingBufferDesc, nullptr, stagingBuffer.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for intermediate staging buffer");

    /* Read data from intermediate stating buffer with CPU access */
    ReadFromStagingBuffer(context, stagingBuffer.Get(), 0, data, dataSize, srcOffset);
}

void D3D11Buffer::WriteWithStagingBuffer(
    ID3D11DeviceContext*    context,
    ID3D11Buffer*           stagingBuffer,
    const void*             data,
    UINT                    dataSize,
    UINT                    dstOffset)
{
    /* Map CPU-access buffer to write data */
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    if (SUCCEEDED(context->Map(stagingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
    {
        /* Copy mapped memory to output data */
        ::memcpy(mappedSubresource.pData, data, dataSize);
        context->Unmap(stagingBuffer, 0);
    }

    /* Copy memory range from CPU-access buffer to GPU buffer */
    const D3D11_BOX srcRange{ 0, 0, 0, dataSize, 1, 1 };
    context->CopySubresourceRegion(GetNative(), 0, dstOffset, 0, 0, stagingBuffer, 0, &srcRange);
}

void D3D11Buffer::WriteWithSubresourceCopyWithCpuAccess(
    ID3D11DeviceContext*    context,
    const void*             data,
    UINT                    dataSize,
    UINT                    dstOffset)
{
    /* Get device this D3D buffer was created with */
    ComPtr<ID3D11Device> device;
    GetNative()->GetDevice(device.GetAddressOf());

    /* Create intermediate staging buffer */
    D3D11_BUFFER_DESC stagingBufferDesc;
    {
        stagingBufferDesc.ByteWidth           = dataSize;
        stagingBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
        stagingBufferDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE; // Must have either SRV or UAV bind flags for D3D11_USAGE_DEFAULT
        stagingBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        stagingBufferDesc.MiscFlags           = 0;
        stagingBufferDesc.StructureByteStride = 0;
    }
    ComPtr<ID3D11Buffer> stagingBuffer;
    HRESULT hr = device->CreateBuffer(&stagingBufferDesc, nullptr, stagingBuffer.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for intermediate staging buffer");

    /* Read data from intermediate stating buffer with CPU access */
    WriteWithStagingBuffer(context, stagingBuffer.Get(), data, dataSize, dstOffset);
}


} // /namespace LLGL



// ================================================================================
