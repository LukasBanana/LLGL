/*
 * MacOSNativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_NATIVE_HANDLE_H
#define LLGL_MACOS_NATIVE_HANDLE_H


#include <Cocoa/Cocoa.h>


namespace LLGL
{


//! MacOS native handle structure.
struct NativeHandle
{
    NSWindow* window;
};

//! MacOS native context handle structure.
struct NativeContextHandle
{
    NSWindow* parentWindow;
};


} // /namespace LLGL


#endif



// ================================================================================
