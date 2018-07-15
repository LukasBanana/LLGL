/*
 * D3D11CommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11CommandQueue.h"
#include "RenderState/D3D11Fence.h"


namespace LLGL
{


D3D11CommandQueue::D3D11CommandQueue(ComPtr<ID3D11DeviceContext>& context) :
    context_ { context }
{
}

/* ----- Command Buffers ----- */

void D3D11CommandQueue::Begin(CommandBuffer& /*commandBuffer*/, long /*flags*/)
{
    // dummy
}

void D3D11CommandQueue::End(CommandBuffer& /*commandBuffer*/)
{
    // dummy
}

void D3D11CommandQueue::Submit(CommandBuffer& /*commandBuffer*/)
{
    // dummy
}

/* ----- Fences ----- */

void D3D11CommandQueue::Submit(Fence& fence)
{
    //TODO: use D3D11Fence
}

bool D3D11CommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    //TODO: use D3D11Fence
    context_->Flush();
    return true;
}

void D3D11CommandQueue::WaitIdle()
{
    context_->Flush();
}


} // /namespace LLGL



// ================================================================================
