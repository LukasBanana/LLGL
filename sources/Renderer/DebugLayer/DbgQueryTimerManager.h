/*
 * DbgQueryTimerManager.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_QUERY_TIMER_MANAGER_H
#define LLGL_DBG_QUERY_TIMER_MANAGER_H


#include <LLGL/ForwardDecls.h>
#include <LLGL/RenderingProfiler.h>
#include <vector>


namespace LLGL
{


//TODO: rename to DbgQueryTimerPool
class DbgQueryTimerManager
{

    public:

        DbgQueryTimerManager(
            RenderSystem&   renderSystemInstance,
            CommandQueue&   commandQueueInstance,
            CommandBuffer&  commandBufferInstance
        );

        // Resets all records in this timer manager.
        void Reset();

        // Starts measuring the time with the specified annotation.
        void Start(const char* annotation);

        // Stops measing the time and stores the current record.
        void Stop();

        // Moves the internal records to the specified output container.
        void TakeRecords(std::vector<ProfileTimeRecord>& outRecords);

    private:

        // Resolves all timer values into the output records.
        void ResolveQueryResults();

    private:

        RenderSystem&                   renderSystem_;
        CommandQueue&                   commandQueue_;
        CommandBuffer&                  commandBuffer_;

        std::vector<QueryHeap*>         queryHeaps_;
        std::uint32_t                   currentQuery_       = 0;
        std::uint32_t                   currentQueryHeap_   = 0;

        std::vector<ProfileTimeRecord>  records_;

};


} // /namespace LLGL


#endif



// ================================================================================
