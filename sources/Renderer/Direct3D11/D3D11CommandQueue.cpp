/*
 * D3D11CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11CommandQueue.h"
#include "RenderState/D3D11Fence.h"
#include "../CheckedCast.h"


namespace LLGL
{


D3D11CommandQueue::D3D11CommandQueue(ID3D11Device* device, ComPtr<ID3D11DeviceContext>& context) :
    context_           { context },
    intermediateFence_ { device  }
{
}

/* ----- Command Buffers ----- */

void D3D11CommandQueue::Submit(CommandBuffer& /*commandBuffer*/)
{
    // dummy
}

/* ----- Fences ----- */

void D3D11CommandQueue::Submit(Fence& fence)
{
    auto& fenceD3D = LLGL_CAST(D3D11Fence&, fence);
    fenceD3D.Submit(context_.Get());
}

bool D3D11CommandQueue::WaitFence(Fence& fence, std::uint64_t /*timeout*/)
{
    auto& fenceD3D = LLGL_CAST(D3D11Fence&, fence);
    fenceD3D.Wait(context_.Get());
    return true;
}

void D3D11CommandQueue::WaitIdle()
{
    /* Submit intermediate fence and wait for it to be signaled */
    Submit(intermediateFence_);
    WaitFence(intermediateFence_, ~0ull);
}


} // /namespace LLGL



// ================================================================================
