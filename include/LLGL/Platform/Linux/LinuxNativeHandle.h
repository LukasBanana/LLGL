/*
 * LinuxNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_NATIVE_HANDLE_H
#define LLGL_LINUX_NATIVE_HANDLE_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>


namespace LLGL
{


//! Linux native handle structure.
struct NativeHandle
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


} // /namespace LLGL


#endif



// ================================================================================
