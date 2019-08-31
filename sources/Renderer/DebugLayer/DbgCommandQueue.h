/*
 * DbgCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMMAND_QUEUE_H
#define LLGL_DBG_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>


namespace LLGL
{


class RenderingProfiler;
class RenderingDebugger;
class DbgQueryHeap;

class DbgCommandQueue final : public CommandQueue
{

    public:

        /* ----- Common ----- */

        DbgCommandQueue(
            CommandQueue&       instance,
            RenderingProfiler*  profiler,
            RenderingDebugger*  debugger
        );

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Queries ----- */

        bool QueryResult(
            QueryHeap&      queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        ) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

    public:

        /* ----- Debugging members ----- */

        CommandQueue& instance;

    private:

        void ValidateQueryResult(
            DbgQueryHeap&   queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        );

    private:

        RenderingProfiler* profiler_ = nullptr;
        RenderingDebugger* debugger_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
