/*
 * D3D12CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "D3D12RenderSystem.h"
#include "../CheckedCast.h"
#include "../DXCommon/DXCore.h"
#include "RenderState/D3D12Fence.h"


namespace LLGL
{


D3D12CommandQueue::D3D12CommandQueue(D3D12RenderSystem& renderSystem)
{
    cmdQueue_ = renderSystem.CreateDXCommandQueue();
    for (auto& cmdAllocator : cmdAllocators_)
        cmdAllocator = renderSystem.CreateDXCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
}

/* ----- Command Buffers ----- */

void D3D12CommandQueue::Begin(CommandBuffer& /*commandBuffer*/, long /*flags*/)
{
    NextCmdAllocator();
}

void D3D12CommandQueue::End(CommandBuffer& commandBuffer)
{
    /* Get native command list */
    auto& commandBufferD3D = LLGL_CAST(D3D12CommandBuffer&, commandBuffer);
    commandBufferD3D.CloseCommandList();

    /* Submit command list if this is not a pre-recorded command buffer */
    //if (TODO)
    {
        Submit(commandBuffer);
    }
}

void D3D12CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    /* Get native command list */
    auto& commandBufferD3D = LLGL_CAST(D3D12CommandBuffer&, commandBuffer);
    auto commandList = commandBufferD3D.GetNative();

    /* Execute command list */
    ID3D12CommandList* cmdLists[] = { commandList };
    cmdQueue_->ExecuteCommandLists(1, cmdLists);

    /* Reset command list */
    ResetCommandList(commandList);
}

/* ----- Fences ----- */

void D3D12CommandQueue::Submit(Fence& fence)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    fenceD3D.Submit(cmdQueue_.Get());
}

bool D3D12CommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence);
    return fenceD3D.Wait(timeout);
}

void D3D12CommandQueue::WaitIdle()
{
    //TODO
    //renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}


/*
 * ======= Private: =======
 */

void D3D12CommandQueue::NextCmdAllocator()
{
    currentCmdAllocator_ = ((currentCmdAllocator_ + 1) % D3D12CommandQueue::g_numCmdAllocators);
}

void D3D12CommandQueue::ResetCommandList(ID3D12GraphicsCommandList* commandList)
{
    auto cmdAllocator = GetCmdAllocator();

    auto hr = cmdAllocator->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    hr = commandList->Reset(cmdAllocator, nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");
}


} // /namespace LLGL



// ================================================================================
