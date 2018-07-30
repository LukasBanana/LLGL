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

void MTCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    //todo
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
    //todo
}


} // /namespace LLGL



// ================================================================================
