/*
 * D3D12CommandContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandContext.h"
#include "D3D12CommandQueue.h"
#include "../D3D12Device.h"
#include "../D3D12Resource.h"
#include "../RenderState/D3D12Fence.h"
#include "../../DXCommon/DXCore.h"
#include <algorithm>


namespace LLGL
{


const UINT D3D12CommandContext::g_maxNumAllocators;
const UINT D3D12CommandContext::g_maxNumResourceBarrieres;
const UINT D3D12CommandContext::g_maxNumDescriptorHeaps;

D3D12CommandContext::D3D12CommandContext()
{
    ClearCache();
}

D3D12CommandContext::D3D12CommandContext(
    D3D12Device&        device,
    D3D12CommandQueue&  commandQueue)
{
    Create(device, commandQueue);
}

void D3D12CommandContext::Create(
    D3D12Device&            device,
    D3D12CommandQueue&      commandQueue,
    D3D12_COMMAND_LIST_TYPE commandListType,
    UINT                    numAllocators,
    bool                    initialClose)
{
    /* Store reference to command queue */
    commandQueue_ = &commandQueue;

    /* Create fence for command allocators */
    allocatorFence_.Create(device.GetNative());

    /* Determine number of command allocators */
    numAllocators_ = std::max(1u, std::min(numAllocators, g_maxNumAllocators));

    /* Create command allocators */
    for (std::uint32_t i = 0; i < numAllocators_; ++i)
        commandAllocators_[i] = device.CreateDXCommandAllocator(commandListType);

    /* Create graphics command list and close it (they are created in recording mode) */
    commandList_ = device.CreateDXCommandList(commandListType, GetCommandAllocator());

    if (initialClose)
        commandList_->Close();

    /* Clear cache alongside device object initialization */
    ClearCache();
}

void D3D12CommandContext::Close()
{
    /* Flush pending resource barriers */
    FlushResourceBarrieres();

    /* Close native command list */
    auto hr = commandList_->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");
}

void D3D12CommandContext::Execute()
{
    /* Submit command list to queue */
    ID3D12CommandList* cmdLists[] = { GetCommandList() };
    commandQueue_->GetNative()->ExecuteCommandLists(1, cmdLists);

    /* Signal current allocator fence value */
    commandQueue_->SignalFence(allocatorFence_, allocatorFenceValues_[currentAllocatorIndex_]);
}

void D3D12CommandContext::Reset()
{
    /* Switch to next command allocator */
    NextCommandAllocator();

    /* Reset graphics command list */
    auto hr = commandList_->Reset(GetCommandAllocator(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");

    /* Invalidate state cache */
    ClearCache();
}

void D3D12CommandContext::Finish(bool waitIdle)
{
    /* Close command list and execute, then reset command allocator for next encoding */
    Close();
    Execute();

    /* Synchronize GPU/CPU */
    if (waitIdle)
        commandQueue_->WaitIdle();

    Reset();
}

void D3D12CommandContext::TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
    auto oldState = resource.transitionState;
    if (oldState != newState)
    {
        auto& barrier = NextResourceBarrier();

        /* Initialize resource barrier for resource transition */
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = resource.native.Get();
        barrier.Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore  = oldState;
        barrier.Transition.StateAfter   = newState;

        /* Store new state in resource */
        resource.transitionState = newState;
    }

    /* Flush resource barrieres if required */
    if (flushImmediate)
        FlushResourceBarrieres();
}

#if 0 //TODO: not used yey
void D3D12CommandContext::TransitionSubresource(
    D3D12Resource&          resource,
    UINT                    subresource,
    D3D12_RESOURCE_STATES   oldState,
    D3D12_RESOURCE_STATES   newState,
    bool                    flushImmediate)
{
    auto& barrier = NextResourceBarrier();

    /* Initialize resource barrier for resource transition */
    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource    = resource.native.Get();
    barrier.Transition.Subresource  = subresource;
    barrier.Transition.StateBefore  = oldState;
    barrier.Transition.StateAfter   = newState;

    /* Flush resource barrieres if required */
    if (flushImmediate)
        FlushResourceBarrieres();
}
#endif

void D3D12CommandContext::InsertUAVBarrier(D3D12Resource& resource, bool flushImmediate)
{
    auto& barrier = NextResourceBarrier();

    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource   = resource.native.Get();

    if (flushImmediate)
        FlushResourceBarrieres();
}

void D3D12CommandContext::FlushResourceBarrieres()
{
    if (numResourceBarriers_ > 0)
    {
        commandList_->ResourceBarrier(numResourceBarriers_, resourceBarriers_);
        numResourceBarriers_ = 0;
    }
}

void D3D12CommandContext::ResolveRenderTarget(
    D3D12Resource&  dstResource,
    UINT            dstSubresource,
    D3D12Resource&  srcResource,
    UINT            srcSubresource,
    DXGI_FORMAT     format)
{
    /* Transition both resources */
    TransitionResource(dstResource, D3D12_RESOURCE_STATE_RESOLVE_DEST);
    TransitionResource(srcResource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, true);

    /* Resolve multi-sampled render targets */
    commandList_->ResolveSubresource(
        dstResource.native.Get(),
        dstSubresource,
        srcResource.native.Get(),
        srcSubresource,
        format
    );

    /* Transition both resources */
    TransitionResource(dstResource, dstResource.usageState);
    TransitionResource(srcResource, srcResource.usageState, true);
}

void D3D12CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature)
{
    if (stateCache_.dirtyBits.graphicsRootSignature != 0 || stateCache_.graphicsRootSignature != rootSignature)
    {
        /* Bind graphics root signature and cache state */
        commandList_->SetGraphicsRootSignature(rootSignature);
        stateCache_.graphicsRootSignature           = rootSignature;
        stateCache_.dirtyBits.graphicsRootSignature = 0;
    }
}

void D3D12CommandContext::SetComputeRootSignature(ID3D12RootSignature* rootSignature)
{
    if (stateCache_.dirtyBits.computeRootSignature != 0 || stateCache_.computeRootSignature != rootSignature)
    {
        /* Bind graphics root signature and cache state */
        commandList_->SetComputeRootSignature(rootSignature);
        stateCache_.computeRootSignature            = rootSignature;
        stateCache_.dirtyBits.computeRootSignature  = 0;
    }
}

void D3D12CommandContext::SetPipelineState(ID3D12PipelineState* pipelineState)
{
    if (stateCache_.dirtyBits.pipelineState != 0 || stateCache_.pipelineState != pipelineState)
    {
        /* Bind pipeline state to command list and cache state */
        commandList_->SetPipelineState(pipelineState);
        stateCache_.pipelineState           = pipelineState;
        stateCache_.dirtyBits.pipelineState = 0;
    }
}

static bool CompareDescriptorHeapRefs(
    UINT                            lhsNumDescriptorHeaps,
    ID3D12DescriptorHeap* const*    lhsDescriptorHeaps,
    UINT                            rhsNumDescriptorHeaps,
    ID3D12DescriptorHeap* const*    rhsDescriptorHeaps)
{
    /* Compare all descriptor heaps in the arrays */
    return
    (
        lhsNumDescriptorHeaps == rhsNumDescriptorHeaps &&
        (::memcmp(lhsDescriptorHeaps, rhsDescriptorHeaps, sizeof(ID3D12DescriptorHeap* const) * lhsNumDescriptorHeaps) == 0)
    );
}

void D3D12CommandContext::SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* const* descriptorHeaps)
{
    if (numDescriptorHeaps <= g_maxNumDescriptorHeaps)
    {
        /* Check if the descriptor heaps are cached */
        if (stateCache_.dirtyBits.descriptorHeaps != 0 ||
            !CompareDescriptorHeapRefs(numDescriptorHeaps, descriptorHeaps, stateCache_.numDescriptorHeaps, stateCache_.descriptorHeaps))
        {
            /* Set new descriptor heaps in D3D command list */
            commandList_->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);

            /* Store new descriptor heaps in state cache */
            stateCache_.numDescriptorHeaps = numDescriptorHeaps;
            ::memcpy(stateCache_.descriptorHeaps, descriptorHeaps, sizeof(ID3D12DescriptorHeap* const) * numDescriptorHeaps);
            stateCache_.dirtyBits.descriptorHeaps = 0;
        }
    }
}

void D3D12CommandContext::SetGraphicsConstant(UINT parameterIndex, D3D12Constant value, UINT offset)
{
    commandList_->SetGraphicsRoot32BitConstant(parameterIndex, value.u32, offset);
}

void D3D12CommandContext::SetComputeConstant(UINT parameterIndex, D3D12Constant value, UINT offset)
{
    commandList_->SetComputeRoot32BitConstant(parameterIndex, value.u32, offset);
}


/*
 * ======= Private: =======
 */

D3D12_RESOURCE_BARRIER& D3D12CommandContext::NextResourceBarrier()
{
    if (numResourceBarriers_ == g_maxNumResourceBarrieres)
        FlushResourceBarrieres();
    return resourceBarriers_[numResourceBarriers_++];
}

void D3D12CommandContext::NextCommandAllocator()
{
    /* Get next command allocator */
    currentAllocatorIndex_ = ((currentAllocatorIndex_ + 1) % numAllocators_);

    /* Wait until fence value of next allocator has been signaled */
    allocatorFence_.WaitForValueAndUpdate(allocatorFenceValues_[currentAllocatorIndex_]);

    /* Reclaim memory allocated by command allocator using <ID3D12CommandAllocator::Reset> */
    auto hr = GetCommandAllocator()->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");
}

void D3D12CommandContext::ClearCache()
{
    stateCache_.dirtyBits.value = ~0u;
}


} // /namespace LLGL



// ================================================================================
