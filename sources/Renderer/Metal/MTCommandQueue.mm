/*
 * MTCommandQueue.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTCommandQueue.h"
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

/* ----- Command queues ----- */

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
