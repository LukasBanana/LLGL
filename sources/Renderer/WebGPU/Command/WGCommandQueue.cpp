/*
 * WGCommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGCommandQueue.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


WGCommandQueue::WGCommandQueue(WGPUDevice device) :
    queue_ { wgpuDeviceGetQueue(device) }
{
    LLGL_ASSERT(queue_ != nullptr);
}

WGCommandQueue::~WGCommandQueue()
{
    wgpuQueueRelease(queue_);
}

void WGCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGCommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandQueue::Submit(Fence& fence)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandQueue::WaitIdle()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
