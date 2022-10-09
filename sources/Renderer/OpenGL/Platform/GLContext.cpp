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


} // /namespace LLGL



// ================================================================================
