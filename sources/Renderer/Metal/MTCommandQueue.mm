/*
 * MTCommandQueue.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTCommandQueue.h"
#include "MTCommandBuffer.h"
#include "../CheckedCast.h"


namespace LLGL
{


MTCommandQueue::MTCommandQueue(id<MTLDevice> device)
{
    native_ = [device newCommandQueue];
}

MTCommandQueue::~MTCommandQueue()
{
    if (lastSubmittedCmdBuffer_ != nil)
        [lastSubmittedCmdBuffer_ release];
    [native_ release];
}

/* ----- Command Buffers ----- */

void MTCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferMT = LLGL_CAST(MTCommandBuffer&, commandBuffer);
    if (!commandBufferMT.IsImmediateCmdBuffer())
    {
        /* Commit command buffer into queue */
        id<MTLCommandBuffer> cmdBuffer = commandBufferMT.GetNative();
        [cmdBuffer commit];

        /* Hold reference to last submitted command buffer */
        /*if (lastSubmittedCmdBuffer_ != cmdBuffer)
        {
            if (lastSubmittedCmdBuffer_ != nil)
                [lastSubmittedCmdBuffer_ release];
            lastSubmittedCmdBuffer_ = [cmdBuffer retain];
        }*/
    }
}

/* ----- Queries ----- */

bool MTCommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    return false; //todo
}

/* ----- Fences ----- */

void MTCommandQueue::Submit(Fence& fence)
{
    //todo
}

bool MTCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    return false;//todo
}

void MTCommandQueue::WaitIdle()
{
    if (lastSubmittedCmdBuffer_ != nil)
    {
        [lastSubmittedCmdBuffer_ waitUntilCompleted];
        [lastSubmittedCmdBuffer_ release];
        lastSubmittedCmdBuffer_ = nil;
    }
}


} // /namespace LLGL



// ================================================================================
