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
    if (depthBits == 24 && stencilBits == 8)
        depthStencilFormat_ = Format::D24UNormS8UInt;
    else if (depthBits == 32 && stencilBits == 8)
        depthStencilFormat_ = Format::D32FloatS8X24UInt;
    else if (depthBits == 16 && stencilBits == 0)
        depthStencilFormat_ = Format::D16UNorm;
    else if (depthBits == 32 && stencilBits == 0)
        depthStencilFormat_ = Format::D32Float;
}

void GLContext::SetDefaultColorFormat()
{
    colorFormat_ = Format::RGBA8UNorm;
}

void GLContext::SetDefaultDepthStencilFormat()
{
    depthStencilFormat_ = Format::D24UNormS8UInt;
}


} // /namespace LLGL



// ================================================================================
