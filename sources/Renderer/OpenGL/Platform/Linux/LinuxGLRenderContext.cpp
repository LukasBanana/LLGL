/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"
#include "../../../../Platform/Linux/LinuxWindow.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"


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

void GLRenderContext::GetNativeContextHandle(NativeContextHandle& windowContext)
{
    /* Open X11 display */
    windowContext.display = XOpenDisplay(nullptr);
    if (!windowContext.display)
        throw std::runtime_error("failed to open X11 display");

    windowContext.parentWindow  = DefaultRootWindow(windowContext.display);
    windowContext.screen        = DefaultScreen(windowContext.display);
    
    GLXFBConfig fbc = 0;
    
    if (desc_.multiSampling.enabled)
    {
        /* Create FB configuration for multi-sampling */
        const int fbAttribs[] =
        {
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            GLX_DEPTH_SIZE,     24,
            GLX_STENCIL_SIZE,   8,
            GLX_DOUBLEBUFFER,   True,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES,        static_cast<int>(desc_.multiSampling.samples),
            None
        };
        
        int attribs[100];
        memcpy(attribs, fbAttribs, sizeof(fbAttribs));
        
        int fbCount = 0;
        GLXFBConfig* fbcList = glXChooseFBConfig(windowContext.display, windowContext.screen, attribs, &fbCount);
        
        if (fbcList)
        {
            if (fbCount > 0)
                fbc = fbcList[0];
            XFree(fbcList);
        }
    }
    
    if (fbc)
    {
        /* Choose XVisualInfo from FB config */
        windowContext.visual = glXGetVisualFromFBConfig(windowContext.display, fbc);
    }
    else
    {
        if (desc_.multiSampling.enabled)
            Log::StdErr() << "failed to choose XVisualInfo for multi-sampling" << std::endl;
        
        /* Choose standard XVisualInfo structure */
        int visualAttribs[] =
        {
            GLX_RGBA,
            GLX_DEPTH_SIZE, 24,
            GLX_DOUBLEBUFFER,
            None
        };

        windowContext.visual = glXChooseVisual(windowContext.display, windowContext.screen, visualAttribs);
    }
    
    /* Create Colormap structure */
    windowContext.colorMap = XCreateColormap(windowContext.display, windowContext.parentWindow, windowContext.visual->visual, AllocNone);
    
    if (!windowContext.visual)
        throw std::runtime_error("failed to choose X11 visual for OpenGL");
}

void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
    GLXContext glcShared = (sharedRenderContext != nullptr ? sharedRenderContext->context_.glc : nullptr);
    
    /* Get X11 display, window, and visual information */
    auto& window = static_cast<const LinuxWindow&>(GetWindow());
    NativeHandle nativeHandle;
    window.GetNativeHandle(&nativeHandle);
    
    context_.display    = nativeHandle.display;
    context_.wnd        = nativeHandle.window;
    context_.visual     = nativeHandle.visual;
    
    if (!context_.display || !context_.wnd || !context_.visual)
        throw std::invalid_argument("failed to create OpenGL context on X11 client, due to missing arguments");
    
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

bool GLRenderContext::SetupVsyncInterval()
{
    /* Load GL extension "glXSwapIntervalSGI" to set v-sync interval */
    if (glXSwapIntervalSGI || LoadSwapIntervalProcs())
    {
        /* Setup v-sync interval */
        int interval = (desc_.vsync.enabled ? static_cast<int>(desc_.vsync.interval) : 0);
        return (glXSwapIntervalSGI(interval) == 0);
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
