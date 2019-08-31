/*
 * DbgCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgCommandQueue.h"
#include "DbgCommandBuffer.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


DbgCommandQueue::DbgCommandQueue(CommandQueue& instance, RenderingProfiler* profiler, RenderingDebugger* debugger) :
    instance  { instance },
    profiler_ { profiler },
    debugger_ { debugger }
{
}

/* ----- Command Buffers ----- */

void DbgCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferDbg = LLGL_CAST(DbgCommandBuffer&, commandBuffer);

    instance.Submit(commandBufferDbg.instance);

    if (profiler_)
    {
        /* Merge frame profile values into rendering profiler */
        FrameProfile profile;
        commandBufferDbg.NextProfile(profile);
        profile.commandBufferSubmittions++;

        profiler_->Accumulate(profile);
    }
}

/* ----- Queries ----- */

bool DbgCommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    auto& queryHeapDbg = LLGL_CAST(DbgQueryHeap&, queryHeap);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateQueryResult(queryHeapDbg, firstQuery, numQueries, data, dataSize);
    }

    return instance.QueryResult(queryHeapDbg.instance, firstQuery, numQueries, data, dataSize);
}

/* ----- Fences ----- */

void DbgCommandQueue::Submit(Fence& fence)
{
    instance.Submit(fence);
    if (profiler_)
        profiler_->frameProfile.fenceSubmissions++;
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
        for (std::uint32_t i = 0; i < numQueries; ++i)
        {
            if (queryHeap.states[firstQuery + i] != DbgQueryHeap::State::Ready)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "result for query with index " + std::to_string(i) + " is not ready");
        }
    }
    else
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "query index range out of bounds: [" + std::to_string(firstQuery) + ".." + std::to_string(firstQuery + numQueries) + ")" +
            " specified, but valid range is [0.." + std::to_string(queryHeap.states.size()) + ")"
        );
    }
}


} // /namespace LLGL



// ================================================================================
