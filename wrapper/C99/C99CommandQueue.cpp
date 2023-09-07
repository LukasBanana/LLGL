/*
 * C99CommandQueue.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/CommandQueue.h>
#include <LLGL-C/CommandQueue.h>
#include "C99Internal.h"
#include "../../sources/Core/Assertion.h"


// namespace LLGL {


using namespace LLGL;

CommandQueue* g_CurrentCmdQueue = NULL;


LLGL_C_EXPORT void llglSubmitCommandBuffer(LLGLCommandBuffer commandBuffer)
{
    g_CurrentCmdQueue->Submit(LLGL_REF(CommandBuffer, commandBuffer));
}

LLGL_C_EXPORT bool llglQueryResult(LLGLQueryHeap queryHeap, uint32_t firstQuery, uint32_t numQueries, void* data, size_t dataSize)
{
    return g_CurrentCmdQueue->QueryResult(LLGL_REF(QueryHeap, queryHeap), firstQuery, numQueries, data, dataSize);
}

LLGL_C_EXPORT void llglSubmitFence(LLGLFence fence)
{
    g_CurrentCmdQueue->Submit(LLGL_REF(Fence, fence));
}

LLGL_C_EXPORT bool llglWaitFence(LLGLFence fence, uint64_t timeout)
{
    return g_CurrentCmdQueue->WaitFence(LLGL_REF(Fence, fence), timeout);
}

LLGL_C_EXPORT void llglWaitIdle()
{
    g_CurrentCmdQueue->WaitIdle();
}


// } /namespace LLGL



// ================================================================================
