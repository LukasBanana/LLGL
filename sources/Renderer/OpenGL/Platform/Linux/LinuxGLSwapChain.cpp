/*
 * LinuxGLSwapChain.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLContextX11.h"
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
    windowContext.type = NativeType::X11;

    /* Get X11 display */
    windowContext.x11.display = LinuxSharedX11Display::GetShared()->GetNative();
    LLGL_ASSERT(windowContext.x11.display != nullptr, "failed to obtain shared X11 display");

    windowContext.x11.window = DefaultRootWindow(windowContext.x11.display);
    windowContext.x11.screen = DefaultScreen(windowContext.x11.display);

    /* Choose X11 visual for pixel format */
    int samples = 0;
    windowContext.x11.visual = LinuxGLContextX11::ChooseVisual(windowContext.x11.display, windowContext.x11.screen, pixelFormat, samples);
    pixelFormat.samples = samples;
    if (!windowContext.x11.visual)
        LLGL_TRAP("failed to choose X11 visual for OpenGL");

    /* Create Colormap structure */
    windowContext.x11.colorMap = XCreateColormap(windowContext.x11.display, windowContext.x11.window, windowContext.x11.visual->visual, AllocNone);

    LLGL_DEPRECATED_IGNORE_PUSH()
    windowContext.display   = windowContext.x11.display;
    windowContext.window    = windowContext.x11.window;
    windowContext.visual    = windowContext.x11.visual;
    windowContext.colorMap  = windowContext.x11.colorMap;
    windowContext.screen    = windowContext.x11.screen;
    LLGL_DEPRECATED_IGNORE_POP()
}


} // /namespace LLGL



// ================================================================================
