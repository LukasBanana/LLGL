/*
 * RenderPassFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_PASS_FLAGS_H
#define LLGL_C99_RENDER_PASS_FLAGS_H


#include <LLGL-C/Format.h>
#include <LLGL/StaticLimits.h>


/* ----- Enumerations ----- */

typedef enum LLGLAttachmentLoadOp
{
    LLGLAttachmentLoadOpUndefined,
    LLGLAttachmentLoadOpLoad,
    LLGLAttachmentLoadOpClear,
}
LLGLAttachmentLoadOp;

typedef enum LLGLAttachmentStoreOp
{
    LLGLAttachmentStoreOpUndefined,
    LLGLAttachmentStoreOpStore,
}
LLGLAttachmentStoreOp;


/* ----- Structures ----- */

typedef struct LLGLAttachmentFormatDescriptor
{
    LLGLFormat              format;
    LLGLAttachmentLoadOp    loadOp;
    LLGLAttachmentStoreOp   storeOp;
}
LLGLAttachmentFormatDescriptor;

typedef struct LLGLRenderPassDescriptor
{
    LLGLAttachmentFormatDescriptor  colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    LLGLAttachmentFormatDescriptor  depthAttachment;
    LLGLAttachmentFormatDescriptor  stencilAttachment;
    uint32_t                        samples;
}
LLGLRenderPassDescriptor;


#endif



// ================================================================================
