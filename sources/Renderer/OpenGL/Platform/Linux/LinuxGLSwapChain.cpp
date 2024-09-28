/*
 * LinuxGLSwapChain.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLContext.h"
#include "../../GLSwapChain.h"
#include "../../../../Platform/Linux/LinuxDisplay.h"
#include "../../../../Core/Assertion.h"
#include "../../../../Core/Exception.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * ======= Private: =======
 */

void GLSwapChain::ChooseGLXVisualAndGetX11WindowContext(GLPixelFormat& pixelFormat, NativeHandle& windowContext)
{
    /* Get X11 display */
    windowContext.display = LinuxSharedX11Display::GetShared()->GetNative();
    LLGL_ASSERT(windowContext.display != nullptr, "failed to obtain shared X11 display");

    windowContext.window = DefaultRootWindow(windowContext.display);
    windowContext.screen = DefaultScreen(windowContext.display);

    /* Choose X11 visual for pixel format */
    int samples = 0;
    windowContext.visual = LinuxGLContext::ChooseVisual(windowContext.display, windowContext.screen, pixelFormat, samples);
    pixelFormat.samples = samples;
    if (!windowContext.visual)
        LLGL_TRAP("failed to choose X11 visual for OpenGL");

    /* Create Colormap structure */
    windowContext.colorMap = XCreateColormap(windowContext.display, windowContext.window, windowContext.visual->visual, AllocNone);
}


} // /namespace LLGL



// ================================================================================
