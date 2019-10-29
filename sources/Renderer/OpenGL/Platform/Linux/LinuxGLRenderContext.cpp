/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../../GLRenderContext.h"
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

void GLRenderContext::GetNativeContextHandle(
    NativeContextHandle&        windowContext,
    const VideoModeDescriptor&  videoModeDesc,
    std::uint32_t&              samples)
{
    /* Open X11 display */
    windowContext.display = XOpenDisplay(nullptr);
    if (!windowContext.display)
        throw std::runtime_error("failed to open X11 display");

    windowContext.parentWindow  = DefaultRootWindow(windowContext.display);
    windowContext.screen        = DefaultScreen(windowContext.display);

    GLXFBConfig framebufferConfig = 0;

    /* Find suitable multi-sample format (for samples > 1) */
    for (samples = GetClampedSamples(samples); samples > 1; --samples)
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
            GLX_ALPHA_SIZE,     (videoModeDesc.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     videoModeDesc.depthBits,
            GLX_STENCIL_SIZE,   videoModeDesc.stencilBits,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES,        static_cast<int>(samples),
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
            GLX_ALPHA_SIZE,     (videoModeDesc.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     videoModeDesc.depthBits,
            GLX_STENCIL_SIZE,   videoModeDesc.stencilBits,
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
