/*
 * D3D12StagingBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12StagingBuffer.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"
#include "../D3DX12/d3dx12.h"
#include <string.h>


namespace LLGL
{


D3D12StagingBuffer::D3D12StagingBuffer(
    ID3D12Device*   device,
    UINT64          size,
    UINT            alignment,
    D3D12_HEAP_TYPE heapType)
{
    Create(device, size, alignment, heapType);
}

D3D12StagingBuffer::D3D12StagingBuffer(D3D12StagingBuffer&& rhs) noexcept :
    native_ { std::move(rhs.native_) },
    size_   { rhs.size_              },
    offset_ { rhs.offset_            }
{
}

D3D12StagingBuffer& D3D12StagingBuffer::operator = (D3D12StagingBuffer&& rhs) noexcept
{
    if (this != &rhs)
    {
        native_ = std::move(rhs.native_);
        size_   = rhs.size_;
        offset_ = rhs.offset_;
    }
    return *this;
}

void D3D12StagingBuffer::Create(
    ID3D12Device*   device,
    UINT64          size,
    UINT            alignment,
    D3D12_HEAP_TYPE heapType)
{
    size = GetAlignedSize<UINT64>(size, alignment);

    /* Create GPU upload buffer */
    const CD3DX12_HEAP_PROPERTIES heapProperties{ heapType };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        (heapType == D3D12_HEAP_TYPE_READBACK ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ),
        nullptr,
        IID_PPV_ARGS(native_.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for staging buffer");

    /* Set name for debugging/diagnostics */
    native_->SetName(L"LLGL::D3D12StagingBuffer");

    /* Store new size and reset write offset */
    size_   = size;
    offset_ = 0;
}

void D3D12StagingBuffer::Reset()
{
    offset_ = 0;
}

bool D3D12StagingBuffer::Capacity(UINT64 dataSize) const
{
    return (offset_ + dataSize <= size_);
}

HRESULT D3D12StagingBuffer::Write(
    ID3D12GraphicsCommandList*  commandList,
    ID3D12Resource*             dstBuffer,
    UINT64                      dstOffset,
    const void*                 data,
    UINT64                      dataSize)
{
    /* Map GPU host memory to CPU memory space CPU-memory to upload buffer */
    char* mappedData = nullptr;
    const D3D12_RANGE readRange{ 0, 0 };

    HRESULT hr = native_->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
    if (FAILED(hr))
        return hr;

    /* Copy input data to staging buffer */
    ::memcpy(mappedData + offset_, data, static_cast<std::size_t>(dataSize));

    /* Unmap buffer with range of written data */
    const D3D12_RANGE writtenRange
    {
        static_cast<SIZE_T>(offset_),
        static_cast<SIZE_T>(offset_ + dataSize)
    };
    native_->Unmap(0, &writtenRange);

    /* Encode copy buffer command */
    commandList->CopyBufferRegion(dstBuffer, dstOffset, native_.Get(), offset_, dataSize);

    return S_OK;
}

HRESULT D3D12StagingBuffer::WriteAndIncrementOffset(
    ID3D12GraphicsCommandList*  commandList,
    ID3D12Resource*             dstBuffer,
    UINT64                      dstOffset,
    const void*                 data,
    UINT64                      dataSize)
{
    HRESULT hr = Write(commandList, dstBuffer, dstOffset, data, dataSize);
    offset_ += dataSize;
    return hr;
}


} // /namespace LLGL



// ================================================================================
