/*
 * D3D12HardwareBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12HardwareBuffer.h"
#include "../../Assertion.h"
#include "../DXCore.h"
#include "../D3DX12/d3dx12.h"
#include <stdexcept>


namespace LLGL
{


void D3D12HardwareBuffer::CreateResource(ID3D12Device* device, UINT bufferSize)
{
    bufferSize_ = bufferSize;

    /* Create generic buffer resource */
    CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize_);

    auto hr = device->CreateCommittedResource(
        &defaultHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource_)
    );

    DXThrowIfFailed(hr, "failed to create comitted resource for D3D12 hardware buffer");
}

void D3D12HardwareBuffer::UpdateSubResource(
    ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
    const void* data, UINT bufferSize, UINT64 offset, D3D12_RESOURCE_STATES uploadState)
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
        IID_PPV_ARGS(&bufferUpload)
    );

    DXThrowIfFailed(hr, "failed to create comitted resource for D3D12 upload buffer");

    /* Copy data into upload buffer */
    void* dest = nullptr;
    
    hr = bufferUpload->Map(0, nullptr, &dest);
    DXThrowIfFailed(hr, "failed to map D3D12 resource");
    {
        ::memcpy(dest, data, bufferSize);
    }
    bufferUpload->Unmap(0, nullptr);

    /* Upload memory to GPU */
    gfxCommandList->CopyBufferRegion(resource_.Get(), 0, bufferUpload.Get(), 0, bufferSize);

    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, uploadState
    );

    gfxCommandList->ResourceBarrier(1, &resourceBarrier);
}


} // /namespace LLGL



// ================================================================================
