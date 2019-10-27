/*
 * D3D11CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11CommandQueue.h"
#include "D3D11CommandBuffer.h"
#include "RenderState/D3D11Fence.h"
#include "RenderState/D3D11QueryHeap.h"
#include "../CheckedCast.h"


namespace LLGL
{


D3D11CommandQueue::D3D11CommandQueue(ID3D11Device* device, ComPtr<ID3D11DeviceContext>& context) :
    context_           { context },
    intermediateFence_ { device  }
{
}

/* ----- Command Buffers ----- */

void D3D11CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D11CommandBuffer&, commandBuffer);
    if (!cmdBufferD3D.IsSecondaryCmdBuffer())
    {
        if (auto commandList = cmdBufferD3D.GetDeferredCommandList())
        {
            /* Execute encoded command list with immediate context but don't restore previous state */
            context_->ExecuteCommandList(commandList, FALSE);
        }
    }
}

/* ----- Queries ----- */

bool D3D11CommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);
    if (dataSize == numQueries * sizeof(std::uint32_t))
        return QueryResultUInt32(queryHeapD3D, firstQuery, numQueries, reinterpret_cast<std::uint32_t*>(data));
    if (dataSize == numQueries * sizeof(std::uint64_t))
        return QueryResultUInt64(queryHeapD3D, firstQuery, numQueries, reinterpret_cast<std::uint64_t*>(data));
    if (dataSize == numQueries * sizeof(QueryPipelineStatistics))
        return QueryResultPipelineStatistics(queryHeapD3D, firstQuery, numQueries, reinterpret_cast<QueryPipelineStatistics*>(data));
    return false;
}

/* ----- Fences ----- */

void D3D11CommandQueue::Submit(Fence& fence)
{
    auto& fenceD3D = LLGL_CAST(D3D11Fence&, fence);
    fenceD3D.Submit(context_.Get());
}

bool D3D11CommandQueue::WaitFence(Fence& fence, std::uint64_t /*timeout*/)
{
    auto& fenceD3D = LLGL_CAST(D3D11Fence&, fence);
    fenceD3D.Wait(context_.Get());
    return true;
}

void D3D11CommandQueue::WaitIdle()
{
    /* Submit intermediate fence and wait for it to be signaled */
    Submit(intermediateFence_);
    WaitFence(intermediateFence_, ~0ull);
}


/*
 * ======= Private: =======
 */

bool D3D11CommandQueue::QueryResultSingleUInt64(
    D3D11QueryHeap& queryHeapD3D,
    std::uint32_t   query,
    std::uint64_t&  data)
{
    switch (queryHeapD3D.GetNativeType())
    {
        /* Query result from data of type: UINT64 */
        case D3D11_QUERY_OCCLUSION:
        {
            UINT64 tempData = 0;
            if (context_->GetData(queryHeapD3D.GetNative(query), &tempData, sizeof(tempData), 0) == S_OK)
            {
                data = tempData;
                return true;
            }
        }
        break;

        /* Query result from special case query type: TimeElapsed */
        case D3D11_QUERY_TIMESTAMP_DISJOINT:
        {
            query *= queryHeapD3D.GetGroupSize();

            UINT64 startTime = 0;
            if (context_->GetData(queryHeapD3D.GetNative(query + 1), &startTime, sizeof(startTime), 0) == S_OK)
            {
                UINT64 endTime = 0;
                if (context_->GetData(queryHeapD3D.GetNative(query + 2), &endTime, sizeof(endTime), 0) == S_OK)
                {
                    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
                    if (context_->GetData(queryHeapD3D.GetNative(query), &disjointData, sizeof(disjointData), 0) == S_OK)
                    {
                        if (disjointData.Disjoint == FALSE)
                        {
                            /* Normalize elapsed time to nanoseconds */
                            static const UINT64 nanosecondFrequency = 1000000000;

                            const auto deltaTime = (endTime - startTime);
                            if (disjointData.Frequency != nanosecondFrequency)
                            {
                                const auto scale        = (static_cast<double>(nanosecondFrequency) / static_cast<double>(disjointData.Frequency));
                                const auto elapsedTime  = (static_cast<double>(deltaTime) * scale);
                                data = static_cast<std::uint64_t>(elapsedTime + 0.5);
                            }
                            else
                                data = deltaTime;
                        }
                        else
                            data = 0;
                        return true;
                    }
                }
            }
        }
        break;

        /* Query result from data of type: BOOL */
        case D3D11_QUERY_OCCLUSION_PREDICATE:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
        {
            BOOL tempData = FALSE;
            if (context_->GetData(queryHeapD3D.GetPredicate(query), &tempData, sizeof(tempData), 0) == S_OK)
            {
                data = tempData;
                return true;
            }
        }
        break;

        /* Query result from data of type: D3D11_QUERY_DATA_SO_STATISTICS */
        case D3D11_QUERY_SO_STATISTICS:
        {
            D3D11_QUERY_DATA_SO_STATISTICS tempData;
            if (context_->GetData(queryHeapD3D.GetNative(query), &tempData, sizeof(tempData), 0) == S_OK)
            {
                data = tempData.NumPrimitivesWritten;
                return true;
            }
        }
        break;

        default:
        break;
    }

    return false;
}

bool D3D11CommandQueue::QueryResultUInt32(
    D3D11QueryHeap& queryHeapD3D,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    std::uint32_t*  data)
{
    for (std::uint32_t i = 0; i < numQueries; ++i)
    {
        std::uint64_t tempData = 0;
        if (QueryResultSingleUInt64(queryHeapD3D, firstQuery + i, tempData))
            data[i] = static_cast<std::uint32_t>(tempData);
        else
            return false;
    }
    return true;
}

bool D3D11CommandQueue::QueryResultUInt64(
    D3D11QueryHeap& queryHeapD3D,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    std::uint64_t*  data)
{
    for (std::uint32_t i = 0; i < numQueries; ++i)
    {
        if (!QueryResultSingleUInt64(queryHeapD3D, firstQuery + i, data[i]))
            return false;
    }
    return true;
}

// Static function (can be checked at compile time) to determine if
// the structs <QueryPipelineStatistics> and <D3D11_QUERY_DATA_PIPELINE_STATISTICS> are compatible.
static constexpr bool IsQueryPipelineStatsD3DCompatible()
{
    return
    (
        sizeof(QueryPipelineStatistics)                                    == sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS)                  &&
        offsetof(QueryPipelineStatistics, inputAssemblyVertices          ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, IAVertices   ) &&
        offsetof(QueryPipelineStatistics, inputAssemblyPrimitives        ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, IAPrimitives ) &&
        offsetof(QueryPipelineStatistics, vertexShaderInvocations        ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, VSInvocations) &&
        offsetof(QueryPipelineStatistics, geometryShaderInvocations      ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, GSInvocations) &&
        offsetof(QueryPipelineStatistics, geometryShaderPrimitives       ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, GSPrimitives ) &&
        offsetof(QueryPipelineStatistics, clippingInvocations            ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, CInvocations ) &&
        offsetof(QueryPipelineStatistics, clippingPrimitives             ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, CPrimitives  ) &&
        offsetof(QueryPipelineStatistics, fragmentShaderInvocations      ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, PSInvocations) &&
        offsetof(QueryPipelineStatistics, tessControlShaderInvocations   ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, HSInvocations) &&
        offsetof(QueryPipelineStatistics, tessEvaluationShaderInvocations) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, DSInvocations) &&
        offsetof(QueryPipelineStatistics, computeShaderInvocations       ) == offsetof(D3D11_QUERY_DATA_PIPELINE_STATISTICS, CSInvocations)
    );
}

//TODO: use <query> index
bool D3D11CommandQueue::QueryResultPipelineStatistics(
    D3D11QueryHeap&             queryHeapD3D,
    std::uint32_t               firstQuery,
    std::uint32_t               numQueries,
    QueryPipelineStatistics*    data)
{
    /* Query result from data of type: D3D11_QUERY_DATA_PIPELINE_STATISTICS */
    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_PIPELINE_STATISTICS)
    {
        for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; ++query)
        {
            if (IsQueryPipelineStatsD3DCompatible())
            {
                /* Use output storage directly when structure is compatible with D3D */
                return (context_->GetData(queryHeapD3D.GetNative(query), &data[query], sizeof(QueryPipelineStatistics), 0) == S_OK);
            }
            else
            {
                /* Copy temporary query data to output */
                D3D11_QUERY_DATA_PIPELINE_STATISTICS tempData;
                if (context_->GetData(queryHeapD3D.GetNative(query), &tempData, sizeof(tempData), 0) == S_OK)
                {
                    data->inputAssemblyVertices             = tempData.IAVertices;
                    data->inputAssemblyPrimitives           = tempData.IAPrimitives;
                    data->vertexShaderInvocations           = tempData.VSInvocations;
                    data->geometryShaderInvocations         = tempData.GSInvocations;
                    data->geometryShaderPrimitives          = tempData.GSPrimitives;
                    data->clippingInvocations               = tempData.CInvocations;
                    data->clippingPrimitives                = tempData.CPrimitives;
                    data->fragmentShaderInvocations         = tempData.PSInvocations;
                    data->tessControlShaderInvocations      = tempData.HSInvocations;
                    data->tessEvaluationShaderInvocations   = tempData.DSInvocations;
                    data->computeShaderInvocations          = tempData.CSInvocations;
                }
                else
                    return false;
            }
        }
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
