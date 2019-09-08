/*
 * D3D12CommandContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandContext.h"
#include "../D3D12Resource.h"
#include "../RenderState/D3D12Fence.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12CommandContext::D3D12CommandContext()
{
    ClearCache();
}

void D3D12CommandContext::SetCommandList(ID3D12GraphicsCommandList* commandList)
{
    if (commandList_ != commandList)
    {
        FlushResourceBarrieres();
        commandList_ = commandList;
    }
}

void D3D12CommandContext::SetCommandQueueAndAllocator(ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* commandAllocator)
{
    commandQueue_       = commandQueue;
    commandAllocator_   = commandAllocator;
}

void D3D12CommandContext::Close()
{
    /* Flush pending resource barriers */
    FlushResourceBarrieres();

    /* Close native command list */
    auto hr = commandList_->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");
}

void D3D12CommandContext::Execute(ID3D12CommandQueue* commandQueue)
{
    ID3D12CommandList* cmdLists[] = { GetCommandList() };
    commandQueue->ExecuteCommandLists(1, cmdLists);
}

void D3D12CommandContext::Reset(ID3D12CommandAllocator* commandAllocator)
{
    /* Reset graphics command list */
    auto hr = commandList_->Reset(commandAllocator, nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");

    /* Invalidate state cache */
    ClearCache();
}

void D3D12CommandContext::Finish(D3D12Fence* fence)
{
    /* Close command list and execute, then reset command allocator for next encoding */
    Close();
    Execute(commandQueue_);

    /* Synchronize GPU/CPU */
    if (fence)
    {
        auto hr = commandQueue_->Signal(fence->GetNative(), fence->NextValue());
        DXThrowIfFailed(hr, "failed to signal D3D12 fence with command queue");
        fence->Wait(~0ull);
    }

    Reset(commandAllocator_);
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

        /* Flush resource barrieres if required */
        if (flushImmediate)
            FlushResourceBarrieres();
    }
}

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

void D3D12CommandContext::ClearCache()
{
    stateCache_.dirtyBits.value = ~0u;
}


} // /namespace LLGL



// ================================================================================
