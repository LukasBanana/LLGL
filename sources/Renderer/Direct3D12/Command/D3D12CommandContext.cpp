/*
 * D3D12CommandContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandContext.h"
#include "../D3D12Resource.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


void D3D12CommandContext::SetCommandList(ID3D12GraphicsCommandList* commandList)
{
    if (commandList_ != commandList)
    {
        FlushResourceBarrieres();
        commandList_ = commandList;
    }
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


} // /namespace LLGL



// ================================================================================
