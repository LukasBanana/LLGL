/*
 * D3D9CommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9CommandQueue.h"
#include "D3D9CommandBuffer.h"
#include "D3D9CommandExecutor.h"
#include "../RenderState/D3D9QueryHeap.h"
#include "../../CheckedCast.h"


namespace LLGL
{


/* ----- Command Buffers ----- */

void D3D9CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferD3D9 = LLGL_CAST(D3D9CommandBuffer&, commandBuffer);
    if ((commandBufferD3D9.desc.flags & (CommandBufferFlags::ImmediateSubmit | CommandBufferFlags::Secondary)) == 0)
        commandBufferD3D9.ExecuteVirtualCommands();
}

/* ----- Queries ----- */

bool D3D9CommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    //auto& queryHeapD3D9 = LLGL_CAST(D3D9QueryHeap&, queryHeap);
    //todo
    return false;
}

/* ----- Fences ----- */

void D3D9CommandQueue::Submit(Fence& fence)
{
    //todo
}

bool D3D9CommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    return false; //todo
}

void D3D9CommandQueue::WaitIdle()
{
    //todo
}


} // /namespace LLGL



// ================================================================================
