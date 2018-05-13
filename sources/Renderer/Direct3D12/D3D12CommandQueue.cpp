/*
 * D3D12CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "../CheckedCast.h"
#include "../DXCommon/DXCore.h"
#include "RenderState/D3D12Fence.h"


namespace LLGL
{


D3D12CommandQueue::D3D12CommandQueue(ComPtr<ID3D12CommandQueue>& queue, ComPtr<ID3D12CommandAllocator>& commandAlloc) :
    queue_        { queue        },
    commandAlloc_ { commandAlloc }
{
}

/* ----- Command queues ----- */

void D3D12CommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferD3D = LLGL_CAST(D3D12CommandBuffer&, commandBuffer);

    /* Get hardware command list */
    auto commandList = commandBufferD3D.GetCommandList();

    /* Close graphics command list */
    auto hr = commandList->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");

    /* Execute command list */
    ID3D12CommandList* cmdLists[] = { commandList };
    queue_->ExecuteCommandLists(1, cmdLists);

    /* Reset command list */
    hr = commandList->Reset(commandAlloc_.Get(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");
}

/* ----- Fences ----- */

void D3D12CommandQueue::Submit(Fence& fence)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence); 
    fenceD3D.Submit(queue_.Get());
}

bool D3D12CommandQueue::WaitForFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceD3D = LLGL_CAST(D3D12Fence&, fence); 
    return fenceD3D.Wait(timeout);
}

void D3D12CommandQueue::WaitForFinish()
{
    //TODO
    //renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}


} // /namespace LLGL



// ================================================================================
