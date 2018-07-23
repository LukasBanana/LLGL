/*
 * GLCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCommandQueue.h"
#include "../CheckedCast.h"
#include "RenderState/GLFence.h"


namespace LLGL
{


/* ----- Command Buffers ----- */

void GLCommandQueue::Submit(CommandBuffer& /*commandBuffer*/)
{
    // dummy
}

/* ----- Fences ----- */

void GLCommandQueue::Submit(Fence& fence)
{
    auto& fenceGL = LLGL_CAST(GLFence&, fence);
    fenceGL.Submit();
}

bool GLCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceGL = LLGL_CAST(GLFence&, fence);
    return fenceGL.Wait(timeout);
}

void GLCommandQueue::WaitIdle()
{
    glFinish();
}


} // /namespace LLGL



// ================================================================================
