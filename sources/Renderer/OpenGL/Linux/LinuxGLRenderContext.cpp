/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLRenderContext.h"


namespace LLGL
{


void GLRenderContext::Present()
{
    //glXSwapBuffers();
}

bool GLRenderContext::GLMakeCurrent(GLRenderContext* renderContext)
{
    /*if (renderContext)
    {
        const auto& ctx = renderContext->context_;
        return glXMakeCurrent(ctx.hDC, ctx.hGLRC);
    }
    else
        return glXMakeCurrent(0, 0));*/
    return false;
}


/*
 * ======= Private: =======
 */

void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
}

void GLRenderContext::DeleteContext()
{
}


} // /namespace LLGL



// ================================================================================
