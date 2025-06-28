/*
 * LinuxWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include <LLGL/Window.h>
#include "../../Core/CoreUtils.h"
#include <X11/Xresource.h>

#include "LinuxWindowWayland.h"
#include "LinuxWindowX11.h"


namespace LLGL
{

static bool IsWaylandSupported()
{
    const char* const session = getenv("XDG_SESSION_TYPE");
    if (session)
    {
        if (strcmp(session, "wayland") == 0 && getenv("WAYLAND_DISPLAY"))
            return true;
    }
    return false;
}

static bool g_isWaylandSupported = IsWaylandSupported();

/*
 * Window class
 */
std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    #if LLGL_LINUX_ENABLE_WAYLAND
    if (g_isWaylandSupported)
    {
        return MakeUnique<LinuxWindowWayland>(desc);
    }
    else
    {
    #endif
        return MakeUnique<LinuxWindowX11>(desc);
    #if LLGL_LINUX_ENABLE_WAYLAND
    }
    #endif
}

/*
 * Surface class
 */
bool Surface::ProcessEvents()
{
#if LLGL_LINUX_ENABLE_WAYLAND
    if (g_isWaylandSupported)
    {
        bool result = (wl_display_dispatch(g_waylandState.display) != -1);

        for (LinuxWindowWayland* window : LinuxWaylandContext::GetWindows())
        {
            window->ProcessEvents();
        }

        return result;
    }
    else
    {
#endif
        ::Display* display = LinuxSharedDisplayX11::GetShared()->GetNative();

        XEvent event;

        XPending(display);

        while (XQLength(display))
        {
            XNextEvent(display, &event);
            if (void* userData = LinuxX11Context::Find(display, event.xany.window))
            {
                LinuxWindowX11* wnd = static_cast<LinuxWindowX11*>(userData);
                wnd->ProcessEvent(event);
            }
        }

        XFlush(display);

        return true;
#if LLGL_LINUX_ENABLE_WAYLAND
    }
#endif
}

} // /namespace LLGL



// ================================================================================
