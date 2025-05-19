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

#if LLGL_EXPOSE_WAYLAND || LLGL_LINUX_ENABLE_WAYLAND
#include <wayland-client.h>
#endif

#include <LLGL/Deprecated.h>


namespace LLGL
{


/**
\brief Type enumeration to distinguish native handles between X11 and Wayland protocols.
\see NativeHandle::type
*/
enum class NativeType
{
    X11,
    Wayland
};

//! Linux native handle structure.
struct NativeHandle
{
    //! Specifies whether this is an X11 or Wayland native handle.
    NativeType type;

    struct NativeHandleX11
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

    struct NativeHandleWayland
    {
        #if LLGL_EXPOSE_WAYLAND || LLGL_LINUX_ENABLE_WAYLAND
        struct wl_surface* window;
        struct wl_display* display;
        #else
        void* window;
        void* display;
        #endif
    };

    union
    {
        struct
        {
            //! \deprecated Since 0.04b
            LLGL_DEPRECATED("Deprecated since 0.04b; Use x11.display instead.", "x11.display")
            ::Display*      display;

            //! \deprecated Since 0.04b
            LLGL_DEPRECATED("Deprecated since 0.04b; Use x11.window instead.")
            ::Window        window;

            //! \deprecated Since 0.04b
            LLGL_DEPRECATED("Deprecated since 0.04b; Use x11.visual instead.")
            ::XVisualInfo*  visual;

            //! \deprecated Since 0.04b
            LLGL_DEPRECATED("Deprecated since 0.04b; Use x11.colorMap instead.")
            ::Colormap      colorMap;

            //! \deprecated Since 0.04b
            LLGL_DEPRECATED("Deprecated since 0.04b; Use x11.screen instead.")
            int             screen;
        };

        //! Native handle for X11 protocol.
        NativeHandleX11     x11;

        //! Native handle for Wayland protocol.
        NativeHandleWayland wayland;
    };
};


} // /namespace LLGL


#endif



// ================================================================================
