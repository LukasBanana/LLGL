/*
 * C99RenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderTarget.h>
#include <LLGL-C/RenderTarget.h>
#include <LLGL/SwapChain.h>
#include <LLGL/TypeInfo.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT void llglGetRenderTargetResolution(LLGLRenderTarget renderTarget, LLGLExtent2D* outResolution)
{
    Extent2D resolution = LLGL_PTR(RenderTarget, renderTarget)->GetResolution();
    outResolution->width = resolution.width;
    outResolution->height = resolution.height;
}

LLGL_C_EXPORT uint32_t llglGetRenderTargetSamples(LLGLRenderTarget renderTarget)
{
    return LLGL_PTR(RenderTarget, renderTarget)->GetSamples();
}

LLGL_C_EXPORT uint32_t llglGetRenderTargetNumColorAttachments(LLGLRenderTarget renderTarget)
{
    return LLGL_PTR(RenderTarget, renderTarget)->GetNumColorAttachments();
}

LLGL_C_EXPORT bool llglHasRenderTargetDepthAttachment(LLGLRenderTarget renderTarget)
{
    return LLGL_PTR(RenderTarget, renderTarget)->HasDepthAttachment();
}

LLGL_C_EXPORT bool llglHasRenderTargetStencilAttachment(LLGLRenderTarget renderTarget)
{
    return LLGL_PTR(RenderTarget, renderTarget)->HasStencilAttachment();
}

LLGL_C_EXPORT LLGLRenderPass llglGetRenderTargetRenderPass(LLGLRenderTarget renderTarget)
{
    return LLGLRenderPass{ const_cast<RenderPass*>(LLGL_PTR(RenderTarget, renderTarget)->GetRenderPass()) };
}

LLGL_C_EXPORT bool llglIsInstanceOfSwapChain(LLGLRenderTarget renderTarget)
{
    return IsInstanceOf<SwapChain>(LLGL_PTR(RenderTarget, renderTarget));
}


// } /namespace LLGL



// ================================================================================
