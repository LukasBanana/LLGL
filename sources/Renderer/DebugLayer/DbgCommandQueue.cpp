/*
 * DbgCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
    //profiler_ { profiler },
    debugger_ { debugger }
{
}

/* ----- Command Buffers ----- */

void DbgCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferDbg = LLGL_CAST(DbgCommandBuffer&, commandBuffer);
    instance.Submit(commandBufferDbg.instance);
}

/* ----- Fences ----- */

void DbgCommandQueue::Submit(Fence& fence)
{
    instance.Submit(fence);
}

bool DbgCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    return instance.WaitFence(fence, timeout);
}

void DbgCommandQueue::WaitIdle()
{
    instance.WaitIdle();
}


} // /namespace LLGL



// ================================================================================
