/*
 * CommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_COMMAND_QUEUE_H
#define LLGL_C99_COMMAND_QUEUE_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


LLGL_C_EXPORT void llglSubmitCommandBuffer(LLGLCommandBuffer commandBuffer);
LLGL_C_EXPORT bool llglQueryResult(LLGLQueryHeap queryHeap, uint32_t firstQuery, uint32_t numQueries, void* data, size_t dataSize);
LLGL_C_EXPORT void llglSubmitFence(LLGLFence fence);
LLGL_C_EXPORT bool llglWaitFence(LLGLFence fence, uint64_t timeout);
LLGL_C_EXPORT void llglWaitIdle();


#endif



// ================================================================================
