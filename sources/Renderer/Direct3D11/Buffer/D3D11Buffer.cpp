/*
 * D3D11Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Buffer.h"
#include "../D3D11Types.h"
#include "../D3D11ResourceFlags.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


D3D11Buffer::D3D11Buffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    Buffer { desc.bindFlags }
{
    CreateGpuBuffer(device, desc, initialData);
}

void D3D11Buffer::SetName(const char* name)
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

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset)
{
    /* Validate parameters */
    LLGL_ASSERT_RANGE(dataSize + offset, GetSize());

    if (GetDXUsage() == D3D11_USAGE_DYNAMIC)
    {
        /* Discard previous content if the entire resource will be updated */
        const bool writeDiscard = (offset == 0 && dataSize == GetSize());

        /* Update partial subresource by mapping buffer from GPU into CPU memory space */
        D3D11_MAPPED_SUBRESOURCE subresource;
        context->Map(GetNative(), 0, DXGetMapWrite(writeDiscard), 0, &subresource);
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
                context->UpdateSubresource(GetNative(), 0, nullptr, data, 0, 0);
            else
                throw std::out_of_range(LLGL_ASSERT_INFO("cannot update D3D11 buffer partially when it is created with static usage"));
        }
        else
        {
            /* Update sub region of buffer */
            const D3D11_BOX dstBox{ offset, 0, 0, offset + dataSize, 1, 1 };
            context->UpdateSubresource(GetNative(), 0, &dstBox, data, 0, 0);
        }
    }
}

void D3D11Buffer::UpdateSubresource(ID3D11DeviceContext* context, const void* data)
{
    context->UpdateSubresource(GetNative(), 0, nullptr, data, 0, 0);
}

static bool HasReadAccess(const CPUAccess access)
{
    return (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite);
}

static bool HasWriteAccess(const CPUAccess access)
{
    return (access >= CPUAccess::WriteOnly && access <= CPUAccess::ReadWrite);
}

// private
D3D11_MAP D3D11Buffer::GetCPUAccessTypeForUsage(const CPUAccess access) const
{
    /* D3D11_MAP_WRITE_DISCARD can only be used for buffers with dynamic usage */
    if (access == CPUAccess::WriteDiscard && usage_ != D3D11_USAGE_DYNAMIC)
        return D3D11_MAP_WRITE;
    else
        return D3D11Types::Map(access);
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
        hr = context->Map(cpuAccessBuffer_.Get(), 0, GetCPUAccessTypeForUsage(access), 0, &mapppedSubresource);
    }
    else
    {
        /* Map buffer */
        hr = context->Map(GetNative(), 0, GetCPUAccessTypeForUsage(access), 0, &mapppedSubresource);
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

void D3D11Buffer::CreateSubresourceSRV(
    ID3D11Device*               device,
    ID3D11ShaderResourceView**  srvOutput,
    const DXGI_FORMAT           format,
    UINT                        firstElement,
    UINT                        numElements,
    bool                        isRawView)
{
    /* Create shader-resource-view (SRV) for subresource */
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    {
        srvDesc.Format                  = D3D11Types::ToDXGIFormatSRV(format);
        srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement   = firstElement;
        srvDesc.BufferEx.NumElements    = numElements;
        srvDesc.BufferEx.Flags          = (isRawView ? D3D11_BUFFEREX_SRV_FLAG_RAW : 0);
    }
    auto hr = device->CreateShaderResourceView(GetNative(), &srvDesc, srvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", "for buffer subresource");
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
        CreateCpuAccessBuffer(device, desc);

    /* Store buffer creation attributes */
    size_   = descD3D.ByteWidth;
    stride_ = (desc.vertexAttribs.empty() ? 0 : desc.vertexAttribs.front().stride);
    format_ = D3D11Types::Map(desc.format);
    usage_  = descD3D.Usage;
}

void D3D11Buffer::CreateCpuAccessBuffer(ID3D11Device* device, const BufferDescriptor& desc)
{
    /* Create new D3D11 hardware buffer (for CPU access) */
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = static_cast<UINT>(desc.size);
        descD3D.Usage               = DXGetCPUAccessBufferUsage(desc);
        descD3D.BindFlags           = 0; // CPU-access buffer cannot have binding flags
        descD3D.CPUAccessFlags      = DXGetCPUAccessFlags(desc.cpuAccessFlags);
        descD3D.MiscFlags           = 0;
        descD3D.StructureByteStride = desc.stride;
    }
    auto hr = device->CreateBuffer(&descD3D, nullptr, cpuAccessBuffer_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for CPU-access buffer");
}


} // /namespace LLGL



// ================================================================================
