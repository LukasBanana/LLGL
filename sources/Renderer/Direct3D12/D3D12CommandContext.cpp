/*
 * D3D12CommandContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandContext.h"
#include "D3D12Resource.h"
#include "../DXCommon/DXCore.h"


namespace LLGL
{


void D3D12CommandContext::TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushBarrieres)
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
        if (flushBarrieres || numResourceBarriers_ == g_maxNumResourceBarrieres)
            FlushResourceBarrieres();
    }
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
    TransitionResource(dstResource, D3D12_RESOURCE_STATE_RESOLVE_DEST, false);
    TransitionResource(srcResource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

    /* Resolve multi-sampled render targets */
    commandList_->ResolveSubresource(
        dstResource.native.Get(),
        dstSubresource,
        srcResource.native.Get(),
        srcSubresource,
        format
    );

    /* Transition both resources */
    TransitionResource(dstResource, dstResource.usageState, false);
    TransitionResource(srcResource, srcResource.usageState);
}


/*
 * ======= Private: =======
 */

D3D12_RESOURCE_BARRIER& D3D12CommandContext::NextResourceBarrier()
{
    return resourceBarriers_[numResourceBarriers_++];
}


} // /namespace LLGL



// ================================================================================
