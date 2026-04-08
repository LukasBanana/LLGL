/*
 * DbgCommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgCommandQueue.h"
#include "DbgCommandBuffer.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include <LLGL/RenderingDebugger.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


DbgCommandQueue::DbgCommandQueue(CommandQueue& instance, FrameProfile& profile, RenderingDebugger* debugger) :
    instance  { instance },
    profile_  { profile  },
    debugger_ { debugger }
{
}

/* ----- Command Buffers ----- */

void DbgCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferDbg = LLGL_CAST(DbgCommandBuffer&, commandBuffer);

    if (LLGL_DBG_SOURCE())
        commandBufferDbg.ValidateSubmit();

    instance.Submit(commandBufferDbg.instance);

    /* Merge frame profile values into rendering profiler */
    FrameProfile profile;
    commandBufferDbg.FlushProfile(profile);

    RenderingDebugger::MergeProfiles(profile_, profile);
    profile_.commandQueueRecord.commandBufferSubmittions++;
}

/* ----- Queries ----- */

bool DbgCommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    auto& queryHeapDbg = LLGL_CAST(DbgQueryHeap&, queryHeap);

    if (LLGL_DBG_SOURCE())
        ValidateQueryResult(queryHeapDbg, firstQuery, numQueries, data, dataSize);

    return instance.QueryResult(queryHeapDbg.instance, firstQuery, numQueries, data, dataSize);
}

/* ----- Fences ----- */

void DbgCommandQueue::Submit(Fence& fence)
{
    instance.Submit(fence);
    profile_.commandQueueRecord.fenceSubmissions++;
}

bool DbgCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    return instance.WaitFence(fence, timeout);
}

void DbgCommandQueue::WaitIdle()
{
    instance.WaitIdle();
}


/*
 * ======= Private: =======
 */

void DbgCommandQueue::ValidateQueryResult(
    DbgQueryHeap&   queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    if (queryHeap.desc.renderCondition)
        LLGL_DBG_ERROR(ErrorType::UndefinedBehavior, "cannot retrieve result from query that was created as render condition");

    if (data == nullptr)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot retrieve result from query with <data> parameter begin a null pointer");

    if (numQueries == 0)
        LLGL_DBG_WARN(WarningType::ImproperArgument, "retrieving result from query has no effect: <numQueries> is zero");

    if ( dataSize != numQueries * sizeof(std::uint32_t          ) &&
         dataSize != numQueries * sizeof(std::uint64_t          ) &&
         dataSize != numQueries * sizeof(QueryPipelineStatistics) )
    {
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "mismatch between required size for query result and <dataSize> parameter");
    }

    if (firstQuery + numQueries <= queryHeap.states.size())
    {
        for_range(i, numQueries)
        {
            if (queryHeap.states[firstQuery + i] != DbgQueryHeap::State::Ready)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "result for query with index %u is not ready", i);
        }
    }
    else
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "query index range out of bounds: [%u, %u) specified, but valid range is [0, %zu)",
            firstQuery, (firstQuery + numQueries), queryHeap.states.size()
        );
    }
}


} // /namespace LLGL



// ================================================================================
