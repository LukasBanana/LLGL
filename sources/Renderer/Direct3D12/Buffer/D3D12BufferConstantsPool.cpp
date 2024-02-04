/*
 * D3D12BufferConstantsPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12BufferConstantsPool.h"
#include "D3D12StagingBufferPool.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Resource.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"
#include <algorithm>


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
    std::vector<std::uint64_t> data;
    {
        RegisterConstants(D3D12BufferConstants::ZeroUInt64, 0, 1, data);
    }
    CreateImmutableBuffer(device, commandContext, commandQueue, stagingBufferPool, data);
}

void D3D12BufferConstantsPool::Clear()
{
    resource_.native.Reset();
}

D3D12BufferConstantsView D3D12BufferConstantsPool::FetchConstants(const D3D12BufferConstants id)
{
    const auto idx = static_cast<std::size_t>(id);
    if (idx < registers_.size())
    {
        const auto& reg = registers_[idx];
        return { resource_.Get(), reg.offset, reg.size };
    }
    return {};
}


/*
 * ======= Private: =======
 */

void D3D12BufferConstantsPool::RegisterConstants(
    const D3D12BufferConstants  id,
    UINT64                      value,
    UINT64                      count,
    std::vector<std::uint64_t>& data)
{
    /* Allocate new register */
    const auto idx = static_cast<std::size_t>(id);
    if (idx >= registers_.size())
        registers_.resize(idx + 1);

    /* Write current offset */
    auto& reg = registers_[idx];
    {
        reg.offset  = sizeof(UINT64) * data.size();
        reg.size    = sizeof(UINT64) * count;
    }

    /* Write data to container */
    const auto first = data.size();
    data.resize(first + static_cast<std::size_t>(count));
    std::fill(data.begin() + first, data.end(), value);
}

void D3D12BufferConstantsPool::CreateImmutableBuffer(
    ID3D12Device*               device,
    D3D12CommandContext&        commandContext,
    D3D12CommandQueue&          commandQueue,
    D3D12StagingBufferPool&     stagingBufferPool,
    std::vector<std::uint64_t>& data)
{
    /* Create generic buffer resource */
    const UINT64 bufferSize = data.size() * sizeof(UINT64);
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        resource_.SetInitialAndUsageStates(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE),
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 buffer constants pool");

    /* Initialize buffer with registered constants */
    stagingBufferPool.WriteImmediate(commandContext, resource_, 0, data.data(), bufferSize);
    commandContext.FinishAndSync(commandQueue);
}


} // /namespace LLGL



// ================================================================================
