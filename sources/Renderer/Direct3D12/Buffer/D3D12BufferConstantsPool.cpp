/*
 * D3D12BufferConstantsPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12BufferConstantsPool.h"
#include "D3D12StagingBufferPool.h"
#include "../Command/D3D12CommandQueue.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Resource.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <string.h>


namespace LLGL
{


D3D12BufferConstantsPool& D3D12BufferConstantsPool::Get()
{
    static D3D12BufferConstantsPool g_instance;
    return g_instance;
}

void D3D12BufferConstantsPool::InitializeDevice(
    ID3D12Device*           device,
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12StagingBufferPool& stagingBufferPool)
{
    /* Register constants */
    SmallVector<std::uint32_t> data;
    {
        if (auto* valueZeroUint64 = AllocConstants<UINT64>(D3D12BufferConstants::ZeroUInt64, data))
            *valueZeroUint64 = 0;
    }
    CreateImmutableBuffer(device, commandContext, commandQueue, stagingBufferPool, data);
}

void D3D12BufferConstantsPool::Clear()
{
    resource_.native.Reset();
}

D3D12BufferConstantsView D3D12BufferConstantsPool::FetchConstantsView(const D3D12BufferConstants id)
{
    const auto idx = static_cast<std::size_t>(id);
    if (idx < constants_.size())
    {
        const ConstantRange& range = constants_[idx];
        return { resource_.Get(), range.offset, range.size };
    }
    return {};
}


/*
 * ======= Private: =======
 */

std::uint32_t* D3D12BufferConstantsPool::AllocConstants(
    const D3D12BufferConstants  id,
    std::size_t                 size,
    SmallVector<std::uint32_t>& data)
{
    LLGL_ASSERT(size % sizeof(std::uint32_t) == 0, "D3D12 constants pool must be 4 byte aligned");
    const std::size_t count = size/sizeof(std::uint32_t);

    /* Allocate new register */
    const auto idx = static_cast<std::size_t>(id);
    if (idx >= constants_.size())
        constants_.resize(idx + 1);

    /* Write current offset */
    auto& reg = constants_[idx];
    {
        reg.offset  = sizeof(std::uint32_t) * data.size();
        reg.size    = sizeof(std::uint32_t) * count;
    }

    /* Write data to container */
    const std::size_t first = data.size();
    data.resize(first + static_cast<std::size_t>(count));
    return &data[first];
}

void D3D12BufferConstantsPool::CreateImmutableBuffer(
    ID3D12Device*                   device,
    D3D12CommandContext&            commandContext,
    D3D12CommandQueue&              commandQueue,
    D3D12StagingBufferPool&         stagingBufferPool,
    const ArrayView<std::uint32_t>& data)
{
    /* Create generic buffer resource */
    const UINT64 bufferSize = data.size() * sizeof(std::uint32_t);
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    resource_.usageState = D3D12_RESOURCE_STATE_COPY_SOURCE;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, // Buffers are effectively created in D3D12_RESOURCE_STATE_COMMON state
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 buffer constants pool");

    /* Initialize buffer with registered constants */
    stagingBufferPool.WriteImmediate(commandContext, resource_, 0, data.data(), bufferSize);
    commandQueue.FinishAndSubmitCommandContext(commandContext, true);
}


} // /namespace LLGL



// ================================================================================
