/*
 * LinuxNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    ::Display*      display;
    ::Window        window;
    ::XVisualInfo*  visual;
};

//! Linux native context handle structure.
struct NativeContextHandle
{
    ::Display*      display;
    ::Window        parentWindow;
    ::XVisualInfo*  visual;
    ::Colormap      colorMap;
    int             screen;
};


} // /namespace LLGL


#endif



// ================================================================================
