/*
 * GLSwapChainContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
