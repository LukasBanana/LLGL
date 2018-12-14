/*
 * D3D12Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Buffer.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
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

    #ifdef LLGL_DEBUG
    uploadBuffer->SetName(L"D3D12Buffer::UpdateStaticSubresource(uploadBuffer)");
    #endif

    /* Copy data into upload buffer, then copy upload buffer into destination resource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = data;
        subresourceData.RowPitch    = static_cast<LONG_PTR>(bufferSize);
        subresourceData.SlicePitch  = subresourceData.RowPitch;
    }

    commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST);

    ::UpdateSubresources<1>(
        commandContext.GetCommandList(),
        resource_.native.Get(),
        uploadBuffer.Get(),
        offset,
        0,
        1,
        &subresourceData
    );

    commandContext.TransitionResource(resource_, resource_.usageState);
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

    commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST);

    ::UpdateSubresources<1>(
        commandContext.GetCommandList(),
        resource_.native.Get(),
        uploadResource_.Get(),
        offset,
        0,
        1,
        &subresourceData
    );

    commandContext.TransitionResource(resource_, resource_.usageState);
}

void D3D12Buffer::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
    {
        viewDesc.BufferLocation = GetNative()->GetGPUVirtualAddress();
        viewDesc.SizeInBytes    = static_cast<UINT>(GetBufferSize());
    }
    device->CreateConstantBufferView(&viewDesc, cpuDescriptorHandle);
}


/*
 * ======= Protected: =======
 */

static D3D12_RESOURCE_FLAGS GetD3DResourceFlags(const BufferDescriptor& desc)
{
    UINT flags = 0;

    if ((desc.bindFlags & BindFlags::RWStorageBuffer) != 0)
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
    if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
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
    bufferSize_ = desc.size;

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

void D3D12Buffer::CreateUploadBuffer(ID3D12Device* device, const BufferDescriptor& desc)
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
