/*
 * ResourceFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RESOURCE_FLAGS_H
#define LLGL_C99_RESOURCE_FLAGS_H


/* ----- Enumerations ----- */

typedef enum LLGLResourceType
{
    LLGLResourceTypeUndefined,
    LLGLResourceTypeBuffer,
    LLGLResourceTypeTexture,
    LLGLResourceTypeSampler,
}
LLGLResourceType;


/* ----- Flags ----- */

enum LLGLBindFlags
{
    LLGLBindVertexBuffer            = (1 << 0),
    LLGLBindIndexBuffer             = (1 << 1),
    LLGLBindConstantBuffer          = (1 << 2),
    LLGLBindStreamOutputBuffer      = (1 << 3),
    LLGLBindIndirectBuffer          = (1 << 4),
    LLGLBindSampled                 = (1 << 5),
    LLGLBindStorage                 = (1 << 6),
    LLGLBindColorAttachment         = (1 << 7),
    LLGLBindDepthStencilAttachment  = (1 << 8),
    LLGLBindCombinedSampler         = (1 << 9),
    LLGLBindCopySrc                 = (1 << 10),
    LLGLBindCopyDst                 = (1 << 11),
};

enum LLGLCPUAccessFlags
{
    LLGLCPUAccessRead   = (1 << 0),
    LLGLCPUAccessWrite  = (1 << 1),
};

enum LLGLMiscFlags
{
    LLGLMiscDynamicUsage    = (1 << 0),
    LLGLMiscFixedSamples    = (1 << 1),
    LLGLMiscGenerateMips    = (1 << 2),
    LLGLMiscNoInitialData   = (1 << 3),
    LLGLMiscAppend          = (1 << 4),
    LLGLMiscCounter         = (1 << 5),
};


#endif



// ================================================================================
