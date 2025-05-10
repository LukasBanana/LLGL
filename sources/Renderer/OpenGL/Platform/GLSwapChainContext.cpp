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

bool GLSwapChainContext::MakeCurrent()
{
    bool result = true;
    if (g_currentSwapChainContext != this)
    {
        result = MakeCurrentUnchecked();
        GLContext::SetCurrent(&context_);
        g_currentSwapChainContext = this;
    }
    return result;
}


} // /namespace LLGL



// ================================================================================
