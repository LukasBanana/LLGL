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


namespace LLGL
{


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
    queryIndex_     = 0;
    queryHeapIndex_ = 0;
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
    if (queryIndex_ == g_queryHeapSize)
    {
        queryIndex_ = 0;
        ++queryHeapIndex_;
    }

    /* Check if new query heap must be created */
    if (queryHeapIndex_ == queryHeaps_.size())
    {
        QueryHeapDescriptor queryDesc;
        {
            queryDesc.type          = QueryType::TimeElapsed;
            queryDesc.numQueries    = g_queryHeapSize;
        }
        queryHeaps_.push_back(renderSystem_.CreateQueryHeap(queryDesc));
    }

    /* Begin timer query */
    commandBuffer_.BeginQuery(*queryHeaps_[queryHeapIndex_], queryIndex_);
}

void DbgQueryTimerManager::Stop()
{
    /* Stop timer query */
    commandBuffer_.EndQuery(*queryHeaps_[queryHeapIndex_], queryIndex_);

    /* Increase queyr index */
    ++queryIndex_;
}

void DbgQueryTimerManager::TakeRecords(std::vector<ProfileTimeRecord>& records)
{
    ResolveQueryResults();
    records = std::move(records_);
}


/*
 * ======= Private: =======
 */

void DbgQueryTimerManager::ResolveQueryResults()
{
    std::size_t queryHeapIdx = 0;
    std::uint32_t queryIdx = 0;

    for (auto& rec : records_)
    {
        if (queryIdx == g_queryHeapSize)
        {
            queryIdx = 0;
            ++queryHeapIdx;
        }

        while (!commandQueue_.QueryResult(*queryHeaps_[queryHeapIdx], queryIdx, 1, &(rec.elapsedTime), sizeof(rec.elapsedTime)))
        {
            // dummy
        }

        ++queryIdx;
    }
}


} // /namespace LLGL



// ================================================================================
