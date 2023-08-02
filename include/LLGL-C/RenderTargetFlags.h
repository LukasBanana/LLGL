/*
 * RenderTargetFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_TARGET_FLAGS_H
#define LLGL_C99_RENDER_TARGET_FLAGS_H


#include <LLGL/StaticLimits.h>
#include <LLGL-C/Format.h>
#include <LLGL-C/TextureFlags.h>
#include <LLGL-C/PipelineStateFlags.h>
#include <stdint.h>


/* ----- Structures ----- */

typedef struct LLGLAttachmentDescriptor
{
    LLGLFormat  format;
    LLGLTexture texture;
    uint32_t    mipLevel;
    uint32_t    arrayLayer;
}
LLGLAttachmentDescriptor;

typedef struct LLGLRenderTargetDescriptor
{
    LLGLRenderPass              renderPass;
    LLGLExtent2D                resolution;
    uint32_t                    samples;
    LLGLAttachmentDescriptor    colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    LLGLAttachmentDescriptor    resolveAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    LLGLAttachmentDescriptor    depthStencilAttachment;
}
LLGLRenderTargetDescriptor;


#endif



// ================================================================================
