/*
 * LinuxGLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
    std::uint32_t               samples)
{
    /* Open X11 display */
    windowContext.display = XOpenDisplay(nullptr);
    if (!windowContext.display)
        throw std::runtime_error("failed to open X11 display");

    windowContext.parentWindow  = DefaultRootWindow(windowContext.display);
    windowContext.screen        = DefaultScreen(windowContext.display);

    GLXFBConfig fbc = 0;

    if (samples > 1)
    {
        /* Create FB configuration for multi-sampling */
        const int fbAttribs[] =
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
            GLX_SAMPLES,        static_cast<int>(GetClampedSamples(samples)),
            None
        };

        int attribs[100];
        ::memcpy(attribs, fbAttribs, sizeof(fbAttribs));

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
        if (samples > 1)
            Log::PostReport(Log::ReportType::Error, "failed to choose XVisualInfo for multi-sampling");

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
