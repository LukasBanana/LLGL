/*
 * DbgQueryTimerManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_QUERY_TIMER_MANAGER_H
#define LLGL_DBG_QUERY_TIMER_MANAGER_H


#include <LLGL/ForwardDecls.h>
#include <LLGL/RenderingProfiler.h>
#include <vector>


namespace LLGL
{


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
        void TakeRecords(std::vector<ProfileTimeRecord>& records);

    private:

        // Resolves all timer values into the output records.
        void ResolveQueryResults();

    private:

        static const std::uint32_t g_queryHeapSize = 64;

        RenderSystem&                   renderSystem_;
        CommandQueue&                   commandQueue_;
        CommandBuffer&                  commandBuffer_;

        std::vector<QueryHeap*>         queryHeaps_;
        std::uint32_t                   queryIndex_     = ~0u;
        std::size_t                     queryHeapIndex_ = 0;

        std::vector<ProfileTimeRecord>  records_;

};


} // /namespace LLGL


#endif



// ================================================================================
