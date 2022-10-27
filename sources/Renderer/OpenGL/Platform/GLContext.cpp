/*
 * GLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLContext.h"


namespace LLGL
{


static GLContext* g_currentGLContext = nullptr;

GLContext::GLContext(GLContext* sharedContext)
{
    if (sharedContext)
        stateMngr_ = sharedContext->stateMngr_;
    else
        stateMngr_ = std::make_shared<GLStateManager>();
}

GLContext::~GLContext()
{
    // dummy
}

bool GLContext::MakeCurrent(GLContext* context)
{
    bool result = true;

    if (g_currentGLContext != context)
    {
        if (context)
        {
            /* Activate new GL context: MakeCurrent(context) */
            GLStateManager::active_ = context->stateMngr_.get();
            result = context->Activate(true);
        }
        else if (g_currentGLContext)
        {
            /* Deactivate previous GL context: MakeCurrent(null) */
            GLStateManager::active_ = nullptr;
            result = g_currentGLContext->Activate(false);
        }

        /* Store pointer to new GL context */
        g_currentGLContext = context;
    }

    return result;
}

GLContext* GLContext::GetCurrent()
{
    return g_currentGLContext;
}


/*
 * ======= Protected: =======
 */

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
