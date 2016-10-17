/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../../../Platform/Linux/LinuxWindow.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


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


} // /namespace LLGL



// ================================================================================
