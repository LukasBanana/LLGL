/*
 * D3D12Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Buffer.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Helper.h"
#include <stdexcept>


namespace LLGL
{


D3D12Buffer::D3D12Buffer(ID3D12Device* device, const BufferDescriptor& desc) :
    Buffer { desc.bindFlags }
{
    InitMemory(vertexBufferView_);
    InitMemory(indexBufferView_);
    CreateNativeBuffer(device, desc);
}

void D3D12Buffer::SetName(const char* name)
{
    D3D12SetObjectName(resource_.Get(), name);
    D3D12SetObjectNameSubscript(uploadResource_.Get(), name, ".Staging");
}

BufferDescriptor D3D12Buffer::GetDesc() const
{
    /* Get native resource descriptor and convert */
    D3D12_RESOURCE_DESC nativeDesc;
    nativeDesc = GetResource().native->GetDesc();

    BufferDescriptor bufferDesc;
    bufferDesc.size             = nativeDesc.Width;
    bufferDesc.bindFlags        = GetBindFlags();
    #if 0//TODO
    bufferDesc.cpuAccessFlags   = 0;
    bufferDesc.miscFlags        = 0;
    #endif

    return bufferDesc;
}

void D3D12Buffer::UpdateStaticSubresource(
    ID3D12Device*               device,
    D3D12CommandContext&        commandContext,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    const void*                 data,
    UINT64                      bufferSize,
    UINT64                      offset)
{
    LLGL_ASSERT_RANGE(offset + bufferSize, GetBufferSize());

    /* Create resource to upload memory from CPU to GPU */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for intermediate upload buffer");

    /* Copy data into upload buffer, then copy upload buffer into destination resource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = data;
        subresourceData.RowPitch    = static_cast<LONG_PTR>(bufferSize);
        subresourceData.SlicePitch  = subresourceData.RowPitch;
    }

    commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST, true);

    ::UpdateSubresources<1>(
        commandContext.GetCommandList(),
        resource_.native.Get(),
        uploadBuffer.Get(),
        offset,
        0,
        1,
        &subresourceData
    );

    commandContext.TransitionResource(resource_, resource_.usageState, true);
}

void D3D12Buffer::UpdateDynamicSubresource(
    D3D12CommandContext&    commandContext,
    const void*             data,
    UINT64                  dataSize,
    UINT64                  offset)
{
    /* Copy data into upload buffer, then copy upload buffer into destination resource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = data;
        subresourceData.RowPitch    = static_cast<LONG_PTR>(dataSize);
        subresourceData.SlicePitch  = subresourceData.RowPitch;
    }

    commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST, true);

    ::UpdateSubresources<1>(
        commandContext.GetCommandList(),
        resource_.native.Get(),
        uploadResource_.Get(),
        offset,
        0,
        1,
        &subresourceData
    );

    commandContext.TransitionResource(resource_, resource_.usageState, true);
}

void D3D12Buffer::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    {
        cbvDesc.BufferLocation  = GetNative()->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes     = static_cast<UINT>(GetBufferSize());
    }
    device->CreateConstantBufferView(&cbvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateShaderResourceView(device, cpuDescHandle, 0, static_cast<UINT>(bufferSize_ / structStride_), structStride_);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, UINT firstElement, UINT numElements, UINT elementStride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension               = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping     = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement         = firstElement;
        srvDesc.Buffer.NumElements          = numElements;
        srvDesc.Buffer.StructureByteStride  = elementStride;
        srvDesc.Buffer.Flags                = D3D12_BUFFER_SRV_FLAG_NONE;
    }
    device->CreateShaderResourceView(GetNative(), &srvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = static_cast<UINT>(bufferSize_ / structStride_);
        uavDesc.Buffer.StructureByteStride  = structStride_;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
    }
    device->CreateUnorderedAccessView(GetNative(), nullptr, &uavDesc, cpuDescHandle);
}


/*
 * ======= Protected: =======
 */

static D3D12_RESOURCE_FLAGS GetD3DResourceFlags(const BufferDescriptor& desc)
{
    UINT flags = 0;

    if ((desc.bindFlags & BindFlags::Storage) != 0)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return static_cast<D3D12_RESOURCE_FLAGS>(flags);
}

static D3D12_RESOURCE_STATES GetD3DUsageState(long bindFlags)
{
    D3D12_RESOURCE_STATES flagsD3D = D3D12_RESOURCE_STATE_COMMON;

    if ((bindFlags & (BindFlags::VertexBuffer | BindFlags::ConstantBuffer)) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if ((bindFlags & BindFlags::Storage) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

    return flagsD3D;
}

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
void D3D12Buffer::CreateNativeBuffer(ID3D12Device* device, const BufferDescriptor& desc)
{
    /* Store buffer attributes */
    bufferSize_     = desc.size;
    structStride_   = std::max(1u, desc.storageBuffer.stride);

    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Constant buffers must be aligned to 256 bytes */
        const UINT64 alignment = 256;
        bufferSize_ = GetAlignedSize(bufferSize_, alignment);
    }

    /* Determine initial resource state */
    resource_.usageState        = GetD3DUsageState(desc.bindFlags);
    resource_.transitionState   = D3D12_RESOURCE_STATE_COPY_DEST;

    /* Create generic buffer resource */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize_, GetD3DResourceFlags(desc)),
        resource_.transitionState,
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware buffer");

    /* Create upload buffer to dynamic buffers */
    CreateUploadBuffer(device, desc);

    /* Create sub-resource views */
    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0)
        CreateVertexBufferView(desc);
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        CreateIndexBufferView(desc);
}

void D3D12Buffer::CreateVertexBufferView(const BufferDescriptor& desc)
{
    vertexBufferView_.BufferLocation    = GetNative()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes       = static_cast<UINT>(GetBufferSize());
    vertexBufferView_.StrideInBytes     = desc.vertexBuffer.format.stride;
}

void D3D12Buffer::CreateIndexBufferView(const BufferDescriptor& desc)
{
    indexBufferView_.BufferLocation = GetNative()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes    = static_cast<UINT>(GetBufferSize());
    indexBufferView_.Format         = D3D12Types::Map(desc.indexBuffer.format);
}


/*
 * ======= Private: =======
 */

void D3D12Buffer::CreateUploadBuffer(ID3D12Device* device, const BufferDescriptor& /*desc*/)
{
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(GetBufferSize()),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadResource_.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "failed to create D3D12 committed resource for upload buffer");
}


} // /namespace LLGL



// ================================================================================
