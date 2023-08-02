/*
 * CommandBufferFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_COMMAND_BUFFER_FLAGS_H
#define LLGL_C99_COMMAND_BUFFER_FLAGS_H


#include <LLGL-C/Types.h>
#include <stdint.h>


/* ----- Enumerations ----- */

typedef enum LLGLRenderConditionMode
{
    LLGLRenderConditionModeWait,
    LLGLRenderConditionModeNoWait,
    LLGLRenderConditionModeByRegionWait,
    LLGLRenderConditionModeByRegionNoWait,
    LLGLRenderConditionModeWaitInverted,
    LLGLRenderConditionModeNoWaitInverted,
    LLGLRenderConditionModeByRegionWaitInverted,
    LLGLRenderConditionModeByRegionNoWaitInverted,
}
LLGLRenderConditionMode;

typedef enum LLGLStencilFace
{
    LLGLStencilFaceFrontAndBack,
    LLGLStencilFaceFront,
    LLGLStencilFaceBack,
}
LLGLStencilFace;


/* ----- Flags ----- */

enum LLGLCommandBufferFlags
{
    LLGLCommandBufferSecondary          = (1 << 0),
    LLGLCommandBufferMultiSubmit        = (1 << 1),
    LLGLCommandBufferImmediateSubmit    = (1 << 2),
};

enum LLGLClearFlags
{
    LLGLClearColor          = (1 << 0),
    LLGLClearDepth          = (1 << 1),
    LLGLClearStencil        = (1 << 2),

    LLGLClearColorDepth     = (LLGLClearColor | LLGLClearDepth),
    LLGLClearDepthStencil   = (LLGLClearDepth | LLGLClearStencil),
    LLGLClearAll            = (LLGLClearColor | LLGLClearDepth | LLGLClearStencil),
};


/* ----- Structures ----- */

typedef struct LLGLClearValue
{
    float       color[4];
    float       depth;
    uint32_t    stencil;
}
LLGLClearValue;

typedef struct LLGLAttachmentClear
{
    long            flags;
    uint32_t        colorAttachment;
    LLGLClearValue  clearValue;
}
LLGLAttachmentClear;

typedef struct LLGLCommandBufferDescriptor
{
    long        flags;
    uint32_t    numNativeBuffers;
    uint64_t    minStagingPoolSize;
}
LLGLCommandBufferDescriptor;


#endif



// ================================================================================
