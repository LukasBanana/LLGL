/*
 * MTCommandQueue.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTCommandQueue.h"
#include "MTCommandBuffer.h"
#include "../CheckedCast.h"


namespace LLGL
{


MTCommandQueue::MTCommandQueue(id<MTLDevice> device)
{
    queue_ = [device newCommandQueue];
}

MTCommandQueue::~MTCommandQueue()
{
    [queue_ release];
}

/* ----- Command Buffers ----- */

void MTCommandQueue::Begin(CommandBuffer& commandBuffer, long /*flags*/)
{
    auto& commandBufferMT = LLGL_CAST(MTCommandBuffer&, commandBuffer);
    commandBufferMT.NextCommandBuffer(queue_);
}

void MTCommandQueue::End(CommandBuffer& commandBuffer)
{
    //todo
}

void MTCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    //todo
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
    //todo
}


} // /namespace LLGL



// ================================================================================
