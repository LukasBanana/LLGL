/*
 * VKCommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCommandQueue.h"
#include "VKCommandBuffer.h"
#include "../RenderState/VKFence.h"
#include "../RenderState/VKQueryHeap.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"


namespace LLGL
{


VKCommandQueue::VKCommandQueue(VkDevice device, const VKSharedCommandQueueSPtr& sharedCmdQueue) :
    device_         { device         },
    sharedCmdQueue_ { sharedCmdQueue }
{
}

/* ----- Command Buffers ----- */

void VKCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferVK = LLGL_CAST(VKCommandBuffer&, commandBuffer);
    if (!commandBufferVK.IsImmediateCmdBuffer())
    {
        VkResult result = commandBufferVK.SubmitToQueue(*sharedCmdQueue_);
        VKThrowIfFailed(result, "failed to submit command buffer to Vulkan graphics queue");
    }
}

/* ----- Queries ----- */

bool VKCommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    auto& queryHeapVK = LLGL_CAST(VKQueryHeap&, queryHeap);

    /*
    We currenlty simply flush the whole queue for query readbacks,
    because resetting query pools are encoded in a concurrent VkCommandBuffer.
    A reset from a previous native command encoding could have become stale,
    which lets QueryResult() return too early before the updated query results are ready.
    This function needs at least to wait for the VkFence of the VkCommandBuffer that encoded the reset of the specified input query.
    This could potentially point to a multitude of command buffers that issued queries, which would require complex book keeping.
    Since GPU readbacks from queries are expected to be slow anyway, we simply flush the entire queue.
    */
    sharedCmdQueue_->WaitIdle();

    /* Store result directly into output parameter */
    VkResult stateResult = GetQueryResults(queryHeapVK, firstQuery, numQueries, data, dataSize);
    if (stateResult == VK_NOT_READY)
        return false;

    VKThrowIfFailed(stateResult, "failed to retrieve results from Vulkan query pool");

    return true;
}

#if 0
bool VKCommandBuffer::QueryPipelineStatisticsResult(QueryHeap& queryHeap, QueryPipelineStatistics& result)
{

    /* Copy result to output parameter */
    result.inputAssemblyVertices            = intermediateResults[ 0]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
    result.inputAssemblyPrimitives          = intermediateResults[ 1]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
    result.vertexShaderInvocations          = intermediateResults[ 2]; // VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
    result.tessControlShaderInvocations     = intermediateResults[ 8]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
    result.tessEvaluationShaderInvocations  = intermediateResults[ 9]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
    result.geometryShaderInvocations        = intermediateResults[ 3]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
    result.fragmentShaderInvocations        = intermediateResults[ 7]; // VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
    result.computeShaderInvocations         = intermediateResults[10]; // VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
    result.geometryShaderPrimitives         = intermediateResults[ 4]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
    result.clippingInvocations              = intermediateResults[ 5]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
    result.clippingPrimitives               = intermediateResults[ 6]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT

    return true;
}
#endif

/* ----- Fences ----- */

void VKCommandQueue::Submit(Fence& fence)
{
    sharedCmdQueue_->isIdle = false;
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    fenceVK.Reset(device_);
    vkQueueSubmit(sharedCmdQueue_->native, 0, nullptr, fenceVK.GetVkFence());
}

bool VKCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    return fenceVK.Wait(device_, timeout);
}

void VKCommandQueue::WaitIdle()
{
    sharedCmdQueue_->WaitIdle();
}


/*
 * ======= Private: =======
 */

VkResult VKCommandQueue::GetQueryResults(
    VKQueryHeap&    queryHeapVK,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    /* Determine flags and stride */
    VkQueryResultFlags  flags   = 0;
    VkDeviceSize        stride  = 0;

    if (dataSize == numQueries * sizeof(std::uint64_t) ||
        dataSize == numQueries * sizeof(QueryPipelineStatistics))
    {
        flags   = VK_QUERY_RESULT_64_BIT;
        stride  = sizeof(std::uint64_t);
    }
    else if (dataSize == numQueries * sizeof(std::uint32_t))
    {
        flags   = 0;
        stride  = sizeof(std::uint32_t);
    }
    else
        return VK_ERROR_VALIDATION_FAILED_EXT;

    /* NOTE: vkGetQueryPoolResults() seems to disregard 32-bit requests and corrupts memory, so we always query with VK_QUERY_RESULT_64_BIT */
    const QueryType queryType = queryHeapVK.GetType();
    if (queryHeapVK.IsAliased())
    {
        /* Query results individually and potentially from aliased queries */
        auto dataByteAligned = static_cast<std::uint8_t*>(data);

        for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; ++query)
        {
            const VKQueryAlias* queryAlias = queryHeapVK.GetAlias(query);
            VkResult stateResult = (queryAlias != nullptr)
                ? GetQuerySingleResult(queryType, *(queryAlias->queryHeap), queryAlias->queryIndex, dataByteAligned, stride, flags)
                : GetQuerySingleResult(queryType, queryHeapVK, query, dataByteAligned, stride, flags);
            if (stateResult != VK_SUCCESS)
                return stateResult;
            dataByteAligned += stride;
        }

        return VK_SUCCESS;
    }
    else
    {
        if (stride != sizeof(std::uint64_t)                     ||
            queryType == QueryType::TimeElapsed                 ||
            queryType == QueryType::StreamOutPrimitivesWritten  ||
            queryType == QueryType::StreamOutOverflow)
        {
            /* Query results individually */
            auto dataByteAligned = static_cast<std::uint8_t*>(data);

            for (std::uint32_t query = firstQuery; query < firstQuery + numQueries; ++query)
            {
                VkResult stateResult = GetQuerySingleResult(queryType, queryHeapVK, query, dataByteAligned, stride, flags);
                if (stateResult != VK_SUCCESS)
                    return stateResult;
                dataByteAligned += stride;
            }

            return VK_SUCCESS;
        }
        else
        {
            /* Get query data directly as batch */
            return GetQueryBatchedResults(queryHeapVK, firstQuery, numQueries, data, dataSize, stride, flags);
        }
    }
}

VkResult VKCommandQueue::GetQueryBatchedResults(
    VKQueryHeap&        queryHeapVK,
    std::uint32_t       firstQuery,
    std::uint32_t       numQueries,
    void*               data,
    std::size_t         dataSize,
    VkDeviceSize        stride,
    VkQueryResultFlags  flags)
{
    /* Use output buffer directly to store query result */
    return vkGetQueryPoolResults(
        device_,
        queryHeapVK.GetVkQueryPool(),
        firstQuery * queryHeapVK.GetGroupSize(),
        numQueries * queryHeapVK.GetGroupSize(),
        dataSize,
        data,
        stride,
        flags
    );
}

VkResult VKCommandQueue::GetQuerySingleResult(
    QueryType           parentQueryType,
    VKQueryHeap&        queryHeapVK,
    std::uint32_t       query,
    void*               data,
    VkDeviceSize        stride,
    VkQueryResultFlags  flags)
{
    VkResult result = VK_NOT_READY;

    query *= queryHeapVK.GetGroupSize();

    auto WriteUint64Output = [data, stride](std::uint64_t value) -> void
        {
            if (stride == sizeof(std::uint64_t))
            {
                auto dst = static_cast<std::uint64_t*>(data);
                dst[0] = value;
            }
            else
            {
                auto dst = static_cast<std::uint32_t*>(data);
                dst[0] = static_cast<std::uint32_t>(value);
            }
        };

    switch (parentQueryType)
    {
        case QueryType::TimeElapsed:
        {
            /* Query start and end timestamps */
            std::uint64_t timestamps[2];
            result = vkGetQueryPoolResults(
                device_,
                queryHeapVK.GetVkQueryPool(),
                query,
                queryHeapVK.GetGroupSize(),
                sizeof(timestamps),
                timestamps,
                sizeof(timestamps[0]),
                VK_QUERY_RESULT_64_BIT
            );

            if (result == VK_SUCCESS)
            {
                /* Store difference between timestamps in output buffer */
                const std::uint64_t elapsedTime = (timestamps[1] - timestamps[0]);
                WriteUint64Output(elapsedTime);
            }
        }
        break;

        case QueryType::StreamOutPrimitivesWritten:
        case QueryType::StreamOutOverflow:
        {
            struct XfbStreamResult
            {
                std::uint64_t numPrimitivesWritten;
                std::uint64_t numPrimitivesGenerated;
            };
            XfbStreamResult xfbStreamResult;

            result = vkGetQueryPoolResults(
                device_,
                queryHeapVK.GetVkQueryPool(),
                query,
                queryHeapVK.GetGroupSize(),
                sizeof(xfbStreamResult),
                &xfbStreamResult,
                sizeof(xfbStreamResult),
                VK_QUERY_RESULT_64_BIT
            );

            if (result == VK_SUCCESS)
            {
                if (parentQueryType == QueryType::StreamOutOverflow)
                    WriteUint64Output(xfbStreamResult.numPrimitivesWritten != xfbStreamResult.numPrimitivesGenerated ? 1 : 0);
                else
                    WriteUint64Output(xfbStreamResult.numPrimitivesWritten);
            }
        }
        break;

        default:
        {
            /* NOTE: vkGetQueryPoolResults() seems to disregard 32-bit requests and corrupts memory, so we always query with 64-bit values */
            std::uint64_t intermediateResult = 0;
            result = vkGetQueryPoolResults(
                device_,
                queryHeapVK.GetVkQueryPool(),
                query,
                1,
                sizeof(intermediateResult),
                &intermediateResult,
                0,
                flags
            );
            WriteUint64Output(intermediateResult);
        }
    }

    return result;
}


/*
 * VKSharedCommandQueue structure
 */

VkResult VKSharedCommandQueue::WaitIdle()
{
    /* Cache idle state to avoid subsequent calls when used to readback query results */
    VkResult result = VK_SUCCESS;
    if (!isIdle)
    {
        result = vkQueueWaitIdle(native);
        if (result== VK_SUCCESS)
            isIdle = true;
    }
    return result;
}

VkResult VKSharedCommandQueue::Submit(const VkSubmitInfo& submitInfo, VkFence fence)
{
    isIdle = false;
    return vkQueueSubmit(native, 1, &submitInfo, fence);
}


} // /namespace LLGL



// ================================================================================
