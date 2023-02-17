/*
 * DbgQueryTimerManager.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgQueryTimerManager.h"
#include "DbgCore.h"
#include <LLGL/RenderSystem.h>
#include <LLGL/CommandQueue.h>
#include <LLGL/QueryHeap.h>
#include <LLGL/Misc/ForRange.h>
#include <thread>


namespace LLGL
{


static constexpr std::uint32_t g_queryTimerHeapSize = 64;

DbgQueryTimerManager::DbgQueryTimerManager(
    RenderSystem&   renderSystemInstance,
    CommandQueue&   commandQueueInstance,
    CommandBuffer&  commandBufferInstance)
:
    renderSystem_  { renderSystemInstance  },
    commandQueue_  { commandQueueInstance  },
    commandBuffer_ { commandBufferInstance }
{
}

void DbgQueryTimerManager::Reset()
{
    records_.clear();
    currentQuery_       = 0;
    currentQueryHeap_   = 0;
}

void DbgQueryTimerManager::Start(const char* annotation)
{
    /* Store annotation only first */
    ProfileTimeRecord record;
    {
        record.annotation   = annotation;
        record.elapsedTime  = 0;
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
}

void DbgQueryTimerManager::Stop()
{
    /* Stop timer query */
    commandBuffer_.EndQuery(*queryHeaps_[currentQueryHeap_], currentQuery_);

    /* Increase query index */
    ++currentQuery_;
}

void DbgQueryTimerManager::TakeRecords(std::vector<ProfileTimeRecord>& outRecords)
{
    ResolveQueryResults();
    outRecords = std::move(records_);
}


/*
 * ======= Private: =======
 */

void DbgQueryTimerManager::ResolveQueryResults()
{
    constexpr int maxAttempts = 100;

    for_range(i, records_.size())
    {
        auto&   rec         = records_[i];
        auto    query       = static_cast<std::uint32_t>(i % g_queryTimerHeapSize);
        auto    queryHeap   = static_cast<std::uint32_t>(i / g_queryTimerHeapSize);

        for_range(i, maxAttempts)
        {
            if (!commandQueue_.QueryResult(*queryHeaps_[queryHeap], query, 1, &(rec.elapsedTime), sizeof(rec.elapsedTime)))
                std::this_thread::yield();
            else
                break;
        }
    }
}


} // /namespace LLGL



// ================================================================================
