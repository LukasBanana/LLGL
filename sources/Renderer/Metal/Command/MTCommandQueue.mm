/*
 * MTCommandQueue.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTCommandQueue.h"
#include "MTDirectCommandBuffer.h"
#include "MTMultiSubmitCommandBuffer.h"
#include "MTCommandExecutor.h"
#include "../../CheckedCast.h"


namespace LLGL
{


MTCommandQueue::MTCommandQueue(id<MTLDevice> device) :
    context_ { device }
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
    if (commandBufferMT.IsMultiSubmitCmdBuffer())
    {
        auto& multiSubmitCommandBufferMT = LLGL_CAST(MTMultiSubmitCommandBuffer&, commandBufferMT);
        context_.Reset([native_ commandBuffer]);
        ExecuteMTMultiSubmitCommandBuffer(multiSubmitCommandBufferMT, context_);
        SubmitCommandBuffer(context_.GetCommandBuffer());
    }
    else
    {
        auto& directCommandBufferMT = LLGL_CAST(MTDirectCommandBuffer&, commandBufferMT);
        if (!directCommandBufferMT.IsImmediateCmdBuffer())
        {
            directCommandBufferMT.MarkSubmitted();
            SubmitCommandBuffer(directCommandBufferMT.GetNative());
        }
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


/*
 * Internal
 */

void MTCommandQueue::SubmitCommandBuffer(id<MTLCommandBuffer> cmdBuffer)
{
    /* Commit command buffer into queue */
    [cmdBuffer commit];

    /* Hold reference to last submitted command buffer */
    if (lastSubmittedCmdBuffer_ != cmdBuffer)
    {
        if (lastSubmittedCmdBuffer_ != nil)
            [lastSubmittedCmdBuffer_ release];
        lastSubmittedCmdBuffer_ = [cmdBuffer retain];
    }
}


} // /namespace LLGL



// ================================================================================
