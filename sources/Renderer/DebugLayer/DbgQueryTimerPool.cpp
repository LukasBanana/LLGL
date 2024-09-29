/*
 * DbgQueryTimerPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgQueryTimerPool.h"
#include "DbgCore.h"
#include <LLGL/RenderSystem.h>
#include <LLGL/CommandQueue.h>
#include <LLGL/QueryHeap.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Timer.h>
#include <thread>


namespace LLGL
{


static constexpr std::uint32_t g_queryTimerHeapSize = 64;

struct DbgQueryTimerIndices
{
    std::uint32_t heapIndex;
    std::uint32_t queryIndex;
};

static DbgQueryTimerIndices GetQueryForRecord(std::size_t recordIndex)
{
    return DbgQueryTimerIndices
    {
        static_cast<std::uint32_t>(recordIndex / g_queryTimerHeapSize),
        static_cast<std::uint32_t>(recordIndex % g_queryTimerHeapSize)
    };
}

DbgQueryTimerPool::DbgQueryTimerPool(
    RenderSystem&   renderSystemInstance,
    CommandQueue&   commandQueueInstance,
    CommandBuffer&  commandBufferInstance)
:
    renderSystem_  { renderSystemInstance  },
    commandQueue_  { commandQueueInstance  },
    commandBuffer_ { commandBufferInstance }
{
}

void DbgQueryTimerPool::Reset()
{
    LLGL_ASSERT(pendingRecordStack_.empty(), "unbalanced calls to Start()/Stop() in query timer pool");
    records_.clear();
    currentQuery_       = 0;
    currentQueryHeap_   = 0;
    cpuTicksBase_       = Timer::Tick();
}

void DbgQueryTimerPool::Start(const char* annotation)
{
    pendingRecordStack_.push(records_.size());

    /* Store annotation only first */
    ProfileTimeRecord record;
    {
        record.annotation       = annotation;
        record.cpuTicksStart    = Timer::Tick() - cpuTicksBase_;
    }
    records_.push_back(record);

    /* Check if end of query heap has been reached */
    if (currentQuery_ == g_queryTimerHeapSize)
    {
        currentQuery_ = 0;
        ++currentQueryHeap_;
    }

    /* Check if new query heap must be created */
    if (currentQueryHeap_ == queryHeaps_.size())
    {
        QueryHeapDescriptor queryDesc;
        {
            queryDesc.type          = QueryType::TimeElapsed;
            queryDesc.numQueries    = g_queryTimerHeapSize;
        }
        queryHeaps_.push_back(renderSystem_.CreateQueryHeap(queryDesc));
    }

    /* Begin timer query */
    commandBuffer_.BeginQuery(*queryHeaps_[currentQueryHeap_], currentQuery_);
    ++currentQuery_;
}

void DbgQueryTimerPool::Stop()
{
    /* Get index to the current pending record */
    const std::size_t recordIndex = pendingRecordStack_.top();
    pendingRecordStack_.pop();
    ProfileTimeRecord& rec = records_[recordIndex];

    /* Record CPU ticks at end */
    rec.cpuTicksEnd = Timer::Tick() - cpuTicksBase_;

    /* Stop timer query */
    const DbgQueryTimerIndices indices = GetQueryForRecord(recordIndex);
    commandBuffer_.EndQuery(*queryHeaps_[indices.heapIndex], indices.queryIndex);
}

void DbgQueryTimerPool::TakeRecords(DynamicVector<ProfileTimeRecord>& outRecords)
{
    ResolveQueryResults();
    outRecords = std::move(records_);
}


/*
 * ======= Private: =======
 */

void DbgQueryTimerPool::ResolveQueryResults()
{
    constexpr int maxAttempts = 100;

    for_range(i, records_.size())
    {
        ProfileTimeRecord& rec = records_[i];
        const DbgQueryTimerIndices indices = GetQueryForRecord(i);

        for_range(i, maxAttempts)
        {
            if (!commandQueue_.QueryResult(*queryHeaps_[indices.heapIndex], indices.queryIndex, 1, &(rec.elapsedTime), sizeof(rec.elapsedTime)))
                std::this_thread::yield();
            else
                break;
        }
    }
}


} // /namespace LLGL



// ================================================================================
