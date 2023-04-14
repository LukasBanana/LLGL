/*
 * D3D12CommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12RenderSystem.h"
#include "../RenderState/D3D12Fence.h"
#include "../RenderState/D3D12QueryHeap.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12CommandQueue::D3D12CommandQueue(
    D3D12Device&            device,
    D3D12_COMMAND_LIST_TYPE type)
:
    native_     { device.CreateDXCommandQueue(type) },
    queueFence_ { device.GetNative()                }
{
    commandContext_.Create(device, *this);
    DetermineTimestampFrequency();
}

void D3D12CommandQueue::SetName(const char* name)
{
    D3D12SetObjectName(native_.Get(), name);
}

/* ----- Command Buffers ----- */

void D3D12CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    /* Execute command list */
    auto& commandBufferD3D = LLGL_CAST(D3D12CommandBuffer&, commandBuffer);
    if (!commandBufferD3D.IsImmediateCmdBuffer())
        commandBufferD3D.Execute();
}

/* ----- Queries ----- */

bool D3D12CommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);

    /* Ensure query results have been resolved */
    if (queryHeapD3D.InsideDirtyRange(firstQuery, numQueries))
    {
        queryHeapD3D.FlushDirtyRange(commandContext_.GetCommandList());
        commandContext_.Finish(true);
    }

    /* Map query result buffer to CPU local memory */
    auto mappedData = queryHeapD3D.Map(firstQuery, numQueries);
    if (mappedData == nullptr)
        return false;

    const auto queryType = queryHeapD3D.GetNativeType();
    bool result = false;

    if (dataSize == numQueries * sizeof(std::uint32_t))
    {
        /* Query 64-bit values and convert them to 32-bit values */
        QueryResultUInt32(queryType, mappedData, firstQuery, numQueries, reinterpret_cast<std::uint32_t*>(data));
        result = true;
    }
    else if (dataSize == numQueries * sizeof(std::uint64_t))
    {
        /* Query 64-bit values and copy them directly to output */
        QueryResultUInt64(queryType, mappedData, firstQuery, numQueries, reinterpret_cast<std::uint64_t*>(data));
        result = true;
    }
    else if (dataSize == numQueries * sizeof(QueryPipelineStatistics))
    {
        /* Query pipeline statistics and copy them directly to output (if structs are compatible) */
        result = QueryResultPipelineStatistics(queryType, mappedData, firstQuery, numQueries, reinterpret_cast<QueryPipelineStatistics*>(data));
    }

    queryHeapD3D.Unmap();

    return result;
}

/* ----- Fences ----- */

void D3D12CommandQueue::Submit(Fence& fence)
{
    /* Schedule signal command into the queue */
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    SignalFence(fenceD3D.GetNative(), fenceD3D.Signal());
}

bool D3D12CommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    return fenceD3D.Wait(timeout);
}

void D3D12CommandQueue::WaitIdle()
{
    /* Submit intermediate fence and wait for it to be signaled */
    if (busy_)
    {
        ++queueFenceValue_;
        SignalFence(queueFence_.Get(), queueFenceValue_);
        queueFence_.WaitForHigherSignal(queueFenceValue_);
        busy_ = false;
    }
}

/* ----- Internal ----- */

void D3D12CommandQueue::SignalFence(ID3D12Fence* fence, UINT64 value)
{
    auto hr = native_->Signal(fence, value);
    DXThrowIfFailed(hr, "failed to signal D3D12 fence with command queue");
    busy_ = true;
}

void D3D12CommandQueue::ExecuteCommandLists(UINT numCommandsLists, ID3D12CommandList* const* commandLists)
{
    native_->ExecuteCommandLists(numCommandsLists, commandLists);
    busy_ = true;
}

void D3D12CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
    native_->ExecuteCommandLists(1, &commandList);
    busy_ = true;
}


/*
 * ======= Private: =======
 */

void D3D12CommandQueue::DetermineTimestampFrequency()
{
    /* Get timestamp frequency for command queue */
    UINT64 timestampFrequency = 0;
    auto hr = native_->GetTimestampFrequency(&timestampFrequency);
    DXThrowIfInvocationFailed(hr, "ID3D12CommandQueue::GetTimestampFrequency");

    /* Determine if a conversion from timestamps to nanoseconds is necessary */
    static const UINT64 nanosecondFrequency = 1000000000;
    if (timestampFrequency != nanosecondFrequency)
    {
        isTimestampNanosecs_    = false;
        timestampScale_         = static_cast<double>(nanosecondFrequency) / static_cast<double>(timestampFrequency);
    }
}

void D3D12CommandQueue::QueryResultSingleUInt64(
    D3D12_QUERY_TYPE    queryType,
    const void*         mappedData,
    std::uint32_t       query,
    std::uint64_t&      data)
{
    auto mappedDataUInt64 = reinterpret_cast<const std::uint64_t*>(mappedData);
    if (queryType == D3D12_QUERY_TYPE_TIMESTAMP)
    {
        /* Compute difference between start and end timestamps for each output entry */
        const auto startTimestamp   = mappedDataUInt64[query*2    ];
        const auto endTimestamp     = mappedDataUInt64[query*2 + 1];
        const auto deltaTime        = (endTimestamp - startTimestamp);

        if (!isTimestampNanosecs_)
        {
            const auto elapsedTime = (static_cast<double>(deltaTime) * timestampScale_);
            data = static_cast<std::uint64_t>(elapsedTime + 0.5);
        }
        else
            data = deltaTime;
    }
    else
    {
        /* Copy mapped data to output data */
        data = mappedDataUInt64[query];
    }
}

void D3D12CommandQueue::QueryResultUInt32(
    D3D12_QUERY_TYPE    queryType,
    const void*         mappedData,
    std::uint32_t       firstQuery,
    std::uint32_t       numQueries,
    std::uint32_t*      data)
{
    for (std::uint32_t i = 0; i < numQueries; ++i)
    {
        std::uint64_t tempData = 0;
        QueryResultSingleUInt64(queryType, mappedData, firstQuery + i, tempData);
        data[i] = static_cast<std::uint32_t>(tempData);
    }
}

void D3D12CommandQueue::QueryResultUInt64(
    D3D12_QUERY_TYPE    queryType,
    const void*         mappedData,
    std::uint32_t       firstQuery,
    std::uint32_t       numQueries,
    std::uint64_t*      data)
{
    if (queryType == D3D12_QUERY_TYPE_TIMESTAMP)
    {
        /* Copy individual values of mapped data to output data */
        for (std::uint32_t i = 0; i < numQueries; ++i)
            QueryResultSingleUInt64(queryType, mappedData, firstQuery + i, data[i]);
    }
    else
    {
        /* Copy mapped data to output data */
        auto mappedDataUInt64 = reinterpret_cast<const std::uint64_t*>(mappedData);
        ::memcpy(data, &(mappedDataUInt64[firstQuery]), numQueries * sizeof(std::uint64_t));
    }
}

// Static function (can be checked at compile time) to determine if
// the structs <QueryPipelineStatistics> and <D3D12_QUERY_DATA_PIPELINE_STATISTICS> are compatible.
static constexpr bool IsQueryPipelineStatsD3DCompatible()
{
    return
    (
        sizeof(QueryPipelineStatistics)                                    == sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)                  &&
        offsetof(QueryPipelineStatistics, inputAssemblyVertices          ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, IAVertices   ) &&
        offsetof(QueryPipelineStatistics, inputAssemblyPrimitives        ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, IAPrimitives ) &&
        offsetof(QueryPipelineStatistics, vertexShaderInvocations        ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, VSInvocations) &&
        offsetof(QueryPipelineStatistics, geometryShaderInvocations      ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, GSInvocations) &&
        offsetof(QueryPipelineStatistics, geometryShaderPrimitives       ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, GSPrimitives ) &&
        offsetof(QueryPipelineStatistics, clippingInvocations            ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, CInvocations ) &&
        offsetof(QueryPipelineStatistics, clippingPrimitives             ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, CPrimitives  ) &&
        offsetof(QueryPipelineStatistics, fragmentShaderInvocations      ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, PSInvocations) &&
        offsetof(QueryPipelineStatistics, tessControlShaderInvocations   ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, HSInvocations) &&
        offsetof(QueryPipelineStatistics, tessEvaluationShaderInvocations) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, DSInvocations) &&
        offsetof(QueryPipelineStatistics, computeShaderInvocations       ) == offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, CSInvocations)
    );
}

bool D3D12CommandQueue::QueryResultPipelineStatistics(
    D3D12_QUERY_TYPE            queryType,
    const void*                 mappedData,
    std::uint32_t               firstQuery,
    std::uint32_t               numQueries,
    QueryPipelineStatistics*    data)
{
    /* Query result from data of type: D3D11_QUERY_DATA_PIPELINE_STATISTICS */
    if (queryType != D3D12_QUERY_TYPE_PIPELINE_STATISTICS)
        return false;

    if (IsQueryPipelineStatsD3DCompatible())
    {
        /* Use output storage directly when structure is compatible with D3D */
        auto mappedDataStats = reinterpret_cast<const D3D12_QUERY_DATA_PIPELINE_STATISTICS*>(mappedData);
        ::memcpy(data, &(mappedDataStats[firstQuery]), numQueries * sizeof(QueryPipelineStatistics));
    }
    else
    {
        for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; ++query)
        {
            /* Copy current query data to output */
            const auto mappedDataStats = reinterpret_cast<const D3D12_QUERY_DATA_PIPELINE_STATISTICS*>(mappedData) + query;
            data->inputAssemblyVertices             = mappedDataStats->IAVertices;
            data->inputAssemblyPrimitives           = mappedDataStats->IAPrimitives;
            data->vertexShaderInvocations           = mappedDataStats->VSInvocations;
            data->geometryShaderInvocations         = mappedDataStats->GSInvocations;
            data->geometryShaderPrimitives          = mappedDataStats->GSPrimitives;
            data->clippingInvocations               = mappedDataStats->CInvocations;
            data->clippingPrimitives                = mappedDataStats->CPrimitives;
            data->fragmentShaderInvocations         = mappedDataStats->PSInvocations;
            data->tessControlShaderInvocations      = mappedDataStats->HSInvocations;
            data->tessEvaluationShaderInvocations   = mappedDataStats->DSInvocations;
            data->computeShaderInvocations          = mappedDataStats->CSInvocations;
        }
    }

    return true;
}


} // /namespace LLGL



// ================================================================================
