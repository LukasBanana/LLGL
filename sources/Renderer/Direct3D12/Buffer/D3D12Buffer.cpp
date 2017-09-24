/*
 * D3D12Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Buffer.h"
#include "../../Assertion.h"
#include "../../DXCommon/DXCore.h"
#include "../D3DX12/d3dx12.h"
#include <stdexcept>


namespace LLGL
{


D3D12Buffer::D3D12Buffer(const BufferType type) :
    Buffer { type }
{
}

void D3D12Buffer::UpdateStaticSubresource(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ComPtr<ID3D12Resource>& uploadBuffer,
    const void* data, UINT64 bufferSize, UINT64 offset, D3D12_RESOURCE_STATES uploadState)
{
    if (offset + bufferSize > bufferSize_)
        throw std::out_of_range(LLGL_ASSERT_INFO("'bufferSize' and/or 'offset' are out of range"));

    /* Create resource to upload memory from CPU to GPU */
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    auto hr = device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
    );

    DXThrowIfFailed(hr, "failed to create D3D12 committed resource for upload buffer");

    /* Copy data into upload buffer */
    void* dest = nullptr;
    
    hr = uploadBuffer->Map(0, nullptr, &dest);
    DXThrowIfFailed(hr, "failed to map D3D12 resource");
    {
        ::memcpy(dest, data, static_cast<std::size_t>(bufferSize));
    }
    uploadBuffer->Unmap(0, nullptr);

    /* Upload memory to GPU */
    commandList->CopyBufferRegion(resource_.Get(), 0, uploadBuffer.Get(), 0, bufferSize);
    
    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, uploadState
    );
    
    commandList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12Buffer::UpdateDynamicSubresource(const void* data, UINT64 bufferSize, UINT64 offset)
{
    void* dest = nullptr;
    
    auto hr = resource_->Map(0, nullptr, &dest);
    DXThrowIfFailed(hr, "failed to map D3D12 resource");
    {
        ::memcpy((reinterpret_cast<char*>(dest) + offset), data, static_cast<std::size_t>(bufferSize));
    }
    resource_->Unmap(0, nullptr);
}


/*
 * ======= Protected: =======
 */

void D3D12Buffer::CreateResource(ID3D12Device* device, UINT64 bufferSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState)
{
    bufferSize_ = bufferSize;

    /* Create generic buffer resource */
    CD3DX12_HEAP_PROPERTIES heapProperties(heapType);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize_);

    auto hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        resourceState, // initial resource state
        nullptr,
        IID_PPV_ARGS(&resource_)
    );

    DXThrowIfFailed(hr, "failed to create comitted resource for D3D12 hardware buffer");
}

void D3D12Buffer::CreateResource(ID3D12Device* device, UINT64 bufferSize)
{
    CreateResource(device, bufferSize, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST);
}


} // /namespace LLGL



// ================================================================================
