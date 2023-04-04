/*
 * LinuxGLSwapChain.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../GLSwapChain.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../../TextureUtils.h"
#include "../../../../Platform/Linux/LinuxWindow.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include <string.h>


namespace LLGL
{


/*
 * ======= Private: =======
 */

void GLSwapChain::ChooseGLXVisualAndGetX11WindowContext(GLPixelFormat& pixelFormat, NativeContextHandle& windowContext)
{
    /* Open X11 display */
    windowContext.display = XOpenDisplay(nullptr);
    if (!windowContext.display)
        throw std::runtime_error("failed to open X11 display");

    windowContext.parentWindow  = DefaultRootWindow(windowContext.display);
    windowContext.screen        = DefaultScreen(windowContext.display);

    GLXFBConfig framebufferConfig = 0;

    /* Find suitable multi-sample format (for samples > 1) */
    for (; pixelFormat.samples > 1; pixelFormat.samples--)
    {
        /* Create framebuffer configuration for multi-sampling */
        const int framebufferAttribs[] =
        {
            GLX_DOUBLEBUFFER,   True,
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     (pixelFormat.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     pixelFormat.depthBits,
            GLX_STENCIL_SIZE,   pixelFormat.stencilBits,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES,        pixelFormat.samples,
            None
        };

        int fbConfigsCount = 0;
        GLXFBConfig* fbConfigs = glXChooseFBConfig(windowContext.display, windowContext.screen, framebufferAttribs, &fbConfigsCount);

        if (fbConfigs != nullptr)
        {
            if (fbConfigsCount > 0)
            {
                framebufferConfig = fbConfigs[0];
                if (framebufferConfig != 0)
                    break;
            }
            XFree(fbConfigs);
        }
    }

    if (framebufferConfig)
    {
        /* Choose XVisualInfo from FB config */
        windowContext.visual = glXGetVisualFromFBConfig(windowContext.display, framebufferConfig);
    }
    else
    {
        /* Choose standard XVisualInfo structure */
        int visualAttribs[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     (pixelFormat.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     pixelFormat.depthBits,
            GLX_STENCIL_SIZE,   pixelFormat.stencilBits,
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
