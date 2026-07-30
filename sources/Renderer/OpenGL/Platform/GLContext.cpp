/*
 * GLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLContext.h"


namespace LLGL
{


bool operator == (const GLPixelFormat& lhs, const GLPixelFormat& rhs)
{
    return
    (
        lhs.colorBits   == rhs.colorBits    &&
        lhs.depthBits   == rhs.depthBits    &&
        lhs.stencilBits == rhs.stencilBits  &&
        lhs.samples     == rhs.samples
    );
}

bool operator != (const GLPixelFormat& lhs, const GLPixelFormat& rhs)
{
    return !(lhs == rhs);
}


/*
 * GLContext class
 */

static GLContext*   g_currentContext;
static unsigned     g_currentGlobalIndex;
static unsigned     g_globalIndexCounter;

bool GLContext::SetCurrentSwapInterval(int interval)
{
    if (g_currentContext != nullptr)
        return g_currentContext->SetSwapInterval(interval);
    else
        return false;
}

void GLContext::SetCurrent(GLContext* context)
{
    if (context != g_currentContext)
    {
        if (context != nullptr)
        {
            g_currentContext        = context;
            g_currentGlobalIndex    = context->GetGlobalIndex();
            GLStateManager::SetCurrentFromGLContext(*context);
        }
        else
        {
            g_currentContext        = nullptr;
            g_currentGlobalIndex    = 0;
        }
    }
}

GLContext* GLContext::GetCurrent()
{
    return g_currentContext;
}

unsigned GLContext::GetCurrentGlobalIndex()
{
    return g_currentGlobalIndex;
}


/*
 * ======= Protected: =======
 */

GLContext::GLContext() :
    globalIndex_ { ++g_globalIndexCounter }
{
}

void GLContext::DeduceColorFormat(int /*rBits*/, int rShift, int /*gBits*/, int gShift, int /*bBits*/, int bShift, int /*aBits*/, int aShift)
{
    if (bShift == 24 && gShift == 16 && rShift == 8 && aShift == 0)
        colorFormat_ = Format::BGRA8UNorm;
    else
        colorFormat_ = Format::RGBA8UNorm;
}

void GLContext::DeduceDepthStencilFormat(int depthBits, int stencilBits)
{
    /*
    The bit sizes come from the pixel format the windowing system selected, which is not restricted to the
    combinations LLGL can request. Map every combination that has a depth or stencil component onto the
    closest LLGL format, so GetDepthStencilFormat() never reports Format::Undefined for a buffer that exists.
    */
    if (depthBits == 0 && stencilBits == 0)
    {
        /* Pixel format has neither a depth nor a stencil buffer */
        depthStencilFormat_ = Format::Undefined;
    }
    else if (stencilBits > 0)
    {
        /* LLGL has no stencil-only format, so a stencil buffer always implies a combined depth-stencil format */
        depthStencilFormat_ = (depthBits > 24 ? Format::D32FloatS8X24UInt : Format::D24UNormS8UInt);
    }
    else
    {
        /* LLGL has no 24-bit depth-only format, so report anything above 16 bits as 32-bit depth */
        depthStencilFormat_ = (depthBits > 16 ? Format::D32Float : Format::D16UNorm);
    }
}

void GLContext::SetDefaultColorFormat()
{
    colorFormat_ = Format::RGBA8UNorm;
}

void GLContext::SetDefaultDepthStencilFormat()
{
    depthStencilFormat_ = Format::D24UNormS8UInt;
}

bool GLContext::IsSharableForSurface(const Surface *surface) const
{
    return true;
}


} // /namespace LLGL



// ================================================================================
