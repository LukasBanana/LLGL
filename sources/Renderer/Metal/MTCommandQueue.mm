/*
 * MTCommandQueue.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
