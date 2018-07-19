/*
 * DbgCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMMAND_QUEUE_H
#define LLGL_DBG_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>


namespace LLGL
{


class RenderingProfiler;
class RenderingDebugger;

class DbgCommandQueue : public CommandQueue
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

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

        /* ----- Debugging members ----- */

        CommandQueue& instance;

    private:

        //RenderingProfiler* profiler_ = nullptr;
        RenderingDebugger* debugger_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
