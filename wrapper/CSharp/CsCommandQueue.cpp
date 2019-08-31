/*
 * CsCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsCommandQueue.h"


namespace SharpLLGL
{


CommandQueue::CommandQueue(LLGL::CommandQueue* native) :
    native_ { native }
{
}

LLGL::CommandQueue* CommandQueue::Native::get()
{
    return native_;
}

/* ----- Command Buffers ----- */

void CommandQueue::Submit(CommandBuffer^ commandBuffer)
{
    native_->Submit(*commandBuffer->Native::get());
}

/* ----- Fences ----- */

void CommandQueue::Submit(Fence^ fence)
{
    native_->Submit(*fence->Native::get());
}

bool CommandQueue::WaitFence(Fence^ fence, UInt64 timeout)
{
    return native_->WaitFence(*fence->Native::get(), static_cast<std::uint64_t>(timeout));
}

void CommandQueue::WaitIdle()
{
    native_->WaitIdle();
}


} // /namespace SharpLLGL



// ================================================================================
