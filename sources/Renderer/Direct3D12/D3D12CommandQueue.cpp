/*
 * D3D12CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "D3D12RenderSystem.h"
#include "../CheckedCast.h"
#include "../DXCommon/DXCore.h"
#include "RenderState/D3D12Fence.h"


namespace LLGL
{


D3D12CommandQueue::D3D12CommandQueue(ID3D12Device* device, ID3D12CommandQueue* queue) :
    queue_             { queue     },
    intermediateFence_ { device, 0 }
{
}

/* ----- Command Buffers ----- */

void D3D12CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    /* Execute command list */
    auto& commandBufferD3D = LLGL_CAST(D3D12CommandBuffer&, commandBuffer);
    ID3D12CommandList* cmdLists[] = { commandBufferD3D.GetNative() };
    queue_->ExecuteCommandLists(1, cmdLists);
}

/* ----- Queries ----- */

// Static function (can be checked at compile time) to determine if
// the structs <QueryPipelineStatistics> and <D3D12_QUERY_DATA_PIPELINE_STATISTICS> are compatible.
static bool IsQueryPipelineStatsD3DCompatible()
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

bool D3D12CommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    //TODO...
    return false;
}

/* ----- Fences ----- */

void D3D12CommandQueue::Submit(Fence& fence)
{
    /* Schedule signal command into the queue */
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    auto hr = queue_->Signal(fenceD3D.GetNative(), fenceD3D.NextValue());
    DXThrowIfFailed(hr, "failed to signal D3D12 fence with command queue");
}

bool D3D12CommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    return fenceD3D.Wait(timeout);
}

void D3D12CommandQueue::WaitIdle()
{
    /* Submit intermediate fence and wait for it to be signaled */
    Submit(intermediateFence_);
    WaitFence(intermediateFence_, ~0ull);
}


} // /namespace LLGL



// ================================================================================
