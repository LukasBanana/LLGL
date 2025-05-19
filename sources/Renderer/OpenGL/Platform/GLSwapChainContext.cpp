/*
 * GLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLSwapChainContext.h"
#include "GLContext.h"


namespace LLGL
{


static GLSwapChainContext* g_currentSwapChainContext;

GLSwapChainContext::GLSwapChainContext(GLContext& context) :
    context_ { context }
{
}

bool GLSwapChainContext::MakeCurrent(GLSwapChainContext* context)
{
    bool result = true;
    if (g_currentSwapChainContext != context)
    {
        result = GLSwapChainContext::MakeCurrentUnchecked(context);
        GLContext::SetCurrent(context != nullptr ? &(context->context_) : nullptr);
        g_currentSwapChainContext = context;
    }
    return result;
}


} // /namespace LLGL



// ================================================================================
