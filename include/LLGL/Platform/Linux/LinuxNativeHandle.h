/*
 * LinuxNativeHandle.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_NATIVE_HANDLE_H
#define LLGL_LINUX_NATIVE_HANDLE_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef LLGL_LINUX_ENABLE_WAYLAND
#include <wayland-client.h>
#endif

namespace LLGL
{

struct X11NativeHandle
{
    //! X11 display connection.
    ::Display*      display;

    //! X11 window object.
    ::Window        window;

    //! X11 visual information.
    ::XVisualInfo*  visual;

    //! X11 colormap object. Used internally by the OpenGL backend.
    ::Colormap      colorMap;

    //! X11 screen index.
    int             screen;
};

#ifdef LLGL_LINUX_ENABLE_WAYLAND

struct WaylandNativeHandle
{
    struct wl_surface* window;
    struct wl_display* display;
};

#else

struct WaylandNativeHandle{};

#endif

enum class NativeHandleType : char
{
    X11 = 0,
    Wayland = 1
};

//! Linux native handle structure.
struct NativeHandle
{
    union {
        X11NativeHandle x11;
        WaylandNativeHandle wayland;
    };

    NativeHandleType type;
};


} // /namespace LLGL


#endif



// ================================================================================
