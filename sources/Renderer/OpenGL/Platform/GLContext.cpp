/*
 * GLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
            /* Activate new GL context: MakeCurrent(context) */
            GLStateManager::active_ = context->stateMngr_.get();
            result = context->Activate(true);
        }
        else if (g_activeGLContext)
        {
            /* Deactivate previous GL context: MakeCurrent(null) */
            GLStateManager::active_ = nullptr;
            result = g_activeGLContext->Activate(false);
        }

        /* Store pointer to new GL context */
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
