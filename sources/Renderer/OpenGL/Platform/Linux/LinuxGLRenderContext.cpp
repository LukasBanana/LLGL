/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"
#include "../../../../Platform/Linux/LinuxWindow.h"
#include <LLGL/Log.h>

#include <iostream>//!!!


namespace LLGL
{


void GLRenderContext::Present()
{
    glXSwapBuffers(context_.display, context_.wnd);
}

bool GLRenderContext::GLMakeCurrent(GLRenderContext* renderContext)
{
    if (renderContext)
    {
        const auto& ctx = renderContext->context_;
        return glXMakeCurrent(ctx.display, ctx.wnd, ctx.glc);
    }
    else
        return glXMakeCurrent(nullptr, 0, 0);
    return false;
}


/*
 * ======= Private: =======
 */

void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
    GLXContext glcShared = (sharedRenderContext != nullptr ? sharedRenderContext->context_.glc : nullptr);
    
    /* Get X11 display, window, and visual information */
    auto& window = static_cast<const LinuxWindow&>(GetWindow());
    
    context_.display    = window.GetDisplay();
    context_.wnd        = window.GetHandle();
    context_.visual     = window.GetVisual();
    
    /* Create OpenGL context with X11 lib */
    context_.glc        = glXCreateContext(context_.display, context_.visual, glcShared, GL_TRUE);
    
    /* Make new OpenGL context current */
    if (glXMakeCurrent(context_.display, context_.wnd, context_.glc) != True)
        Log::StdErr() << "failed to make OpenGL render context current (glXMakeCurrent)" << std::endl;
}

void GLRenderContext::DeleteContext()
{
    glXDestroyContext(context_.display, context_.glc);
}


} // /namespace LLGL



// ================================================================================
