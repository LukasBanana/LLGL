/*
 * RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_TARGET_H
#define LLGL_C99_RENDER_TARGET_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stdint.h>


LLGL_C_EXPORT void llglGetRenderTargetResolution(LLGLRenderTarget renderTarget, LLGLExtent2D* outResolution);
LLGL_C_EXPORT uint32_t llglGetRenderTargetSamples(LLGLRenderTarget renderTarget);
LLGL_C_EXPORT uint32_t llglGetRenderTargetNumColorAttachments(LLGLRenderTarget renderTarget);
LLGL_C_EXPORT bool llglHasRenderTargetDepthAttachment(LLGLRenderTarget renderTarget);
LLGL_C_EXPORT bool llglHasRenderTargetStencilAttachment(LLGLRenderTarget renderTarget);
LLGL_C_EXPORT LLGLRenderPass llglGetRenderTargetRenderPass(LLGLRenderTarget renderTarget);


#endif



// ================================================================================
