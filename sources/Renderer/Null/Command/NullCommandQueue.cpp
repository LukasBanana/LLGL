/*
 * NullCommandQueue.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullCommandQueue.h"
#include "NullCommandBuffer.h"
#include "NullCommandExecutor.h"
#include "../RenderState/NullQueryHeap.h"
#include "../../CheckedCast.h"


namespace LLGL
{


/* ----- Command Buffers ----- */

void NullCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferNull = LLGL_CAST(NullCommandBuffer&, commandBuffer);
    if ((commandBufferNull.desc.flags & (CommandBufferFlags::ImmediateSubmit | CommandBufferFlags::Secondary)) == 0)
        commandBufferNull.ExecuteVirtualCommands();
}

/* ----- Queries ----- */

bool NullCommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    //auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
    return false;
}

/* ----- Fences ----- */

void NullCommandQueue::Submit(Fence& fence)
{
    //todo
}

bool NullCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    return false; //todo
}

void NullCommandQueue::WaitIdle()
{
    //todo
}


} // /namespace LLGL



// ================================================================================
