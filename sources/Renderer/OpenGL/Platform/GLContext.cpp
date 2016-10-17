/*
 * GLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLContext.h"


namespace LLGL
{


static GLContext* g_activeGLContext = nullptr;

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

    if (g_activeGLContext != context)
    {
        if (context)
        {
            GLStateManager::active = context->stateMngr_.get();
            result = context->Activate(true);
        }
        else if (g_activeGLContext)
        {
            GLStateManager::active = nullptr;
            result = g_activeGLContext->Activate(false);
        }
        else
            result = false;

        g_activeGLContext = context;
    }

    return result;
}

GLContext* GLContext::Active()
{
    return g_activeGLContext;
}


} // /namespace LLGL



// ================================================================================
