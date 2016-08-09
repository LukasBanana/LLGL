/*
 * LinuxNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_LINUX_NATIVE_HANDLE_H__
#define __LLGL_LINUX_NATIVE_HANDLE_H__


#include <X11/Xlib.h>


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
