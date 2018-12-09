/*
 * D3D12Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Buffer.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../D3D12CommandContext.h"
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
    UINT64                      offset,
    D3D12_RESOURCE_STATES       stateAfter)
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

    DXThrowIfFailed(hr, "failed to create D3D12 committed resource for upload buffer");

    #ifdef LLGL_DEBUG
    uploadBuffer->SetName(L"D3D12Buffer.uploadBuffer");
    #endif

    /* Copy data into upload buffer, then copy upload buffer into destination resource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = data;
        subresourceData.RowPitch    = static_cast<LONG_PTR>(bufferSize);
        subresourceData.SlicePitch  = subresourceData.RowPitch;
    }
    UpdateSubresources<1>(commandContext.GetCommandList(), resource_.native.Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);

    commandContext.TransitionResource(resource_, stateAfter);
}

void D3D12Buffer::UpdateDynamicSubresource(const void* data, UINT64 bufferSize, UINT64 offset)
{
    void* dest = nullptr;

    auto hr = resource_.native->Map(0, nullptr, &dest);
    DXThrowIfFailed(hr, "failed to map D3D12 resource");
    {
        ::memcpy((reinterpret_cast<char*>(dest) + offset), data, static_cast<std::size_t>(bufferSize));
    }
    resource_.native->Unmap(0, nullptr);
}

void D3D12Buffer::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
    {
        viewDesc.BufferLocation = GetNative()->GetGPUVirtualAddress();
        viewDesc.SizeInBytes    = bufferSize_;
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

static D3D12_HEAP_TYPE GetD3DHeapType(const BufferDescriptor& desc)
{
    if ((desc.miscFlags & MiscFlags::DynamicUsage) != 0)
        return D3D12_HEAP_TYPE_UPLOAD;
    else
        return D3D12_HEAP_TYPE_DEFAULT;
}

static D3D12_RESOURCE_STATES GetD3DUsageState(long bindFlags)
{
    if ((bindFlags & (BindFlags::VertexBuffer | BindFlags::ConstantBuffer)) != 0)
        return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    else if ((bindFlags & BindFlags::VertexBuffer) != 0)
        return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    else if ((bindFlags & BindFlags::IndexBuffer) != 0)
        return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    else if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    else if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        return D3D12_RESOURCE_STATE_STREAM_OUT;
    else if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    return (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
void D3D12Buffer::CreateNativeBuffer(ID3D12Device* device, const BufferDescriptor& desc)
{
    UINT64 alignment = 0;

    /* Store buffer attributes */
    bufferSize_ = desc.size;

    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Constant buffers must be aligned to 256 bytes */
        alignment    = 256;
        bufferSize_  = GetAlignedSize(bufferSize_, alignment);
    }

    /* Determine initial resource state */
    auto heapType = GetD3DHeapType(desc);

    D3D12_RESOURCE_STATES initialState;
    if (heapType == D3D12_HEAP_TYPE_UPLOAD)
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
    else
        initialState = GetD3DUsageState(desc.bindFlags);

    resource_.SetInitialState(initialState);

    /* Create generic buffer resource */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(heapType),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize_, GetD3DResourceFlags(desc), alignment),
        initialState,
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware buffer");

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
    indexBufferView_.Format         = D3D12Types::Map(desc.indexBuffer.format.GetDataType());
}


} // /namespace LLGL



// ================================================================================
