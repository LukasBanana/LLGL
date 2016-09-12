/*
 * MacOSGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"
#include "../../../../Platform/MacOS/MacOSWindow.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


void GLRenderContext::Present()
{
    //glXSwapBuffers(context_.display, context_.wnd);
}

bool GLRenderContext::GLMakeCurrent(GLRenderContext* renderContext)
{
    /*if (renderContext)
    {
        const auto& ctx = renderContext->context_;
        return glXMakeCurrent(ctx.display, ctx.wnd, ctx.glc);
    }
    else
        return glXMakeCurrent(nullptr, 0, 0);*/
    return false;
}


/*
 * ======= Private: =======
 */

void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
    //todo...
}

void GLRenderContext::DeleteContext()
{
    //todo...
}

bool GLRenderContext::SetupVsyncInterval()
{
    //GLint sync = (desc_.vsync.enabled ? std::max(1, std::min(static_cast<GLint>(desc_.vsync.interval), 4)) : 0);
    //CGLSetParameter(ctx, kCGLCPSwapInterval, &sync);
    return true;
}


} // /namespace LLGL



// ================================================================================
